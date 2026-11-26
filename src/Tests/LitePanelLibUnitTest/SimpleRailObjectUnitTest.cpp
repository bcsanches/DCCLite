#include <gtest/gtest.h>

#include <optional>
#include <utility>

#include "RailObject.h"

using namespace LitePanel;

namespace
{
	constexpr TileMapSize_t MAP_SIZE{ 10, 10 };

	void ExpectNeighbors(
		const SimpleRailObject &rail,
		const std::optional<TileCoord_t> &expectedFirst,
		const std::optional<TileCoord_t> &expectedSecond)
	{
		const auto [first, second] = rail.GetNeighborTiles(MAP_SIZE);

		ASSERT_EQ(first.has_value(), expectedFirst.has_value());
		ASSERT_EQ(second.has_value(), expectedSecond.has_value());

		if (expectedFirst)
			EXPECT_EQ(*first, *expectedFirst);

		if (expectedSecond)
			EXPECT_EQ(*second, *expectedSecond);
	}
}

TEST(SimpleRailObjectUnitTest, StraightRailEastReturnsHorizontalNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, ObjectAngles::EAST, SimpleRailTypes::STRAIGHT);
	ExpectNeighbors(rail, TileCoord_t{ 4, 5 }, TileCoord_t{ 6, 5 });
}

TEST(SimpleRailObjectUnitTest, StraightRailNorthReturnsVerticalNeighborsInYOrder)
{
	const SimpleRailObject rail({ 5, 5 }, ObjectAngles::NORTH, SimpleRailTypes::STRAIGHT);
	ExpectNeighbors(rail, TileCoord_t{ 5, 4 }, TileCoord_t{ 5, 6 });
}


TEST(SimpleRailObjectUnitTest, StraightRailWestReturnsHorizontalNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, ObjectAngles::WEST, SimpleRailTypes::STRAIGHT);
	ExpectNeighbors(rail, TileCoord_t{ 4, 5 }, TileCoord_t{ 6, 5 });
}

TEST(SimpleRailObjectUnitTest, StraightRailSouthReturnsVerticalNeighborsInYOrder)
{
	const SimpleRailObject rail({ 5, 5 }, ObjectAngles::SOUTH, SimpleRailTypes::STRAIGHT);
	ExpectNeighbors(rail, TileCoord_t{ 5, 4 }, TileCoord_t{ 5, 6 });
}

TEST(SimpleRailObjectUnitTest, StraightRailNortheastReturnsDiagonalNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, ObjectAngles::NORTHEAST, SimpleRailTypes::STRAIGHT);
	ExpectNeighbors(rail, TileCoord_t{ 4, 6 }, TileCoord_t{ 6, 4 });
}

TEST(SimpleRailObjectUnitTest, StraightRailNorthwestReturnsDiagonalNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, ObjectAngles::NORTHWEST, SimpleRailTypes::STRAIGHT);
	ExpectNeighbors(rail, TileCoord_t{ 4, 4 }, TileCoord_t{ 6, 6 });
}

TEST(SimpleRailObjectUnitTest, StraightRailSouthwestReturnsDiagonalNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, ObjectAngles::SOUTHWEST, SimpleRailTypes::STRAIGHT);
	ExpectNeighbors(rail, TileCoord_t{ 4, 6 }, TileCoord_t{ 6, 4 });
}

TEST(SimpleRailObjectUnitTest, StraightRailSoutheastReturnsDiagonalNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, ObjectAngles::SOUTHEAST, SimpleRailTypes::STRAIGHT);
	ExpectNeighbors(rail, TileCoord_t{ 4, 4 }, TileCoord_t{ 6, 6 });
}

TEST(SimpleRailObjectUnitTest, TerminalUsesStraightRailGeometry)
{
	const SimpleRailObject rail({ 5, 5 }, ObjectAngles::WEST, SimpleRailTypes::TERMINAL);
	ExpectNeighbors(rail, TileCoord_t{ 4, 5 }, TileCoord_t{ 6, 5 });
}

TEST(SimpleRailObjectUnitTest, LeftCurveEastReturnsExpectedNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, ObjectAngles::EAST, SimpleRailTypes::CURVE_LEFT);
	ExpectNeighbors(rail, TileCoord_t{ 4, 5 }, TileCoord_t{ 6, 4 });
}

TEST(SimpleRailObjectUnitTest, LeftCurveWestReturnsExpectedNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, ObjectAngles::WEST, SimpleRailTypes::CURVE_LEFT);
	ExpectNeighbors(rail, TileCoord_t{ 4, 6 }, TileCoord_t{ 6, 5 });
}

TEST(SimpleRailObjectUnitTest, LeftCurveNorthReturnsExpectedNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, ObjectAngles::NORTH, SimpleRailTypes::CURVE_LEFT);
	ExpectNeighbors(rail, TileCoord_t{ 4, 4 }, TileCoord_t{ 5, 6 });
}

TEST(SimpleRailObjectUnitTest, LeftCurveSouthReturnsExpectedNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, ObjectAngles::SOUTH, SimpleRailTypes::CURVE_LEFT);
	ExpectNeighbors(rail, TileCoord_t{ 5, 4 }, TileCoord_t{ 6, 6 });
}

TEST(SimpleRailObjectUnitTest, RightCurveEastReturnsExpectedNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, ObjectAngles::EAST, SimpleRailTypes::CURVE_RIGHT);
	ExpectNeighbors(rail, TileCoord_t{ 4, 5 }, TileCoord_t{ 6, 6 });
}

TEST(SimpleRailObjectUnitTest, RightCurveWestReturnsExpectedNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, ObjectAngles::WEST, SimpleRailTypes::CURVE_RIGHT);
	ExpectNeighbors(rail, TileCoord_t{ 4, 4 }, TileCoord_t{ 6, 5 });
}

TEST(SimpleRailObjectUnitTest, RightCurveNorthReturnsExpectedNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, ObjectAngles::NORTH, SimpleRailTypes::CURVE_RIGHT);
	ExpectNeighbors(rail, TileCoord_t{ 5, 6 }, TileCoord_t{ 6, 4 });
}

TEST(SimpleRailObjectUnitTest, RightCurveSouthReturnsExpectedNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, ObjectAngles::SOUTH, SimpleRailTypes::CURVE_RIGHT);
	ExpectNeighbors(rail, TileCoord_t{ 4, 6 }, TileCoord_t{ 5, 4 });
}

TEST(SimpleRailObjectUnitTest, HorizontalRailAtLeftMapEdgeHasNoLeftNeighbor)
{
	const SimpleRailObject rail({ 0, 5 }, ObjectAngles::EAST, SimpleRailTypes::STRAIGHT);
	ExpectNeighbors(rail, std::nullopt, TileCoord_t{ 1, 5 });
}

TEST(SimpleRailObjectUnitTest, HorizontalRailAtRightMapEdgeHasNoRightNeighbor)
{
	const SimpleRailObject rail({ 9, 5 }, ObjectAngles::EAST, SimpleRailTypes::STRAIGHT);
	ExpectNeighbors(rail, TileCoord_t{ 8, 5 }, std::nullopt);
}

TEST(SimpleRailObjectUnitTest, VerticalRailAtTopMapEdgeHasNoUpperNeighbor)
{
	const SimpleRailObject rail({ 5, 0 }, ObjectAngles::NORTH, SimpleRailTypes::STRAIGHT);
	ExpectNeighbors(rail, std::nullopt, TileCoord_t{ 5, 1 });
}

TEST(SimpleRailObjectUnitTest, VerticalRailAtBottomMapEdgeHasNoLowerNeighbor)
{
	const SimpleRailObject rail({ 5, 9 }, ObjectAngles::NORTH, SimpleRailTypes::STRAIGHT);
	ExpectNeighbors(rail, TileCoord_t{ 5, 8 }, std::nullopt);
}

TEST(SimpleRailObjectUnitTest, DiagonalRailAtMapEdgeHasMissingNeighbor)
{
	const SimpleRailObject rail({ 0, 5 }, ObjectAngles::NORTHEAST, SimpleRailTypes::STRAIGHT);
	ExpectNeighbors(rail, std::nullopt, TileCoord_t{ 1, 4 });
}
