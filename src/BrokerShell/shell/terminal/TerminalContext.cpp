// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "TerminalContext.h"

#include <dcclite/IFolderObject.h>

namespace dcclite::broker::shell::terminal
{
	TerminalContext::TerminalContext(dcclite::IFolderObject &root, ITerminalClient_ContextServices &terminalClientServices):
		m_pclRoot(&root),
		m_pthLocation{ root.GetPath() },
		m_rclTerminalClientServices(terminalClientServices)
	{
		//empty
	}

	void TerminalContext::SetLocation(const dcclite::IFolderObject &newLocation)
	{
		m_pthLocation = newLocation.GetPath();
	}

	dcclite::IObject *TerminalContext::TryGetItem() const
	{
		return m_pclRoot->TryNavigate(m_pthLocation);
	}
}
