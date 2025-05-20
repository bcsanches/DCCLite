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

#include <dcclite/Object.h>

namespace dcclite
{
	class IFolderObject;
}

namespace dcclite::broker::shell::terminal
{	
	class TerminalCmdFiber;
	class TaskManager;

	class ITerminalClient_ContextServices
	{
		public:
			virtual TaskManager &GetTaskManager() = 0;
			virtual void SendClientNotification(const std::string_view msg) = 0;

			virtual void DestroyFiber(TerminalCmdFiber &fiber) = 0;
	};

	/**

	The terminal context is a little helper to store the cmd current location, last acessed object etc


	*/
	class TerminalContext
	{
		public:
			explicit TerminalContext(dcclite::IFolderObject &root, ITerminalClient_ContextServices &terminalClientServices);

			TerminalContext(const TerminalContext &) = delete;

			TerminalContext(TerminalContext &&other) = delete;

			TerminalContext &operator=(TerminalContext &&other) = delete;

			void SetLocation(const dcclite::IFolderObject &newLocation);

			inline const dcclite::Path_t &GetLocation() const
			{
				return m_pthLocation;
			}

			inline TaskManager &GetTaskManager() const noexcept
			{
				return m_rclTerminalClientServices.GetTaskManager();
			}

			inline void SendClientNotification(const std::string_view msg)
			{
				m_rclTerminalClientServices.SendClientNotification(msg);
			}

			void DestroyFiber(TerminalCmdFiber &fiber)
			{
				m_rclTerminalClientServices.DestroyFiber(fiber);
			}

			/**

				returns:
					The item pointed by the m_pthLocation

			*/
			dcclite::IObject *TryGetItem() const;

			TerminalContext &operator=(TerminalContext &rhs) = delete;

		private:
			dcclite::IFolderObject *m_pclRoot;
			dcclite::Path_t m_pthLocation;

			ITerminalClient_ContextServices &m_rclTerminalClientServices;
	};
}
