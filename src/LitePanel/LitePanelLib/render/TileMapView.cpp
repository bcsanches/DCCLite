// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "TileMapView.h"

#include <fmt/format.h>

#include "../TileMap.h"
#include "../LitePanelLibDefs.h"

#include "ColorStyle.h"
#include "IRenderer.h"

template <typename T>
inline T FloatToInt(float x)
{
	return x >= 0 ? ((T)(x + 0.5f)) : ((T)(x - 0.5f));
}

namespace LitePanel::Render
{
	struct ScaleInfo
	{
		uint8_t m_uTileSize;
		uint8_t m_uLineWidth;
		uint8_t m_uDiagonalLineWidth;
	};

	constexpr auto MAX_ZOOM_LEVELS = 5;

	constexpr ScaleInfo g_tScales[MAX_ZOOM_LEVELS] =
	{
		{8, 2, 3},
		{16, 4, 5},
		{32, 8, 16},
		{64, 16, 32},
		{128, 32, 44}
	};

	#define CURRENT_TILE_SIZE (g_tScales[m_uZoomLevel].m_uTileSize)

	TileCoord_t ViewInfo::WorldToTile(const FloatPoint_t &worldPoint) const
	{
		auto localCoord = worldPoint / CURRENT_TILE_SIZE;

		return TileCoord_t{ FloatToInt<TileCoord_t::Type_t>(localCoord.m_tX), FloatToInt<TileCoord_t::Type_t>(localCoord.m_tY) };
	}

	static FloatPoint_t TilePointToFloat(const TileCoord_t &tile) noexcept
	{
		return FloatPoint_t{ static_cast<float>(tile.m_tX), static_cast<float>(tile.m_tY) };
	}

	static FloatPoint_t IntPointToFloat(const IntPoint_t &tile) noexcept
	{
		return FloatPoint_t{ static_cast<float>(tile.m_tX), static_cast<float>(tile.m_tY) };
	}

	struct RenderArgs
	{
		//Visible screen rect size in pixels (how many pixels we will draw or client rect on GUI)
		IntPoint_t m_tViewClientSize;

		//View position in world space - upper left corner
		IntPoint_t m_tViewOrigin;

		//First visible tile on view (upper left corner) - can be out of bounds
		TileCoord_t m_tTilePos_ViewOrigin;

		//The last visible tile on view (right bottom corner) - can be out of bounds
		TileCoord_t m_tTilePos_LastVisible;

		//Number of visible tiles that can fit on screen, including partially visible tiles on screen borders
		//if any of this one is zero, no visible tiles
		TileCoord_t m_tNumVisibleTiles;
	};

	void TileMapView::UpdateViewInfo()
	{
		m_tViewInfo.m_uTileSize = g_tScales[m_tViewInfo.m_uZoomLevel].m_uTileSize;
		m_tViewInfo.m_uHalfTileSize = g_tScales[m_tViewInfo.m_uZoomLevel].m_uTileSize / 2;
		m_tViewInfo.m_uLineWidth = g_tScales[m_tViewInfo.m_uZoomLevel].m_uLineWidth;
		m_tViewInfo.m_uDiagonalLineWidth = g_tScales[m_tViewInfo.m_uZoomLevel].m_uDiagonalLineWidth;
	}

	void TileMapView::SetupFrame(IRenderer &renderer, const FloatPoint_t &clientSize)
	{
		m_ptClientSize = clientSize;
	}

	TileMapView::RenderArgs TileMapView::MakeRenderArgs() const
	{		
		RenderArgs rargs;

		rargs.m_tViewClientSize = m_ptClientSize;
		rargs.m_tViewOrigin = m_ptOrigin;

		//Not possible, due to bugs...
		if ((m_ptOrigin.m_tX < 0) || (m_ptOrigin.m_tY < 0))
		{
			return rargs;
		}

		const auto &tileMapSize = m_rclTileMap.GetSize();

		rargs.m_tTilePos_ViewOrigin = m_tViewInfo.WorldToTile(m_ptOrigin);
		if ((rargs.m_tTilePos_ViewOrigin.m_tX >= tileMapSize.m_tX) || (rargs.m_tTilePos_ViewOrigin.m_tY >= tileMapSize.m_tY))
			return rargs;

		//Get the client area last pixel (right bottom corner) position
		auto viewLimit = m_ptOrigin + rargs.m_tViewClientSize - FloatPoint_t{ 1, 1 };				

		//get the world size in pixels right bottom corner (less one pixel)
		auto worldBounds = (TilePointToFloat(tileMapSize) * static_cast<float>(m_tViewInfo.m_uTileSize)) - FloatPoint_t{ 1.0f, 1.0f };

		//Now if the viewLimit is outside the tile Map, clamp it to inside the tileMap
		worldBounds.m_tX = std::min(viewLimit.m_tX, worldBounds.m_tX);
		worldBounds.m_tY = std::min(viewLimit.m_tY, worldBounds.m_tY);

		rargs.m_tTilePos_LastVisible = m_tViewInfo.WorldToTile(worldBounds) + TileCoord_t{ 1, 1 };

		rargs.m_tNumVisibleTiles = rargs.m_tTilePos_LastVisible - rargs.m_tTilePos_ViewOrigin;

		return rargs;
	}

	void TileMapView::Draw(IRenderer &renderer)
	{		
		auto &colorStyle = GetCurrentColorStyle();

		auto rargs = this->MakeRenderArgs();

		for (int i = 0; i < rargs.m_tNumVisibleTiles.m_tX; ++i)
		{
			LitePanel::IntPoint_t tilePos{ rargs.m_tTilePos_ViewOrigin.m_tX + i, rargs.m_tTilePos_ViewOrigin.m_tY };			

			auto tileWorldPos = IntPointToFloat(tilePos * m_tViewInfo.m_uTileSize);

			renderer.DrawLine(
				tileWorldPos, 
				FloatPoint_t{ tileWorldPos.m_tX, static_cast<float>((rargs.m_tTilePos_LastVisible.m_tY - 1) * m_tViewInfo.m_uTileSize) },
				colorStyle.m_tGridLine
			);
		}		

		for (int i = 0; i < rargs.m_tNumVisibleTiles.m_tY; ++i)
		{
			LitePanel::IntPoint_t tilePos{ rargs.m_tTilePos_ViewOrigin.m_tX, rargs.m_tTilePos_ViewOrigin.m_tY + i};			

			auto tileWorldPos = IntPointToFloat(tilePos * m_tViewInfo.m_uTileSize);

			renderer.DrawLine(
				tileWorldPos, 
				FloatPoint_t{ static_cast<float>((rargs.m_tTilePos_LastVisible.m_tX - 1) * m_tViewInfo.m_uTileSize), tileWorldPos.m_tY}, 
				colorStyle.m_tGridLine
			);
		}	

#if 1
		for (int i = 0; i <= rargs.m_tNumVisibleTiles.m_tX; ++i)
		{
			for (int j = 0; j <= rargs.m_tNumVisibleTiles.m_tY; ++j)
			{				
				LitePanel::IntPoint_t tilePos{ rargs.m_tTilePos_ViewOrigin.m_tX + i, rargs.m_tTilePos_ViewOrigin.m_tY + j };

				if (!m_rclTileMap.IsInside(tilePos))
					break;				

				auto str = fmt::format("{},{}", tilePos.m_tX, tilePos.m_tY);

				auto tileWorldPos = IntPointToFloat(tilePos * m_tViewInfo.m_uTileSize) + FloatPoint_t{ static_cast<float>(m_tViewInfo.m_uHalfTileSize), static_cast<float>(m_tViewInfo.m_uHalfTileSize) };

				renderer.DrawText(8.0f, tileWorldPos, LP_COL32(255, 255, 255, 255), &str[0], &str[str.size()]);
			}
		}
#endif

		renderer.DrawLine(
			FloatPoint_t{ 0.0f, 0.0f }, 
			m_ptClientSize, 
			LP_COL32(255, 0, 255, 255)
		);
	}
}

