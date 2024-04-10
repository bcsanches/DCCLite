// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "TileMapRenderer.h"

#include <cmath>

#include <fmt/format.h>

#include "../TileMap.h"
#include "../LitePanelLibDefs.h"

#include "ColorStyle.h"
#include "IRenderer.h"

template <typename T>
inline T Round(float x)
{
	return x >= 0 ? ((T)(x + 0.5f)) : ((T)(x - 0.5f));
}

namespace LitePanel::Render
{
	struct ScaleInfo
	{
		uint8_t m_uTileSize;
		float	m_fpLineWidth;
		uint8_t m_uDiagonalLineWidth;
	};

	constexpr auto MAX_ZOOM_LEVELS = 5;

	constexpr ScaleInfo g_tScales[MAX_ZOOM_LEVELS] =
	{
		{10, 2, 3},
		{20, 4, 5},
		{40, 8, 16},
		{80, 16, 32},
		{160, 32, 44}
	};

	struct RenderArgs
	{
		//Visible screen rect size in pixels (how many pixels we will draw or client rect on GUI)
		FloatPoint_t m_tViewClientSize;

		//View position in world space - upper left corner
		FloatPoint_t m_tViewOrigin;

		//First visible tile on view (upper left corner)
		TileCoord_t m_tTilePos_FirstOrigin;

		//The last visible tile on view (right bottom corner)
		TileCoord_t m_tTilePos_LastVisible;

		//Number of visible tiles that can fit on screen, including partially visible tiles on screen borders
		//if any of this one is zero, no visible tiles
		TileCoord_t m_tNumVisibleTiles;
	};

	#define CURRENT_TILE_SIZE (g_tScales[m_stInfo.m_uZoomLevel].m_uTileSize)

	TileCoord_t TileMapRenderer::ViewHelper::WorldToTile(const FloatPoint_t &worldPoint) const
	{
		auto localCoord = worldPoint / CURRENT_TILE_SIZE;

		return TileCoord_t{ Round<TileCoord_t::Type_t>(localCoord.m_tX), Round<TileCoord_t::Type_t>(localCoord.m_tY) };
	}

	TileCoord_t TileMapRenderer::ViewHelper::WorldToTileCeil(const FloatPoint_t &worldPoint) const
	{
		auto localCoord = worldPoint / CURRENT_TILE_SIZE;

		return TileCoord_t{ static_cast<TileCoord_t::Type_t>(std::ceilf(localCoord.m_tX)), static_cast<TileCoord_t::Type_t>(std::ceilf(localCoord.m_tY)) };
	}

	TileCoord_t TileMapRenderer::ViewHelper::WorldToTileFloor(const FloatPoint_t &worldPoint) const
	{
		auto localCoord = worldPoint / CURRENT_TILE_SIZE;

		return TileCoord_t{ static_cast<TileCoord_t::Type_t>(std::floorf(localCoord.m_tX)), static_cast<TileCoord_t::Type_t>(std::floorf(localCoord.m_tY)) };
	}

	inline FloatPoint_t TilePointToFloat(const TileCoord_t &tile) noexcept
	{
		return FloatPoint_t{ static_cast<float>(tile.m_tX), static_cast<float>(tile.m_tY) };
	}

	inline FloatPoint_t IntPointToFloat(const IntPoint_t &tile) noexcept
	{
		return FloatPoint_t{ static_cast<float>(tile.m_tX), static_cast<float>(tile.m_tY) };
	}

	void TileMapRenderer::UpdateViewInfo()
	{
		m_tViewHelper.m_stInfo.m_uTileSize = g_tScales[m_tViewHelper.m_stInfo.m_uZoomLevel].m_uTileSize;
		m_tViewHelper.m_stInfo.m_uHalfTileSize = g_tScales[m_tViewHelper.m_stInfo.m_uZoomLevel].m_uTileSize / 2;
		m_tViewHelper.m_stInfo.m_fpLineWidth = g_tScales[m_tViewHelper.m_stInfo.m_uZoomLevel].m_fpLineWidth;
		m_tViewHelper.m_stInfo.m_uDiagonalLineWidth = g_tScales[m_tViewHelper.m_stInfo.m_uZoomLevel].m_uDiagonalLineWidth;
	}

	void TileMapRenderer::SetupFrame(IRenderer &renderer, const FloatPoint_t &clientSize)
	{
		m_ptClientSize = clientSize;

		if (!m_pclTileMap)
			return;

		this->ClipOrigin();
	}

	void TileMapRenderer::ClipOrigin()
	{
		assert(m_pclTileMap);

		m_ptOrigin.m_tX = std::max(0.0f, m_ptOrigin.m_tX);
		m_ptOrigin.m_tY = std::max(0.0f, m_ptOrigin.m_tY);

		const auto &tileMapSize = m_pclTileMap->GetSize();
		FloatPoint_t mapSizeInPixels = TilePointToFloat(tileMapSize) * static_cast<float>(m_tViewHelper.m_stInfo.m_uTileSize);
		
		if (m_ptClientSize.m_tX > mapSizeInPixels.m_tX)
		{
			// do not move if map is smaller than client size
			m_ptOrigin.m_tX = 0.0f;
		}
		else if ((m_ptOrigin.m_tX + m_ptClientSize.m_tX) > mapSizeInPixels.m_tX)
		{
			//map is bigger than client size, but origin is going beyond, so clip it...
			m_ptOrigin.m_tX = mapSizeInPixels.m_tX - m_ptClientSize.m_tX;
		}

		if (m_ptClientSize.m_tY > mapSizeInPixels.m_tY)
		{
			m_ptOrigin.m_tY = 0.0f;
		}
		else if ((m_ptOrigin.m_tY + m_ptClientSize.m_tY) > mapSizeInPixels.m_tY)
		{
			//map is bigger than client size, but origin is going beyond, so clip it...
			m_ptOrigin.m_tY = mapSizeInPixels.m_tY - m_ptClientSize.m_tY;
		}

	}

	RenderArgs TileMapRenderer::MakeRenderArgs() const
	{		
		assert(m_pclTileMap);

		RenderArgs rargs;		

		rargs.m_tViewClientSize = m_ptClientSize;
		rargs.m_tViewOrigin = m_ptOrigin;

		const auto &tileMapSize = m_pclTileMap->GetSize();

		rargs.m_tTilePos_FirstOrigin = m_tViewHelper.WorldToTileFloor(m_ptOrigin);				

		FloatPoint_t fvisibleTiles = m_ptClientSize / static_cast<float>(m_tViewHelper.m_stInfo.m_uTileSize);

		rargs.m_tNumVisibleTiles.m_tX = static_cast<TileCoord_t::Type_t>(roundf(fvisibleTiles.m_tX));
		rargs.m_tNumVisibleTiles.m_tY = static_cast<TileCoord_t::Type_t>(roundf(fvisibleTiles.m_tY));

		rargs.m_tTilePos_LastVisible.m_tX = std::min(
			static_cast<TileCoord_t::Type_t>(rargs.m_tNumVisibleTiles.m_tX + rargs.m_tTilePos_FirstOrigin.m_tX), 
			static_cast<TileCoord_t::Type_t>(tileMapSize.m_tX - 1)
		);

		rargs.m_tTilePos_LastVisible.m_tY = std::min(
			static_cast<TileCoord_t::Type_t>(rargs.m_tNumVisibleTiles.m_tY + rargs.m_tTilePos_FirstOrigin.m_tY), 
			static_cast<TileCoord_t::Type_t>(tileMapSize.m_tY - 1)
		);

		rargs.m_tNumVisibleTiles = (rargs.m_tTilePos_LastVisible - rargs.m_tTilePos_FirstOrigin) + TileCoord_t{ 1, 1 };
#if 0
		
		if ((rargs.m_tTilePos_ViewOrigin.m_tX >= tileMapSize.m_tX) || (rargs.m_tTilePos_ViewOrigin.m_tY >= tileMapSize.m_tY))
			return rargs;

		//Get the client area last pixel (right bottom corner) position
		auto viewLimit = m_ptOrigin + rargs.m_tViewClientSize - FloatPoint_t{ 1, 1 };				

		//get the world size in pixels right bottom corner (less one pixel)
		auto worldBounds = (TilePointToFloat(tileMapSize) * static_cast<float>(m_tViewInfo.m_uTileSize)) - FloatPoint_t{ 1.0f, 1.0f };

		//Now if the viewLimit is outside the tile Map, clamp it to inside the tileMap
		worldBounds.m_tX = std::min(viewLimit.m_tX, worldBounds.m_tX);
		worldBounds.m_tY = std::min(viewLimit.m_tY, worldBounds.m_tY);

		rargs.m_tTilePos_LastVisible = m_tViewInfo.WorldToTile(worldBounds) + TileCoord_t{ 2, 2 };

		//clamp it to map limit
		rargs.m_tTilePos_LastVisible.m_tX = std::min(rargs.m_tTilePos_LastVisible.m_tX, tileMapSize.m_tX);
		rargs.m_tTilePos_LastVisible.m_tY = std::min(rargs.m_tTilePos_LastVisible.m_tY, tileMapSize.m_tY);

		rargs.m_tNumVisibleTiles = rargs.m_tTilePos_LastVisible - rargs.m_tTilePos_ViewOrigin;

		//clamp it too
		rargs.m_tNumVisibleTiles.m_tX = std::min(rargs.m_tNumVisibleTiles.m_tX, tileMapSize.m_tX);
		rargs.m_tNumVisibleTiles.m_tY = std::min(rargs.m_tNumVisibleTiles.m_tY, tileMapSize.m_tY);

#endif

		return rargs;
	}

	void TileMapRenderer::Draw(IRenderer &renderer)
	{		
		if (!m_pclTileMap)
			return;

		auto &colorStyle = GetCurrentColorStyle();

		auto rargs = this->MakeRenderArgs();	

		const FloatPoint_t tileSize{ static_cast<float>(m_tViewHelper.m_stInfo.m_uTileSize), static_cast<float>(m_tViewHelper.m_stInfo.m_uTileSize) };

		for (int layerIndex = 0, numLayers = m_pclTileMap->GetNumLayers(); layerIndex < numLayers; ++layerIndex)
		{
			const auto &layer = m_pclTileMap->GetLayers()[layerIndex];

			for (int x = rargs.m_tTilePos_FirstOrigin.m_tX; x <= rargs.m_tTilePos_LastVisible.m_tX; ++x)
			{
				for (int y = rargs.m_tTilePos_FirstOrigin.m_tY; y <= rargs.m_tTilePos_LastVisible.m_tY; ++y)
				{
					TileCoord_t tilePos{ static_cast<uint16_t>(x), static_cast<uint16_t>(y) };
					auto *obj = layer.TryGetMapObject(tilePos);

					if (!obj)
						continue;

					auto tileWorldOrigin = TilePointToFloat(tilePos) * static_cast<float>(m_tViewHelper.m_stInfo.m_uTileSize);

					renderer.PushClipRect(tileWorldOrigin, tileWorldOrigin + tileSize);

					obj->Draw(renderer, m_tViewHelper.m_stInfo, tileWorldOrigin);

					renderer.PopClipRect();
				}
			}
		}


		for (int i = rargs.m_tTilePos_FirstOrigin.m_tX; i <= rargs.m_tTilePos_LastVisible.m_tX + 1; ++i)
		{
			//LitePanel::IntPoint_t tilePos{ rargs.m_tTilePos_ViewOrigin.m_tX + i, rargs.m_tTilePos_ViewOrigin.m_tY };			
			LitePanel::IntPoint_t tilePos{ i, rargs.m_tTilePos_FirstOrigin.m_tY };

			auto tileWorldPos = IntPointToFloat(tilePos * m_tViewHelper.m_stInfo.m_uTileSize);

			renderer.DrawLine(
				tileWorldPos - rargs.m_tViewOrigin,
				FloatPoint_t{ tileWorldPos.m_tX - rargs.m_tViewOrigin.m_tX, static_cast<float>((rargs.m_tNumVisibleTiles.m_tY) * m_tViewHelper.m_stInfo.m_uTileSize) },
				colorStyle.m_tGridLine
			);
		}		

		for (int i = rargs.m_tTilePos_FirstOrigin.m_tY; i <= rargs.m_tTilePos_LastVisible.m_tY + 1; ++i)
		{
			//LitePanel::IntPoint_t tilePos{ rargs.m_tTilePos_ViewOrigin.m_tX, rargs.m_tTilePos_ViewOrigin.m_tY + i};			
			LitePanel::IntPoint_t tilePos{ rargs.m_tTilePos_FirstOrigin.m_tX, i };

			auto tileWorldPos = IntPointToFloat(tilePos * m_tViewHelper.m_stInfo.m_uTileSize);

			renderer.DrawLine(
				tileWorldPos - rargs.m_tViewOrigin,				
				FloatPoint_t{ static_cast<float>((rargs.m_tNumVisibleTiles.m_tX) * m_tViewHelper.m_stInfo.m_uTileSize), tileWorldPos.m_tY - rargs.m_tViewOrigin.m_tY },
				colorStyle.m_tGridLine
			);
		}

		

#if 0
		for (int i = rargs.m_tTilePos_FirstOrigin.m_tX; i <= rargs.m_tTilePos_LastVisible.m_tX + 1; ++i)
		{
			for (int j = rargs.m_tTilePos_FirstOrigin.m_tY; j <= rargs.m_tTilePos_LastVisible.m_tY + 1; ++j)
			{				
				LitePanel::IntPoint_t tilePos{ i, j };

				if (!m_rclTileMap.IsInside(tilePos))
					break;				
				
				auto str = fmt::format("{},{}", tilePos.m_tX, tilePos.m_tY);

				auto tileWorldPos = IntPointToFloat(tilePos * m_tViewInfo.m_uTileSize) + FloatPoint_t{ static_cast<float>(m_tViewInfo.m_uHalfTileSize), static_cast<float>(m_tViewInfo.m_uHalfTileSize) };

				renderer.DrawText(6.0f, tileWorldPos, LP_COL32(255, 255, 255, 255), &str[0], &str[str.size()]);
			}
		}
#endif
	}
}

