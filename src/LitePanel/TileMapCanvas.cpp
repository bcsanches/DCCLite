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
#include "TempObjects.h"
#include "RailObject.h"

struct ScaleInfo
{
	uint8_t m_uScale;
	uint8_t m_uLineWidth;
};

constexpr auto MAX_ZOOM_LEVELS = 5;

constexpr ScaleInfo g_tScales[MAX_ZOOM_LEVELS] = 
{
	{8, 1},
	{16, 2},
	{32, 4},
	{64, 8},
	{128, 16}
};

#define CURRENT_SCALE (g_tScales[m_uZoomLevel].m_uScale)

namespace LitePanel::Gui
{		
	wxDEFINE_EVENT(EVT_TILE_UNDER_MOUSE_CHANGED, TileEvent);
	wxDEFINE_EVENT(EVT_TILE_LEFT_CLICK, TileEvent);
	wxDEFINE_EVENT(EVT_TILE_RIGHT_CLICK, TileEvent);

	TileCoord_t ViewInfo::WorldToTile(const IntPoint_t &worldPoint) const
	{
		auto localCoord = worldPoint / CURRENT_SCALE;

		return TileCoord_t{ static_cast<TileCoord_t::Type_t>(localCoord.m_tX), static_cast<TileCoord_t::Type_t>(localCoord.m_tY)};
	}

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

	TileMapCanvas::~TileMapCanvas()
	{
		//empty - only to allow forward declaring RenderTimer on header file
	}

	void TileMapCanvas::SetTileMap(const LitePanel::TileMap *tileMap) noexcept
	{
		m_pclTileMap = tileMap;

		if (m_pclTileMap)
		{
			m_upRenderTimer->Start(25);
		}
		else
		{
			m_upRenderTimer->Stop();
		}
	
		this->UpdateRenderInfo();
	}

	void TileMapCanvas::UpdateRenderInfo()
	{
		assert(m_pclTileMap);

		m_tViewInfo.m_uTileScale = g_tScales[m_tViewInfo.m_uZoomLevel].m_uScale;
		m_tViewInfo.m_uHalfTileScale = g_tScales[m_tViewInfo.m_uZoomLevel].m_uScale / 2;
		m_tViewInfo.m_uLineWidth = g_tScales[m_tViewInfo.m_uZoomLevel].m_uLineWidth;
		m_tViewInfo.m_tWorldSize = LitePanel::IntPoint_t{m_pclTileMap->GetSize()} * m_tViewInfo.m_uTileScale;

		this->RequestDraw();
	}

	void TileMapCanvas::RequestDraw()
	{
		if(m_fPendingDraw)
			return;

		m_fPendingDraw = true;
		this->Refresh();
	}

	TileMapCanvas::RenderArgs TileMapCanvas::MakeRenderArgs() const
	{
		assert(m_pclTileMap);

		RenderArgs rargs;

		rargs.m_tViewClientSize = LitePanel::IntPoint_t{ this->GetSize().x, this->GetSize().y };	
		rargs.m_tViewOrigin = m_tOrigin;					

		//Not possible, due to bugs...
		if((m_tOrigin.m_tX < 0) || (m_tOrigin.m_tY < 0))
		{
			return rargs;
		}

		const auto tileMapSize = m_pclTileMap->GetSize();

		rargs.m_tTilePos_ViewOrigin = m_tViewInfo.WorldToTile(m_tOrigin);
		if((rargs.m_tTilePos_ViewOrigin.m_tX >= tileMapSize.m_tX) || (rargs.m_tTilePos_ViewOrigin.m_tY >= tileMapSize.m_tY))
			return rargs;

		//Get the client area last pixel (right bottom corner) position
		IntPoint_t viewLimit = m_tOrigin + rargs.m_tViewClientSize - IntPoint_t{ 1, 1 };

		//get the world size in pixels right bottom corner (less one pixel)
		IntPoint_t worldBounds = (IntPoint_t{ m_pclTileMap->GetSize() } * m_tViewInfo.m_uTileScale) - IntPoint_t{1, 1};		

		//Now if the viewLimit is outside the tile Map, clamp it to inside the tileMap
		worldBounds.m_tX = std::min(viewLimit.m_tX, worldBounds.m_tX);
		worldBounds.m_tY = std::min(viewLimit.m_tY, worldBounds.m_tY);

		rargs.m_tTilePos_LastVisible = m_tViewInfo.WorldToTile(worldBounds) + TileCoord_t{ 1, 1 };

		rargs.m_tNumVisibleTiles = rargs.m_tTilePos_LastVisible - rargs.m_tTilePos_ViewOrigin ;		
	
		return rargs;
	}

	void TileMapCanvas::DrawGrid(const RenderArgs &rargs)
	{
		assert(m_pclTileMap);

		glDisable(GL_LINE_SMOOTH);
		glLineWidth(1.0f);

		glColor4f(0.85f, 0.85f, 0.85f, 1.0f);

		glBegin(GL_LINES);

		//glPushMatrix();	
		//glTranslatef(-rargs.m_tViewOrigin.m_tX, -rargs.m_tViewOrigin.m_tY, 0);

		auto lastVisibleTileCorner = IntPoint_t{rargs.m_tNumVisibleTiles} * m_tViewInfo.m_uTileScale;
	
		for (int i = 0; i <= rargs.m_tNumVisibleTiles.m_tX; ++i)
		{
			LitePanel::IntPoint_t tilePos{ rargs.m_tTilePos_ViewOrigin.m_tX + i, rargs.m_tTilePos_ViewOrigin.m_tY };

			if (tilePos.m_tX > m_pclTileMap->GetSize().m_tX)
				break;

			auto tileWorldPos = tilePos * m_tViewInfo.m_uTileScale;

			glVertex2i(tileWorldPos.m_tX, tileWorldPos.m_tY);
			glVertex2i(tileWorldPos.m_tX, (rargs.m_tTilePos_LastVisible.m_tY) * m_tViewInfo.m_uTileScale);
		}

		for (int i = 0; i <= rargs.m_tNumVisibleTiles.m_tY; ++i)
		{
			LitePanel::IntPoint_t tilePos{ rargs.m_tTilePos_ViewOrigin.m_tX, rargs.m_tTilePos_ViewOrigin.m_tY + i };

			if (tilePos.m_tY > m_pclTileMap->GetSize().m_tY)
				break;

			auto tileWorldPos = tilePos * m_tViewInfo.m_uTileScale;
	
			glVertex2i(tileWorldPos.m_tX, tileWorldPos.m_tY);
			glVertex2i((rargs.m_tTilePos_LastVisible.m_tX) * m_tViewInfo.m_uTileScale, tileWorldPos.m_tY);
		}	

		glEnd();

		//glPopMatrix();
	}

	void TileMapCanvas::DrawStraightRail(const LitePanel::SimpleRailObject &rail) const
	{
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		glLineWidth(m_tViewInfo.m_uLineWidth);

		switch(rail.GetAngle())
		{
			case LitePanel::ObjectAngles::EAST:
			case LitePanel::ObjectAngles::NORTH:
			case LitePanel::ObjectAngles::SOUTH:
			case LitePanel::ObjectAngles::WEST:
				{
					glRotatef(static_cast<GLfloat>(rail.GetAngle()), 0, 0, 1);					

					glBegin(GL_LINES);

					glVertex2f(-static_cast<GLfloat>(m_tViewInfo.m_uHalfTileScale), 0);
					glVertex2f(m_tViewInfo.m_uHalfTileScale, 0);

					glEnd();
				}
				break;

			default:
				{
					glLineWidth(m_tViewInfo.m_uLineWidth * 2);
					if ((rail.GetAngle() == LitePanel::ObjectAngles::NORTHEAST) || (rail.GetAngle() == LitePanel::ObjectAngles::SOUTHWEST))
						glRotatef(90, 0, 0, 1);

					glBegin(GL_LINES);

					glVertex2f(-static_cast<GLfloat>(m_tViewInfo.m_uHalfTileScale), -static_cast<GLfloat>(m_tViewInfo.m_uHalfTileScale));
					glVertex2f(m_tViewInfo.m_uHalfTileScale, m_tViewInfo.m_uHalfTileScale);					

					glEnd();
				}
		}		
	}

	void TileMapCanvas::DrawCurveRail(const LitePanel::SimpleRailObject &obj) const
	{

	}

	void TileMapCanvas::DrawSimpleRail(const LitePanel::SimpleRailObject &rail) const
	{
		switch (rail.GetType())
		{
			case SimpleRailTypes::STRAIGHT:
			case SimpleRailTypes::TERMINAL:
				this->DrawStraightRail(rail);
				break;

			case SimpleRailTypes::CURVE_LEFT:
			case SimpleRailTypes::CURVE_RIGHT:
				this->DrawCurveRail(rail);
				break;		

			default:
				throw std::exception("[MapCanvas::DrawSimpleRail] Invalid rail type");
		}		
	}

	void TileMapCanvas::DrawObject(const MapObject &obj) const
	{
		auto simpleRailObject = dynamic_cast<const LitePanel::SimpleRailObject *>(&obj);
		if (simpleRailObject)
		{
			this->DrawSimpleRail(*simpleRailObject);			
		}		
		else if (auto tempObject = dynamic_cast<const LitePanel::TempObject*>(&obj))
		{
			tempObject->Draw(m_tViewInfo);
		}
	}

	void TileMapCanvas::Render(const RenderArgs &rargs)
	{
		assert(m_pclTileMap);
		
		assert((rargs.m_tNumVisibleTiles.m_tX > 0) && (rargs.m_tNumVisibleTiles.m_tY > 0));

		glMatrixMode(GL_MODELVIEW);		

		for(auto y = rargs.m_tTilePos_ViewOrigin.m_tY; y < rargs.m_tTilePos_LastVisible.m_tY; ++y)
		{
			for(auto x = rargs.m_tTilePos_ViewOrigin.m_tX; x < rargs.m_tTilePos_LastVisible.m_tX; ++x)
			{
				const auto position = TileCoord_t{x, y};				
				
				auto layers = m_pclTileMap->GetLayers();
				for (auto layer = 0; layer < m_pclTileMap->GetNumLayers(); ++layer)
				{
					auto obj = layers[layer].TryGetMapObject(position);

					if(!obj)
						continue;

					//go to center of tile
					IntPoint_t tileWorldPos = IntPoint_t{position} * m_tViewInfo.m_uTileScale;
					tileWorldPos += IntPoint_t{1, 1} * (m_tViewInfo.m_uTileScale / 2);

					glPushMatrix();
					glTranslatef(tileWorldPos.m_tX, tileWorldPos.m_tY, 0);		

					DrawObject(*obj);					

					glPopMatrix();
				}
			}			
		}
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
			auto rargs = this->MakeRenderArgs();

			if((rargs.m_tNumVisibleTiles.m_tX == 0) || (rargs.m_tNumVisibleTiles.m_tY == 0))
				goto NOTILES;

			// Setup the viewport
			glViewport(0, 0, GetSize().x, GetSize().y);

			// Setup the screen projection
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			glOrtho(
				rargs.m_tViewOrigin.m_tX, 
				static_cast<GLdouble>(GetSize().x + rargs.m_tViewOrigin.m_tX),
				static_cast<GLdouble>(GetSize().y + rargs.m_tViewOrigin.m_tY),
				rargs.m_tViewOrigin.m_tY,
				-1, 
				1
			);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();			

	#if 0

			int r = 0, g = 1, b = 0;
		
			for (int y = 0; y < m_pclTileMap->GetSize().m_tY; ++y)
			{		
				for (int x = 0; x < m_pclTileMap->GetSize().m_tX; ++x)
				{	
					glBegin(GL_QUADS);
						glColor3f(255 * r, 255, 0);
						glVertex2i((x + 0) * m_tRenderInfo.m_uTileScale, (y + 0) * m_tRenderInfo.m_uTileScale);

						glColor3f(255, 255 * g, 0);
						glVertex2i((x + 1) * m_tRenderInfo.m_uTileScale, (y + 0) * m_tRenderInfo.m_uTileScale);

						glColor3f(255, 0, 255 * b);
						glVertex2i((x + 1) * m_tRenderInfo.m_uTileScale, (y + 1) * m_tRenderInfo.m_uTileScale);

						glColor3f(255, 255, 255);
						glVertex2i((x + 0) * m_tRenderInfo.m_uTileScale, (y + 1) * m_tRenderInfo.m_uTileScale);
					glEnd();

					r = !r;
					g = !g;
					b = !b;
				}
			}

	#endif

			this->DrawGrid(rargs);
			this->Render(rargs);
		}		

NOTILES:
		this->SwapBuffers();

		m_fPendingDraw = false;
	}

	void TileMapCanvas::OnMouseWheel(wxMouseEvent &event)
	{	
		if (event.GetWheelRotation() > 0)
		{
			m_tViewInfo.m_uZoomLevel = std::min(m_tViewInfo.m_uZoomLevel + 1, MAX_ZOOM_LEVELS-1);

			m_tViewInfo.m_uTileScale = g_tScales[m_tViewInfo.m_uZoomLevel].m_uScale;
		}
		else
		{
			if(m_tViewInfo.m_uZoomLevel == 0)
				return;

			m_tViewInfo.m_uZoomLevel --;
			m_tViewInfo.m_uTileScale = g_tScales[m_tViewInfo.m_uZoomLevel].m_uScale;

			if(m_tOrigin.m_tX > 0)
				m_tOrigin.m_tX -= m_tViewInfo.m_uTileScale * 2;

			if (m_tOrigin.m_tY > 0)
				m_tOrigin.m_tY -= m_tViewInfo.m_uTileScale * 2;

			m_tOrigin.m_tX = std::max(m_tOrigin.m_tX, 0);
			m_tOrigin.m_tY = std::max(m_tOrigin.m_tY, 0);
		}

		this->UpdateRenderInfo();
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
			auto currentPos = LitePanel::IntPoint_t{ event.GetX(), event.GetY() };

			m_tOrigin += currentPos - m_tMoveStartPos;
			m_tOrigin.m_tX = std::max(0, m_tOrigin.m_tX);
			m_tOrigin.m_tY = std::max(0, m_tOrigin.m_tY);

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
		
		const auto tilePos = m_tViewInfo.WorldToTile(mousePos + m_tOrigin);

		const auto mapSize = m_pclTileMap->GetSize();

		if((tilePos.m_tX >= mapSize.m_tX) || (tilePos.m_tY >= mapSize.m_tY))
			return {};

		return tilePos;
	}

	void TileMapCanvas::OnClose(wxCloseEvent& evt)
	{
		evt.Skip();
		m_upRenderTimer->Stop();
	}
}
