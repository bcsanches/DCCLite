// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Block.h"

#include <stdexcept>

namespace LitePanel
{
	Block::Block(IntPoint_t leftTile, IntPoint_t rightTile, std::string userName):
		m_clLeftTile{leftTile},
		m_clRightTile{rightTile},
		m_strUserName{std::move(userName)}
	{
		if (leftTile.m_tX > rightTile.m_tX)
			throw std::invalid_argument("left tile must be at left of right tile");
	}
}
