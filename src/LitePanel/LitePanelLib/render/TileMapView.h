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

#include "../LitePanelLibDefs.h"
#include "../Point.h"

namespace LitePanel
{
	class TileMap;
}

namespace LitePanel::Render
{	
	class IRenderer;

	constexpr auto DEFAULT_ZOOM_LEVEL = 1;

	struct ViewInfo
	{
		uint8_t m_uZoomLevel = DEFAULT_ZOOM_LEVEL;
		unsigned m_uTileSize;
		unsigned m_uHalfTileSize;
		unsigned m_uLineWidth;
		unsigned m_uDiagonalLineWidth;

		LitePanel::TileCoord_t WorldToTile(const FloatPoint_t &worldPoint) const;
	};

	class TileMapView
	{
		public:
			TileMapView(const TileMap &map) :
				m_rclTileMap{ map }
			{
				this->UpdateViewInfo();
			}

			void SetupFrame(IRenderer &renderer, const FloatPoint_t &clientSize);

			void Draw(IRenderer &renderer);

			inline void Move(FloatPoint_t delta) noexcept
			{
				m_ptOrigin += delta;				
			}

		private:
			struct RenderArgs
			{
				//Visible screen rect size in pixels (how many pixels we will draw or client rect on GUI)
				FloatPoint_t m_tViewClientSize;

				//View position in world space - upper left corner
				FloatPoint_t m_tViewOrigin;

				//First visible tile on view (upper left corner) - can be out of bounds
				TileCoord_t m_tTilePos_ViewOrigin;

				//The last visible tile on view (right bottom corner) - can be out of bounds
				TileCoord_t m_tTilePos_LastVisible;

				//Number of visible tiles that can fit on screen, including partially visible tiles on screen borders
				//if any of this one is zero, no visible tiles
				TileCoord_t m_tNumVisibleTiles;
			};

			RenderArgs MakeRenderArgs() const;

			void UpdateViewInfo();

			void ClipOrigin();

		private:			
			const TileMap	&m_rclTileMap;

			FloatPoint_t	m_ptClientSize;
			FloatPoint_t	m_ptOrigin;

			ViewInfo		m_tViewInfo;
	};
}
