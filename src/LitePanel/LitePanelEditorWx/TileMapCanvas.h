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
#include "render/TileMapRenderer.h"

#include <optional>

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
				wxEvent(id, commandType), m_tTilePosition(tilePos) {}

			TileEvent(const TileEvent &other):
				wxEvent(other), m_tTilePosition(other.m_tTilePosition) {}

			wxEvent *Clone() const override { return new TileEvent(*this); }

			const std::optional<TileCoord_t> &GetTilePosition() const { return m_tTilePosition; }

		private:
			const std::optional<TileCoord_t> m_tTilePosition;
	};

	class TileMapCanvas: public OGLCanvas
	{
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
			void RequestDraw();
			void OnClose(wxCloseEvent& evt);

			LitePanel::Render::TileMapRenderer m_tRenderer;
			IntPoint_t m_tMoveStartPos;
			std::unique_ptr<RenderTimer> m_upRenderTimer;
			std::optional<TileCoord_t> m_tTileUnderMouse;
			bool m_fMapMouseScroll = false;
			bool m_fPendingDraw = false;

			friend class RenderTimer;
	};
}
