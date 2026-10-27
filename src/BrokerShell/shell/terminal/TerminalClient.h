// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include <list>
#include <map>

#include <spdlog/sinks/base_sink.h>

#include <dcclite/NetMessenger.h>

#include "sys/EventHub.h"
#include "sys/Service.h"

#include "TerminalCmd.h"
#include "TerminalContext.h"
#include "TerminalService.h"

namespace dcclite::broker::exec::dcc
{
	class NetworkTask;
}

namespace dcclite::broker::shell::terminal
{	
	class CmdHostService;
	class TerminalClient;
	
	class TerminalClientLogSink : public spdlog::sinks::base_sink <std::mutex>
	{
		public:
			TerminalClientLogSink(TerminalClient &client);

		protected:
			void sink_it_(const spdlog::details::log_msg &msg) override;
			void flush_() override;

		private:
			TerminalClient &m_rclClient;
	};

	class TaskManager
	{
		public:
			TaskManager() = default;
			TaskManager(TaskManager &&other) = default;
			TaskManager(const TaskManager &manager) = delete;

			TaskManager &operator=(TaskManager &&other) = default;

			void AddTask(std::shared_ptr<exec::dcc::NetworkTask> task);

			exec::dcc::NetworkTask *TryFindTask(uint32_t taskId) const noexcept;

			void RemoveTask(uint32_t taskId) noexcept;

		private:
			std::map<uint32_t, std::shared_ptr<exec::dcc::NetworkTask>>	m_mapNetworkTasks;
	};

	class TerminalClient: private sys::IObjectManagerListener, ITerminalClient_ContextServices, sys::EventHub::IEventTarget
	{
		public:
			TerminalClient(
				ITerminalServiceClientProxy &owner, 
				CmdHostService &cmdHost,
				sys::Broker &broker,
				const dcclite::IFolderObject &currentLocation, 
				const NetworkAddress address, 
				Socket &&socket
			);
			TerminalClient(const TerminalClient &client) = delete;
			TerminalClient(TerminalClient &&other) = delete;

			virtual ~TerminalClient();

		private:
			void OnObjectManagerEvent(const sys::ObjectManagerEvent &event) override;

			void RegisterListeners();

			void SendItemPropertyValueChangedNotification(const sys::ObjectManagerEvent &event);

			void ReceiveDataThreadProc();

			void OnMsg(const std::string &msg);

			void OnLogSinkMsg(std::string logmsg);

			void SendLogMsg(std::string logmsg);

			//
			//
			// ITerminalClient_ContextServices 
			//
			//
			void DestroyFiber(TerminalCmdFiber &fiber) override;
			TaskManager &GetTaskManager() override;
			void SendClientNotification(const std::string_view msg) override;

			friend class MsgArrivedEvent;
			friend class LogMsgEvent;
			friend class TerminalClientLogSink;

		private:
			NetMessenger	m_clMessenger;
			TerminalContext m_clContext;

			ITerminalServiceClientProxy &m_rclOwner;
			CmdHostService				&m_rclCmdHost;
			sys::Broker					&m_rclBroker;

			std::list<std::unique_ptr<TerminalCmdFiber>>	m_lstFibers;

			TaskManager										m_clTaskManager;

			std::thread										m_thReceiveThread;

			const NetworkAddress							m_clAddress;

			std::shared_ptr<TerminalClientLogSink>			m_spLogSink;
		};
}
