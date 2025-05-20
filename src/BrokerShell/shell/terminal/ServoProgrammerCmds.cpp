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

#include "dcc/NetworkDevice.h"
#include "dcc/TurnoutDecoder.h"

#include "TerminalClient.h"
#include "TerminalUtils.h"

namespace dcclite::broker::shell::terminal
{	

	/////////////////////////////////////////////////////////////////////////////
	//
	// StartServoProgrammerCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	TerminalCmd::CmdResult_t StartServoProgrammerCmd::Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request)
	{
		auto paramsIt = request.FindMember("params");
		if (paramsIt->value.Size() < 1)
		{
			throw TerminalCmdException(fmt::format("Usage: {} <path>", this->GetName()), id);
		}

		auto path = paramsIt->value[0].GetString();

		auto *obj = dynamic_cast<IFolderObject *>(context.TryGetItem());
		if (!obj)
		{
			throw TerminalCmdException("Terminal path is invalid! No folder found!", id);
		}

		auto decoder = dynamic_cast<Decoder *>(obj->TryNavigate(dcclite::Path_t{ path }));
		if (!decoder)
		{
			throw TerminalCmdException(fmt::format("Path {} does not result on a Decoder", path), id);
		}

		auto taskProvider = decoder->GetDevice().TryGetINetworkTaskProvider();
		if (!taskProvider)
		{
			throw TerminalCmdException(fmt::format("Device {} not based on a INetworkTaskProvider!!", decoder->GetName()), id);
		}
		
		auto task = taskProvider->StartServoTurnoutProgrammerTask(nullptr, *decoder);

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
		return detail::RunStopTaskCmd(context, this->GetName(), id, request);
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
		{nullptr, nullptr}
	};

	TerminalCmd::CmdResult_t EditServoProgrammerCmd::Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request)
	{
		auto paramsIt = request.FindMember("params");
		if (paramsIt->value.Size() < 3)
		{
			throw TerminalCmdException(fmt::format("Usage: {} <taskId> <cmdType> <params>", this->GetName()), id);
		}

		auto task = detail::GetValidTask(context, this->GetName(), id, paramsIt->value[0]);

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
					auto msg = detail::MakeRpcResultMessage(m_tCmdId, [](Result_t &results)
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

		auto task = detail::GetValidTask(context, this->GetName(), id, paramsIt->value[0]);

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

