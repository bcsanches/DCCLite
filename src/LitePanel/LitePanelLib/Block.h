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

#include <string>

#include <array>
#include <optional>

#include "Point.h"

namespace LitePanel
{
	class BlockIndex;

	class Block
	{
		public:
			Block(IntPoint_t leftTile, IntPoint_t rightTile, std::string userName = "");

		private:
			std::string m_strUserName;

			//Those points always point to valid positions
			//First and last can be same (single tile blocks)			
			IntPoint_t m_clLeftTile;
			IntPoint_t m_clRightTile;
	};

	constexpr auto MAX_BLOCKS = 1024;

	class BlockManager
	{
		public:
			BlockIndex AllocBlock(IntPoint_t leftTile, IntPoint_t rightTile, std::string userName = "");
			void ReleaseBlock(BlockIndex index);

			const Block &GetBlock(BlockIndex index) const;

		private:
			std::array<std::optional<Block>, MAX_BLOCKS> m_arBlocks;
	};
}