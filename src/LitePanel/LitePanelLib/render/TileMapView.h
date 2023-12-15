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

	struct RenderArgs;

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
			RenderArgs MakeRenderArgs() const;

			void UpdateViewInfo();

			void ClipOrigin();

		private:			
			struct ViewInfo
			{
				uint8_t m_uZoomLevel = 1;
				unsigned m_uTileSize;
				unsigned m_uHalfTileSize;
				unsigned m_uLineWidth;
				unsigned m_uDiagonalLineWidth;

				LitePanel::TileCoord_t WorldToTile(const FloatPoint_t &worldPoint) const;
				LitePanel::TileCoord_t WorldToTileCeil(const FloatPoint_t &worldPoint) const;
				LitePanel::TileCoord_t WorldToTileFloor(const FloatPoint_t &worldPoint) const;
			};

			const TileMap	&m_rclTileMap;

			FloatPoint_t	m_ptClientSize;
			FloatPoint_t	m_ptOrigin;

			ViewInfo		m_tViewInfo;
	};
}
