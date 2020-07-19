// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include "TerminalService.h"

#include <sstream>
#include <stdexcept>

#include <Log.h>

#include <JsonCreator/StringWriter.h>
#include <JsonCreator/Object.h>

#include <rapidjson/document.h>

#include "Broker.h"
#include "DccLiteService.h"
#include "NetMessenger.h"
#include "OutputDecoder.h"
#include "SpecialFolders.h"
#include "TerminalCmd.h"

using namespace dcclite;

static ServiceClass terminalService("Terminal",
	[](const ServiceClass &serviceClass, const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project) -> 
	std::unique_ptr<Service> { return std::make_unique<TerminalService>(serviceClass, name, broker, params, project); }
);

class GetChildItemCmd : public TerminalCmd
{
	public:
		GetChildItemCmd(std::string name = "Get-ChildItem"):
			TerminalCmd(std::move(name))
		{
			//empty
		}

		virtual void Run(TerminalContext &context, Result_t &results, const CmdId_t id, const rapidjson::Document &request)
		{
			auto item = context.GetItem();
			if (!item->IsFolder())
			{
				throw TerminalCmdException(fmt::format("Current location {} is invalid", context.GetLocation().string()), id);
			}
			auto folder = static_cast<FolderObject *>(item);

			auto paramsIt = request.FindMember("params");
			if (paramsIt != request.MemberEnd())
			{
				auto locationParam = paramsIt->value[0].GetString();
				item = folder->TryNavigate(dcclite::Path_t(locationParam));
				if (!item)
				{
					throw TerminalCmdException(fmt::format("Invalid location {}", locationParam), id);
				}

				if (!item->IsFolder())
				{
					throw TerminalCmdException(fmt::format("Location is not a folder {}", locationParam), id);
				}

				folder = static_cast<FolderObject *>(item);
			}			

			results.AddStringValue("classname", "ChildItem");
			results.AddStringValue("location", folder->GetPath().string());

			auto dataArray = results.AddArray("children");
												
			auto enumerator = folder->GetEnumerator();

			while (enumerator.MoveNext())
			{
				item = enumerator.TryGetCurrent();

				auto itemObject = dataArray.AddObject();
				item->Serialize(itemObject);				
			}
		}
};

class GetItemCmd: public TerminalCmd
{
	public:
		GetItemCmd(std::string name = "Get-Item"):
			TerminalCmd(std::move(name))
		{
			//empty
		}

		virtual void Run(TerminalContext &context, Result_t &results, const CmdId_t id, const rapidjson::Document &request)
		{
			auto item = context.GetItem();
			if (!item->IsFolder())
			{
				throw TerminalCmdException(fmt::format("Current location {} is invalid", context.GetLocation().string()), id);
			}
			auto folder = static_cast<FolderObject *>(item);

			auto paramsIt = request.FindMember("params");
			if (paramsIt == request.MemberEnd())
			{
				throw TerminalCmdException(fmt::format("Usage: {} <path>", this->GetName()), id);
			}

			auto locationParam = paramsIt->value[0].GetString();
			item = folder->TryNavigate(dcclite::Path_t(locationParam));
			if (!item)
			{
				throw TerminalCmdException(fmt::format("Invalid location {}", locationParam), id);
			}			

			results.AddStringValue("classname", "Item");
			results.AddStringValue("location", item->GetPath().string());

			auto dataObj = results.AddObject("item");			
			item->Serialize(dataObj);			
		}
};

class SetLocationCmd : public TerminalCmd
{
	public:
		SetLocationCmd(std::string name = "Set-Location") :
			TerminalCmd(std::move(name))
		{
			//empty
		}

		virtual void Run(TerminalContext &context, Result_t &results, const CmdId_t id, const rapidjson::Document &request)
		{
			auto item = context.GetItem();
			if (!item->IsFolder())
			{
				throw TerminalCmdException(fmt::format("Current location {} is invalid", context.GetLocation().string()), id);
			}

			auto folder = static_cast<FolderObject *>(item);

			auto paramsIt = request.FindMember("params");
			if (paramsIt != request.MemberEnd())
			{							
				if (!paramsIt->value.IsArray())
				{
					throw TerminalCmdException("Expected positional parameters", id);
				}

				auto destinationPath = paramsIt->value[0].GetString();

				auto destinationObj = folder->TryNavigate(Path_t(destinationPath));
				if (!destinationObj)
				{
					throw TerminalCmdException(fmt::format("Invalid path {}", destinationPath), id);
				}

				if (!destinationObj->IsFolder())
				{
					throw TerminalCmdException(fmt::format("Path {} led to an IObject, not IFolder", destinationPath), id);
				}


				context.SetLocation(destinationObj->GetPath());
				item = destinationObj;
			}
		
			results.AddStringValue("classname", "Location");
			results.AddStringValue("location", item->GetPath().string());
		}
};

class GetCommandCmd : public TerminalCmd
{
	public:
		GetCommandCmd(std::string name = "Get-Command") :
			TerminalCmd(std::move(name))
		{
			//empty
		}

		virtual void Run(TerminalContext &context, Result_t &results, const CmdId_t id, const rapidjson::Document &request)
		{
			auto item = this->GetParent();

			assert(item->IsFolder());

			auto folder = static_cast<FolderObject*>(item);

			results.AddStringValue("classname", "CmdList");			

			auto dataArray = results.AddArray("cmds");

			auto enumerator = folder->GetEnumerator();

			while (enumerator.MoveNext())
			{
				auto cmd = enumerator.TryGetCurrent();

				auto itemObject = dataArray.AddObject();
				cmd->Serialize(itemObject);
			}			
		}
};

class DecoderCmdBase : public TerminalCmd
{
	protected:
		DecoderCmdBase(std::string name) :
			TerminalCmd(std::move(name))
		{
			//empty
		}

		std::tuple<Decoder *, const char *, const char *> FindDecoder(const TerminalContext &context, const CmdId_t id, const rapidjson::Document &request)
		{
			auto paramsIt = request.FindMember("params");
			if ((paramsIt == request.MemberEnd()) || (!paramsIt->value.IsArray()) || (paramsIt->value.Size() != 2))
			{
				throw TerminalCmdException(fmt::format("Usage: {} <dccSystem> <decoder>", this->GetName()), id);
			}

			auto dccSystemName = paramsIt->value[0].GetString();
			auto decoderId = paramsIt->value[1].GetString();

			auto &root = static_cast<FolderObject &>(context.GetItem()->GetRoot());

			ObjectPath path = { SpecialFolders::GetPath(SpecialFolders::ServicesFolderId) };
			path.append(dccSystemName);

			auto *service = dynamic_cast<DccLiteService *>(root.TryNavigate(path));
			if (service == nullptr)
			{
				throw TerminalCmdException(fmt::format("DCC System {} not found", dccSystemName), id);
			}

			auto *decoder = service->TryFindDecoder(decoderId);
			if (decoder == nullptr)
			{
				throw TerminalCmdException(fmt::format("Decoder {} not found on DCC System {}", decoderId, dccSystemName), id);
			}

			return std::make_tuple(decoder, dccSystemName, decoderId);
		}

		OutputDecoder *FindOutputDecoder(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request)
		{
			auto[decoder, dccSystemName, decoderName] = this->FindDecoder(context, id, request);

			auto outputDecoder = dynamic_cast<OutputDecoder *>(decoder);
			if (outputDecoder == nullptr)
			{
				throw TerminalCmdException(fmt::format("Decoder {} on DCC System {} is not an output type", decoderName, dccSystemName), id);
			}

			return outputDecoder;
		}
};

class ActivateItemCmd : public DecoderCmdBase
{
	public:
		ActivateItemCmd(std::string name = "Activate-Item") :
			DecoderCmdBase(std::move(name))
		{
			//empty
		}

		virtual void Run(TerminalContext &context, Result_t &results, const CmdId_t id, const rapidjson::Document &request)
		{			
			auto outputDecoder = this->FindOutputDecoder(context, id, request);

			outputDecoder->Activate("ActivateItemCmd");

			results.AddStringValue("classname", "string");
			results.AddStringValue("msg", "OK");
		}
};

class DeactivateItemCmd : public DecoderCmdBase
{
	public:
		DeactivateItemCmd(std::string name = "Deactivate-Item") :
			DecoderCmdBase(std::move(name))
		{
			//empty
		}

		virtual void Run(TerminalContext &context, Result_t &results, const CmdId_t id, const rapidjson::Document &request)
		{
			auto outputDecoder = this->FindOutputDecoder(context, id, request);

			outputDecoder->Deactivate("DeactivateItemCmd");

			results.AddStringValue("classname", "string");
			results.AddStringValue("msg", "OK");
		}
};

class FlipItemCmd : public DecoderCmdBase
{
	public:
		FlipItemCmd(std::string name = "Flip-Item") :
			DecoderCmdBase(std::move(name))
		{
			//empty
		}

		virtual void Run(TerminalContext &context, Result_t &results, const CmdId_t id, const rapidjson::Document &request)
		{
			auto outputDecoder = this->FindOutputDecoder(context, id, request);

			outputDecoder->ToggleState("FlipItemCmd");

			results.AddStringValue("classname", "string");
			results.AddStringValue("msg", fmt::format("OK: {}", dcclite::DecoderStateName(outputDecoder->GetRequestedState())));
		}
};



class TerminalClient: private IDccLiteServiceListener
{
	public:
		TerminalClient(TerminalService &owner, TerminalCmdHost &cmdHost, const NetworkAddress address, Socket &&socket);
		TerminalClient(const TerminalClient &client) = delete;
		TerminalClient(TerminalClient &&other) noexcept;

		virtual ~TerminalClient();

		TerminalClient &operator=(TerminalClient &&other) noexcept
		{
			if (this != &other)
			{
				m_clMessenger = std::move(other.m_clMessenger);
			}

			return *this;
		}

		bool Update();

	private:
		void OnDeviceConnected(const Device &device) override;
		void OnDeviceDisconnected(const Device &device) override;

		void OnDecoderStateChange(Decoder &decoder) override;

		void RegisterListeners();

		FolderObject *TryGetServicesFolder() const;

	private:
		static std::string CreateErrorResponse(const std::string &msg, const CmdId_t id);

	private:
		NetMessenger m_clMessenger;
		TerminalService &m_rclOwner;
		TerminalContext m_clContext;
		TerminalCmdHost &m_rclCmdHost;

		const NetworkAddress	m_clAddress;
};

TerminalClient::TerminalClient(TerminalService &owner, TerminalCmdHost &cmdHost, const NetworkAddress address, Socket &&socket) :	
	m_clMessenger(std::move(socket)),
	m_rclOwner(owner),	
	m_rclCmdHost(cmdHost),
	m_clContext(static_cast<dcclite::FolderObject &>(owner.GetRoot())),
	m_clAddress(address)
{
	m_clContext.SetLocation(owner.GetPath());

	this->RegisterListeners();
}

TerminalClient::TerminalClient(TerminalClient &&other) noexcept:		
	m_clMessenger(std::move(other.m_clMessenger)),
	m_rclOwner(other.m_rclOwner),	
	m_clContext(std::move(other.m_clContext)),
	m_rclCmdHost(other.m_rclCmdHost),
	m_clAddress(std::move(other.m_clAddress))
{
	this->RegisterListeners();
}

TerminalClient::~TerminalClient()
{
	auto *servicesFolder = this->TryGetServicesFolder();
	if (servicesFolder == nullptr)
		return;

	auto enumerator = servicesFolder->GetEnumerator();
	while (enumerator.MoveNext())
	{
		auto *obj = enumerator.TryGetCurrent();
		auto *dccService = dynamic_cast<DccLiteService *>(obj);
		if (dccService == nullptr)
			continue;

		dccService->RemoveListener(*this);
	}
}

FolderObject *TerminalClient::TryGetServicesFolder() const
{
	auto item = m_clContext.GetItem();
	if (!item->IsFolder())
	{
		dcclite::Log::Error("[TerminalClient::RegisterListeners] Current location {} is invalid", m_clContext.GetLocation().string());

		return nullptr;
	}
	auto folder = static_cast<FolderObject *>(item);

	ObjectPath servicesPath = { SpecialFolders::GetPath(SpecialFolders::ServicesFolderId) };
	auto *servicesObj = folder->TryNavigate(servicesPath);

	if (servicesObj == nullptr)
	{
		dcclite::Log::Error("[TerminalClient::RegisterListeners] Cannot find services folder at {}", servicesPath.string());

		return nullptr;
	}

	if (!servicesObj->IsFolder())
	{
		dcclite::Log::Error("[TerminalClient::RegisterListeners] Services object is not a folder: {}", servicesPath.string());

		return nullptr;
	}

	return static_cast<FolderObject *>(servicesObj);
}

void TerminalClient::RegisterListeners()
{	
	auto *servicesFolder = this->TryGetServicesFolder();
	if(servicesFolder == nullptr)
		return;

	auto enumerator = servicesFolder->GetEnumerator();
	while (enumerator.MoveNext())
	{
		auto *obj = enumerator.TryGetCurrent();
		auto *dccService = dynamic_cast<DccLiteService *>(obj);
		if(dccService == nullptr)
			continue;

		dccService->AddListener(*this);
	}
}

std::string TerminalClient::CreateErrorResponse(const std::string &msg, const CmdId_t id)
{
	JsonCreator::StringWriter responseWriter;

	{
		auto responseObj = JsonCreator::MakeObject(responseWriter);

		responseObj.AddStringValue("jsonrpc", "2.0");

		if (id >= 0)
			responseObj.AddIntValue("id", id);

		{
			auto errorObj = responseObj.AddObject("error");

			errorObj.AddStringValue("message", msg);
		}
	}

	return std::string(responseWriter.GetString());
}

void TerminalClient::OnDeviceConnected(const Device &device)
{

}

void TerminalClient::OnDeviceDisconnected(const Device &device)
{
	
}

void TerminalClient::OnDecoderStateChange(Decoder &decoder)
{

}


bool TerminalClient::Update()
{
	for (;;)
	{
		auto[status, msg] = m_clMessenger.Poll();

		if (status == Socket::Status::DISCONNECTED)
			return false;

		if (status == Socket::Status::WOULD_BLOCK)
			return true;

		if (status == Socket::Status::OK)
		{
			std::string response;

			try
			{
				//dcclite::Log::Trace("Received {}", msg);

				rapidjson::Document doc;
				doc.Parse(msg.c_str());

				if (doc.HasParseError())
				{
					throw TerminalCmdException(fmt::format("Invalid json: {}", msg), -1);
				}

				auto jsonrpcKey = doc.FindMember("jsonrpc");
				if ((jsonrpcKey == doc.MemberEnd()) || (!jsonrpcKey->value.IsString()) || (strcmp(jsonrpcKey->value.GetString(), "2.0")))
				{
					throw TerminalCmdException(fmt::format("Invalid rpc version or was not set: {}", msg), -1);
				}

				auto methodKey = doc.FindMember("method");
				if ((methodKey == doc.MemberEnd()) || (!methodKey->value.IsString()))
				{
					throw TerminalCmdException(fmt::format("Invalid method name in msg: {}", msg), -1);
				}

				const auto methodName = methodKey->value.GetString();

				auto idKey = doc.FindMember("id");
				if ((idKey == doc.MemberEnd()) || (!idKey->value.IsInt()))
				{
					throw TerminalCmdException(fmt::format("No method id in: {}", msg), -1);
				}				

				int id = idKey->value.GetInt();								

				auto cmd = m_rclCmdHost.TryFindCmd(methodName);
				if (cmd == nullptr)
				{
					dcclite::Log::Error("Invalid cmd: {}", methodName);
					throw TerminalCmdException(fmt::format("Invalid cmd name: {}", methodName), id);					

					continue;
				}

				JsonCreator::StringWriter responseWriter;
				{
					auto responseObj = JsonCreator::MakeObject(responseWriter);

					responseObj.AddStringValue("jsonrpc", "2.0");
					responseObj.AddIntValue("id", id);
					auto resultObj = responseObj.AddObject("result");

					cmd->Run(m_clContext, resultObj, id, doc);
				}

				response = responseWriter.GetString();

				dcclite::Log::Trace("response {}", response);
			}
			catch (TerminalCmdException &ex)
			{
				response = CreateErrorResponse(ex.what(), ex.GetId());
			}
			catch (std::exception &ex)
			{
				response = CreateErrorResponse(ex.what(), -1);
			}

			if (!m_clMessenger.Send(m_clAddress, response))
			{
				dcclite::Log::Error("message for {} not sent, contents: {}", m_clAddress.GetIpString(), response);
			}
		}
	}
	

	return true;
}

TerminalService::TerminalService(const ServiceClass &serviceClass, const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project) :
	Service(serviceClass, name, broker, params, project)	
{	
	auto cmdHost = broker.GetTerminalCmdHost();

	assert(cmdHost);	

	{
		auto getChildItemCmd = cmdHost->AddCmd(std::make_unique<GetChildItemCmd>());
		cmdHost->AddAlias("dir", *getChildItemCmd);
		cmdHost->AddAlias("ls", *getChildItemCmd);
	}

	{
		cmdHost->AddCmd(std::make_unique<GetItemCmd>());	
	}

	{
		auto setLocationCmd = cmdHost->AddCmd(std::make_unique<SetLocationCmd>());
		cmdHost->AddAlias("cd", *setLocationCmd);
	}	

	{
		auto getCommandCmd = cmdHost->AddCmd(std::make_unique<GetCommandCmd>());
		cmdHost->AddAlias("gcm", *getCommandCmd);
	}	

	{
		cmdHost->AddCmd(std::make_unique<ActivateItemCmd>());
	}

	{
		cmdHost->AddCmd(std::make_unique<DeactivateItemCmd>());
	}

	{
		cmdHost->AddCmd(std::make_unique<FlipItemCmd>());
	}

	const auto port = params["port"].GetInt();
	if (!m_clSocket.Open(port, dcclite::Socket::Type::STREAM))
	{
		throw std::runtime_error("[TerminalService] Cannot open socket");
	}

	if (!m_clSocket.Listen())
	{
		throw std::runtime_error("[TerminalService] Cannot put socket on listen mode");
	}

	dcclite::Log::Info("[TerminalService] Started, listening on port {}", port);
}


TerminalService::~TerminalService()
{
	//empty
}

void TerminalService::Update(const dcclite::Clock &clock)
{
	auto [status, socket, address] = m_clSocket.TryAccept();

	if (status == Socket::Status::OK)
	{
		dcclite::Log::Info("[TerminalService] Client connected {}", address.GetIpString());

		m_vecClients.emplace_back(*this, *m_rclBroker.GetTerminalCmdHost(), address, std::move(socket));
	}

	for (size_t i = 0; i < m_vecClients.size(); ++i)
	{
		auto &client = m_vecClients[i];

		if (!client.Update())
		{
			dcclite::Log::Info("[TerminalService] Client disconnected");			

			m_vecClients.erase(m_vecClients.begin() + i);
		}
	}
}


