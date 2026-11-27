#include <gtest/gtest.h>

#include <optional>
#include <utility>

#include "RailObject.h"

using namespace LitePanel;

namespace
{
	constexpr TileMapSize_t MAP_SIZE{ 10, 10 };

	void ExpectNeighbors(const SimpleRailObject &rail, const std::optional<TileCoord_t> &expectedLeft, const std::optional<TileCoord_t> &expectedRight)
	{
		const auto [left, right] = rail.GetNeighborTiles(MAP_SIZE);

		ASSERT_EQ(left.has_value(), expectedLeft.has_value());
		ASSERT_EQ(right.has_value(), expectedRight.has_value());

		if (expectedLeft)
			EXPECT_EQ(*left, *expectedLeft);

		if (expectedRight)
			EXPECT_EQ(*right, *expectedRight);
	}
}

TEST(SimpleRailObjectUnitTest, HorizontalStraightReturnsPreviousAndNextNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, RailLink{ ObjectAngles::EAST, ObjectAngles::WEST });

	ExpectNeighbors(rail, TileCoord_t{ 4, 5 }, TileCoord_t{ 6, 5 });
}

TEST(SimpleRailObjectUnitTest, VerticalStraightReturnsPreviousAndNextNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, RailLink{ ObjectAngles::NORTH, ObjectAngles::SOUTH });

	ExpectNeighbors(rail, TileCoord_t{ 5, 4 }, TileCoord_t{ 5, 6 });
}

TEST(SimpleRailObjectUnitTest, NortheastSouthwestStraightReturnsDiagonalNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, RailLink{ ObjectAngles::NORTHEAST, ObjectAngles::SOUTHWEST });

	ExpectNeighbors(rail, TileCoord_t{ 4, 6 }, TileCoord_t{ 6, 4 });
}

TEST(SimpleRailObjectUnitTest, NorthwestSoutheastStraightReturnsDiagonalNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, RailLink{ ObjectAngles::NORTHWEST, ObjectAngles::SOUTHEAST });

	ExpectNeighbors(rail, TileCoord_t{ 4, 4 }, TileCoord_t{ 6, 6 });
}

TEST(SimpleRailObjectUnitTest, LeftCurveFromEastReturnsExpectedNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, RailLink{ ObjectAngles::EAST, ObjectAngles::SOUTHWEST });

	ExpectNeighbors(rail, TileCoord_t{ 4, 6 }, TileCoord_t{ 6, 5 });
}

TEST(SimpleRailObjectUnitTest, LeftCurveFromWestReturnsExpectedNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, RailLink{ ObjectAngles::WEST, ObjectAngles::NORTHEAST });

	ExpectNeighbors(rail, TileCoord_t{ 4, 5 }, TileCoord_t{ 6, 4 });
}

TEST(SimpleRailObjectUnitTest, LeftCurveFromNorthReturnsExpectedNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, RailLink{ ObjectAngles::NORTH, ObjectAngles::SOUTHEAST });

	ExpectNeighbors(rail, TileCoord_t{ 5, 4 }, TileCoord_t{ 6, 6 });
}

TEST(SimpleRailObjectUnitTest, LeftCurveFromSouthReturnsExpectedNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, RailLink{ ObjectAngles::SOUTH, ObjectAngles::NORTHWEST });

	ExpectNeighbors(rail, TileCoord_t{ 4, 4 }, TileCoord_t{ 5, 6 });
}

TEST(SimpleRailObjectUnitTest, RightCurveFromEastReturnsExpectedNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, RailLink{ ObjectAngles::EAST, ObjectAngles::NORTHWEST });

	ExpectNeighbors(rail, TileCoord_t{ 4, 4 }, TileCoord_t{ 6, 5 });
}

TEST(SimpleRailObjectUnitTest, RightCurveFromWestReturnsExpectedNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, RailLink{ ObjectAngles::WEST, ObjectAngles::SOUTHEAST });

	ExpectNeighbors(rail, TileCoord_t{ 4, 5 }, TileCoord_t{ 6, 6 });
}

TEST(SimpleRailObjectUnitTest, RightCurveFromNorthReturnsExpectedNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, RailLink{ ObjectAngles::NORTH, ObjectAngles::SOUTHWEST });

	ExpectNeighbors(rail, TileCoord_t{ 4, 6 }, TileCoord_t{ 5, 4 });
}

TEST(SimpleRailObjectUnitTest, RightCurveFromSouthReturnsExpectedNeighbors)
{
	const SimpleRailObject rail({ 5, 5 }, RailLink{ ObjectAngles::SOUTH, ObjectAngles::NORTHEAST });

	ExpectNeighbors(rail, TileCoord_t{ 5, 6 }, TileCoord_t{ 6, 4 });
}

TEST(SimpleRailObjectUnitTest, HorizontalRailAtLeftMapEdgeHasNoLeftNeighbor)
{
	const SimpleRailObject rail({ 0, 5 }, RailLink{ ObjectAngles::EAST, ObjectAngles::WEST });

	ExpectNeighbors(rail, std::nullopt, TileCoord_t{ 1, 5 });
}

TEST(SimpleRailObjectUnitTest, HorizontalRailAtRightMapEdgeHasNoRightNeighbor)
{
	const SimpleRailObject rail({ 9, 5 }, RailLink{ ObjectAngles::EAST, ObjectAngles::WEST });

	ExpectNeighbors(rail, TileCoord_t{ 8, 5 }, std::nullopt);
}

TEST(SimpleRailObjectUnitTest, VerticalRailAtTopMapEdgeHasNoLeftNeighbor)
{
	const SimpleRailObject rail({ 5, 0 }, RailLink{ ObjectAngles::NORTH, ObjectAngles::SOUTH });

	ExpectNeighbors(rail, std::nullopt, TileCoord_t{ 5, 1 });
}

TEST(SimpleRailObjectUnitTest, VerticalRailAtBottomMapEdgeHasNoRightNeighbor)
{
	const SimpleRailObject rail({ 5, 9 }, RailLink{ ObjectAngles::NORTH, ObjectAngles::SOUTH });

	ExpectNeighbors(rail, TileCoord_t{ 5, 8 }, std::nullopt);
}

TEST(SimpleRailObjectUnitTest, DiagonalRailAtMapEdgeHasMissingPreviousNeighbor)
{
	const SimpleRailObject rail({ 0, 5 }, RailLink{ ObjectAngles::NORTHEAST, ObjectAngles::SOUTHWEST });

	ExpectNeighbors(rail, std::nullopt, TileCoord_t{ 1, 4 });
}
