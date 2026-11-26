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
#include <ranges>

#include <fmt/format.h>

#include "BlockIndex.h"

namespace LitePanel
{
	Block::Block(TileCoord_t leftTile, TileCoord_t rightTile, std::string userName):
		m_clLeftTile{leftTile},
		m_clRightTile{rightTile},
		m_strUserName{std::move(userName)}
	{
		if (leftTile.m_tX > rightTile.m_tX)
			throw std::invalid_argument("[Block::Block] left tile must be at left of right tile");
	}

	BlockIndex BlockManager::AllocBlock(TileCoord_t leftTile, TileCoord_t rightTile, std::string userName)
	{
		auto it = std::ranges::find_if(m_arBlocks, [](const auto &block)
		{
			return !block.has_value();
		});

		if (it == m_arBlocks.end())
			throw std::out_of_range("[BlockManager::AllocBlock] No more free blocks");

		auto index = std::distance(m_arBlocks.begin(), it);

		it->emplace(leftTile, rightTile, std::move(userName));

		return BlockIndex{ index };
	}

	void BlockManager::ReleaseBlock(BlockIndex index)
	{
		if (!index)
		{
			//free null... do nothing
			return;
		}

		m_arBlocks[index.GetValue()] = std::nullopt;
	}

	const Block &BlockManager::GetBlock(BlockIndex index) const
	{
		if (!index)
			throw std::invalid_argument("[BlockManager::GetBlock] index is null");

		auto pos = index.GetValue();		

		if (!m_arBlocks[pos].has_value())
			throw std::invalid_argument(fmt::format("[BlockManager::GetBlock] BlockIndex {} points to a null block", pos));

		return m_arBlocks[pos].value();
	}

	Block &BlockManager::GetBlock(BlockIndex index)
	{
		if (!index)
			throw std::invalid_argument("[BlockManager::GetBlock] index is null");

		auto pos = index.GetValue();

		if (!m_arBlocks[pos].has_value())
			throw std::invalid_argument(fmt::format("[BlockManager::GetBlock] BlockIndex {} points to a null block", pos));

		return m_arBlocks[pos].value();
	}

}
