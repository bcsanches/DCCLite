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


MapCanvas::MapCanvas(wxWindow *parent, int id):
	OGLCanvas{parent, id}	
{
	Bind(wxEVT_MIDDLE_DOWN, &MapCanvas::OnMouseMiddleDown, this);	
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

	m_tRenderInfo.m_tWorldSize = m_pclTileMap->GetSize() * m_tRenderInfo.m_uTileScale;
}

MapCanvas::RenderArgs MapCanvas::MakeRenderArgs() const
{
	assert(m_pclTileMap);

	RenderArgs args;

	args.m_tViewClientSize = LitePanel::IntPoint_t{ this->GetSize().x, this->GetSize().y };	
	args.m_tViewOrigin = m_tOrigin;	

	args.m_tTilePos_ViewOrigin = args.m_tViewOrigin / m_tRenderInfo.m_uTileScale;

	args.m_tTilesClientSize = args.m_tViewClientSize / m_tRenderInfo.m_uTileScale;

	args.m_tTilePos_LastVisible = args.m_tTilePos_ViewOrigin + args.m_tTilesClientSize + LitePanel::IntPoint_t{ 2, 2 };
	args.m_tTilePos_LastVisible.m_tX = std::min(args.m_tTilePos_LastVisible.m_tX, m_pclTileMap->GetSize().m_tX - 1);
	args.m_tTilePos_LastVisible.m_tY = std::min(args.m_tTilePos_LastVisible.m_tY, m_pclTileMap->GetSize().m_tY - 1);

	args.m_tNumVisibleTiles = (args.m_tTilePos_LastVisible - args.m_tTilePos_ViewOrigin) + LitePanel::IntPoint_t{ 1, 1 };

	if((args.m_tNumVisibleTiles.m_tX + args.m_tTilePos_ViewOrigin.m_tX) > m_pclTileMap->GetSize().m_tX)
		--args.m_tNumVisibleTiles.m_tX;

	if ((args.m_tNumVisibleTiles.m_tY + args.m_tTilePos_ViewOrigin.m_tY) > m_pclTileMap->GetSize().m_tY)
		--args.m_tNumVisibleTiles.m_tY;
	
	return args;
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

	auto lastVisibleTileCorner = rargs.m_tNumVisibleTiles * m_tRenderInfo.m_uTileScale;

#if 1
	for (int i = 0; i <= rargs.m_tNumVisibleTiles.m_tX; ++i)
	{
		LitePanel::IntPoint_t tilePos{ rargs.m_tTilePos_ViewOrigin.m_tX + i, rargs.m_tTilePos_ViewOrigin.m_tY };

		if (tilePos.m_tX > m_pclTileMap->GetSize().m_tX)
			break;

		auto tileWorldPos = tilePos * m_tRenderInfo.m_uTileScale;

		glVertex2i(tileWorldPos.m_tX, tileWorldPos.m_tY);
		glVertex2i(tileWorldPos.m_tX, (rargs.m_tTilePos_LastVisible.m_tY + 1) * m_tRenderInfo.m_uTileScale);
	}

	for (int i = 0; i <= rargs.m_tNumVisibleTiles.m_tY; ++i)
	{
		LitePanel::IntPoint_t tilePos{ rargs.m_tTilePos_ViewOrigin.m_tX, rargs.m_tTilePos_ViewOrigin.m_tY + i };

		if (tilePos.m_tY > m_pclTileMap->GetSize().m_tY)
			break;

		auto tileWorldPos = tilePos * m_tRenderInfo.m_uTileScale;
	
		glVertex2i(tileWorldPos.m_tX, tileWorldPos.m_tY);
		glVertex2i((rargs.m_tTilePos_LastVisible.m_tX + 1) * m_tRenderInfo.m_uTileScale, tileWorldPos.m_tY);
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

	this->SwapBuffers();
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
