// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "EditCmds.h"

#include <fmt/format.h>
#include <stdexcept>

#include "Panel.h"
#include "RailObject.h"

namespace LitePanel
{
	InsertRailCmd::InsertRailCmd(std::unique_ptr<RailObject> rail) :
		m_spRailObject(std::move(rail))
	{
		//empty
	}

	std::unique_ptr<EditCmd> InsertRailCmd::Run(Panel &panel) noexcept
	{
		TileCoord_t coord = m_spRailObject->GetPosition();

		panel.RegisterRail(std::move(m_spRailObject));

		return std::make_unique<RemoveRailCmd>(coord);
	}

	RemoveRailCmd::RemoveRailCmd(const TileCoord_t &coord) :
		m_Position(coord)
	{
		//empty
	}

	std::unique_ptr<EditCmd> RemoveRailCmd::Run(Panel &panel) noexcept
	{
		auto obj = panel.TryUnregisterRail(m_Position);

		return std::make_unique<InsertRailCmd>(std::move(obj));
	}
}

