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

#include "GLCanvas.h"

#include "Point.h"
#include "TileMap.h"

constexpr auto DEFAULT_TILE_SIZE = 16;

namespace LitePanel
{
	class MapCanvas: public OGLCanvas
	{
		private:
			struct RenderArgs;

		public:
			MapCanvas(wxWindow *parent, int id = -1);

			void SetTileMap(const LitePanel::TileMap *tileMap) noexcept;

			protected:
			void OnDraw() override;

			void OnMouseWheel(wxMouseEvent &event);
			void OnMouseMiddleDown(wxMouseEvent &event);
			void OnMouseMiddleUp(wxMouseEvent &event);
			void OnMouseMove(wxMouseEvent &event);

			void OnMouseCaptureLost(wxMouseCaptureLostEvent &event);

			void OnMouseLost();

		private:
			void DrawGrid(const RenderArgs &rargs);

			void UpdateRenderInfo();

			RenderArgs MakeRenderArgs() const;

		private:
			struct ViewInfo
			{
				unsigned m_uTileScale = DEFAULT_TILE_SIZE;

				//size of the world in pixes, based on tileScale
				IntPoint_t m_tWorldSize;

				TileCoord_t WorldToTile(const IntPoint_t &worldPoint) const;
			};

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

			IntPoint_t m_tOrigin;

			IntPoint_t m_tMoveStartPos;

			const TileMap *m_pclTileMap = nullptr;

			ViewInfo m_tViewInfo;
	};
}


