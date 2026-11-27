// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "RailObject.h"

#include <fmt/format.h>
#include <magic_enum/magic_enum.hpp>

#include "render/ColorStyle.h"
#include "render/IRenderer.h"
#include "render/TileMapRenderer.h"

namespace LitePanel
{	
	RailObject::RailObject(const TileCoord_t &position, const RailLink link):
		MapObject(position),
		m_tLink{link}
	{
		//empty
	}	

	RailObject::RailObject(const rapidjson::Value &params) :
		MapObject(params),
		m_tLink{
			static_cast<ObjectAngles>(params["previousTile"].GetInt()), 
			static_cast<ObjectAngles>(params["nextTile"].GetInt())
		}
	{
		//empty
	}

	void RailObject::OnSave(JsonOutputStream_t& stream) const noexcept
	{
		MapObject::OnSave(stream);

		stream.AddIntValue("previousTile", static_cast<int>(m_tLink.GetPrevious()));
		stream.AddIntValue("nextTile", static_cast<int>(m_tLink.GetNext()));
	}

	SimpleRailObject::SimpleRailObject(const TileCoord_t& position, const RailLink link, const SimpleRailTypes type, const BlockSplitTypes splitTypes) :
		RailObject(position, link),
		m_kType(type),
		m_kBlockSplit(splitTypes)
	{
		//empty
	}

	SimpleRailObject::SimpleRailObject(const rapidjson::Value &params):
		RailObject(params),		
		m_kType{params["type"].GetInt()},
		m_kBlockSplit{static_cast<BlockSplitTypes>(params["blockSplit"].GetInt())}
	{
		//empty
	}

	void SimpleRailObject::OnSave(JsonOutputStream_t& stream) const noexcept
	{
		RailObject::OnSave(stream);
		
		stream.AddIntValue("type", static_cast<int>(m_kType));
		stream.AddIntValue("blockSplit", static_cast<int>(m_kBlockSplit));
	}	

	static void DrawHalfLine(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin, const ObjectAngles angle)
	{
		const auto index = static_cast<unsigned>(angle);

		//do some padding to hide "holes"
		static const FloatPoint_t startPoints[8] =
		{
			FloatPoint_t{0,		0.5},		//EAST
			FloatPoint_t{0.5f,	0.5f},		//NORTHEAST
			FloatPoint_t{0.5,	0.5f},		//NORTH
			FloatPoint_t{0.53f,	0.530f},	//NORTHWEST
			FloatPoint_t{1,		0.5},		//WEST
			FloatPoint_t{0.53f,	0.47f},		//SOUTHWEST
			FloatPoint_t{0.5,	0},			//SOUTH
			FloatPoint_t{0.47f,	0.47f},		//SOUTHEAST
		};

		static const FloatPoint_t endPoints[8] =
		{
			FloatPoint_t{0.535f,	0.5f},	//EAST
			FloatPoint_t{1,			0},		//NORTHEAST
			FloatPoint_t{0.5f,		1},		//NORTH
			FloatPoint_t{0.0f,		0.0f},	//NORTHWEST
			FloatPoint_t{0.465f,	0.5},	//WEST
			FloatPoint_t{0,			1},		//SOUTHWEST
			FloatPoint_t{0.5,		0.5},	//SOUTH
			FloatPoint_t{1,			1},		//SOUTHEAST
		};

		auto &colorStyle = LitePanel::Render::GetCurrentColorStyle();

		FloatPoint_t tileSize{ static_cast<float>(viewInfo.m_uTileSize), static_cast<float>(viewInfo.m_uTileSize)};

		renderer.DrawLine(
			tileOrigin + (startPoints[index] * tileSize),
			tileOrigin + (endPoints[index] * tileSize),
			colorStyle.m_tRail,
			viewInfo.m_fpLineWidth
		);
	}

	static void DrawLine(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin, const ObjectAngles angle)
	{
		const auto index = static_cast<unsigned>(angle);

		static const FloatPoint_t startPoints[8] =
		{
			FloatPoint_t{0,		0.5},	//EAST
			FloatPoint_t{0,		1},		//NORTHEAST
			FloatPoint_t{0.5,	0},		//NORTH
			FloatPoint_t{0,		0},		//NORTHWEST
			FloatPoint_t{0,		0.5},	//WEST
			FloatPoint_t{0,		1},		//SOUTHWEST
			FloatPoint_t{0.5,	0},		//SOUTH
			FloatPoint_t{0,		0},		//SOUTHEAST
		};

		static const FloatPoint_t endPoints[8] =
		{
			FloatPoint_t{1,		0.5},	//EAST
			FloatPoint_t{1,		0},		//NORTHEAST
			FloatPoint_t{0.5,	1},		//NORTH
			FloatPoint_t{1,		1},		//NORTHWEST
			FloatPoint_t{1,		0.5},	//WEST
			FloatPoint_t{1,		0},		//SOUTHWEST
			FloatPoint_t{0.5,	1},		//SOUTH
			FloatPoint_t{1,		1},		//SOUTHEAST
		};

		auto &colorStyle = LitePanel::Render::GetCurrentColorStyle();

		FloatPoint_t tileSize{ static_cast<float>(viewInfo.m_uTileSize), static_cast<float>(viewInfo.m_uTileSize) };

		renderer.DrawLine(
			tileOrigin + (startPoints[index] * tileSize),
			tileOrigin + (endPoints[index] * tileSize),
			colorStyle.m_tRail,
			viewInfo.m_fpLineWidth
		);
	}

	void SimpleRailObject::DrawVerticalBlockSplit(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const
	{
		auto halfTileSize = static_cast<float>(viewInfo.m_uHalfTileSize);
		auto tileSize = static_cast<float>(viewInfo.m_uTileSize);

		auto x = (m_kBlockSplit == BlockSplitTypes::LEFT) ? tileOrigin.m_tX + 1 : tileOrigin.m_tX + tileSize - 1;
		auto y = tileOrigin.m_tY + (halfTileSize / 2);

		auto &colorStyle = LitePanel::Render::GetCurrentColorStyle();

		renderer.DrawLine(
			FloatPoint_t{ x, y },
			FloatPoint_t{ x, y + halfTileSize },
			colorStyle.m_tRail,
			viewInfo.m_fpLineWidth
		);
	}

	void SimpleRailObject::DrawHorizontalBlockSplit(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const
	{
		auto halfTileSize = static_cast<float>(viewInfo.m_uHalfTileSize);
		auto tileSize = static_cast<float>(viewInfo.m_uTileSize);

		auto angle = m_tLink.GetNext();
		
		auto x = tileOrigin.m_tX + (halfTileSize / 2);
		auto y = (m_kBlockSplit == BlockSplitTypes::TOP) ? tileOrigin.m_tY + 1 : tileOrigin.m_tY + tileSize - 1;

		auto &colorStyle = LitePanel::Render::GetCurrentColorStyle();

		renderer.DrawLine(
			FloatPoint_t{ x, y },
			FloatPoint_t{ x + halfTileSize, y },
			colorStyle.m_tRail,
			viewInfo.m_fpLineWidth
		);
	}

	void SimpleRailObject::DrawStraightRail(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const
	{
		DrawLine(renderer, viewInfo, tileOrigin, m_tLink.GetNext());

		if (m_kBlockSplit != BlockSplitTypes::NONE)
		{
			this->DrawVerticalBlockSplit(renderer, viewInfo, tileOrigin);
		}
	}

	void SimpleRailObject::DrawCurveLeftRail(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const
	{
		auto angle = m_tLink.GetNext();
		DrawHalfLine(renderer, viewInfo, tileOrigin, angle);

		switch (angle)
		{
			case ObjectAngles::EAST:
				DrawHalfLine(renderer, viewInfo, tileOrigin, ObjectAngles::NORTHEAST);
				break;

			case ObjectAngles::WEST:
				DrawHalfLine(renderer, viewInfo, tileOrigin, ObjectAngles::SOUTHWEST);
				break;

			case ObjectAngles::NORTH:
				DrawHalfLine(renderer, viewInfo, tileOrigin, ObjectAngles::NORTHWEST);
				if (m_kBlockSplit != BlockSplitTypes::NONE)
				{
					this->DrawHorizontalBlockSplit(renderer, viewInfo, tileOrigin);
				}
				break;

			case ObjectAngles::SOUTH:
				DrawHalfLine(renderer, viewInfo, tileOrigin, ObjectAngles::SOUTHEAST);
				if (m_kBlockSplit != BlockSplitTypes::NONE)
				{
					this->DrawHorizontalBlockSplit(renderer, viewInfo, tileOrigin);
				}
				break;
		}
	}

	void SimpleRailObject::DrawCurveRightRail(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const
	{
		auto angle = m_tLink.GetNext();
		DrawHalfLine(renderer, viewInfo, tileOrigin, angle);

		switch (angle)
		{
			case ObjectAngles::EAST:
				DrawHalfLine(renderer, viewInfo, tileOrigin, ObjectAngles::SOUTHEAST);
				break;

			case ObjectAngles::WEST:
				DrawHalfLine(renderer, viewInfo, tileOrigin, ObjectAngles::NORTHWEST);
				break;

			case ObjectAngles::NORTH:
				DrawHalfLine(renderer, viewInfo, tileOrigin, ObjectAngles::NORTHEAST);
				if (m_kBlockSplit != BlockSplitTypes::NONE)
				{
					this->DrawHorizontalBlockSplit(renderer, viewInfo, tileOrigin);
				}
				break;

			case ObjectAngles::SOUTH:
				DrawHalfLine(renderer, viewInfo, tileOrigin, ObjectAngles::SOUTHWEST);
				if (m_kBlockSplit != BlockSplitTypes::NONE)
				{
					this->DrawHorizontalBlockSplit(renderer, viewInfo, tileOrigin);
				}
				break;
		}
	}

	void SimpleRailObject::Draw(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const
	{		
		switch (m_kType)
		{
			case SimpleRailTypes::STRAIGHT:
				this->DrawStraightRail(renderer, viewInfo, tileOrigin);
				break;

			case SimpleRailTypes::CURVE_LEFT:
				this->DrawCurveLeftRail(renderer, viewInfo, tileOrigin);
				break;

			case SimpleRailTypes::CURVE_RIGHT:
				this->DrawCurveRightRail(renderer, viewInfo, tileOrigin);
				break;

		}		
	}

	inline std::optional<TileCoord_t> NormalizeTileCoord(IntPoint_t point, const TileMapSize_t &mapSize)
	{
		if (point.m_tX < 0 || point.m_tY < 0 || point.m_tX >= mapSize.m_tWidth || point.m_tY >= mapSize.m_tHeight)
		{
			return std::nullopt;
		}

		return TileCoord_t{
			static_cast<TileCoord_t::Type_t>(point.m_tX),
			static_cast<TileCoord_t::Type_t>(point.m_tY)
		};
	}

	static IntPoint_t GetPointForNeighbor(const IntPoint_t &position, const ObjectAngles neighbor)
	{	
		static constexpr std::array<IntPoint_t, 8> shift =
		{
			{
				{1, 0},		//EAST
				{1, -1},	//NORTHEAST
				{0, -1},	//NORTH
				{-1, -1},	//NORTHWEST
				{-1, 0},	//WEST
				{-1, 1},	//SOUTHWEST
				{0, 1},		//SOUTH
				{1, 1}		//SOUTHEAST
			}
		};		
		
		return position + shift[static_cast<int>(neighbor)];
	}

	NeighborPositions_t SimpleRailObject::GetNeighborPositions() const
	{
		const auto position = IntPoint_t{ GetPosition() };

		IntPoint_t first;
		IntPoint_t second;

		const auto previousTile = m_tLink.GetPrevious();
		const auto nextTile = m_tLink.GetNext();

		if (((previousTile == ObjectAngles::NORTH) && (nextTile == ObjectAngles::SOUTH)) ||
			((previousTile == ObjectAngles::SOUTH) && (nextTile == ObjectAngles::NORTH)))
		{
			//special case for vertical rail line, top is left, bottom is right
			return { {position.m_tX, position.m_tY - 1}, {position.m_tX, position.m_tY + 1} };
		}

		first = GetPointForNeighbor(position, previousTile);
		second = GetPointForNeighbor(position, nextTile);

		if (first.m_tX > second.m_tX || (first.m_tX == second.m_tX && first.m_tY > second.m_tY))
		{
			std::swap(first, second);
		}

		return { first, second };
	}

	std::optional<TileCoord_t> SimpleRailObject::GetLeftNeighbor(const TileMapSize_t &mapSize) const
	{
		const auto [left, right] = GetNeighborPositions();

		return NormalizeTileCoord(left, mapSize);
	}

	std::optional<TileCoord_t> SimpleRailObject::GetRightNeighbor(const TileMapSize_t &mapSize) const
	{
		const auto [left, right] = GetNeighborPositions();

		return NormalizeTileCoord(right, mapSize);
	}

	NeighborTiles_t SimpleRailObject::GetNeighborTiles(const TileMapSize_t &mapSize) const
	{
		return {
			GetLeftNeighbor(mapSize),
			GetRightNeighbor(mapSize)
		};
	}

	static constexpr std::array<std::optional<ObjectAngles>, 8> g_kJunctionValidAngles =
	{				
		{
			ObjectAngles::WEST, // From EAST
			std::nullopt,		// From NORTHEAST
			std::nullopt,		// From NORTH
			std::nullopt,		// From NORTHWEST
			ObjectAngles::EAST,	// From WEST
			std::nullopt,		// From SOUTHWEST
			std::nullopt,		// From SOUTH
			std::nullopt,		// From SOUTHEAST
		}
	};

	JunctionRailObject::JunctionRailObject(const TileCoord_t &position, const RailLink link, const JunctionTypes type):
		RailObject(position, link),
		m_tType(type)
	{
		const auto index = static_cast<int>(link.GetPrevious());

		if (index >= g_kJunctionValidAngles.size())
			throw std::out_of_range(fmt::format("[JunctionRailObject] Invalid angle : {}", magic_enum::enum_name(link.GetPrevious())));

		const auto angle = g_kJunctionValidAngles[index];
		if(!angle.has_value())
			throw std::out_of_range(fmt::format("[JunctionRailObject] Invalid angle : {}", magic_enum::enum_name(link.GetPrevious())));
	}	

	void JunctionRailObject::Draw(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const
	{		
		const auto angle = m_tLink.GetNext();
		DrawLine(renderer, viewInfo, tileOrigin, angle);

		switch(angle)
		{			
			case ObjectAngles::EAST:			
				if (m_tType == JunctionTypes::LEFT_TURNOUT)					
					DrawHalfLine(renderer, viewInfo, tileOrigin, ObjectAngles::NORTHEAST);
				else
					DrawHalfLine(renderer, viewInfo, tileOrigin, ObjectAngles::SOUTHEAST);
				break;

			case ObjectAngles::WEST:
				if (m_tType == JunctionTypes::LEFT_TURNOUT)
					DrawHalfLine(renderer, viewInfo, tileOrigin, ObjectAngles::SOUTHWEST);
				else
					DrawHalfLine(renderer, viewInfo, tileOrigin, ObjectAngles::NORTHWEST);
				break;
		}
	}
}
