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
#include <magic_enum.hpp>

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
		m_tType(type),
		m_fBlockSplit(splitTypes)
	{
		//empty
	}

	SimpleRailObject::SimpleRailObject(const rapidjson::Value &params):
		RailObject(params),		
		m_tType{params["type"].GetInt()},
		m_fBlockSplit{static_cast<uint8_t>(params["blockSplit"].GetInt())}
	{
		//empty
	}

	void SimpleRailObject::OnSave(JsonOutputStream_t& stream) const noexcept
	{
		RailObject::OnSave(stream);
		
		stream.AddIntValue("type", static_cast<int>(m_tType));
		stream.AddIntValue("blockSplit", m_fBlockSplit);
	}	

	static void DrawHalfLine(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin, const ObjectAngles angle)
	{
		const auto index = static_cast<unsigned>(angle);

		//do some padding to hide "holes"
		static const FloatPoint_t startPoints[8] =
		{
			FloatPoint_t{0,		0.5},		//EAST
			FloatPoint_t{0.5f,	0.5f},		//NORTHEAST
			FloatPoint_t{0.5,	0},			//NORTH
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
			FloatPoint_t{0.5f,		0.5f},	//NORTH
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

	void SimpleRailObject::DrawStraightRail(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const
	{
		DrawLine(renderer, viewInfo, tileOrigin, this->GetAngle());
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
				break;

			case ObjectAngles::SOUTH:
				DrawHalfLine(renderer, viewInfo, tileOrigin, ObjectAngles::SOUTHEAST);
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
				break;

			case ObjectAngles::SOUTH:
				DrawHalfLine(renderer, viewInfo, tileOrigin, ObjectAngles::SOUTHWEST);
				break;
		}
	}

	void SimpleRailObject::Draw(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const
	{		
		switch (m_tType)
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
