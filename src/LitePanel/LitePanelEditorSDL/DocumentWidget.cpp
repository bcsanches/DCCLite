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
#include "PanelEditorApp.h"

#include "imgui.h"

#include "Document.h"
#include "ImGuiTileMapRenderer.h"

#include "LitePanelLib/render/TileMapRenderer.h"

namespace dcclite::PanelEditor
{	
	void DocumentView::SetDocument(Document *doc)
	{
		if (doc == m_pclDocument)
			return;

		m_vecRenderers.clear();

		m_pclDocument = doc;
		if (!m_pclDocument)
			return;

		auto numPanels = m_pclDocument->GetNumPanels();
		auto panels = m_pclDocument->GetPanels();

		m_vecRenderers.reserve(numPanels);

		for (int i = 0; i < numPanels; ++i)
		{
			m_vecRenderers.push_back(LitePanel::Render::TileMapRenderer{ &panels[i].GetTileMap() });
		}
	}

	void DocumentView::Display(PanelEditorApp &app, const bool debugTileClipping)
	{
		if (!m_pclDocument)
			return;

		auto panels = m_pclDocument->GetPanels();
		auto numPanels = m_pclDocument->GetNumPanels();		

		for (int i = 0; i < numPanels; ++i)
		{
			if (ImGui::BeginTabItem(panels[i].GetName().c_str(), nullptr, 0))
			{
				//ImGui::Text("Scroll %f %f", scrolling.x, scrolling.y);

				ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
				ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
				if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
				if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
				ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

				// Draw border and background color
				ImGuiIO &io = ImGui::GetIO();
				ImDrawList *draw_list = ImGui::GetWindowDrawList();

				if (debugTileClipping)
				{
					canvas_p0 += ImVec2{ 100.0f, 100.0f };
					canvas_p1 -= ImVec2{ 100.0f, 100.0f };
				}								
			
				// Using InvisibleButton() as a convenience 1) it will advance the layout cursor and 2) allows us to use IsItemHovered()/IsItemActive()
				// This will catch our interactions
				ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
				const bool is_hovered = ImGui::IsItemHovered(); // Hovered
				const bool is_active = ImGui::IsItemActive();   // Held

				if (debugTileClipping)
				{
					canvas_sz -= ImVec2{ 200.0f, 200.0f };
				}
				else
				{
					draw_list->PushClipRect(canvas_p0, canvas_p1, true);
				}

				//Visible panel changed... do some cleanup
				if (m_iVisiblePanel != i)
				{
					if (m_iVisiblePanel >= 0)
					{
						if (m_pclCursorObject)
						{
							panels[i].UnregisterTempObject(*m_pclCursorObject);
							m_pclCursorObject = nullptr;
						}

						//force tool update, crate temp object, etc
						m_pclCurrentTool = nullptr;
					}

					m_iVisiblePanel = i;					
				}

				//Is mouse over tileMap?
				if (ImGui::IsMouseHoveringRect(canvas_p0, canvas_sz))
				{
					auto &toolBar = app.GetToolBar();
					auto newTool = toolBar.TryGetCurrentTool();

					if (m_pclCurrentTool != newTool)
					{
						if (m_pclCursorObject)
						{
							panels[i].UnregisterTempObject(*m_pclCursorObject);
							m_pclCursorObject = nullptr;
						}

						m_pclCurrentTool = newTool;
						if (m_pclCurrentTool)
						{
							auto tmpObj = m_pclCurrentTool->MakeTempObject();

							m_pclCursorObject = tmpObj.get();

							panels[i].RegisterTempObject(std::move(tmpObj));							
						}
					}

					m_fMouseHovering = true;
				}
				else if(m_fMouseHovering)
				{
					m_fMouseHovering = false;

					if (m_pclCursorObject)
					{
						//we will generate a lot of garbage... but does it matter here?
						//We could store the tmp object on a local variable for reuse, but does not seems to be worth it... 
						//just force recreation if cursor comes back
						panels[i].UnregisterTempObject(*m_pclCursorObject);
						m_pclCursorObject = nullptr;

						m_pclCurrentTool = nullptr;
					}
				}

				if (ImGui::IsKeyDown(ImGuiKey_MouseWheelY))
				{
					auto &io = ImGui::GetIO();
					if (io.MouseWheel < 0)
					{
						m_vecRenderers[i].ZoomOut();
					}
					else
					{
						m_vecRenderers[i].ZoomIn();
					}
				}


				//const ImVec2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
				//const ImVec2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);				

				// Pan (we use a zero mouse threshold when there's no context menu)
				// You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.						
				if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
				{					
					m_vecRenderers[i].Move(ImGuiVecToPoint(io.MouseDelta));
				}							

				ImGuiTileMapRenderer renderer(*draw_list, canvas_p0);

				m_vecRenderers[i].SetupFrame(renderer, ImGuiVecToPoint(canvas_sz));
				m_vecRenderers[i].Draw(renderer);

				ImGui::EndTabItem();
			}
		}


	}
	
	void DocumentWidget::Display(PanelEditorApp &app)
	{
		if (ImGui::Begin("WorkArea", nullptr, ImGuiWindowFlags_NoCollapse))
		{
			if (ImGui::BeginTabBar("MainTabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton))
			{
				m_clView.Display(app, m_fTileClippingDebug);
				
				ImGui::EndTabBar();
			}
		}
		ImGui::End();
	}
}