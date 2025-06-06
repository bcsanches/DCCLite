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

#include <dcclite/Benchmark.h>
#include <dcclite/FmtUtils.h>
#include <dcclite/JsonUtils.h>
#include <dcclite/Log.h>

#include "sys/Broker.h"
#include "sys/FileWatcher.h"
#include "sys/Project.h"

#include "Decoder.h"

#include "IDccLiteService.h"
#include "OutputDecoder.h"
#include "StorageManager.h"

namespace dcclite::broker::exec::dcc
{
	Device::Device(RName name, sys::Broker &broker, IDccLite_DeviceServices &dccService, const rapidjson::Value &params):
		FolderObject{ name },
		m_rclDccService{ dccService },
		m_strConfigFileName{ std::string(this->GetName().GetData()) + ".decoders.json" },
		m_pathConfigFile{ sys::Project::GetFilePath(m_strConfigFileName) }
	{
		sys::FileWatcher::TryWatchFile(m_pathConfigFile, [this, &broker](const dcclite::fs::path path, std::string fileName)
			{
				dcclite::Log::Info("[Device::{}] [FileWatcher::Reload] Attempting to reload config: {}", this->GetName(), fileName);
				
				broker.SignalExecutiveChangeStart();
				try
				{
					this->Load();
				}
				catch (const std::exception &ex)
				{
					dcclite::Log::Error("[Device::{}] [FileWatcher::Reload] failed: {}", this->GetName(), ex.what());
				}

				broker.SignalExecutiveChangeEnd();
			});
	}

	Device::Device(RName name, IDccLite_DeviceServices &dccService):
		FolderObject{ name },
		m_rclDccService{ dccService },
		m_pathConfigFile{ sys::Project::GetFilePath(m_strConfigFileName) }
	{
		//emtpy
	}

	Device::~Device()
	{
		if (!m_pathConfigFile.empty())
			sys::FileWatcher::UnwatchFile(m_pathConfigFile);
	}

	void Device::OnUnload()
	{
		//empty
	}

	void Device::Unload()
	{
		this->OnUnload();

		if(!m_vecDecoders.empty())
			StorageManager::SaveState(*this);

		//clear the token
		m_ConfigToken = {};

		for (auto dec : m_vecDecoders)
		{
			//The null may happen when an exception is threw during load and we do the cleanup before letting the exception go
			if (!dec)
				continue;

			m_rclDccService.Device_NotifyInternalItemDestroyed(*(this->TryGetChild(dec->GetName())));

			auto shortcut = this->RemoveChild(dec->GetName());

			m_rclDccService.Device_DestroyDecoder(*dec);
		}

		m_vecDecoders.clear();
	}

	Decoder &Device::CreateInternalDecoder(const char *className, Address address, RName name, const rapidjson::Value &params)
	{
		if (!this->IsInternalDecoderAllowed())
			throw std::logic_error(fmt::format("[Device::CreateInternalDecoder] Not supported by {}", this->GetName()));

		auto &decoder = m_rclDccService.Device_CreateDecoder(*this, className, address, name, params);

		this->RegisterDecoder(decoder);

		return decoder;
	}

    void Device::RegisterDecoder(Decoder &decoder)
	{
		this->CheckIfDecoderTypeIsAllowed(decoder);

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

		m_rclDccService.Device_NotifyInternalItemCreated(decoder);
		m_rclDccService.Device_NotifyInternalItemCreated(*decShortcut);
	}

	void Device::Load()
	{
		BenchmarkLogger benchmark{ "Device::Load", this->GetNameData() };

		dcclite::Log::Info("[Device::{}] [Load] Loading {}", this->GetName(), m_pathConfigFile.string());
		std::ifstream configFile(m_pathConfigFile);
		if (!configFile)
		{
			dcclite::Log::Error("[Device::{} [Load] cannot find {}", this->GetName(), m_pathConfigFile.string());

			return;
		}

		auto storedConfigToken = StorageManager::GetFileToken(m_strConfigFileName);

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
				if (className.compare("IgnoreMe") == 0)
					continue;

				auto decoderName = RName{ json::GetString(element, "name", "Device") };
				Address address{ json::GetValue(element, "address", "Device") };

				auto &decoder = m_rclDccService.Device_CreateDecoder(*this, className, address, decoderName, element);

				this->RegisterDecoder(decoder);				
			}

			//
			//Load state data
			auto decodersState = StorageManager::LoadState(this->GetName(), storedConfigToken);

#if 1
			for (auto &it : decodersState)
			{
				auto shortcut = dynamic_cast<Shortcut *>(this->TryGetChild(it.first));
				if (!shortcut)
					continue;

				auto outputDecoder = dynamic_cast<OutputDecoder *>(shortcut->TryResolve());
				if (!outputDecoder)
					continue;

				outputDecoder->SetState(it.second, "StorageData");
			}
#else
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
#endif

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
