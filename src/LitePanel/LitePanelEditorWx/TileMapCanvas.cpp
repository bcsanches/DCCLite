// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include "GL_Renderer.h"
#include "TileMapCanvas.h"
#include "TempObjects.h"

#include <LitePanelLib/RailObject.h>

namespace LitePanel::Gui
{		
	wxDEFINE_EVENT(EVT_TILE_UNDER_MOUSE_CHANGED, TileEvent);
	wxDEFINE_EVENT(EVT_TILE_LEFT_CLICK, TileEvent);
	wxDEFINE_EVENT(EVT_TILE_RIGHT_CLICK, TileEvent);

	class RenderTimer : public wxTimer
	{
		private:	
			TileMapCanvas &m_rclPanel;

		public:
			RenderTimer(TileMapCanvas& panel):
				m_rclPanel(panel)
			{
				//empty
			}

			void Notify()
			{
				m_rclPanel.RequestDraw();
			}	
	};



	TileMapCanvas::TileMapCanvas(wxWindow *parent, int id):
		GL_Canvas{parent, id}	
	{		
		Bind(wxEVT_LEFT_DOWN, &TileMapCanvas::OnMouseLeftDown, this);
		Bind(wxEVT_MIDDLE_DOWN, &TileMapCanvas::OnMouseMiddleDown, this);
		Bind(wxEVT_MOUSEWHEEL, &TileMapCanvas::OnMouseWheel, this);
		Bind(wxEVT_MOTION, &TileMapCanvas::OnMouseMove, this);
		Bind(wxEVT_LEAVE_WINDOW, &TileMapCanvas::OnMouseLeaveWindow, this);

		Bind(wxEVT_CLOSE_WINDOW, &TileMapCanvas::OnClose, this);

		m_upRenderTimer = std::make_unique<RenderTimer>(*this);

		m_clTileMapRenderer.SetRenderFlags(LitePanel::Render::TileMapRendererFlags_CenterMap);
	}

	TileMapCanvas::~TileMapCanvas()
	{
		//empty - only to allow forward declaring RenderTimer on header file
	}

	void TileMapCanvas::SetTileMap(const LitePanel::TileMap *tileMap) noexcept
	{
		m_pclTileMap = tileMap;
		m_clTileMapRenderer.SetTileMap(m_pclTileMap);

		if (m_pclTileMap)
		{						
			m_upRenderTimer->Start(25);			
		}
		else
		{
			m_upRenderTimer->Stop();
		}

		this->RequestDraw();
	}

	void TileMapCanvas::RequestDraw()
	{
		if(m_fPendingDraw)
			return;

		m_fPendingDraw = true;
		this->Refresh();
	}

	void TileMapCanvas::OnDraw()
	{
		// Clear
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);	

		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);

		if (m_pclTileMap)
		{
			GL_Renderer renderer;

			// Setup the viewport
			glViewport(0, 0, GetSize().x, GetSize().y);

			// Setup the screen projection
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();			

			auto clientSize = this->GetClientSize();

			m_clTileMapRenderer.SetupFrame(renderer, LitePanel::FloatPoint_t{ (float)clientSize.GetWidth(), (float)clientSize.GetHeight() });

			glOrtho(
				0,
				static_cast<GLdouble>(GetSize().x + 0),
				static_cast<GLdouble>(GetSize().y + 0),
				0,
				-1,
				1
			);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			m_clTileMapRenderer.Draw(renderer);
		}

		this->SwapBuffers();

		m_fPendingDraw = false;
	}

	void TileMapCanvas::OnMouseWheel(wxMouseEvent &event)
	{	
		const FloatPoint_t mousePosition{static_cast<float>(event.GetX()), static_cast<float>(event.GetY())};

		if (event.GetWheelRotation() > 0)
		{
			m_clTileMapRenderer.ZoomIn(mousePosition);
		}
		else
		{
			m_clTileMapRenderer.ZoomOut(mousePosition);
		}

		this->RequestDraw();
	}

	void TileMapCanvas::OnMouseLeftDown(wxMouseEvent &event)
	{
		event.Skip();

		auto tileCoord = this->TryFindMouseTile(event);
		if(!tileCoord)
			return;

		TileEvent ev{ EVT_TILE_LEFT_CLICK, *tileCoord};

		wxPostEvent(this, ev);
	}

	void TileMapCanvas::OnMouseMiddleDown(wxMouseEvent &event)
	{
		event.Skip();
		
		this->Bind(wxEVT_MIDDLE_UP, &TileMapCanvas::OnMouseMiddleUp, this);
		this->Bind(wxEVT_MOUSE_CAPTURE_LOST, &TileMapCanvas::OnMouseCaptureLost, this);

		this->CaptureMouse();

		m_tMoveStartPos = LitePanel::IntPoint_t{event.GetX(), event.GetY()};	
		m_fMapMouseScroll = true;
	}

	void TileMapCanvas::OnMouseMove(wxMouseEvent &event)
	{
		event.Skip();

		if (m_fMapMouseScroll)
		{
			const auto currentPos = LitePanel::IntPoint_t{ event.GetX(), event.GetY() };
			const auto delta = currentPos - m_tMoveStartPos;

			m_clTileMapRenderer.Move(LitePanel::IntPoint2Float(delta));
			m_tMoveStartPos = currentPos;

			this->RequestDraw();
		}
		else
		{
			auto newTilePosition = this->TryFindMouseTile(event);

			if(newTilePosition == m_tTileUnderMouse)
				return;

			m_tTileUnderMouse = newTilePosition;

			TileEvent ev{ EVT_TILE_UNDER_MOUSE_CHANGED, m_tTileUnderMouse };

			wxPostEvent(this, ev);
		}	
	}

	void TileMapCanvas::OnMouseLeaveWindow(wxMouseEvent& event)
	{
		if (!m_pclTileMap)
			return;

		m_tTileUnderMouse = {};
		TileEvent ev{EVT_TILE_UNDER_MOUSE_CHANGED, m_tTileUnderMouse};

		wxPostEvent(this, ev);
	}

	void TileMapCanvas::OnMouseCaptureLost(wxMouseCaptureLostEvent &event)
	{
		event.Skip();

		this->OnMouseLost();		
	}

	void TileMapCanvas::OnMouseLost()
	{
		this->ReleaseMouse();
		
		m_fMapMouseScroll = false;
		this->Unbind(wxEVT_MIDDLE_UP, &TileMapCanvas::OnMouseMiddleUp, this);
		this->Unbind(wxEVT_MOUSE_CAPTURE_LOST, &TileMapCanvas::OnMouseCaptureLost, this);
	}


	void TileMapCanvas::OnMouseMiddleUp(wxMouseEvent &event)
	{
		event.Skip();
		m_fMapMouseScroll = false;

		this->OnMouseLost();
	}

	std::optional<TileCoord_t> TileMapCanvas::TryFindMouseTile(const wxMouseEvent &event) const noexcept
	{
		if(!m_pclTileMap)
			return {};

		const IntPoint_t mousePos{event.GetX(), event.GetY()};

		return m_clTileMapRenderer.GetTilePos(LitePanel::IntPoint2Float(mousePos));
	}

	void TileMapCanvas::OnClose(wxCloseEvent& evt)
	{
		evt.Skip();
		m_upRenderTimer->Stop();
	}
}
