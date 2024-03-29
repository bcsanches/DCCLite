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

	struct ViewInfo
	{
		uint8_t		m_uZoomLevel = 1;
		unsigned	m_uTileSize;
		unsigned	m_uHalfTileSize;
		float		m_fpLineWidth;
		unsigned	m_uDiagonalLineWidth;
	};

	class TileMapView
	{
		public:			
			TileMapView(const TileMap *map = nullptr):
				m_pclTileMap{ map }
			{
				this->UpdateViewInfo();
			}

			void SetupFrame(IRenderer &renderer, const FloatPoint_t &clientSize);

			void Draw(IRenderer &renderer);

			inline void Move(FloatPoint_t delta) noexcept
			{
				m_ptOrigin += delta;				
			}

			inline void SetTileMap(const TileMap *map)
			{
				m_pclTileMap = map;

				m_ptOrigin = {};
				m_tViewHelper.m_stInfo.m_uZoomLevel = 1;
			}

		private:			
			RenderArgs MakeRenderArgs() const;

			void UpdateViewInfo();

			void ClipOrigin();

		private:			
			struct ViewHelper
			{
				ViewInfo m_stInfo;

				LitePanel::TileCoord_t WorldToTile(const FloatPoint_t &worldPoint) const;
				LitePanel::TileCoord_t WorldToTileCeil(const FloatPoint_t &worldPoint) const;
				LitePanel::TileCoord_t WorldToTileFloor(const FloatPoint_t &worldPoint) const;
			};

			const TileMap	*m_pclTileMap = nullptr;

			FloatPoint_t	m_ptClientSize;
			FloatPoint_t	m_ptOrigin;

			ViewHelper		m_tViewHelper;
	};
}
