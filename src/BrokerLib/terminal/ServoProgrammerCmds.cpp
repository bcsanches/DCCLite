// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "ServoProgrammerCmds.h"

#include <dcclite/FmtUtils.h>
#include <dcclite/Util.h>

#include "../dcc/DccLiteService.h"
#include "../dcc/NetworkDevice.h"

#include "TerminalClient.h"
#include "TerminalUtils.h"

namespace dcclite::broker
{	

	/////////////////////////////////////////////////////////////////////////////
	//
	// StartServoProgrammerCmd
	//
	/////////////////////////////////////////////////////////////////////////////

	TerminalCmd::CmdResult_t StartServoProgrammerCmd::Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request)
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

		return detail::MakeRpcResultMessage(id, [taskId](Result_t &results)
			{
				results.AddStringValue("classname", "TaskId"); //useless, but makes life easier to debug, we can call from the console
				results.AddIntValue("taskId", taskId);
			}
		);
	}	

	/////////////////////////////////////////////////////////////////////////////
	//
	// ServoProgrammerBaseCmd
	//
	/////////////////////////////////////////////////////////////////////////////

	dcclite::broker::NetworkTask *ServoProgrammerBaseCmd::GetTask(TerminalContext &context, const CmdId_t id, const rapidjson::Value &taskIdData)
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

	static int ParseNumParam(const rapidjson::Value &p)
	{
		return p.IsString() ? dcclite::ParseNumber(p.GetString()) : p.GetInt();
	}

	/////////////////////////////////////////////////////////////////////////////
	//
	// StopServoProgrammerCmd
	//
	/////////////////////////////////////////////////////////////////////////////

	TerminalCmd::CmdResult_t StopServoProgrammerCmd::Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request)
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
		return detail::MakeRpcResultMessage(id, [](Result_t &results)
			{
				results.AddStringValue("classname", "string");
				results.AddStringValue("msg", "OK");
			}
		);
	}

	/////////////////////////////////////////////////////////////////////////////
	//
	// EditServoProgrammerCmd
	//
	/////////////////////////////////////////////////////////////////////////////

	typedef void (*EditProc_t)(dcclite::broker::IServoProgrammerTask &task, const rapidjson::Value &params);

	static void HandleMoveAction(dcclite::broker::IServoProgrammerTask &task, const rapidjson::Value &params)
	{
		task.SetPosition(static_cast<uint8_t>(ParseNumParam(params[2])));
	}

	struct Action
	{
		const char *m_szName;
		EditProc_t m_pfnProc;
	};

	static const Action g_Actions[] =
	{
		{"position", HandleMoveAction},
		nullptr, nullptr
	};

	TerminalCmd::CmdResult_t EditServoProgrammerCmd::Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request)
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

		return detail::MakeRpcResultMessage(id, [](Result_t &results)
			{
				results.AddStringValue("classname", "string");
				results.AddStringValue("msg", "OK");
			}
		);
	}

	/////////////////////////////////////////////////////////////////////////////
	//
	// DeployServoProgrammerCmd
	//
	/////////////////////////////////////////////////////////////////////////////

	/**
	*
	* This "fiber" is only used to monitor the ServoProgrammer task and notify the client when its finishes
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
					m_rclContext.SendClientNotification(detail::MakeRpcErrorResponse(m_tCmdId, task.GetMessage()));
				}
				else if (task.HasFinished())
				{
					auto msg = detail::MakeRpcResultMessage(m_tCmdId, [this](Result_t &results)
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

	TerminalCmd::CmdResult_t DeployServoProgrammerCmd::Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request)
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
			static_cast<uint8_t>(ParseNumParam(paramsIt->value[1])),		//flags
			static_cast<uint8_t>(ParseNumParam(paramsIt->value[2])),		//startPos
			static_cast<uint8_t>(ParseNumParam(paramsIt->value[3])),		//endPos
			std::chrono::milliseconds{ ParseNumParam(paramsIt->value[4]) }	//operationTime
		);

		//
		//we do not need to track it anymore...
		context.GetTaskManager().RemoveTask(task->GetTaskId());

		return std::make_unique<ServoProgrammerDeployMonitorFiber>(id, context, *task);
	}
}

