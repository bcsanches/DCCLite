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

#include <optional>

#include "BlockIndex.h"
#include "MapObject.h"
#include "RailLink.h"

namespace LitePanel
{	
	/**
	* By design, those are ALMOST immutable (block ref can change, but object geometry can never change
	
	*/
	class RailObject: public MapObject
	{
		public:
			RailObject(const TileCoord_t &position, const RailLink link);
			RailObject(const rapidjson::Value& params);

			const char* GetTypeName() const noexcept override
			{
				return TYPE_NAME;
			}

			inline BlockIndex GetBlockIndex() const noexcept
			{
				return m_tBlockIndex;
			}

			inline void SetBlockIndex(BlockIndex blockIndex) noexcept
			{
				m_tBlockIndex = blockIndex;
			}

			static constexpr auto TYPE_NAME = "RailObject";

		protected:
			void OnSave(JsonOutputStream_t& stream) const noexcept override;

		protected:
			const RailLink	m_tLink;

		private:
			BlockIndex		m_tBlockIndex;
	};

	enum class SimpleRailTypes
	{
		STRAIGHT,
		CURVE_LEFT,
		CURVE_RIGHT,
		//CROSSING,
		TERMINAL
	};

	enum class BlockSplitTypes
	{
		NONE = 0,
		LEFT,
		RIGHT,
		TOP,
		BOTTOM
	};

	using NeighborTiles_t = std::pair<std::optional<TileCoord_t>, std::optional<TileCoord_t>>;
	using NeighborPositions_t = std::pair<IntPoint_t, IntPoint_t>;

	class SimpleRailObject: public RailObject
	{
		public:
			SimpleRailObject(
				const TileCoord_t &position, 
				const RailLink link,
				const SimpleRailTypes type, 
				const BlockSplitTypes splitTypes = BlockSplitTypes::NONE
			);

			SimpleRailObject(const rapidjson::Value &params);

			inline const SimpleRailTypes GetType() const noexcept
			{
				return m_kType;
			}

			const char* GetTypeName() const noexcept override
			{
				return TYPE_NAME;
			}

			inline BlockSplitTypes GetBlockSplit() const noexcept
			{
				return m_kBlockSplit;
			}

			std::optional<TileCoord_t> GetLeftNeighbor(const TileMapSize_t &mapSize) const;
			std::optional<TileCoord_t> GetRightNeighbor(const TileMapSize_t &mapSize) const;

			NeighborTiles_t GetNeighborTiles(const TileMapSize_t &mapSize) const;

			void Draw(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const override;

			static constexpr auto TYPE_NAME = "SimpleRailObject";

		protected:
			void OnSave(JsonOutputStream_t& stream) const noexcept override;

		private:
			void DrawStraightRail(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const;
			void DrawCurveRail(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const;			

			void DrawVerticalBlockSplit(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const;
			void DrawHorizontalBlockSplit(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const;

			NeighborPositions_t GetNeighborPositions() const;

		private:
			const SimpleRailTypes	m_kType;			
			const BlockSplitTypes   m_kBlockSplit;
	};

	enum class JunctionTypes
	{
		LEFT_TURNOUT,
		RIGHT_TURNOUT
	};

	class JunctionRailObject: public RailObject
	{
		public:
			JunctionRailObject(const TileCoord_t &position, const RailLink link, const JunctionTypes type);

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
