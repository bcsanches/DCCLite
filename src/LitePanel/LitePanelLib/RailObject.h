// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include "MapObject.h"

namespace LitePanel
{	
	/**
	* By design, those are immutable
	
	*/
	class RailObject: public MapObject
	{
		public:
			RailObject(const TileCoord_t &position, ObjectAngles angle = ObjectAngles::EAST);
			RailObject(const rapidjson::Value& params);

			inline const ObjectAngles GetAngle() const
			{
				return m_tAngle;
			}

			const char* GetTypeName() const noexcept override
			{
				return TYPE_NAME;
			}			

			static constexpr auto TYPE_NAME = "RailObject";

		protected:
			void OnSave(JsonOutputStream_t& stream) const noexcept override;

		private:
			const ObjectAngles m_tAngle = ObjectAngles::EAST;
	};

	enum class SimpleRailTypes
	{
		STRAIGHT,
		CURVE_LEFT,
		CURVE_RIGHT,
		//CROSSING,
		TERMINAL
	};

	enum BlockSplitTypes
	{
		kBLOCK_SPLIT_NONE = 0,
		kBLOCK_SPLIT_LEFT = 0x02,
		kBLOCK_SPLIT_RIGHT = 0x04
	};

	class SimpleRailObject: public RailObject
	{
		public:
			SimpleRailObject(
				const TileCoord_t &position, 
				ObjectAngles angle, 
				const SimpleRailTypes type, 
				const BlockSplitTypes splitTypes = kBLOCK_SPLIT_NONE
			);

			SimpleRailObject(const rapidjson::Value &params);

			inline const SimpleRailTypes GetType() const noexcept
			{
				return m_tType;
			}

			const char* GetTypeName() const noexcept override
			{
				return TYPE_NAME;
			}

			inline uint8_t GetBlockSplit() const noexcept
			{
				return m_fBlockSplit;
			}

			void Draw(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const override;

			static constexpr auto TYPE_NAME = "SimpleRailObject";

		protected:
			void OnSave(JsonOutputStream_t& stream) const noexcept override;

		private:
			void DrawStraightRail(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const;
			void DrawCurveLeftRail(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const;
			void DrawCurveRightRail(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const;

		private:
			const SimpleRailTypes	m_tType;			
			const uint8_t			m_fBlockSplit;
	};

	enum class JunctionTypes
	{
		LEFT_TURNOUT,
		RIGHT_TURNOUT
	};

	class JunctionRailObject: public RailObject
	{
		public:
			JunctionRailObject(const TileCoord_t &position, ObjectAngles angle, const JunctionTypes type);

			const char* GetTypeName() const noexcept override
			{
				return TYPE_NAME;
			}

			static constexpr auto TYPE_NAME = "JunctionRailObject";

			void Draw(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const override;

		private:
			const JunctionTypes m_tType;
	};
}
