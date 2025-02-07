// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Device.h"

#include <fstream>

#include <rapidjson/istreamwrapper.h>

#include "../sys/FileWatcher.h"
#include "../sys/Project.h"

#include "Decoder.h"
#include "FmtUtils.h"
#include "IDccLiteService.h"
#include "JsonUtils.h"
#include "Log.h"
#include "OutputDecoder.h"
#include "StorageManager.h"

namespace dcclite::broker
{
	Device::Device(RName name, IDccLite_DeviceServices &dccService, const rapidjson::Value &params, const Project &project) :
		FolderObject{ name },
		m_clDccService{ dccService },
		m_strConfigFileName{ std::string(this->GetName().GetData()) + ".decoders.json" },
		m_pathConfigFile{ project.GetFilePath(m_strConfigFileName) },
		m_rclProject{ project }
	{
		FileWatcher::TryWatchFile(m_pathConfigFile, [this](const dcclite::fs::path path, std::string fileName)
			{
				dcclite::Log::Info("[Device::{}] [FileWatcher::Reload] Attempting to reload config: {}", this->GetName(), fileName);

				try
				{
					this->Load();
				}
				catch (const std::exception &ex)
				{
					dcclite::Log::Error("[Device::{}] [FileWatcher::Reload] failed: {}", this->GetName(), ex.what());
				}

			});
	}

	Device::Device(RName name, IDccLite_DeviceServices &dccService, const Project &project) :
		FolderObject{ name },
		m_clDccService{ dccService },
		m_rclProject{ project },
		m_pathConfigFile{ project.GetFilePath(m_strConfigFileName) }
	{
		//emtpy
	}

	Device::~Device()
	{
		if (!m_pathConfigFile.empty())
			FileWatcher::UnwatchFile(m_pathConfigFile);		
	}

	void Device::OnUnload()
	{
		//empty
	}

	void Device::Unload()
	{
		this->OnUnload();

		if(!m_vecDecoders.empty())
			StorageManager::SaveState(*this, m_rclProject);

		//clear the token
		m_ConfigToken = {};

		for (auto dec : m_vecDecoders)
		{
			//The null may happen when an exception is threw during load and we do the cleanup before letting the exception go
			if (!dec)
				continue;

			m_clDccService.Device_NotifyInternalItemDestroyed(*(this->TryGetChild(dec->GetName())));

			auto shortcut = this->RemoveChild(dec->GetName());

			m_clDccService.Device_DestroyDecoder(*dec);
		}

		m_vecDecoders.clear();
	}

	Decoder &Device::CreateInternalDecoder(const char *className, DccAddress address, RName name, const rapidjson::Value &params)
	{
		if (!this->IsInternalDecoderAllowed())
			throw std::logic_error(fmt::format("[Device::CreateInternalDecoder] Not supported by {}", this->GetName()));

		auto &decoder = m_clDccService.Device_CreateDecoder(*this, className, address, name, params);

		this->RegisterDecoder(decoder);

		return decoder;
	}

    void Device::RegisterDecoder(Decoder &decoder)
	{
		this->CheckLoadedDecoder(decoder);

		m_vecDecoders.push_back(&decoder);

		dcclite::IObject *decShortcut;
		try
		{
			decShortcut = this->AddChild(std::make_unique<dcclite::Shortcut>(decoder.GetName(), decoder));			
		}	
		catch (...)
		{
			m_vecDecoders.pop_back();

			throw;
		}		

		m_clDccService.Device_NotifyInternalItemCreated(decoder);
		m_clDccService.Device_NotifyInternalItemCreated(*decShortcut);
	}

	void Device::Load()
	{
		dcclite::Log::Info("[Device::{}] [Load] Loading {}", this->GetName(), m_pathConfigFile.string());
		std::ifstream configFile(m_pathConfigFile);
		if (!configFile)
		{
			dcclite::Log::Error("[Device::{} [Load] cannot find {}", this->GetName(), m_pathConfigFile.string());

			return;
		}

		auto storedConfigToken = StorageManager::GetFileToken(m_strConfigFileName, m_rclProject);

		if (storedConfigToken == m_ConfigToken)
		{
			dcclite::Log::Info("[Device::{}] [Load] Stored config token is the same loaded token, ignoring load request", this->GetName());

			return;
		}

		dcclite::Log::Trace("[Device::{}] [Load] stored config token {}", this->GetName(), storedConfigToken);
		dcclite::Log::Trace("[Device::{}] [Load] currently config token {}", this->GetName(), m_ConfigToken);
		dcclite::Log::Trace("[Device::{}] [Load] reading config {}", this->GetName(), m_pathConfigFile.string());

		rapidjson::IStreamWrapper isw(configFile);
		rapidjson::Document decodersData;
		decodersData.ParseStream(isw);

		if (!decodersData.IsArray())
			throw std::runtime_error(fmt::format("[Device::{}] [Load] error: invalid config, expected decoders array inside Node", this->GetName()));

		//
		//
		//At this point, we did everything we could trying to check the data on disk
		//So now, unload what we have and proceed loading new data		

		//Unload all decoders
		//we clear the current config token, because if anything fails after this point and user rollback file to a previous version
		//we want to make sure the previous file is loaded, if we did not clear the token, it may get ignored
		//also this indicates that we have an inconsistent state	
		this->Unload();		

		try
		{
			for (auto &element : decodersData.GetArray())
			{
				auto className = json::GetString(element, "class", "Device");

				//Just used for annotations on JSON file...as the parser does not support comments
				if (strcmp(className, "IgnoreMe") == 0)
					continue;

				auto decoderName = RName{ json::GetString(element, "name", "Device") };
				DccAddress address{ json::GetValue(element, "address", "Device") };

				auto &decoder = m_clDccService.Device_CreateDecoder(*this, className, address, decoderName, element);

				this->RegisterDecoder(decoder);				
			}

			//
			//Load state data
			auto decodersState = StorageManager::LoadState(this->GetName(), m_rclProject, storedConfigToken);
			if (!decodersState.empty())
			{
				for (auto dec : m_vecDecoders)
				{
					auto outputDecoder = dynamic_cast<OutputDecoder *>(dec);
					if (!outputDecoder)
						continue;

					auto it = decodersState.find(outputDecoder->GetName());
					if (it == decodersState.end())
						continue;

					outputDecoder->SetState(it->second, "StorageData");
				}
			}

			//let decoder know that each decoder on this device has been created
			for (auto dec : m_vecDecoders)
			{
				dec->InitAfterDeviceLoad();
			}
		}
		catch (...)
		{
			//bad things happened, cleanup any loaded decoder
			this->Unload();

			//blow up
			throw;
		}


		//if this point is reached, data is loaded, so store new token
		m_ConfigToken = storedConfigToken;
		dcclite::Log::Info("[Device::{}] [Load] loaded {}.", this->GetName(), m_ConfigToken);
	}

	void Device::ConstVisitDecoders(ConstVisitor_t visitor) const
	{
		for (auto it : m_vecDecoders)
		{
			if (!visitor(*it))
				break;
		}
	}

}
