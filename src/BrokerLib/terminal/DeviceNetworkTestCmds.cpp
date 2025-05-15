// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "DeviceNetworkTestCmds.h"

#include <fmt/format.h>
#include <fmt/chrono.h>

#include <dcclite/FmtUtils.h>

#include "TerminalClient.h"
#include "TerminalUtils.h"

#include "../exec/NetworkDevice.h"

namespace dcclite::broker
{	
	TerminalCmd::CmdResult_t StartNetworkTestCmd::Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request)
	{
		auto paramsIt = request.FindMember("params");
		if ((paramsIt == request.MemberEnd()) || (!paramsIt->value.IsArray()) || (paramsIt->value.Size() < 1))
		{
			throw TerminalCmdException(fmt::format("Usage: {} <NetworkDevicePath>", this->GetName()), id);
		}

		auto path = paramsIt->value[0].GetString();

		auto &device = detail::GetNetworkDevice(dcclite::Path_t{ path }, context, id);

		std::chrono::milliseconds timeout = TASK_NETWORK_TEST_DEFAULT_TIMEOUT;
		if (paramsIt->value.Size() == 2)
		{
			timeout = std::chrono::milliseconds{ paramsIt->value[1].GetInt() };
			if ((timeout <= 0ms) || (timeout >= 500ms))
			{
				throw TerminalCmdException(fmt::format("Rate must be >= 0 and <= 500, not {}", timeout), id);
			}
		}

		auto task = device.StartDeviceNetworkTestTask(nullptr, timeout);
		context.GetTaskManager().AddTask(task);

		const auto taskId = task->GetTaskId();

		return detail::MakeRpcResultMessage(id, [taskId](Result_t &results)
			{
				results.AddStringValue("classname", "TaskId"); //useless, but makes life easier to debug, we can call from the console
				results.AddIntValue("taskId", taskId);
			}
		);
	}

	TerminalCmd::CmdResult_t StopNetworkTestCmd::Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request)
	{
		return detail::RunStopTaskCmd(context, this->GetName(), id, request);
	}

	TerminalCmd::CmdResult_t ReceiveNetworkTestDataCmd::Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request)
	{
		auto paramsIt = request.FindMember("params");
		if (paramsIt->value.Size() < 1)
		{
			throw TerminalCmdException(fmt::format("Usage: {} <taskId>", this->GetName()), id);
		}

		auto task = detail::GetValidTask(context, this->GetName(), id, paramsIt->value[0]);

		auto netTestTask = dynamic_cast<INetworkDeviceTestTask *>(task);
		if (!netTestTask)
		{
			throw TerminalCmdException(fmt::format("Task id {} is not a valid NetworkTestTask", paramsIt->value[0].GetInt()), id);
		}

		auto netTastkResults = netTestTask->GetCurrentResults();

		return detail::MakeRpcResultMessage(id, [netTastkResults](auto &results)
			{
				results.AddStringValue("classname", "NetworkTestResults"); //useless, but makes life easier to debug, we can call from the console

				results.AddIntValue("lostPacketsCount", netTastkResults.m_uLostPacketsCount);
				results.AddIntValue("sentPacketsCount", netTastkResults.m_uSentPacketsCount);
				results.AddIntValue("receivedPacketsCount", netTastkResults.m_uReceivedPacketsCount);
				results.AddIntValue("outOfSyncPacketsCount", netTastkResults.m_uOutOfSyncPacketsCount);
				results.AddIntValue("latency", static_cast<int>(netTastkResults.m_tLatency.count()));
			}
		);
	}
}

