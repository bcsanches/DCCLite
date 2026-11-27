#include <gtest/gtest.h>

#include "Block.h"
#include "Panel.h"
#include "RailObject.h"

using namespace LitePanel;

static const RailLink g_lnkWestBound{ ObjectAngles::EAST, ObjectAngles::WEST };
static const RailLink g_lnkLeftCurveNorthEast{ ObjectAngles::WEST, ObjectAngles::NORTHEAST };

TEST(PanelUnitTest, RegisterSingleRailCreatesBlock)
{
	Panel panel({ 10, 10 }, "Test");

	const TileCoord_t position{ 4, 5 };

	auto &rail = panel.RegisterRail(std::make_unique<SimpleRailObject>(position, g_lnkWestBound));

	ASSERT_TRUE(rail.GetBlockIndex());

	auto &block = panel.GetBlock(rail.GetBlockIndex());

	ASSERT_EQ(block.GetLeftTile(), position);
	ASSERT_EQ(block.GetRightTile(), position);	
}

TEST(PanelUnitTest, RegisterRailToTheRightExpandsExistingBlock)
{
	Panel panel({ 10, 10 }, "Test");

	const TileCoord_t leftPosition{ 4, 5 };
	const TileCoord_t rightPosition{ 5, 5 };

	const auto &leftRail = panel.RegisterRail(std::make_unique<SimpleRailObject>(leftPosition, g_lnkWestBound));
	 
	const auto originalBlockIndex = leftRail.GetBlockIndex();

	const auto &rightRail = panel.RegisterRail(std::make_unique<SimpleRailObject>(rightPosition, g_lnkWestBound));

	ASSERT_EQ(leftRail.GetBlockIndex(), rightRail.GetBlockIndex());
	ASSERT_EQ(leftRail.GetBlockIndex(), originalBlockIndex);

	const auto &block = panel.GetBlock(leftRail.GetBlockIndex());
	ASSERT_EQ(block.GetLeftTile(), leftRail.GetPosition());
	ASSERT_EQ(block.GetRightTile(), rightRail.GetPosition());	
}

TEST(PanelUnitTest, RegisterRailToTheRightNoConnectionWithUnsuitableTrack)
{
	Panel panel({ 10, 10 }, "Test");

	const TileCoord_t leftPosition{ 4, 5 };
	const TileCoord_t rightPosition{ 5, 5 };

	const auto &leftRail = panel.RegisterRail(std::make_unique<SimpleRailObject>(leftPosition, g_lnkLeftCurveNorthEast));

	const auto originalBlockIndex = leftRail.GetBlockIndex();

	const auto &rightRail = panel.RegisterRail(std::make_unique<SimpleRailObject>(rightPosition, g_lnkWestBound));

	//We made:
	//      
	//   -- \- 
	//

	ASSERT_NE(leftRail.GetBlockIndex(), rightRail.GetBlockIndex());
	ASSERT_EQ(leftRail.GetBlockIndex(), originalBlockIndex);	

	{
		const auto &block = panel.GetBlock(leftRail.GetBlockIndex());
		ASSERT_EQ(block.GetLeftTile(), leftRail.GetPosition());
		ASSERT_EQ(block.GetRightTile(), leftRail.GetPosition());
	}

	{
		const auto &block = panel.GetBlock(rightRail.GetBlockIndex());
		ASSERT_EQ(block.GetLeftTile(), rightRail.GetPosition());
		ASSERT_EQ(block.GetRightTile(), rightRail.GetPosition());
	}
}
