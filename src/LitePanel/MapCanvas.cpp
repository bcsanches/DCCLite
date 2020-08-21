// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include "MapCanvas.h"


namespace LitePanel
{

	TileCoord_t MapCanvas::ViewInfo::WorldToTile(const IntPoint_t &worldPoint) const
	{
		auto localCoord = worldPoint / m_uTileScale;

		return TileCoord_t{ static_cast<TileCoord_t::Type_t>(localCoord.m_tX), static_cast<TileCoord_t::Type_t>(localCoord.m_tY)};
	}


	MapCanvas::MapCanvas(wxWindow *parent, int id):
		OGLCanvas{parent, id}	
	{
		Bind(wxEVT_MIDDLE_DOWN, &MapCanvas::OnMouseMiddleDown, this);	
		Bind(wxEVT_MOUSEWHEEL, &MapCanvas::OnMouseWheel, this);
	}

	void MapCanvas::SetTileMap(const LitePanel::TileMap *tileMap) noexcept
	{
		m_pclTileMap = tileMap;

		if(!m_pclTileMap)
			return;

		this->UpdateRenderInfo();
	}

	void MapCanvas::UpdateRenderInfo()
	{
		assert(m_pclTileMap);

		m_tViewInfo.m_tWorldSize = LitePanel::IntPoint_t{m_pclTileMap->GetSize()} * m_tViewInfo.m_uTileScale;

		this->Refresh();
	}

	MapCanvas::RenderArgs MapCanvas::MakeRenderArgs() const
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

	void MapCanvas::DrawGrid(const RenderArgs &rargs)
	{
		assert(m_pclTileMap);

		glDisable(GL_LINE_SMOOTH);
		glLineWidth(1.0f);

		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);		

		glBegin(GL_LINES);

		//glPushMatrix();	
		//glTranslatef(-rargs.m_tViewOrigin.m_tX, -rargs.m_tViewOrigin.m_tY, 0);

		auto lastVisibleTileCorner = IntPoint_t{rargs.m_tNumVisibleTiles} * m_tViewInfo.m_uTileScale;

	#if 1
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
	#else
		for (int i = 0; i <= m_pclTileMap->GetSize().m_tX; ++i)
		{
			LitePanel::IntPoint_t tilePos{ i,  0 };

			auto tileWorldPos = tilePos * m_tRenderInfo.m_uTileScale;		

			glVertex2i(tileWorldPos.m_tX, 0);
			glVertex2i(tileWorldPos.m_tX, m_pclTileMap->GetSize().m_tY * m_tRenderInfo.m_uTileScale);
		}

		for (int i = 0; i <= m_pclTileMap->GetSize().m_tY; ++i)
		{
			LitePanel::IntPoint_t tilePos{ 0,  i };

			auto tileWorldPos = tilePos * m_tRenderInfo.m_uTileScale;		

			glVertex2i(0, tileWorldPos.m_tY);
			glVertex2i(m_pclTileMap->GetSize().m_tX * m_tRenderInfo.m_uTileScale, tileWorldPos.m_tY);
		}
	#endif

		glEnd();

		//glPopMatrix();
	}

	void MapCanvas::OnDraw()
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
				GetSize().x + rargs.m_tViewOrigin.m_tX,
				GetSize().y + rargs.m_tViewOrigin.m_tY,
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
		}		

NOTILES:
		this->SwapBuffers();
	}

	void MapCanvas::OnMouseWheel(wxMouseEvent &event)
	{	
		if (event.GetWheelRotation() > 0)
		{
			m_tViewInfo.m_uTileScale *= 2;

			if(m_tViewInfo.m_uTileScale > 128)
				m_tViewInfo.m_uTileScale = 128;
		}
		else
		{
			if(m_tViewInfo.m_uTileScale == 8)
				return;

			m_tViewInfo.m_uTileScale /= 2;

			if(m_tOrigin.m_tX > 0)
				m_tOrigin.m_tX -= m_tViewInfo.m_uTileScale * 2;

			if (m_tOrigin.m_tY > 0)
				m_tOrigin.m_tY -= m_tViewInfo.m_uTileScale * 2;

			m_tOrigin.m_tX = std::max(m_tOrigin.m_tX, 0);
			m_tOrigin.m_tY = std::max(m_tOrigin.m_tY, 0);
		}

		this->UpdateRenderInfo();
	}

	void MapCanvas::OnMouseMiddleDown(wxMouseEvent &event)
	{
		event.Skip();

		this->Bind(wxEVT_MOTION, &MapCanvas::OnMouseMove, this);
		this->Bind(wxEVT_MIDDLE_UP, &MapCanvas::OnMouseMiddleUp, this);
		this->Bind(wxEVT_MOUSE_CAPTURE_LOST, &MapCanvas::OnMouseCaptureLost, this);

		this->CaptureMouse();

		m_tMoveStartPos = LitePanel::IntPoint_t{event.GetX(), event.GetY()};	
	}

	void MapCanvas::OnMouseMove(wxMouseEvent &event)
	{
		event.Skip();

		auto currentPos = LitePanel::IntPoint_t{ event.GetX(), event.GetY() };

		m_tOrigin += currentPos - m_tMoveStartPos;
		m_tOrigin.m_tX = std::max(0, m_tOrigin.m_tX);
		m_tOrigin.m_tY = std::max(0, m_tOrigin.m_tY);

		m_tMoveStartPos = currentPos;

		this->Refresh();
	}

	void MapCanvas::OnMouseCaptureLost(wxMouseCaptureLostEvent &event)
	{
		event.Skip();

		this->OnMouseLost();
	}

	void MapCanvas::OnMouseLost()
	{
		this->ReleaseMouse();

		this->Unbind(wxEVT_MOTION, &MapCanvas::OnMouseMove, this);
		this->Unbind(wxEVT_MIDDLE_UP, &MapCanvas::OnMouseMiddleUp, this);
		this->Unbind(wxEVT_MOUSE_CAPTURE_LOST, &MapCanvas::OnMouseCaptureLost, this);
	}


	void MapCanvas::OnMouseMiddleUp(wxMouseEvent &event)
	{
		event.Skip();

		this->OnMouseLost();
	}
}