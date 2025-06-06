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

#include <future>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <thread>

#include <dcclite/FmtUtils.h>
#include <dcclite/Util.h>

#include "exec/dcc/DccLiteService.h"
#include "exec/dcc/IResettableObject.h"
#include "exec/dcc/NetworkDevice.h"
#include "exec/dcc/OutputDecoder.h"
#include "exec/dcc/SignalDecoder.h"

#include "sys/BonjourService.h"
#include "sys/Broker.h"
#include "sys/Project.h"
#include "sys/ServiceFactory.h"
#include "sys/ZeroConfSystem.h"

#include "CmdHostService.h"
#include "DeviceClearEEPromCmd.h"
#include "DeviceNetworkTestCmds.h"
#include "DeviceRenameCmd.h"
#include "ServoProgrammerCmds.h"
#include "TerminalClient.h"
#include "TerminalCmd.h"
#include "TerminalUtils.h"

using namespace std::chrono_literals;

namespace dcclite::broker::shell::terminal
{
	const char *TerminalService::TYPE_NAME = "TerminalService";	

	using namespace dcclite;

	/////////////////////////////////////////////////////////////////////////////
	//
	// GetChildItemCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	class GetChildItemCmd : public TerminalCmd
	{
		public:
			explicit GetChildItemCmd(RName name = RName{ "Get-ChildItem" }) :
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{				
				auto folder = &detail::GetCurrentFolder(context, id);

				auto paramsIt = request.FindMember("params");
				if (paramsIt != request.MemberEnd())
				{
					auto locationParam = paramsIt->value[0].GetString();
					auto item = folder->TryNavigate(dcclite::Path_t(locationParam));
					if (!item)
					{
						throw TerminalCmdException(fmt::format("Invalid location {}", locationParam), id);
					}

					if (!item->IsFolder())
					{
						throw TerminalCmdException(fmt::format("Location is not a folder {}", locationParam), id);
					}

					folder = static_cast<IFolderObject *>(item);
				}		

				return detail::MakeRpcResultMessage(id, [folder](Result_t &results) 
				{
					results.AddStringValue("classname", "ChildItem");
					results.AddStringValue("location", folder->GetPath().string());

					auto dataArray = results.AddArray("children");

					folder->ConstVisitChildren([&dataArray](const IObject &item)
						{
							auto itemObject = dataArray.AddObject();
							item.Serialize(itemObject);

							return true;
						}
					);
				});
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// GetItemCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	class GetItemCmd: public TerminalCmd
	{
		public:
			explicit GetItemCmd(RName name = RName{ "Get-Item" }) :
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{				
				auto &folder = detail::GetCurrentFolder(context, id);

				auto paramsIt = request.FindMember("params");
				if (paramsIt == request.MemberEnd())
				{
					throw TerminalCmdException(fmt::format("Usage: {} <path>", this->GetName()), id);
				}

				auto locationParam = paramsIt->value[0].GetString();
				auto item = folder.TryNavigate(dcclite::Path_t(locationParam));
				if (!item)
				{
					throw TerminalCmdException(fmt::format("Invalid location {}", locationParam), id);
				}	

				return detail::MakeRpcResultMessage(id, [item](Result_t &results)
					{
						results.AddStringValue("classname", "Item");
						results.AddStringValue("location", item->GetPath().string());

						auto dataObj = results.AddObject("item");
						item->Serialize(dataObj);
					}
				);
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// SetLocationCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	class SetLocationCmd : public TerminalCmd
	{
		public:
			explicit SetLocationCmd(RName name = RName{ "Set-Location" }) :
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto &folder = detail::GetCurrentFolder(context, id);

				dcclite::Path_t path;

				auto paramsIt = request.FindMember("params");
				if (paramsIt != request.MemberEnd())
				{							
					if (!paramsIt->value.IsArray())
					{
						throw TerminalCmdException("Expected positional parameters", id);
					}

					auto destinationPath = paramsIt->value[0].GetString();

					auto destinationObj = dynamic_cast<IFolderObject *>(folder.TryNavigate(Path_t(destinationPath)));
					if (!destinationObj)
					{
						throw TerminalCmdException(fmt::format("Invalid path {}", destinationPath), id);
					}					
					
					context.SetLocation(*destinationObj);
				}
				else
				{
					path = folder.GetPath();
				}

				return detail::MakeRpcResultMessage(id, [&path](Result_t &results)
					{
						results.AddStringValue("classname", "Location");
						results.AddStringValue("location", path.string());
					}
				);
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// GetCommandCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	class GetCommandCmd : public TerminalCmd
	{
		public:
			explicit GetCommandCmd(RName name = RName{ "Get-Command" }) :
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto item = this->GetParent();

				assert(item->IsFolder());

				auto folder = static_cast<FolderObject*>(item);

				return detail::MakeRpcResultMessage(id, [folder](Result_t &results)
					{
						results.AddStringValue("classname", "CmdList");

						auto dataArray = results.AddArray("cmds");

						folder->ConstVisitChildren([&dataArray](auto &cmd)
							{
								auto itemObject = dataArray.AddObject();
								cmd.Serialize(itemObject);

								return true;
							}
						);						
					}
				);				
			}
	};		

	/////////////////////////////////////////////////////////////////////////////
	//
	// ResetItemCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	class ResetItemCmd : public TerminalCmd
	{
		public:
			explicit ResetItemCmd(RName name = RName{ "Reset-Item" }) :
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto paramsIt = request.FindMember("params");
				if ((paramsIt == request.MemberEnd()) || (!paramsIt->value.IsArray()) || (paramsIt->value.Size() < 1))
				{
					throw TerminalCmdException(fmt::format("Usage: {} <item_path>", this->GetName()), id);
				}

				auto itemPath = paramsIt->value[0].GetString();				

				auto obj = dynamic_cast<IFolderObject *>(context.TryGetItem());
				if (!obj)
				{
					throw TerminalCmdException("Terminal is not on a valid location", id);
				}

				auto ireset = dynamic_cast<IResettableObject *>(obj->TryNavigate(Path_t{ itemPath }));
				if (!ireset)
				{
					throw TerminalCmdException(fmt::format("Service at {} it not resetable or not found", itemPath), id);
				}

				dcclite::Log::Info("[ResetCmd] Resetting {}.", itemPath);
				ireset->Reset();

				return detail::MakeRpcResultMessage(id, [](Result_t &results)
					{
						results.AddStringValue("classname", "string");
						results.AddStringValue("msg", "OK");
					}
				);
			}
	};	

	/////////////////////////////////////////////////////////////////////////////
	//
	// RebootDeviceCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	class RebootDeviceCmd: public TerminalCmd
	{
		public:
			explicit RebootDeviceCmd(RName name = RName{ "Reboot-Device" }):
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto paramsIt = request.FindMember("params");
				if ((paramsIt == request.MemberEnd()) || (!paramsIt->value.IsArray()) || (paramsIt->value.Size() < 1))
				{
					throw TerminalCmdException(fmt::format("Usage: {} <NetworkDevicePath>", this->GetName()), id);
				}

				auto path = paramsIt->value[0].GetString();

				auto &device = detail::GetNetworkDevice(dcclite::Path_t{ path }, context, id);						

				device.ResetRemoteDevice();

				return detail::MakeRpcResultMessage(id, [](Result_t &results)
					{
						results.AddStringValue("classname", "string");
						results.AddStringValue("msg", "OK");
					}
				);
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// DisconnectDeviceCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	class DisconnectDeviceCmd : public TerminalCmd
	{
		public:
			explicit DisconnectDeviceCmd(RName name = RName{ "Disconnect-Device" }) :
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext& context, const CmdId_t id, const rapidjson::Document& request) override
			{
				auto paramsIt = request.FindMember("params");
				if ((paramsIt == request.MemberEnd()) || (!paramsIt->value.IsArray()) || (paramsIt->value.Size() < 1))
				{
					throw TerminalCmdException(fmt::format("Usage: {} <NetworkDevicePath>", this->GetName()), id);
				}

				auto path = paramsIt->value[0].GetString();

				auto& device = detail::GetNetworkDevice(dcclite::Path_t{ path }, context, id);

				device.DisconnectDevice();

				return detail::MakeRpcResultMessage(id, [](Result_t& results)
					{
						results.AddStringValue("classname", "string");
						results.AddStringValue("msg", "OK");
					}
				);
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// BlockDeviceCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	class BlockDeviceCmd: public TerminalCmd
	{
		public:
			explicit BlockDeviceCmd(RName name = RName{ "Block-Device" }):
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto paramsIt = request.FindMember("params");
				if ((paramsIt == request.MemberEnd()) || (!paramsIt->value.IsArray()) || (paramsIt->value.Size() < 1))
				{
					throw TerminalCmdException(fmt::format("Usage: {} <NetworkDevicePath>", this->GetName()), id);
				}

				auto path = paramsIt->value[0].GetString();

				auto &device = detail::GetNetworkDevice(dcclite::Path_t{ path }, context, id);
				
				device.Block();								

				return detail::MakeRpcResultMessage(id, [](Result_t &results)
					{
						results.AddStringValue("classname", "string");
						results.AddStringValue("msg", "OK");
					}
				);
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// ClearDccLiteBlockList
	//
	/////////////////////////////////////////////////////////////////////////////
	class ClearDccLiteBlockListCmd: public TerminalCmd
	{
		public:
			explicit ClearDccLiteBlockListCmd(RName name = RName{ "Clear-DccLiteBlockList" }):
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto paramsIt = request.FindMember("params");
				if ((paramsIt == request.MemberEnd()) || (!paramsIt->value.IsArray()) || (paramsIt->value.Size() < 1))
				{
					throw TerminalCmdException(fmt::format("Usage: {} <DccLiteService>", this->GetName()), id);
				}

				auto path = paramsIt->value[0].GetString();

				auto &folder = detail::GetCurrentFolder(context, id);

				auto obj = folder.TryNavigate(Path_t{ path });
				if (obj == nullptr)
				{
					throw TerminalCmdException(fmt::format("Object {} not found", path), id);
				}

				auto dccLiteService = dynamic_cast<exec::dcc::DccLiteService *>(obj);
				if(dccLiteService == nullptr)
				{
					throw TerminalCmdException(fmt::format("Object {} is not an DccLiteService", path), id);
				}

				dccLiteService->ClearBlockList();

				return detail::MakeRpcResultMessage(id, [](Result_t &results)
					{
						results.AddStringValue("classname", "string");
						results.AddStringValue("msg", "OK");
					}
				);
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// DecoderCmdBase
	//
	/////////////////////////////////////////////////////////////////////////////
	class DecoderCmdBase : public TerminalCmd
	{
		protected:
			explicit DecoderCmdBase(RName name) :
				TerminalCmd(name)
			{
				//empty
			}

			exec::dcc::Decoder &FindDecoder(const TerminalContext &context, const CmdId_t id, const rapidjson::Document &request)
			{
				auto paramsIt = request.FindMember("params");
				if ((paramsIt == request.MemberEnd()) || (!paramsIt->value.IsArray()) || (paramsIt->value.Size() < 1))
				{
					throw TerminalCmdException(fmt::format("Usage: {} <decoder_path>", this->GetName()), id);
				}

				auto folder = dynamic_cast<IFolderObject *>(context.TryGetItem());
				if (folder == nullptr)
				{
					throw TerminalCmdException(fmt::format("Currently location is invalid"), id);
				}

				auto path = Path_t{ paramsIt->value[0].GetString() };

				auto obj = folder->TryNavigate(path);
				if (obj == nullptr)
				{
					throw TerminalCmdException(fmt::format("Decoder not found: {}", path.string()), id);
				}				

				auto decoder = dynamic_cast<exec::dcc::Decoder *>(obj);
				if (decoder == nullptr)
				{
					throw TerminalCmdException(fmt::format("Path does not lead to a decoder: {}", path.string()), id);
				}

				return *decoder;
			}

			exec::dcc::OutputDecoder *FindOutputDecoder(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request)
			{
				auto &decoder = this->FindDecoder(context, id, request);

				auto outputDecoder = dynamic_cast<exec::dcc::OutputDecoder *>(&decoder);
				if (outputDecoder == nullptr)
				{
					throw TerminalCmdException(fmt::format("Decoder {} is not an output type", decoder.GetName()), id);
				}

				return outputDecoder;
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// ActivateItemCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	class ActivateItemCmd : public DecoderCmdBase
	{
		public:
			explicit ActivateItemCmd(RName name = RName{ "Activate-Item" }) :
				DecoderCmdBase(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{			
				auto outputDecoder = this->FindOutputDecoder(context, id, request);

				outputDecoder->Activate("ActivateItemCmd");

				return detail::MakeRpcResultMessage(id, [](Result_t &results)
					{
						results.AddStringValue("classname", "string");
						results.AddStringValue("msg", "OK");
					}
				);				
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// DeactivateItemCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	class DeactivateItemCmd : public DecoderCmdBase
	{
		public:
			explicit DeactivateItemCmd(RName name = RName{ "Deactivate-Item" }) :
				DecoderCmdBase(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto outputDecoder = this->FindOutputDecoder(context, id, request);

				outputDecoder->Deactivate("DeactivateItemCmd");

				return detail::MakeRpcResultMessage(id, [](Result_t &results)
					{
						results.AddStringValue("classname", "string");
						results.AddStringValue("msg", "OK");
					}
				);
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// FlipItemCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	class FlipItemCmd : public DecoderCmdBase
	{
		public:
			explicit FlipItemCmd(RName name = RName{ "Flip-Item" }) :
				DecoderCmdBase(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto outputDecoder = this->FindOutputDecoder(context, id, request);

				outputDecoder->ToggleState("FlipItemCmd");

				return detail::MakeRpcResultMessage(id, [outputDecoder](Result_t &results)
					{
						results.AddStringValue("classname", "string");
						results.AddStringValue("msg", fmt::format("OK: {}", dcclite::DecoderStateName(outputDecoder->GetRequestedState())));
					}
				);
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// SetAspectCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	class SetAspectCmd : public DecoderCmdBase
	{
		public:
			explicit SetAspectCmd(RName name = RName{ "Set-Aspect" }) :
				DecoderCmdBase(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto &decoder = this->FindDecoder(context, id, request);

				auto signalDecoder = dynamic_cast<exec::dcc::SignalDecoder *>(&decoder);
				if (signalDecoder == nullptr)
				{
					throw TerminalCmdException(fmt::format("Decoder {} on DCC System is not an Signal type", decoder.GetName()), id);
				}

				auto paramsIt = request.FindMember("params");
				if (paramsIt->value.Size() < 2)
				{
					throw TerminalCmdException(fmt::format("Usage: {} <decoder_path> <aspect>", this->GetName()), id);
				}

				auto aspectName = paramsIt->value[1].GetString();
				auto aspect = dcclite::TryConvertNameToAspect(aspectName);
				if (!aspect.has_value())
				{
					throw TerminalCmdException(fmt::format("Invalid aspect name {}", aspectName), id);
				}

				signalDecoder->SetAspect(aspect.value(), this->GetName().GetData().data(), "Json proc");

				return detail::MakeRpcResultMessage(id, [aspectName](Result_t &results)
					{
						results.AddStringValue("classname", "string");
						results.AddStringValue("msg", fmt::format("OK: {}", aspectName));
					}
				);
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// ReadEEPromCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	class ReadEEPromFiber : public TerminalCmdFiber, private exec::dcc::NetworkTask::IObserver, public sys::EventHub::IEventTarget
	{
		public:
			ReadEEPromFiber(const CmdId_t id, TerminalContext &context, exec::dcc::NetworkDevice &device):
				TerminalCmdFiber(id, context),
				m_spTask{ device.StartDownloadEEPromTask(this, m_vecEEPromData)}				
			{
				if (!m_spTask)
					throw TerminalCmdException("No task provided for ReadEEPromFiber", id);
				
				std::string fileName{ device.GetName().GetData() };
				fileName.append(".rom");
				
				m_pathRomFileName = sys::Project::GetAppFilePath(fileName);
			}	

			~ReadEEPromFiber()
			{
				//
				//Must wait, so we make sure the thread will not fire a event after it is destroyed
				if(m_Future.valid())
					m_Future.wait();

				sys::EventHub::CancelEvents(*this);
			}

		private:
			void OnNetworkTaskStateChanged(exec::dcc::NetworkTask &task) override
			{				
				assert(&task == m_spTask.get());

				m_spTask->SetObserver(nullptr);

				if (m_spTask->HasFailed())
				{
					m_rclContext.SendClientNotification(detail::MakeRpcErrorResponse(m_tCmdId, fmt::format("Download task failed: {}", m_spTask->GetMessage())));

					//suicide, we are useless now
					m_rclContext.DestroyFiber(*this);

					return;
				}

				if (m_spTask->HasFinished())
				{	
					//we do not need the task anymore
					m_spTask.reset();

					//save the data to disk...
					m_Future = std::async(SaveEEprom, std::ref(*this), std::ref(m_vecEEPromData), std::ref(m_pathRomFileName));
				}								
			}

			class DiskWriteFinishedEvent: public sys::EventHub::IEvent
			{
				public:
					explicit DiskWriteFinishedEvent(ReadEEPromFiber &target):
						IEvent{ target }
					{
						//empty
					}

					void Fire() override
					{
						static_cast<ReadEEPromFiber &>(this->GetTarget()).OnSaveEEPromFinished();
					}
			};

			void OnSaveEEPromFinished()
			{
				auto msg = detail::MakeRpcResultMessage(m_tCmdId, [this](Result_t &results)
					{
						results.AddStringValue("classname", "ReadEEPromResult");
						results.AddStringValue("filepath", m_pathRomFileName.string());
					}
				);

				m_rclContext.SendClientNotification(msg);

				//finally finished, so go away like MR. MEESEEKS
				m_rclContext.DestroyFiber(*this);
			}

			static void SaveEEprom(ReadEEPromFiber &fiber, const exec::dcc::DownloadEEPromTaskResult_t &data, const dcclite::fs::path &fileName)
			{
				dcclite::Log::Info("[ReadEEPromFiber::SaveEEprom] Saving EEPROM at {}", fileName.string());

				std::ofstream epromFile(fileName, std::ios::binary);
				if (!epromFile)
				{
					dcclite::Log::Error("[ReadEEPromFiber::SaveEEprom] cannot create {}", fileName.string());

					return;
				}				

				epromFile.write(reinterpret_cast<const char *>(&data.front()), data.size());
				dcclite::Log::Info("[ReadEEPromFiber::SaveEEprom] Finished saving EEPROM at {}", fileName.string());

				//
				//Send results.. but we need to do this on main thread
				sys::EventHub::PostEvent< DiskWriteFinishedEvent>(std::ref(fiber));
			}

		private:			
			exec::dcc::DownloadEEPromTaskResult_t m_vecEEPromData;

			std::shared_ptr<exec::dcc::NetworkTask> m_spTask;

			dcclite::fs::path m_pathRomFileName;

			std::future<void> m_Future;
	};

	class ReadEEPromCmd : public TerminalCmd
	{
		public:
			explicit ReadEEPromCmd(RName name = RName{ "Read-EEProm" }) :
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document& request) override
			{
				auto paramsIt = request.FindMember("params");
				if (paramsIt->value.Size() < 1)
				{
					throw TerminalCmdException(fmt::format("Usage: {} <itemPath>", this->GetName()), id);
				}

				auto locationParam = paramsIt->value[0].GetString();

				auto &networkDevice = detail::GetNetworkDevice(dcclite::Path_t(locationParam), context, id);				

				return std::make_unique<ReadEEPromFiber>(id, context, networkDevice);
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// GetRNames
	//
	/////////////////////////////////////////////////////////////////////////////
	class GetRNames : public TerminalCmd
	{
		public:
			explicit GetRNames(RName name = RName{ "Get-RNames" }) :
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto names = dcclite::detail::RName_GetAll();				

				return detail::MakeRpcResultMessage(id, [&names](Result_t &results)
					{
						results.AddStringValue("classname", "RNames");
						auto dataArray = results.AddArray("rnames");
						for (const auto &it : names)
						{
							auto obj = dataArray.AddObject();

							obj.AddStringValue("name", it.GetData());
							obj.AddIntValue("index", it.GetIndex());

							auto clusterInfo = it.FindClusterInfo();
							obj.AddIntValue("cluster", clusterInfo.first);
							obj.AddIntValue("position", clusterInfo.second);
						}						
					}
				);
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// TerminalService Events
	//
	/////////////////////////////////////////////////////////////////////////////
	class TerminalServiceAcceptConnectionEvent: public sys::EventHub::IEvent
	{
		public:
			TerminalServiceAcceptConnectionEvent(TerminalService &target, const dcclite::NetworkAddress &address, Socket s):
				IEvent(target),
				m_clSocket{ std::move(s) },
				m_clAddress{ address }

			{
				//empty
			}

			void Fire() override
			{
				static_cast<TerminalService &>(this->GetTarget()).OnAcceptConnection(m_clAddress, std::move(m_clSocket));
			}

		private:
			Socket m_clSocket;
			dcclite::NetworkAddress m_clAddress;
	};

	class TerminalServiceClientDisconnectedEvent: public sys::EventHub::IEvent
	{
		public:
			TerminalServiceClientDisconnectedEvent(TerminalService &target, TerminalClient &client):
				IEvent(target),
				m_rclClient(client)
			{
				//empty
			}

			void Fire() override
			{
				static_cast<TerminalService &>(this->GetTarget()).OnClientDisconnect(m_rclClient);
			}

		private:
			TerminalClient &m_rclClient;
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// TerminalService
	//
	/////////////////////////////////////////////////////////////////////////////
	void TerminalService::RegisterFactory()
	{
		static sys::GenericServiceWithDependenciesFactory<TerminalService> g_clTerminalServiceFactory;
	}

	TerminalService::TerminalService(RName name, sys::Broker &broker, const rapidjson::Value &params, CmdHostService &cmdHost) :
		Service(name, broker, params),
		m_rclCmdHost{cmdHost}
	{					
		{
			auto getChildItemCmd = cmdHost.AddCmd(std::make_unique<GetChildItemCmd>());
			cmdHost.AddAlias(RName{ "dir" }, *getChildItemCmd);
			cmdHost.AddAlias(RName{ "ls" }, *getChildItemCmd);
		}

		{
			cmdHost.AddCmd(std::make_unique<GetItemCmd>());	
		}

		{
			auto setLocationCmd = cmdHost.AddCmd(std::make_unique<SetLocationCmd>());
			cmdHost.AddAlias(RName{ "cd" }, *setLocationCmd);
		}	

		{
			auto getCommandCmd = cmdHost.AddCmd(std::make_unique<GetCommandCmd>());
			cmdHost.AddAlias(RName{ "gcm" }, *getCommandCmd);
		}	

		{
			cmdHost.AddCmd(std::make_unique<ActivateItemCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<DeactivateItemCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<FlipItemCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<StartServoProgrammerCmd>());
			cmdHost.AddCmd(std::make_unique<StopServoProgrammerCmd>());
			cmdHost.AddCmd(std::make_unique<EditServoProgrammerCmd>());
			cmdHost.AddCmd(std::make_unique<DeployServoProgrammerCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<SetAspectCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<ReadEEPromCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<ResetItemCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<BlockDeviceCmd>());
		}
		
		{
			cmdHost.AddCmd(std::make_unique<ClearDccLiteBlockListCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<DisconnectDeviceCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<RebootDeviceCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<ClearEEPromCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<GetRNames>());
		}

		{
			auto renameItemCmd = cmdHost.AddCmd(std::make_unique<RenameItemCmd>());

			cmdHost.AddAlias(RName{ "ren" }, *renameItemCmd);
		}

		{
			cmdHost.AddCmd(std::make_unique<StartNetworkTestCmd>());
			cmdHost.AddCmd(std::make_unique<StopNetworkTestCmd>());
			cmdHost.AddCmd(std::make_unique<ReceiveNetworkTestDataCmd>());
		}

		const auto port = dcclite::json::TryGetDefaultInt(params, "port", DEFAULT_TERMINAL_SERVER_PORT);
		
		m_thListenThread = std::thread{ [port, this] {this->ListenThreadProc(port); } };		
		dcclite::SetThreadName(m_thListenThread, "TerminalService::ListenThread");

		if(auto bonjourService = m_rclBroker.TryFindServiceByType<sys::BonjourService>())		
			bonjourService->Register("terminal", "dcclite", sys::NetworkProtocol::TCP, port, 36);

		sys::ZeroConfSystem::Register(this->GetTypeName(), port);
	}

	TerminalService::~TerminalService()
	{
		//close socket, so listen thread stops...
		m_clSocket.Close();

		//kill all clients...
		m_vecClients.clear();

		//wait for listen thread to finish, so we are sure no more events will be posted
		m_thListenThread.join();

		//Cancel any events, because no one will be able to handle those
		sys::EventHub::CancelEvents(*this);
	}	

	void TerminalService::OnClientDisconnect(TerminalClient &client)
	{
		auto it = std::find_if(m_vecClients.begin(), m_vecClients.end(), [&client](auto &item)
			{
				return item.get() == &client;
			}
		);

		assert(it != m_vecClients.end());

		m_vecClients.erase(it);

		dcclite::Log::Info("[TerminalService] Client disconnected");
	}

	void TerminalService::Async_DisconnectClient(TerminalClient &client)
	{
		sys::EventHub::PostEvent< TerminalServiceClientDisconnectedEvent>(std::ref(*this), std::ref(client));
	}

	void TerminalService::OnAcceptConnection(const dcclite::NetworkAddress &address, dcclite::Socket &&s)
	{
		dcclite::Log::Info("[TerminalService] Client connected {}", address.GetIpString());

		ITerminalServiceClientProxy &proxy = *this;

		m_vecClients.push_back(
			std::make_unique<TerminalClient>(
				proxy,
				m_rclCmdHost,
				m_rclBroker, 
				*this,
				address, 
				std::move(s)
			)
		);
	}

	void TerminalService::ListenThreadProc(const int port)
	{	
		if (!m_clSocket.Open(port, dcclite::Socket::Type::STREAM, Socket::FLAG_BLOCKING_MODE))
		{
			throw std::runtime_error("[TerminalService] Cannot open socket");
		}

		if (!m_clSocket.Listen())
		{
			throw std::runtime_error("[TerminalService] Cannot put socket on listen mode");
		}

		dcclite::Log::Info("[TerminalService] Started, listening on port {}", port);

		for (;;)
		{
			auto [status, socket, address] = m_clSocket.TryAccept();

			if (status != Socket::Status::OK)
				break;			
			
			sys::EventHub::PostEvent<TerminalServiceAcceptConnectionEvent>(
				std::ref(*this), 
				address, 
				std::move(socket)				
			);
		}
	}	
}
