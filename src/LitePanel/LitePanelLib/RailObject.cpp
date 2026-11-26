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
	RailObject::RailObject(const TileCoord_t &position, ObjectAngles angle):
		MapObject(position),
		m_tAngle(angle)
	{
		//empty
	}	

	RailObject::RailObject(const rapidjson::Value &params) :
		MapObject(params),
		m_tAngle{params["angle"].GetInt()}
	{
		//empty
	}

	void RailObject::OnSave(JsonOutputStream_t& stream) const noexcept
	{
		MapObject::OnSave(stream);

		stream.AddIntValue("angle", static_cast<int>(m_tAngle));
	}

	SimpleRailObject::SimpleRailObject(const TileCoord_t& position, ObjectAngles angle, const SimpleRailTypes type, const BlockSplitTypes splitTypes) :
		RailObject(position, angle),
		m_kType(type),
		m_kBlockSplit(splitTypes)
	{
		//empty
	}

	SimpleRailObject::SimpleRailObject(const rapidjson::Value &params):
		RailObject(params),		
		m_kType{params["type"].GetInt()},
		m_kBlockSplit{static_cast<uint8_t>(params["blockSplit"].GetInt())}
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

		auto angle = this->GetAngle();
		
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
		DrawLine(renderer, viewInfo, tileOrigin, this->GetAngle());

		if (m_kBlockSplit != BlockSplitTypes::NONE)
		{
			this->DrawVerticalBlockSplit(renderer, viewInfo, tileOrigin);
		}
	}

	void SimpleRailObject::DrawCurveLeftRail(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const
	{
		auto angle = this->GetAngle();
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
		auto angle = this->GetAngle();
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

	NeighborPositions_t SimpleRailObject::GetNeighborPositions() const
	{
		const auto position = IntPoint_t{ GetPosition() };

		IntPoint_t first;
		IntPoint_t second;

		switch (m_kType)
		{
			case SimpleRailTypes::STRAIGHT:
			case SimpleRailTypes::TERMINAL:
			{
				switch (GetAngle())
				{
					case ObjectAngles::EAST:
					case ObjectAngles::WEST:
						first = { position.m_tX - 1, position.m_tY };
						second = { position.m_tX + 1, position.m_tY };
						break;

					case ObjectAngles::NORTH:
					case ObjectAngles::SOUTH:
						first = { position.m_tX, position.m_tY - 1 };
						second = { position.m_tX, position.m_tY + 1 };
						break;

					case ObjectAngles::NORTHEAST:
					case ObjectAngles::SOUTHWEST:
						first = { position.m_tX - 1, position.m_tY + 1 };
						second = { position.m_tX + 1, position.m_tY - 1 };
						break;

					case ObjectAngles::NORTHWEST:
					case ObjectAngles::SOUTHEAST:
						first = { position.m_tX - 1, position.m_tY - 1 };
						second = { position.m_tX + 1, position.m_tY + 1 };
						break;

					default:
						throw std::invalid_argument("[SimpleRailObject::GetNeighborPositions] Invalid angle");
				}
				break;
			}

			case SimpleRailTypes::CURVE_LEFT:
			case SimpleRailTypes::CURVE_RIGHT:
			{
				const bool left = m_kType == SimpleRailTypes::CURVE_LEFT;

				switch (GetAngle())
				{
					case ObjectAngles::EAST:
						first = { position.m_tX - 1, position.m_tY };
						second = { position.m_tX + 1, position.m_tY + (left ? -1 : 1) };
						break;

					case ObjectAngles::WEST:
						first = { position.m_tX - 1, position.m_tY + (left ? 1 : -1) };
						second = { position.m_tX + 1, position.m_tY };
						break;

					case ObjectAngles::NORTH:
						first = { position.m_tX + (left ? -1 : 0), position.m_tY + (left ? -1 : 1) };
						second = { position.m_tX + (left ? 0 : 1), position.m_tY + (left ? 1 : -1) };
						break;

					case ObjectAngles::SOUTH:
						first = { position.m_tX + (left ? 0 : -1), position.m_tY + (left ? -1 : 1) };
						second = { position.m_tX + (left ? 1 : 0), position.m_tY + (left ? 1 : -1) };
						break;

					default:
						throw std::invalid_argument("[SimpleRailObject::GetNeighborPositions] Invalid curve angle");
				}
				break;
			}

			default:
				throw std::invalid_argument("[SimpleRailObject::GetNeighborPositions] Invalid rail type");
		}

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

	JunctionRailObject::JunctionRailObject(const TileCoord_t &position, ObjectAngles angle, const JunctionTypes type):
		RailObject(position, angle),
		m_tType(type)
	{
		switch (angle)
		{
			case ObjectAngles::EAST:
			case ObjectAngles::WEST:
				break;

			default:
				throw std::invalid_argument(fmt::format("[JunctionRailObject] Invalid angle: {}", magic_enum::enum_name(angle)));
		}
	}	

	void JunctionRailObject::Draw(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const
	{
		auto angle = this->GetAngle();
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
