// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "DocumentWidget.h"

#include "imgui.h"

#include "Document.h"

#include "LitePanelLib/Point.h"

#include "LitePanelLib/render/IRenderer.h"
#include "LitePanelLib/render/TileMapView.h"

namespace dcclite::panel_editor
{
	inline ImVec2 PointToImGuiVec(LitePanel::FloatPoint_t p)
	{
		return ImVec2{ p.m_tX, p.m_tY };
	}	

	inline LitePanel::FloatPoint_t ImGuiVecToPoint(ImVec2 vec)
	{
		return LitePanel::FloatPoint_t{ vec.x, vec.y };
	}

	class ImGuiRenderer : public LitePanel::Render::IRenderer
	{
		public:
			ImGuiRenderer(ImDrawList &drawList, ImVec2 clientOrigin) :
				m_clDrawList{ drawList },
				m_ptClientOrigin{clientOrigin}
			{
				//empty
			}

			void DrawLine(LitePanel::FloatPoint_t p1, LitePanel::FloatPoint_t p2, LitePanel::Render::Color_t color, float thickness = 1.0f) override
			{
				m_clDrawList.AddLine(m_ptClientOrigin + PointToImGuiVec(p1), m_ptClientOrigin + PointToImGuiVec(p2), color, thickness);				
			}

			void DrawText(float fontSize, LitePanel::FloatPoint_t pos, LitePanel::Render::Color_t color, const char *textBegin, const char *textEnd) override
			{
				m_clDrawList.AddText(nullptr, fontSize, m_ptClientOrigin + PointToImGuiVec(pos), color, textBegin, textEnd);
			}

		private:		
			ImDrawList	&m_clDrawList;

			ImVec2		m_ptClientOrigin;
	};

	void DocumentView::SetDocument(Document *doc)
	{
		if (doc == m_pclDocument)
			return;

		m_vecViews.clear();

		m_pclDocument = doc;
		if (!m_pclDocument)
			return;

		auto numPanels = m_pclDocument->GetNumPanels();
		auto panels = m_pclDocument->GetPanels();
		for (int i = 0; i < numPanels; ++i)
		{
			m_vecViews.push_back(LitePanel::Render::TileMapView{ panels[i].GetTileMap() });
		}
	}

	void DocumentView::Display()
	{
		if (!m_pclDocument)
			return;

		auto panels = m_pclDocument->GetPanels();
		auto numPanels = m_pclDocument->GetNumPanels();

		static ImVec2 scrolling(0.0f, 0.0f);

		for (int i = 0; i < numPanels; ++i)
		{
			if (ImGui::BeginTabItem(panels[i].GetName().c_str(), nullptr, 0))
			{
				ImGui::Text("Scroll %f %f", scrolling.x, scrolling.y);

				ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
				ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
				if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
				if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
				ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

				// Draw border and background color
				ImGuiIO &io = ImGui::GetIO();
				ImDrawList *draw_list = ImGui::GetWindowDrawList();
				draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
				draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));				
				
				//draw_list->PushClipRect(canvas_p0, canvas_p1, true);

				ImGuiRenderer renderer(*draw_list, canvas_p0);

				m_vecViews[i].SetupFrame(renderer, ImGuiVecToPoint(canvas_sz));
				m_vecViews[i].Draw(renderer);

#if 0
				// Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
				// This will catch our interactions
				ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
				const bool is_hovered = ImGui::IsItemHovered(); // Hovered
				const bool is_active = ImGui::IsItemActive();   // Held
				const ImVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
				const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

				// Pan (we use a zero mouse threshold when there's no context menu)
				// You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.						
				if (is_active && ImGui::IsMouseDragging(ImGuiButtonFlags_MouseButtonRight))
				{
					scrolling.x += io.MouseDelta.x;
					scrolling.y += io.MouseDelta.y;

					if (scrolling.x < 0)
						scrolling.x = 0;

					if (scrolling.y < 0)
						scrolling.y = 0;
				}

				// Draw grid + all lines in the canvas
				draw_list->PushClipRect(canvas_p0, canvas_p1, true);

				const float GRID_STEP = 16.0f;
				for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
					draw_list->AddLine(ImVec2(canvas_p0.x + x, canvas_p0.y), ImVec2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));

				for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
					draw_list->AddLine(ImVec2(canvas_p0.x, canvas_p0.y + y), ImVec2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));
#endif

				ImGui::EndTabItem();
			}
		}


	}
	
	void DocumentWidget::Display()
	{
		if (ImGui::Begin("WorkArea", nullptr, ImGuiWindowFlags_NoCollapse))
		{
			if (ImGui::BeginTabBar("MainTabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton))
			{
				m_clView.Display();
				
				ImGui::EndTabBar();
			}
		}
		ImGui::End();
	}
}