// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "DeviceClearEEPromCmd.h"

#include "FmtUtils.h"
#include "Util.h"

#include "../dcc/DccLiteService.h"
#include "../dcc/NetworkDevice.h"

#include "TerminalClient.h"
#include "TerminalUtils.h"

namespace dcclite::broker
{	
	class DeviceClearEEPromFiber: public TerminalCmdFiber, private NetworkTask::IObserver
	{
		public:
			DeviceClearEEPromFiber(const CmdId_t id, TerminalContext &context, NetworkDevice &device):
				TerminalCmdFiber(id, context),
				m_spTask{ device.StartDeviceClearEEPromTask(this) }
			{
				if (!m_spTask)
					throw TerminalCmdException("No task provided for DeviceClearEEPromFiber", id);
			}

			~DeviceClearEEPromFiber() = default;

		private:
			void OnNetworkTaskStateChanged(NetworkTask &task) override
			{
				assert(&task == m_spTask.get());

				m_spTask->SetObserver(nullptr);

				if (m_spTask->HasFailed())
				{
					m_rclContext.SendClientNotification(detail::MakeRpcErrorResponse(m_tCmdId, fmt::format("Clear EEPROM task failed: {}", m_spTask->GetMessage())));

					//suicide, we are useless now
					m_rclContext.DestroyFiber(*this);

					return;
				}

				if (m_spTask->HasFinished())
				{					
					auto msg = detail::MakeRpcResultMessage(m_tCmdId, [this](Result_t &results)
						{
							results.AddStringValue("classname", "ClearEEPromResult");							
						}
					);

					m_rclContext.SendClientNotification(msg);

					//finally finished, so go away like MR. MEESEEKS
					m_rclContext.DestroyFiber(*this);
				}
			}			

		private:			
			std::shared_ptr<NetworkTask> m_spTask;
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// RenameItemCmd
	//
	/////////////////////////////////////////////////////////////////////////////

	TerminalCmd::CmdResult_t ClearEEPromCmd::Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request)
	{
		auto item = context.GetItem();
		if (!item->IsFolder())
		{
			throw TerminalCmdException(fmt::format("Current location {} is invalid", context.GetLocation().string()), id);
		}
		auto folder = static_cast<IFolderObject *>(item);

		auto paramsIt = request.FindMember("params");
		if (paramsIt->value.Size() < 1)
		{
			throw TerminalCmdException(fmt::format("Usage: {} <itemPath>", this->GetName()), id);
		}

		auto locationParam = paramsIt->value[0].GetString();
		item = folder->TryNavigate(dcclite::Path_t(locationParam));
		if (!item)
		{
			throw TerminalCmdException(fmt::format("Invalid location {}", locationParam), id);
		}

		auto networkDevice = dynamic_cast<NetworkDevice *>(item);
		if (!networkDevice)
		{
			throw TerminalCmdException(fmt::format("Clear EEPROM operation only supported by NetworkDevice, {} is not a network device", networkDevice->GetName()), id);
		}			

		return std::make_unique<DeviceClearEEPromFiber>(id, context, *networkDevice);
	}	
}

