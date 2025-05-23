// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include "TerminalClient.h"

#include <magic_enum/magic_enum.hpp>

#include <dcclite/FmtUtils.h>
#include <dcclite/Log.h>
#include <dcclite/Util.h>

#include "exec/dcc/NetworkDeviceTasks.h"
#include "sys/Broker.h"

#include "CmdHostService.h"
#include "TerminalUtils.h"

namespace dcclite::broker::shell::terminal
{
	/////////////////////////////////////////////////////////////////////////////
	//
	// TaskManager
	//
	/////////////////////////////////////////////////////////////////////////////

	void TaskManager::AddTask(std::shared_ptr<exec::dcc::NetworkTask> task)
	{
		m_mapNetworkTasks.insert(std::make_pair(task->GetTaskId(), task));
	}

	exec::dcc::NetworkTask *TaskManager::TryFindTask(uint32_t taskId) const noexcept
	{
		auto it = m_mapNetworkTasks.find(taskId);

		return it == m_mapNetworkTasks.end() ? nullptr : it->second.get();
	}

	void TaskManager::RemoveTask(uint32_t taskId) noexcept
	{
		m_mapNetworkTasks.erase(taskId);
	}


	/////////////////////////////////////////////////////////////////////////////
	//
	// TerminalClient
	//
	/////////////////////////////////////////////////////////////////////////////

	TerminalClient::TerminalClient(
		ITerminalServiceClientProxy &owner, 
		CmdHostService &cmdHost,
		sys::Broker &broker,
		const dcclite::IFolderObject &currentLocation, 
		const NetworkAddress address, 
		Socket &&socket
	):
		m_clMessenger(std::move(socket)),
		m_clContext(broker.GetRoot(), *this),
		m_rclOwner(owner),
		m_rclCmdHost(cmdHost),
		m_rclBroker(broker),
		m_clAddress(address)
	{
		m_clContext.SetLocation(currentLocation);

		this->RegisterListeners();

		m_thReceiveThread = std::thread{ [this] {this->ReceiveDataThreadProc(); } };
		dcclite::SetThreadName(m_thReceiveThread, "TerminalClient::ReceiveThread");
	}

	TerminalClient::~TerminalClient()
	{
		m_clMessenger.Close();

		m_thReceiveThread.join();

		m_rclBroker.VisitServices(
			[this](auto &item)
			{
				auto *service = dynamic_cast<sys::Service *>(&item);
				if (service != nullptr)
					service->m_sigEvent.disconnect(this);

				return true;
			}
		);

		sys::EventHub::CancelEvents(*this);
	}	

	void TerminalClient::RegisterListeners()
	{
		m_rclBroker.VisitServices(
			[this](auto &item)
			{
				auto *service = dynamic_cast<sys::Service *>(&item);

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

	void TerminalClient::SendItemPropertyValueChangedNotification(const sys::ObjectManagerEvent &event)
	{
		m_clMessenger.Send(
			m_clAddress,
			detail::MakeRpcNotificationMessage(
				-1,
				"On-ItemPropertyValueChanged",
				[&event](JsonOutputStream_t &params)
				{
					event.m_pfnSerializeDeltaProc ? event.m_pfnSerializeDeltaProc(params) : event.m_pclItem->Serialize(params);
				}
			)
		);
	}

	void TerminalClient::OnObjectManagerEvent(const sys::ObjectManagerEvent &event)
	{
		switch (event.m_kType)
		{
			case sys::ObjectManagerEvent::ITEM_CHANGED:
				SendItemPropertyValueChangedNotification(event);
				break;

			case sys::ObjectManagerEvent::ITEM_CREATED:
				m_clMessenger.Send(
					m_clAddress,
					detail::MakeRpcNotificationMessage(
						-1,
						"On-ItemCreated",
						[&event](JsonOutputStream_t &params)
						{
							event.m_pclItem->Serialize(params);
						}
					)
				);
				break;

			case sys::ObjectManagerEvent::ITEM_DESTROYED:
				m_clMessenger.Send(
					m_clAddress,
					detail::MakeRpcNotificationMessage(
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

			auto jsonrpcKey = doc.FindMember(detail::JSONRPC_KEY);
			if ((jsonrpcKey == doc.MemberEnd()) || (!jsonrpcKey->value.IsString()) || (strcmp(jsonrpcKey->value.GetString(), detail::JSONRPC_VERSION)))
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
			result = detail::MakeRpcErrorResponse(ex.GetId(), ex.what());
		}
		catch (std::exception &ex)
		{
			result = detail::MakeRpcErrorResponse(cmdId, ex.what());
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
			auto [status, msg] = m_clMessenger.Poll();

			if (status == Socket::Status::DISCONNECTED)
				break;

			if (status == Socket::Status::WOULD_BLOCK)
				continue;

			if (status != Socket::Status::OK)
				throw std::logic_error(fmt::format("[TerminalClient::ReceiveDataThreadProc] Unexpected socket error: {}", magic_enum::enum_name(status)));

			//dcclite::Log::Trace("[TerminalClient::ReceiveDataThreadProc] Got data");
			sys::EventHub::PostEvent<TerminalClient::MsgArrivedEvent>(std::ref(*this), std::move(msg));
		}

		m_rclOwner.Async_DisconnectClient(*this);
	}
}
