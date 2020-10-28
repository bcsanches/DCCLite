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

#include <optional>

constexpr auto DEFAULT_ZOOM_LEVEL = 1;

namespace LitePanel
{
	class SimpleRailObject;
}

namespace LitePanel::Gui
{
	class TileEvent;
	class RenderTimer;

	wxDECLARE_EVENT(EVT_TILE_UNDER_MOUSE_CHANGED, TileEvent);
	wxDECLARE_EVENT(EVT_TILE_LEFT_CLICK, TileEvent);
	wxDECLARE_EVENT(EVT_TILE_RIGHT_CLICK, TileEvent);

	class TileEvent: public wxEvent
	{
		public:
			TileEvent(wxEventType commandType, std::optional<TileCoord_t> tilePos, int id = 0):
				wxEvent(id, commandType),
				m_tTilePosition(tilePos)
			{
				//empty
			}

			TileEvent(const TileEvent &other):
				wxEvent(other),
				m_tTilePosition(other.m_tTilePosition)
			{
				//empty
			}

			wxEvent *Clone() const
			{
				return new TileEvent(*this);
			}

			const std::optional<TileCoord_t> &GetTilePosition() const
			{
				return m_tTilePosition;
			}

		private:
			const std::optional<TileCoord_t> m_tTilePosition;
	};				

	struct ViewInfo
	{
		uint8_t m_uZoomLevel = DEFAULT_ZOOM_LEVEL;
		unsigned m_uTileScale;
		unsigned m_uHalfTileScale;
		unsigned m_uLineWidth;

		//size of the world in pixes, based on tileScale
		IntPoint_t m_tWorldSize;

		TileCoord_t WorldToTile(const IntPoint_t& worldPoint) const;
	};

	class TileMapCanvas: public OGLCanvas
	{
		private:
			struct RenderArgs;

		public:
			TileMapCanvas(wxWindow *parent, int id = -1);
			~TileMapCanvas() override;

			void SetTileMap(const LitePanel::TileMap *tileMap) noexcept;

			std::optional<TileCoord_t> TryFindMouseTile(const wxMouseEvent &event) const noexcept;

		protected:
			void OnDraw() override;

			void OnMouseWheel(wxMouseEvent &event);
			void OnMouseLeftDown(wxMouseEvent &event);
			void OnMouseMiddleDown(wxMouseEvent &event);
			void OnMouseMiddleUp(wxMouseEvent &event);
			void OnMouseMove(wxMouseEvent &event);

			void OnMouseCaptureLost(wxMouseCaptureLostEvent &event);

			void OnMouseLost();		

			void OnMouseLeaveWindow(wxMouseEvent &event);

		private:
			void DrawGrid(const RenderArgs &rargs);

			void UpdateRenderInfo();

			RenderArgs MakeRenderArgs() const;

			void Render(const RenderArgs &rargs);

			void DrawStraightRail(const LitePanel::SimpleRailObject &obj) const;
			void DrawCurveRail(const LitePanel::SimpleRailObject &obj) const;

			void DrawObject(const MapObject &obj) const;
			void DrawSimpleRail(const LitePanel::SimpleRailObject &obj) const;			

			void RequestDraw();

			void OnClose(wxCloseEvent& evt);

		private:
			

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

			std::unique_ptr<RenderTimer> m_upRenderTimer;

			ViewInfo m_tViewInfo;

			std::optional<TileCoord_t> m_tTileUnderMouse;
			bool m_fMapMouseScroll = false;

			bool m_fPendingDraw = false;

			friend class RenderTimer;
	};
}


