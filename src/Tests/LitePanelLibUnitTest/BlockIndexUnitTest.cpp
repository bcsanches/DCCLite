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
#include <limits>
#include <stdexcept>

#include "BlockIndex.h"

using namespace LitePanel;

TEST(BlockIndexUnitTest, DefaultConstructedIndexIsNull)
{
	const BlockIndex index;

	EXPECT_FALSE(index);
	EXPECT_TRUE(index.IsNull());
	EXPECT_EQ(index.GetValue(), BlockIndex::NULL_VALUE);
}

TEST(BlockIndexUnitTest, AcceptsValidIndexes)
{
	const BlockIndex first{ 0 };
	const BlockIndex last{ MAX_BLOCKS - 1 };

	ASSERT_TRUE(first);
	ASSERT_TRUE(last);
	EXPECT_EQ(first.GetValue(), 0u);
	EXPECT_EQ(last.GetValue(), MAX_BLOCKS - 1);
}

TEST(BlockIndexUnitTest, NegativeValuesAreNull)
{
	const BlockIndex negative{ -1 };

	EXPECT_FALSE(negative);
	EXPECT_TRUE(negative.IsNull());
}

TEST(BlockIndexUnitTest, NullSentinelIsAccepted)
{
	const BlockIndex nullIndex{ BlockIndex::NULL_VALUE };

	EXPECT_FALSE(nullIndex);
	EXPECT_TRUE(nullIndex.IsNull());
}

TEST(BlockIndexUnitTest, RejectsIndexesAboveMaximum)
{
	EXPECT_THROW(BlockIndex{ MAX_BLOCKS }, std::out_of_range);
	EXPECT_THROW(BlockIndex{ static_cast<std::int64_t>(BlockIndex::NULL_VALUE) - 1 }, std::out_of_range);
}

TEST(BlockIndexUnitTest, EqualityComparesIndexValues)
{
	EXPECT_EQ(BlockIndex{ 7 }, BlockIndex{ 7 });
	EXPECT_NE(BlockIndex{ 7 }, BlockIndex{ 8 });
	EXPECT_EQ(BlockIndex{}, BlockIndex{ -1 });
}
