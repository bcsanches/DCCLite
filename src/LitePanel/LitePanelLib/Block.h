// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This Source Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include <string>

#include <array>
#include <optional>

#include "LitePanelLibDefs.h"
#include "BlockIndex.h"

namespace LitePanel
{
	class Block
	{
		public:
			Block(TileCoord_t leftTile, TileCoord_t rightTile, std::string userName = "");

			inline void ExpandLeft(TileCoord_t leftTile) noexcept
			{
				m_clLeftTile = leftTile;
			}

			inline void ExpandRight(TileCoord_t rightTile) noexcept
			{
				m_clRightTile = rightTile;
			}

			inline TileCoord_t GetLeftTile() const noexcept
			{
				return m_clLeftTile;
			}

			inline TileCoord_t GetRightTile() const noexcept
			{
				return m_clRightTile;
			}

		private:
			std::string m_strUserName;

			// Those points always point to valid positions.
			// First and last can be the same (single tile blocks).
			TileCoord_t m_clLeftTile;
			TileCoord_t m_clRightTile;
	};

	class BlockManager
	{
		public:
			BlockIndex AllocBlock(TileCoord_t leftTile, TileCoord_t rightTile, std::string userName = "");
			void ReleaseBlock(BlockIndex index);

			const Block &GetBlock(BlockIndex index) const;
			Block &GetBlock(BlockIndex index);

		private:
			std::array<std::optional<Block>, MAX_BLOCKS> m_arBlocks;
	};
}
