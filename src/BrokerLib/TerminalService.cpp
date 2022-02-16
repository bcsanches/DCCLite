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

#include "BonjourService.h"
#include "Broker.h"
#include "DccLiteService.h"
#include "NetworkDevice.h"
#include "NetMessenger.h"
#include "OutputDecoder.h"
#include "SignalDecoder.h"
#include "SpecialFolders.h"
#include "TerminalCmd.h"
#include "ZeroconfService.h"

namespace dcclite::broker
{

	constexpr auto JSONRPC_KEY = "jsonrpc";
	constexpr auto JSONRPC_VERSION = "2.0";

	using namespace dcclite;

	static std::string MakeRpcMessage(CmdId_t id, std::string_view *methodName, std::string_view nestedObjName, std::function<void(JsonOutputStream_t &object)> filler)
	{
		JsonCreator::StringWriter messageWriter;

		{
			auto messageObj = JsonCreator::MakeObject(messageWriter);

			messageObj.AddStringValue(JSONRPC_KEY, JSONRPC_VERSION);

			if (id >= 0)
			{
				messageObj.AddIntValue("id", id);
			}

			if (methodName)
				messageObj.AddStringValue("method", *methodName);

			if (filler)
			{
				auto params = messageObj.AddObject(nestedObjName);

				filler(params);
			}
		}

		return messageWriter.GetString();
	}

	static std::string MakeRpcResultMessage(const CmdId_t id, std::function<void(JsonOutputStream_t &object)> filler)
	{
		return MakeRpcMessage(id, nullptr, "result", filler);
	}


	class GetChildItemCmd : public TerminalCmd
	{
		public:
			GetChildItemCmd(std::string name = "Get-ChildItem"):
				TerminalCmd(std::move(name))
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
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

				return MakeRpcResultMessage(id, [folder](Result_t &results) 
				{
					results.AddStringValue("classname", "ChildItem");
					results.AddStringValue("location", folder->GetPath().string());

					auto dataArray = results.AddArray("children");

					auto enumerator = folder->GetEnumerator();

					while (enumerator.MoveNext())
					{
						auto item = enumerator.TryGetCurrent();

						auto itemObject = dataArray.AddObject();
						item->Serialize(itemObject);
					}
				});
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

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
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

				return MakeRpcResultMessage(id, [item](Result_t &results)
					{
						results.AddStringValue("classname", "Item");
						results.AddStringValue("location", item->GetPath().string());

						auto dataObj = results.AddObject("item");
						item->Serialize(dataObj);
					}
				);
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

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
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

				return MakeRpcResultMessage(id, [item](Result_t &results)
					{
						results.AddStringValue("classname", "Location");
						results.AddStringValue("location", item->GetPath().string());
					}
				);
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

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto item = this->GetParent();

				assert(item->IsFolder());

				auto folder = static_cast<FolderObject*>(item);

				return MakeRpcResultMessage(id, [folder](Result_t &results)
					{
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
				);				
			}
	};

	class DccSystemCmdBase : public TerminalCmd
	{
		protected:
			DccSystemCmdBase(std::string name) :
				TerminalCmd(std::move(name))
			{
				//empty
			}

			DccLiteService &GetService(const TerminalContext& context, const CmdId_t id, std::string_view dccSystemName)
			{				
				auto& root = static_cast<FolderObject&>(context.GetItem()->GetRoot());

				ObjectPath path = { SpecialFolders::GetPath(SpecialFolders::Folders::ServicesId) };
				path.append(dccSystemName);

				auto *service = dynamic_cast<DccLiteService *>(root.TryNavigate(path));
				if (service == nullptr)
				{
					throw TerminalCmdException(fmt::format("DCC System {} not found", dccSystemName), id);
				}

				return *service;
			}

	};

	class DecoderCmdBase : public DccSystemCmdBase
	{
		protected:
			DecoderCmdBase(std::string name) :
				DccSystemCmdBase(std::move(name))
			{
				//empty
			}

			std::tuple<Decoder *, const char *, const char *> FindDecoder(const TerminalContext &context, const CmdId_t id, const rapidjson::Document &request)
			{
				auto paramsIt = request.FindMember("params");
				if ((paramsIt == request.MemberEnd()) || (!paramsIt->value.IsArray()) || (paramsIt->value.Size() < 2))
				{
					throw TerminalCmdException(fmt::format("Usage: {} <dccSystem> <decoder>", this->GetName()), id);
				}

				auto dccSystemName = paramsIt->value[0].GetString();
				auto decoderId = paramsIt->value[1].GetString();

				auto &service = GetService(context, id, dccSystemName);

				auto *decoder = service.TryFindDecoder(decoderId);
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

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{			
				auto outputDecoder = this->FindOutputDecoder(context, id, request);

				outputDecoder->Activate("ActivateItemCmd");

				return MakeRpcResultMessage(id, [](Result_t &results)
					{
						results.AddStringValue("classname", "string");
						results.AddStringValue("msg", "OK");
					}
				);				
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

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto outputDecoder = this->FindOutputDecoder(context, id, request);

				outputDecoder->Deactivate("DeactivateItemCmd");

				return MakeRpcResultMessage(id, [](Result_t &results)
					{
						results.AddStringValue("classname", "string");
						results.AddStringValue("msg", "OK");
					}
				);
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

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto outputDecoder = this->FindOutputDecoder(context, id, request);

				outputDecoder->ToggleState("FlipItemCmd");

				return MakeRpcResultMessage(id, [outputDecoder](Result_t &results)
					{
						results.AddStringValue("classname", "string");
						results.AddStringValue("msg", fmt::format("OK: {}", dcclite::DecoderStateName(outputDecoder->GetRequestedState())));
					}
				);
			}
	};

	class SetAspectCmd : public DecoderCmdBase
	{
		public:
			SetAspectCmd(std::string name = "Set-Aspect") :
				DecoderCmdBase(std::move(name))
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto [decoder, dccSystemName, decoderName] = this->FindDecoder(context, id, request);

				auto signalDecoder = dynamic_cast<SignalDecoder *>(decoder);
				if (signalDecoder == nullptr)
				{
					throw TerminalCmdException(fmt::format("Decoder {} on DCC System {} is not an Signal type", decoderName, dccSystemName), id);
				}

				auto paramsIt = request.FindMember("params");
				if (paramsIt->value.Size() < 3)
				{
					throw TerminalCmdException(fmt::format("Usage: {} <dccSystem> <decoder> <aspect>", this->GetName()), id);
				}

				auto aspectName = paramsIt->value[2].GetString();
				auto aspect = dcclite::TryConvertNameToAspect(aspectName);
				if (!aspect.has_value())
				{
					throw TerminalCmdException(fmt::format("Invalid aspect name {}", aspectName), id);
				}

				signalDecoder->SetAspect(aspect.value(), this->GetName().data());		

				return MakeRpcResultMessage(id, [aspectName](Result_t &results)
					{
						results.AddStringValue("classname", "string");
						results.AddStringValue("msg", fmt::format("OK: {}", aspectName));
					}
				);
			}
	};

	class ReadEEPromFiber : public TerminalCmdFiber
	{
		public:
			ReadEEPromFiber(const CmdId_t id, NetworkDevice &device):
				TerminalCmdFiber(id),
				m_spTask{ device.StartDownloadEEPromTask(m_vecEEPromData)}
			{
				if (!m_spTask)
					throw TerminalCmdException("No task provided for ReadEEPromFiber", id);
			}

			bool Run(TerminalContext& context) noexcept override
			{
				if (m_spTask->HasFailed())
				{

					return false;
				}

				if (m_spTask->HasFinished())
				{

					return false;
				}

				//still working...
				return true;
			}

		private:
			DownloadEEPromTaskResult_t m_vecEEPromData;

			std::shared_ptr<NetworkTask> m_spTask;
	};

	class ReadEEPromCmd : public DccSystemCmdBase
	{
		public:
			ReadEEPromCmd(std::string name = "Read-EEProm"):
				DccSystemCmdBase(std::move(name))
			{
				//empty
			}

			CmdResult_t Run(TerminalContext& context, const CmdId_t id, const rapidjson::Document& request) override
			{
				auto paramsIt = request.FindMember("params");
				if (paramsIt->value.Size() < 2)
				{
					throw TerminalCmdException(fmt::format("Usage: {} <dccSystem> <device>", this->GetName()), id);
				}				

				auto systemName = paramsIt->value[0].GetString();
				auto deviceName = paramsIt->value[1].GetString();

				auto& service = this->GetService(context, id, systemName);

				auto device = service.TryFindDeviceByName(deviceName);
				if (device == nullptr)
				{
					throw TerminalCmdException(fmt::format("Device {} not found on {} system", deviceName, systemName), id);
				}

				auto networkDevice = dynamic_cast<NetworkDevice *>(device);
				if (networkDevice == nullptr)
				{
					throw TerminalCmdException(fmt::format("Device {} on {} system is NOT a network device", deviceName, systemName), id);
				}				

				return std::make_unique< ReadEEPromFiber>(id, *networkDevice);
			}
	};



	class TerminalClient: private IObjectManagerListener
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
			void OnObjectManagerEvent(const ObjectManagerEvent &event) override;

			void RegisterListeners();

			void SendItemPropertyValueChangedNotification(const ObjectManagerEvent &event);

			FolderObject *TryGetServicesFolder() const;	

		private:
			NetMessenger m_clMessenger;
			TerminalService &m_rclOwner;
			TerminalContext m_clContext;
			TerminalCmdHost &m_rclCmdHost;

			std::list<std::shared_ptr<TerminalCmdFiber>> m_lstFibers;

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
		m_clAddress(other.m_clAddress)
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
			auto *service = dynamic_cast<Service *>(obj);
			if (service == nullptr)
				continue;

			service->RemoveListener(*this);
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

		ObjectPath servicesPath = { SpecialFolders::GetPath(SpecialFolders::Folders::ServicesId) };
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
			auto *service = dynamic_cast<Service *>(obj);
			if (service == nullptr)
			{
				dcclite::Log::Warn("[TerminalClient::RegisterListeners] Object {} is not a service, it is {}", obj->GetName(), obj->GetTypeName());
				continue;
			}

			service->AddListener(*this);
		}
	}	

	std::string MakeRpcNotificationMessage(CmdId_t id, std::string_view methodName, std::function<void(JsonOutputStream_t &object)> filler)
	{
		return MakeRpcMessage(id, &methodName, "params", filler);	
	}

	static std::string MakeRpcErrorResponse(const CmdId_t id, const std::string &msg)
	{
		return MakeRpcMessage(id, nullptr, "error", [&, msg](JsonOutputStream_t &params) { params.AddStringValue("message", msg); });
	}	

	void TerminalClient::SendItemPropertyValueChangedNotification(const ObjectManagerEvent &event)
	{	
		m_clMessenger.Send(
			m_clAddress, 
			MakeRpcNotificationMessage(
				-1,
				"On-ItemPropertyValueChanged",
				[&event](JsonOutputStream_t &params)
				{
					event.m_pfnSerializeDeltaProc ? event.m_pfnSerializeDeltaProc(params) : event.m_pclItem->Serialize(params);					
				}
			)
		);
	}

	void TerminalClient::OnObjectManagerEvent(const ObjectManagerEvent &event)
	{
		switch (event.m_kType)
		{
			case ObjectManagerEvent::ITEM_CHANGED:				
				SendItemPropertyValueChangedNotification(event);
				break;				

			case ObjectManagerEvent::ITEM_CREATED:
				m_clMessenger.Send(
					m_clAddress,
					MakeRpcNotificationMessage(
						-1,
						"On-ItemCreated",
						[&event](JsonOutputStream_t &params)
						{
							event.m_pclItem->Serialize(params);
						}
					)
				);
				break;

			case ObjectManagerEvent::ITEM_DESTROYED:
				m_clMessenger.Send(
					m_clAddress,
					MakeRpcNotificationMessage(
						-1,
						"On-ItemDestroyed",
						[&event](JsonOutputStream_t &params)
						{
							event.m_pclItem->Serialize(params);
						}
					)
				);
				break;
		}
	}

	bool TerminalClient::Update()
	{
		for(auto it = m_lstFibers.begin(), end = m_lstFibers.end(); it != end;)
		{
			auto current = it;
			++it;

			if (!(*current)->Run(m_clContext))
			{
				//fiber has finished
				m_lstFibers.erase(current);
			}
		}

		for (;;)
		{
			auto[status, msg] = m_clMessenger.Poll();

			if (status == Socket::Status::DISCONNECTED)
				return false;

			if (status == Socket::Status::WOULD_BLOCK)
				return true;

			if (status == Socket::Status::OK)
			{
				TerminalCmd::CmdResult_t result;

				int cmdId = -1;
				try
				{
					//dcclite::Log::Trace("Received {}", msg);

					rapidjson::Document doc;
					doc.Parse(msg.c_str());

					if (doc.HasParseError())
					{
						throw TerminalCmdException(fmt::format("Invalid json: {}", msg), -1);
					}

					auto jsonrpcKey = doc.FindMember(JSONRPC_KEY);
					if ((jsonrpcKey == doc.MemberEnd()) || (!jsonrpcKey->value.IsString()) || (strcmp(jsonrpcKey->value.GetString(), JSONRPC_VERSION)))
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

					cmdId = idKey->value.GetInt();

					auto cmd = m_rclCmdHost.TryFindCmd(methodName);
					if (cmd == nullptr)
					{
						dcclite::Log::Error("Invalid cmd: {}", methodName);
						throw TerminalCmdException(fmt::format("Invalid cmd name: {}", methodName), cmdId);

						continue;
					}					

					result = cmd->Run(m_clContext, cmdId, doc);					
				}
				catch (TerminalCmdException &ex)
				{
					result = MakeRpcErrorResponse(ex.GetId(), ex.what());
				}
				catch (std::exception &ex)
				{
					result = MakeRpcErrorResponse(cmdId, ex.what());
				}

				if (std::holds_alternative<std::string>(result))
				{
					auto const &response = std::get<std::string>(result);
					if (!m_clMessenger.Send(m_clAddress, response))
					{
						dcclite::Log::Error("[TerminalClient::Update] message for {} not sent, contents: {}", m_clAddress.GetIpString(), response);
					}
				}
				else
				{
					m_lstFibers.push_back(std::get<std::unique_ptr<TerminalCmdFiber>>(std::move(result)));
				}				
			}
		}
	

		return true;
	}

	TerminalService::TerminalService(const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project) :
		Service(name, broker, params, project)	
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

		{
			cmdHost->AddCmd(std::make_unique<SetAspectCmd>());
		}

		{
			cmdHost->AddCmd(std::make_unique<ReadEEPromCmd>());
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

		if(auto bonjourService = static_cast<BonjourService *>(m_rclBroker.TryFindService(BONJOUR_SERVICE_NAME)))
			bonjourService->Register("terminal", "dcclitet", NetworkProtocol::TCP, port, 36);

		auto zeroconfService = static_cast<ZeroconfService *>(m_rclBroker.TryFindService(ZEROCONF_SERVICE_NAME));
		zeroconfService->Register(this->GetTypeName(), port);
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

}
