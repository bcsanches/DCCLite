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

#include <chrono>
#include <list>
#include <future>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include <Log.h>

#include <JsonCreator/StringWriter.h>
#include <JsonCreator/Object.h>

#include <rapidjson/document.h>

#include <magic_enum/magic_enum.hpp>

#include <NetMessenger.h>

#include "FmtUtils.h"

#include "../dcc/DccLiteService.h"
#include "../dcc/NetworkDevice.h"
#include "../dcc/OutputDecoder.h"
#include "../dcc/SignalDecoder.h"

#include "../sys/BonjourService.h"
#include "../sys/Broker.h"
#include "../sys/ZeroConfSystem.h"
#include "../sys/SpecialFolders.h"

#include "TerminalCmd.h"
#include "Util.h"



#include <thread>

using namespace std::chrono_literals;

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

	std::string MakeRpcNotificationMessage(CmdId_t id, std::string_view methodName, std::function<void(JsonOutputStream_t &object)> filler)
	{
		return MakeRpcMessage(id, &methodName, "params", filler);
	}

	static std::string MakeRpcErrorResponse(const CmdId_t id, const std::string &msg)
	{
		return MakeRpcMessage(id, nullptr, "error", [&, msg](JsonOutputStream_t &params) { params.AddStringValue("message", msg); });
	}

	static std::string MakeRpcResultMessage(const CmdId_t id, std::function<void(JsonOutputStream_t &object)> filler)
	{
		return MakeRpcMessage(id, nullptr, "result", filler);
	}

	class TaskManager
	{
		public:
			TaskManager() = default;
			TaskManager(TaskManager &&other) = default;
			TaskManager(const TaskManager &manager) = delete;

			TaskManager &operator=(TaskManager &&other) = default;

			void AddTask(std::shared_ptr<NetworkTask> task)
			{
				m_mapNetworkTasks.insert(std::make_pair(task->GetTaskId(), task));
			}

			NetworkTask *TryFindTask(uint32_t taskId) const noexcept
			{
				auto it = m_mapNetworkTasks.find(taskId);

				return it == m_mapNetworkTasks.end() ? nullptr : it->second.get();
			}

			inline void RemoveTask(uint32_t taskId) noexcept
			{
				m_mapNetworkTasks.erase(taskId);
			}

		private:
			std::map<uint32_t, std::shared_ptr<NetworkTask>>	m_mapNetworkTasks;
	};


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
				auto item = context.GetItem();
				if (!item->IsFolder())
				{
					throw TerminalCmdException(fmt::format("Current location {} is invalid", context.GetLocation().string()), id);
				}
				auto folder = static_cast<IFolderObject *>(item);

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

					folder = static_cast<IFolderObject *>(item);
				}		

				return MakeRpcResultMessage(id, [folder](Result_t &results) 
				{
					results.AddStringValue("classname", "ChildItem");
					results.AddStringValue("location", folder->GetPath().string());

					auto dataArray = results.AddArray("children");

					folder->VisitChildren([&dataArray](IObject &item)
						{
							auto itemObject = dataArray.AddObject();
							item.Serialize(itemObject);

							return true;
						}
					);					
				});
			}
	};

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
				auto item = context.GetItem();
				if (!item->IsFolder())
				{
					throw TerminalCmdException(fmt::format("Current location {} is invalid", context.GetLocation().string()), id);
				}
				auto folder = static_cast<IFolderObject *>(item);

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
			explicit SetLocationCmd(RName name = RName{ "Set-Location" }) :
				TerminalCmd(name)
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

				auto folder = static_cast<IFolderObject *>(item);

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

				return MakeRpcResultMessage(id, [folder](Result_t &results)
					{
						results.AddStringValue("classname", "CmdList");

						auto dataArray = results.AddArray("cmds");

						folder->VisitChildren([&dataArray](auto &cmd)
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

	class ServiceCmdBase : public TerminalCmd
	{
		protected:
			explicit ServiceCmdBase(RName name) :
				TerminalCmd(name)
			{
				//empty
			}

			template <typename T>
			T &GetService(const TerminalContext &context, const CmdId_t id, std::string_view serviceName)
			{
				auto &root = static_cast<FolderObject &>(context.GetItem()->GetRoot());

				ObjectPath path{ SpecialFolders::GetPath(SpecialFolders::Folders::ServicesId) };
				path.append(serviceName);

				auto obj = root.TryNavigate(path);
				if (obj == nullptr)
				{
					throw TerminalCmdException(fmt::format("Service {} not found", serviceName), id);
				}

				auto *service = dynamic_cast<T *>(obj);
				if (service == nullptr)
				{
					throw TerminalCmdException(fmt::format("Service {} does not has requested type", serviceName, typeid(T).name()), id);
				}

				return *service;
			}
	};

	class ResetCmd : public ServiceCmdBase
	{
		public:
			explicit ResetCmd(RName name = RName{ "Reset" }) :
				ServiceCmdBase(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto paramsIt = request.FindMember("params");
				if ((paramsIt == request.MemberEnd()) || (!paramsIt->value.IsArray()) || (paramsIt->value.Size() < 2))
				{
					throw TerminalCmdException(fmt::format("Usage: {} <service> <item>", this->GetName()), id);
				}

				auto serviceName = paramsIt->value[0].GetString();
				auto itemName = RName::Get(paramsIt->value[1].GetString());

				auto &ireset = this->GetService<IResettableService>(context, id, serviceName);

				dcclite::Log::Info("[ResetCmd] Resetting {} at {}.", itemName, serviceName);
				ireset.IResettableService_ResetItem(itemName);

				return MakeRpcResultMessage(id, [](Result_t &results)
					{
						results.AddStringValue("classname", "string");
						results.AddStringValue("msg", "OK");
					}
				);
			}
	};

	class DccLiteCmdBase : public ServiceCmdBase
	{
		protected:
			explicit DccLiteCmdBase(RName name) :
				ServiceCmdBase(name)
			{
				//empty
			}

			inline DccLiteService &GetDccLiteService(const TerminalContext& context, const CmdId_t id, std::string_view dccSystemName)
			{		
				return this->GetService<DccLiteService>(context, id, dccSystemName);
			}

	};

	class DecoderCmdBase : public DccLiteCmdBase
	{
		protected:
			explicit DecoderCmdBase(RName name) :
				DccLiteCmdBase(name)
			{
				//empty
			}

			std::tuple<Decoder *, const char *, RName> FindDecoder(const TerminalContext &context, const CmdId_t id, const rapidjson::Document &request)
			{
				auto paramsIt = request.FindMember("params");
				if ((paramsIt == request.MemberEnd()) || (!paramsIt->value.IsArray()) || (paramsIt->value.Size() < 2))
				{
					throw TerminalCmdException(fmt::format("Usage: {} <dccSystem> <decoder>", this->GetName()), id);
				}

				auto dccSystemName = paramsIt->value[0].GetString();
				auto decoderId = RName::Get(paramsIt->value[1].GetString());

				auto &service = this->GetDccLiteService(context, id, dccSystemName);

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
			explicit ActivateItemCmd(RName name = RName{ "Activate-Item" }) :
				DecoderCmdBase(name)
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
			explicit DeactivateItemCmd(RName name = RName{ "Deactivate-Item" }) :
				DecoderCmdBase(name)
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
			explicit FlipItemCmd(RName name = RName{ "Flip-Item" }) :
				DecoderCmdBase(name)
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
			explicit SetAspectCmd(RName name = RName{ "Set-Aspect" }) :
				DecoderCmdBase(name)
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

				signalDecoder->SetAspect(aspect.value(), this->GetName().GetData().data());

				return MakeRpcResultMessage(id, [aspectName](Result_t &results)
					{
						results.AddStringValue("classname", "string");
						results.AddStringValue("msg", fmt::format("OK: {}", aspectName));
					}
				);
			}
	};

	//
	//
	// ReadEEPromCmd	
	//
	//


	class ReadEEPromFiber : public TerminalCmdFiber, private NetworkTask::IObserver, public EventHub::IEventTarget
	{
		public:
			ReadEEPromFiber(const CmdId_t id, TerminalContext &context, NetworkDevice &device):
				TerminalCmdFiber(id, context),
				m_spTask{ device.StartDownloadEEPromTask(this, m_vecEEPromData)}				
			{
				if (!m_spTask)
					throw TerminalCmdException("No task provided for ReadEEPromFiber", id);
				
				std::string fileName{ device.GetName().GetData() };
				fileName.append(".rom");
				
				m_pathRomFileName = device.GetProject().GetAppFilePath(fileName);				
			}	

			~ReadEEPromFiber()
			{
				//
				//Must wait, so we make sure the thread will not fire a event after it is destroyed
				m_Future.wait();

				EventHub::CancelEvents(*this);
			}

		private:
			void OnNetworkTaskStateChanged(NetworkTask &task) override
			{				
				assert(&task == m_spTask.get());

				m_spTask->SetObserver(nullptr);

				if (m_spTask->HasFailed())
				{
					m_rclContext.SendClientNotification(MakeRpcErrorResponse(m_tCmdId, fmt::format("Download task failed: {}", m_spTask->GetMessage())));

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

			class DiskWriteFinishedEvent: public EventHub::IEvent
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
				auto msg = MakeRpcResultMessage(m_tCmdId, [this](Result_t &results)
					{
						results.AddStringValue("classname", "ReadEEPromResult");
						results.AddStringValue("filepath", m_pathRomFileName.string());
					}
				);

				m_rclContext.SendClientNotification(msg);

				//finally finished, so go away like MR. MEESEEKS
				m_rclContext.DestroyFiber(*this);
			}

			static void SaveEEprom(ReadEEPromFiber &fiber, const DownloadEEPromTaskResult_t &data, const dcclite::fs::path &fileName)
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
				EventHub::PostEvent< DiskWriteFinishedEvent>(std::ref(fiber));
			}

		private:			
			DownloadEEPromTaskResult_t m_vecEEPromData;

			std::shared_ptr<NetworkTask> m_spTask;

			dcclite::fs::path m_pathRomFileName;

			std::future<void> m_Future;
	};

	class ReadEEPromCmd : public DccLiteCmdBase
	{
		public:
			explicit ReadEEPromCmd(RName name = RName{ "Read-EEProm" }) :
				DccLiteCmdBase(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document& request) override
			{
				auto paramsIt = request.FindMember("params");
				if (paramsIt->value.Size() < 2)
				{
					throw TerminalCmdException(fmt::format("Usage: {} <dccSystem> <device>", this->GetName()), id);
				}				

				auto systemName = paramsIt->value[0].GetString();
				auto deviceName{ RName::Get(paramsIt->value[1].GetString()) };

				auto &service = this->GetDccLiteService(context, id, systemName);

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

				return std::make_unique<ReadEEPromFiber>(id, context, *networkDevice);
			}
	};

	//
	//
	// ServoProgrammer
	//
	//

	class StartServoProgrammerCmd: public DccLiteCmdBase
	{
		public:
			explicit StartServoProgrammerCmd(RName name = RName{ "Start-ServoProgrammer" }) :
				DccLiteCmdBase(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto paramsIt = request.FindMember("params");
				if (paramsIt->value.Size() < 3)
				{
					throw TerminalCmdException(fmt::format("Usage: {} <dccSystem> <device> <decoder>", this->GetName()), id);
				}

				auto systemName = paramsIt->value[0].GetString();
				auto deviceName{ RName::Get(paramsIt->value[1].GetString()) };
				auto decoderName{ RName::Get(paramsIt->value[2].GetString()) };

				auto &service = this->GetDccLiteService(context, id, systemName);

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

				auto task = networkDevice->StartServoTurnoutProgrammerTask(nullptr, decoderName);

				//
				//store the task, so future cmds can reference it
				context.GetTaskManager().AddTask(task);

				const auto taskId = task->GetTaskId();

				return MakeRpcResultMessage(id, [taskId](Result_t &results)
					{
						results.AddStringValue("classname", "TaskId"); //useless, but makes life easier to debug, we can call from the console
						results.AddIntValue("taskId", taskId);						
					}
				);
			}
	};

	class ServoProgrammerBaseCmd: public DccLiteCmdBase
	{
		protected:
			explicit ServoProgrammerBaseCmd(RName name):
				DccLiteCmdBase(name)
			{
				//empty
			}

			NetworkTask *GetTask(TerminalContext &context, const CmdId_t id, const rapidjson::Value &taskIdData)
			{
				auto taskId = taskIdData.IsString() ? dcclite::ParseNumber(taskIdData.GetString()) : taskIdData.GetInt();

				auto &taskManager = context.GetTaskManager();

				auto task = taskManager.TryFindTask(taskId);
				if (!task)
				{
					throw TerminalCmdException(fmt::format("{}: task {} not found", this->GetName(), taskId), id);
				}

				if (task->HasFailed())
				{
					//
					//forget about it
					taskManager.RemoveTask(task->GetTaskId());

					throw TerminalCmdException(fmt::format("{}: task {} failed", this->GetName(), taskId), id);
				}

				if (task->HasFinished())
				{
					//
					//forget about it
					taskManager.RemoveTask(task->GetTaskId());

					throw TerminalCmdException(fmt::format("{}: task {} finished", this->GetName(), taskId), id);

				}

				return task;
			}
			
			static inline int ParseNumParam(const rapidjson::Value &p)
			{
				return p.IsString() ? dcclite::ParseNumber(p.GetString()) : p.GetInt();
			}
	};

	class StopServoProgrammerCmd: public ServoProgrammerBaseCmd
	{
		public:
			explicit StopServoProgrammerCmd(RName name = RName{ "Stop-ServoProgrammer" }) :
				ServoProgrammerBaseCmd(name)
			{
				//empty
			}			

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto paramsIt = request.FindMember("params");
				if (paramsIt->value.Size() < 1)
				{
					throw TerminalCmdException(fmt::format("Usage: {} <taskId>", this->GetName()), id);
				}
								
				auto task = this->GetTask(context, id, paramsIt->value[0]);

				//
				//tell the task to stop
				task->Stop();
				
				//
				//forget about it
				context.GetTaskManager().RemoveTask(task->GetTaskId());
				
				//notify client
				return MakeRpcResultMessage(id, [](Result_t &results)
					{
						results.AddStringValue("classname", "string");
						results.AddStringValue("msg", "OK");
					}
				);
			}
	};

	class EditServoProgrammerCmd: public ServoProgrammerBaseCmd
	{
		private:
			typedef void (*EditProc_t)(dcclite::broker::IServoProgrammerTask &task, const rapidjson::Value &params);

			struct Action
			{
				const char *m_szName;
				EditProc_t m_pfnProc;
			};

			static void HandleMoveAction(dcclite::broker::IServoProgrammerTask &task, const rapidjson::Value &params)
			{				
				task.SetPosition(static_cast<uint8_t>(ServoProgrammerBaseCmd::ParseNumParam(params[2])));
			}

			const static Action g_Actions[];

		public:
			explicit EditServoProgrammerCmd(RName name = RName{ "Edit-ServoProgrammer" }) :
				ServoProgrammerBaseCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto paramsIt = request.FindMember("params");
				if (paramsIt->value.Size() < 3)
				{
					throw TerminalCmdException(fmt::format("Usage: {} <taskId> <cmdType> <params>", this->GetName()), id);
				}				

				auto task = this->GetTask(context, id, paramsIt->value[0]);

				auto programmerTask = dynamic_cast<dcclite::broker::IServoProgrammerTask *>(task);
				if (!programmerTask)
				{
					throw TerminalCmdException(fmt::format("{}: task {} is not a programmer task", this->GetName(), task->GetTaskId()), id);
				}		

				auto actionName = paramsIt->value[1].GetString();

				bool found = false;
				for (int i = 0; g_Actions[i].m_szName; ++i)
				{
					if (strcmp(g_Actions[i].m_szName, actionName) == 0)
					{
						g_Actions[i].m_pfnProc(*programmerTask, paramsIt->value);
						found = true;
					}
				}

				if (!found)
				{
					throw TerminalCmdException(fmt::format("{}:cmdType {} not found", this->GetName(), actionName), id);
				}

				return MakeRpcResultMessage(id, [](Result_t &results)
					{
						results.AddStringValue("classname", "string");
						results.AddStringValue("msg", "OK");
					}
				);
			}			
	};

	const EditServoProgrammerCmd::Action EditServoProgrammerCmd::g_Actions[] =
	{
		{"position", EditServoProgrammerCmd::HandleMoveAction},
		nullptr, nullptr
	};

	/**
	* 
	* This fiber is only used to monitor the ServoProgrammer task and notify the client when its finishes
	*
	*
	*/
	class ServoProgrammerDeployMonitorFiber: public TerminalCmdFiber, private NetworkTask::IObserver
	{
		public:
			ServoProgrammerDeployMonitorFiber(const CmdId_t id, TerminalContext &context, NetworkTask &task):
				TerminalCmdFiber(id, context)				
			{
				task.SetObserver(this);
			}			

		private:
			void OnNetworkTaskStateChanged(NetworkTask &task) override
			{
				//Ignore us from now...
				task.SetObserver(nullptr);

				//Notify SharpTerminal if failed or succeed
				if (task.HasFailed())
				{
					m_rclContext.SendClientNotification(MakeRpcErrorResponse(m_tCmdId, task.GetMessage()));
				}
				else if (task.HasFinished())
				{
					auto msg = MakeRpcResultMessage(m_tCmdId, [this](Result_t &results)
						{
							results.AddStringValue("classname", "string");
							results.AddStringValue("msg", "OK");
						}
					);

					m_rclContext.SendClientNotification(msg);
				}				

				//suicide, we are useless now
				m_rclContext.DestroyFiber(*this);			
			}		
	};

	class DeployServoProgrammerCmd: public ServoProgrammerBaseCmd
	{
		public:
			explicit DeployServoProgrammerCmd(RName name = RName{ "Deploy-ServoProgrammer" }) :
				ServoProgrammerBaseCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto paramsIt = request.FindMember("params");
				if (paramsIt->value.Size() < 5)
				{
					throw TerminalCmdException(fmt::format("Usage: {} <taskId> <flags> <startPos> <endPos> <operationTimeMs>", this->GetName()), id);
				}

				auto task = this->GetTask(context, id, paramsIt->value[0]);

				auto programmerTask = dynamic_cast<dcclite::broker::IServoProgrammerTask *>(task);
				if (!programmerTask)
				{
					throw TerminalCmdException(fmt::format("{}: task {} is not a programmer task", this->GetName(), task->GetTaskId()), id);
				}

				programmerTask->DeployChanges(
					static_cast<uint8_t>(ServoProgrammerBaseCmd::ParseNumParam(paramsIt->value[1])),		//flags
					static_cast<uint8_t>(ServoProgrammerBaseCmd::ParseNumParam(paramsIt->value[2])),		//startPos
					static_cast<uint8_t>(ServoProgrammerBaseCmd::ParseNumParam(paramsIt->value[3])),		//endPos
					std::chrono::milliseconds{ ServoProgrammerBaseCmd::ParseNumParam(paramsIt->value[4]) }	//operationTime
				);

				//
				//we do not need to track it anymore...
				context.GetTaskManager().RemoveTask(task->GetTaskId());

				return std::make_unique<ServoProgrammerDeployMonitorFiber>(id, context, *task);				
			}
	};

	//
	//
	// RName
	//
	//

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

				return MakeRpcResultMessage(id, [&names](Result_t &results)
					{
						results.AddStringValue("classname", "RNames");
						auto dataArray = results.AddArray("rnames");
						for (auto it : names)
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
	

	//
	//
	// TerminalClient
	//
	//

	class TerminalClient: private IObjectManagerListener, ITerminalClient_ContextServices, EventHub::IEventTarget
	{
		public:
			TerminalClient(ITerminalServiceClientProxy &owner, TerminalCmdHost &cmdHost, dcclite::IObject &root, const dcclite::Path_t &ownerPath, const NetworkAddress address, Socket &&socket);
			TerminalClient(const TerminalClient &client) = delete;
			TerminalClient(TerminalClient &&other) = delete;

			virtual ~TerminalClient();			

		private:
			void OnObjectManagerEvent(const ObjectManagerEvent &event) override;

			void RegisterListeners();

			void SendItemPropertyValueChangedNotification(const ObjectManagerEvent &event);

			FolderObject *TryGetServicesFolder() const;	

			void ReceiveDataThreadProc();

			void OnMsg(const std::string &msg);

			//
			//
			// ITerminalClient_ContextServices 
			//
			//
			void DestroyFiber(TerminalCmdFiber &fiber) override;
			TaskManager &GetTaskManager() override;
			void SendClientNotification(const std::string_view msg) override;

			class MsgArrivedEvent: public EventHub::IEvent
			{
				public:
					MsgArrivedEvent(TerminalClient &target, std::string &&msg):
						IEvent(target),
						m_strMessage(msg)
					{
						//empty
					}

					void Fire() override
					{
						static_cast<TerminalClient &>(this->GetTarget()).OnMsg(m_strMessage);
					}

				private:
					std::string m_strMessage;
			};

		private:
			NetMessenger	m_clMessenger;			
			TerminalContext m_clContext;

			ITerminalServiceClientProxy &m_rclOwner;
			TerminalCmdHost				&m_rclCmdHost;

			std::list<std::unique_ptr<TerminalCmdFiber>>	m_lstFibers;	

			TaskManager										m_clTaskManager;

			std::thread										m_thReceiveThread;

			const NetworkAddress	m_clAddress;
	};

	TerminalClient::TerminalClient(ITerminalServiceClientProxy &owner, TerminalCmdHost &cmdHost, dcclite::IObject &root, const dcclite::Path_t &ownerPath, const NetworkAddress address, Socket &&socket) :
		m_clMessenger(std::move(socket)),
		m_rclOwner(owner),	
		m_rclCmdHost(cmdHost),
		m_clContext(static_cast<dcclite::FolderObject &>(root), *this),
		m_clAddress(address)
	{
		m_clContext.SetLocation(ownerPath);

		this->RegisterListeners();

		m_thReceiveThread = std::thread{ [this] {this->ReceiveDataThreadProc(); } };
		dcclite::SetThreadName(m_thReceiveThread, "TerminalClient::ReceiveThread");
	}

	TerminalClient::~TerminalClient()
	{
		m_clMessenger.Close();

		m_thReceiveThread.join();

		auto *servicesFolder = this->TryGetServicesFolder();
		if (servicesFolder == nullptr)
			return;

		servicesFolder->VisitChildren([this](auto &item)
			{
				auto *service = dynamic_cast<Service *>(&item);
				if (service != nullptr)
					service->m_sigEvent.disconnect(this);

				return true;
			}
		);		

		EventHub::CancelEvents(*this);
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

		ObjectPath servicesPath{ SpecialFolders::GetPath(SpecialFolders::Folders::ServicesId) };
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

		servicesFolder->VisitChildren([this](auto &item)
			{
				auto *service = dynamic_cast<Service *>(&item);
				
				if (service != nullptr)
				{
					service->m_sigEvent.connect(&TerminalClient::OnObjectManagerEvent, this);
				}
				else
				{
					dcclite::Log::Warn("[TerminalClient::RegisterListeners] Object {} is not a service, it is {}", item.GetName(), item.GetTypeName());
				}

				return true;
			}
		);		
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

	TaskManager &TerminalClient::GetTaskManager()
	{
		return m_clTaskManager;
	}

	void TerminalClient::SendClientNotification(const std::string_view msg)
	{
		if (!m_clMessenger.Send(m_clAddress, msg))
		{
			dcclite::Log::Error("[TerminalClient::Update] fiber result for {} not sent, contents: {}", m_clAddress.GetIpString(), msg);
		}
	}

	void TerminalClient::DestroyFiber(TerminalCmdFiber &fiber)
	{
#if 1
		auto it = std::find_if(
			m_lstFibers.begin(), 
			m_lstFibers.end(), 
			[&fiber](const std::unique_ptr<TerminalCmdFiber> &item)
			{
				return item.get() == &fiber;
			}
		);
#endif

#if 0
		m_lstFibers.clear();
#endif
		
#if 1
		if (it != m_lstFibers.end())
			m_lstFibers.erase(it);
#endif
	}

	void TerminalClient::OnMsg(const std::string &msg)
	{
		TerminalCmd::CmdResult_t result;

		int cmdId = -1;
		try
		{
			//dcclite::Log::Trace("[TerminalClient::OnMsg] Got msg");

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

			auto idKey = doc.FindMember("id");
			if ((idKey == doc.MemberEnd()) || (!idKey->value.IsInt()))
			{
				throw TerminalCmdException(fmt::format("No method id in: {}", msg), -1);
			}

			cmdId = idKey->value.GetInt();

			auto methodKey = doc.FindMember("method");
			if ((methodKey == doc.MemberEnd()) || (!methodKey->value.IsString()))
			{
				throw TerminalCmdException(fmt::format("Invalid method name in msg: {}", msg), cmdId);
			}

			const auto methodName = RName::TryGetName(methodKey->value.GetString());
			if (!methodName)
			{
				dcclite::Log::Error("Cmd {} is not registered in name system", methodKey->value.GetString());
				throw TerminalCmdException(fmt::format("Cmd {} is not registered in name system", methodKey->value.GetString()), cmdId);
			}

			auto cmd = m_rclCmdHost.TryFindCmd(methodName);
			if (cmd == nullptr)
			{
				dcclite::Log::Error("Invalid cmd: {}", methodName);
				throw TerminalCmdException(fmt::format("Invalid cmd name: {}", methodName), cmdId);
			}

			//dcclite::Log::Trace("[TerminalClient::OnMsg] Running cmd {} - {}", cmd->GetName(), cmdId);
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

	void TerminalClient::ReceiveDataThreadProc()
	{		
		for (;;)
		{
			auto[status, msg] = m_clMessenger.Poll();

			if (status == Socket::Status::DISCONNECTED)
				break;

			if (status == Socket::Status::WOULD_BLOCK)
				continue;

			if (status != Socket::Status::OK)
				throw std::logic_error(fmt::format("[TerminalClient::ReceiveDataThreadProc] Unexpected socket error: {}", magic_enum::enum_name(status)));
			
			//dcclite::Log::Trace("[TerminalClient::ReceiveDataThreadProc] Got data");
			EventHub::PostEvent<TerminalClient::MsgArrivedEvent>(std::ref(*this), std::move(msg));
		}

		m_rclOwner.Async_DisconnectClient(*this);
	}

	//
	//
	//
	//
	//

	class TerminalServiceAcceptConnectionEvent: public EventHub::IEvent
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

	class TerminalServiceClientDisconnectedEvent: public EventHub::IEvent
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

	//
	//
	// TerminalService
	//
	//

	TerminalService::TerminalService(RName name, Broker &broker, const rapidjson::Value &params, const Project &project) :
		Service(name, broker, params, project)		
	{	
		auto cmdHost = broker.GetTerminalCmdHost();

		assert(cmdHost);	

		{
			auto getChildItemCmd = cmdHost->AddCmd(std::make_unique<GetChildItemCmd>());
			cmdHost->AddAlias(RName{ "dir" }, *getChildItemCmd);
			cmdHost->AddAlias(RName{ "ls" }, *getChildItemCmd);
		}

		{
			cmdHost->AddCmd(std::make_unique<GetItemCmd>());	
		}

		{
			auto setLocationCmd = cmdHost->AddCmd(std::make_unique<SetLocationCmd>());
			cmdHost->AddAlias(RName{ "cd" }, *setLocationCmd);
		}	

		{
			auto getCommandCmd = cmdHost->AddCmd(std::make_unique<GetCommandCmd>());
			cmdHost->AddAlias(RName{ "gcm" }, *getCommandCmd);
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
			cmdHost->AddCmd(std::make_unique<StartServoProgrammerCmd>());
			cmdHost->AddCmd(std::make_unique<StopServoProgrammerCmd>());
			cmdHost->AddCmd(std::make_unique<EditServoProgrammerCmd>());
			cmdHost->AddCmd(std::make_unique<DeployServoProgrammerCmd>());
		}

		{
			cmdHost->AddCmd(std::make_unique<SetAspectCmd>());
		}

		{
			cmdHost->AddCmd(std::make_unique<ReadEEPromCmd>());
		}

		{
			cmdHost->AddCmd(std::make_unique<ResetCmd>());
		}

		{
			cmdHost->AddCmd(std::make_unique<GetRNames>());
		}

		const auto port = params["port"].GetInt();
		
		m_thListenThread = std::thread{ [port, this] {this->ListenThreadProc(port); } };		
		dcclite::SetThreadName(m_thListenThread, "TerminalService::ListenThread");

		if (auto bonjourService = static_cast<BonjourService *>(m_rclBroker.TryFindService(RName{ BONJOUR_SERVICE_NAME })))
			bonjourService->Register("terminal", "dcclite", NetworkProtocol::TCP, port, 36);

		ZeroConfSystem::Register(this->GetTypeName(), port);
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
		EventHub::CancelEvents(*this);
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
		EventHub::PostEvent< TerminalServiceClientDisconnectedEvent>(std::ref(*this), std::ref(client));
	}

	void TerminalService::OnAcceptConnection(const dcclite::NetworkAddress &address, dcclite::Socket &&s)
	{
		dcclite::Log::Info("[TerminalService] Client connected {}", address.GetIpString());

		ITerminalServiceClientProxy &proxy = *this;

		m_vecClients.push_back(
			std::make_unique<TerminalClient>(
				proxy,
				*m_rclBroker.GetTerminalCmdHost(), 
				this->GetRoot(), 
				this->GetPath(), 
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
			
			EventHub::PostEvent<TerminalServiceAcceptConnectionEvent>(
				std::ref(*this), 
				address, 
				std::move(socket)				
			);
		}
	}	
}
