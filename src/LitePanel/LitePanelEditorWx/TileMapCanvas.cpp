// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "TileMapCanvas.h"

#include <algorithm>

#include "ImGuiTileMapRenderer.h"
#include "imgui.h"

namespace LitePanel::Gui
{
	wxDEFINE_EVENT(EVT_TILE_UNDER_MOUSE_CHANGED, TileEvent);
	wxDEFINE_EVENT(EVT_TILE_LEFT_CLICK, TileEvent);
	wxDEFINE_EVENT(EVT_TILE_RIGHT_CLICK, TileEvent);

	class RenderTimer : public wxTimer
	{
		public:
			explicit RenderTimer(TileMapCanvas& canvas): m_rclCanvas(canvas) {}
			void Notify() override { m_rclCanvas.RequestDraw(); }
		private:
			TileMapCanvas &m_rclCanvas;
	};

	TileMapCanvas::TileMapCanvas(wxWindow *parent, int id):
		OGLCanvas{parent, id}
	{
		Bind(wxEVT_LEFT_DOWN, &TileMapCanvas::OnMouseLeftDown, this);
		Bind(wxEVT_MIDDLE_DOWN, &TileMapCanvas::OnMouseMiddleDown, this);
		Bind(wxEVT_MOUSEWHEEL, &TileMapCanvas::OnMouseWheel, this);
		Bind(wxEVT_MOTION, &TileMapCanvas::OnMouseMove, this);
		Bind(wxEVT_LEAVE_WINDOW, &TileMapCanvas::OnMouseLeaveWindow, this);
		Bind(wxEVT_CLOSE_WINDOW, &TileMapCanvas::OnClose, this);

		m_upRenderTimer = std::make_unique<RenderTimer>(*this);
	}

	TileMapCanvas::~TileMapCanvas() = default;

	void TileMapCanvas::SetTileMap(const LitePanel::TileMap *tileMap) noexcept
	{
		m_tRenderer.SetTileMap(tileMap);
		if (tileMap)
		{
			m_upRenderTimer->Start(25);
			RequestDraw();
		}
		else
		{
			m_upRenderTimer->Stop();
			RequestDraw();
		}
	}

	void TileMapCanvas::RequestDraw()
	{
		if (m_fPendingDraw)
			return;
		m_fPendingDraw = true;
		Refresh(false);
	}

	void TileMapCanvas::OnDraw()
	{
		const auto clientSize = GetClientSize();
		glViewport(0, 0, clientSize.x, clientSize.y);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		dcclite::PanelEditor::ImGuiTileMapRenderer renderer(
			*ImGui::GetBackgroundDrawList(),
			ImVec2(0.0f, 0.0f)
		);
		m_tRenderer.SetupFrame(
			renderer,
			LitePanel::FloatPoint_t{static_cast<float>(clientSize.x), static_cast<float>(clientSize.y)}
		);
		m_tRenderer.Draw(renderer);

		m_fPendingDraw = false;
	}

	void TileMapCanvas::OnMouseWheel(wxMouseEvent &event)
	{
		if (event.GetWheelRotation() > 0)
			m_tRenderer.ZoomIn();
		else
			m_tRenderer.ZoomOut();
		RequestDraw();
	}

	void TileMapCanvas::OnMouseLeftDown(wxMouseEvent &event)
	{
		event.Skip();
		auto tileCoord = TryFindMouseTile(event);
		if (!tileCoord)
			return;
		TileEvent ev{EVT_TILE_LEFT_CLICK, *tileCoord};
		wxPostEvent(this, ev);
	}

	void TileMapCanvas::OnMouseMiddleDown(wxMouseEvent &event)
	{
		event.Skip();
		Bind(wxEVT_MIDDLE_UP, &TileMapCanvas::OnMouseMiddleUp, this);
		Bind(wxEVT_MOUSE_CAPTURE_LOST, &TileMapCanvas::OnMouseCaptureLost, this);
		CaptureMouse();
		m_tMoveStartPos = LitePanel::IntPoint_t{event.GetX(), event.GetY()};
		m_fMapMouseScroll = true;
	}

	void TileMapCanvas::OnMouseMove(wxMouseEvent &event)
	{
		event.Skip();
		if (m_fMapMouseScroll)
		{
			auto currentPos = LitePanel::IntPoint_t{event.GetX(), event.GetY()};
			m_tRenderer.Move(LitePanel::FloatPoint_t{
				static_cast<float>(currentPos.m_tX - m_tMoveStartPos.m_tX),
				static_cast<float>(currentPos.m_tY - m_tMoveStartPos.m_tY)
			});
			m_tMoveStartPos = currentPos;
			RequestDraw();
			return;
		}

		auto newTilePosition = TryFindMouseTile(event);
		if (newTilePosition == m_tTileUnderMouse)
			return;
		m_tTileUnderMouse = newTilePosition;
		TileEvent ev{EVT_TILE_UNDER_MOUSE_CHANGED, m_tTileUnderMouse};
		wxPostEvent(this, ev);
	}

	void TileMapCanvas::OnMouseLeaveWindow(wxMouseEvent& event)
	{
		m_tTileUnderMouse = {};
		TileEvent ev{EVT_TILE_UNDER_MOUSE_CHANGED, m_tTileUnderMouse};
		wxPostEvent(this, ev);
	}

	void TileMapCanvas::OnMouseCaptureLost(wxMouseCaptureLostEvent &event)
	{
		event.Skip();
		OnMouseLost();
	}

	void TileMapCanvas::OnMouseLost()
	{
		if (HasCapture())
			ReleaseMouse();
		m_fMapMouseScroll = false;
		Unbind(wxEVT_MIDDLE_UP, &TileMapCanvas::OnMouseMiddleUp, this);
		Unbind(wxEVT_MOUSE_CAPTURE_LOST, &TileMapCanvas::OnMouseCaptureLost, this);
	}

	void TileMapCanvas::OnMouseMiddleUp(wxMouseEvent &event)
	{
		event.Skip();
		OnMouseLost();
	}

	std::optional<TileCoord_t> TileMapCanvas::TryFindMouseTile(const wxMouseEvent &event) const noexcept
	{
		return m_tRenderer.GetTilePos(LitePanel::FloatPoint_t{
			static_cast<float>(event.GetX()),
			static_cast<float>(event.GetY())
		});
	}

	void TileMapCanvas::OnClose(wxCloseEvent& evt)
	{
		evt.Skip();
		m_upRenderTimer->Stop();
	}
}
