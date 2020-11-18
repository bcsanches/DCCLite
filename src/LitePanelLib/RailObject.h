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

			static constexpr char* TYPE_NAME = "RailObject";

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

	class SimpleRailObject: public RailObject
	{
		public:
			SimpleRailObject(const TileCoord_t &position, ObjectAngles angle, const SimpleRailTypes type);
			SimpleRailObject(const rapidjson::Value &params);

			inline const SimpleRailTypes GetType() const noexcept
			{
				return m_tType;
			}

			const char* GetTypeName() const noexcept override
			{
				return TYPE_NAME;
			}

			static constexpr char* TYPE_NAME = "SimpleRailObject";

		protected:
			void OnSave(JsonOutputStream_t& stream) const noexcept override;

		private:
			const SimpleRailTypes m_tType;
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

			static constexpr char *TYPE_NAME = "JunctionRailObject";

		private:
			const JunctionTypes m_tType;
	};
}
