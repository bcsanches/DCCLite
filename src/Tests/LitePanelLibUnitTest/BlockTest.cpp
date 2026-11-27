#include <gtest/gtest.h>

#include "Block.h"

using namespace LitePanel;

TEST(BlockUnitTest, CanCreateSingleTileBlock)
{
    EXPECT_NO_THROW(Block({ 5, 5 }, { 5, 5 }));
}

TEST(BlockUnitTest, CanCreateHorizontalBlock)
{
    EXPECT_NO_THROW(Block({ 2, 5 }, { 8, 5 }));
}

TEST(BlockUnitTest, RejectsInvalidHorizontalLimits)
{
    EXPECT_THROW(Block({ 8, 5 }, { 2, 5 }), std::invalid_argument);
}
