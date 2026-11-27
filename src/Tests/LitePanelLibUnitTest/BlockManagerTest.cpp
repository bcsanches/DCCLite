// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This Source Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>

#include "Block.h"
#include "BlockIndex.h"

using namespace LitePanel;

TEST(BlockManagerUnitTest, AllocatesBlocksUsingFirstAvailableIndexes)
{
	BlockManager manager;

	const auto first = manager.AllocBlock({ 1, 2 }, { 1, 2 });
	const auto second = manager.AllocBlock({ 3, 2 }, { 5, 2 });

	ASSERT_TRUE(first);
	ASSERT_TRUE(second);

	EXPECT_EQ(first.GetValue(), 0u);
	EXPECT_EQ(second.GetValue(), 1u);

	EXPECT_NO_THROW(manager.GetBlock(first));
	EXPECT_NO_THROW(manager.GetBlock(second));
}

TEST(BlockManagerUnitTest, ReusesReleasedIndex)
{
	BlockManager manager;

	const auto first = manager.AllocBlock({ 1, 2 }, { 1, 2 });
	const auto second = manager.AllocBlock({ 3, 2 }, { 3, 2 });

	manager.ReleaseBlock(first);

	const auto reused = manager.AllocBlock({ 5, 2 }, { 5, 2 });

	ASSERT_TRUE(reused);
	EXPECT_EQ(reused.GetValue(), first.GetValue());
	EXPECT_NE(reused.GetValue(), second.GetValue());
	EXPECT_NO_THROW(manager.GetBlock(reused));
}

TEST(BlockManagerUnitTest, ReleasingNullIndexDoesNothing)
{
	BlockManager manager;

	EXPECT_NO_THROW(manager.ReleaseBlock(BlockIndex{}));

	const auto allocated = manager.AllocBlock({ 1, 2 }, { 1, 2 });

	ASSERT_TRUE(allocated);
	EXPECT_EQ(allocated.GetValue(), 0u);
}

TEST(BlockManagerUnitTest, GetBlockRejectsNullIndex)
{
	const BlockManager manager;

	EXPECT_THROW(manager.GetBlock(BlockIndex{}), std::invalid_argument);
}

TEST(BlockManagerUnitTest, GetBlockRejectsReleasedIndex)
{
	BlockManager manager;
	const auto index = manager.AllocBlock({ 1, 2 }, { 1, 2 });

	manager.ReleaseBlock(index);

	EXPECT_THROW(manager.GetBlock(index), std::invalid_argument);
}

TEST(BlockManagerUnitTest, ThrowsWhenAllSlotsAreAllocated)
{
	BlockManager manager;

	static_assert(MAX_BLOCKS <= std::numeric_limits<TileCoord_t::Type_t>::max());

	for (std::uint32_t i = 0; i < MAX_BLOCKS; ++i)
	{
		const auto coordinate = static_cast<TileCoord_t::Type_t>(i);
		const auto index = manager.AllocBlock({ coordinate, 0 }, { coordinate, 0 });

		ASSERT_TRUE(index);
		EXPECT_EQ(index.GetValue(), i);
	}

	EXPECT_THROW(manager.AllocBlock({ 0, 1 }, { 0, 1 }), std::out_of_range);
}
