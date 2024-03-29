// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "ToolBarWidget.h"

#include "imgui.h"

#include "ImGuiTileMapRenderer.h"
#include "LitePanelLib/RailObject.h"

namespace dcclite::panel_editor
{
	ToolBarWidget::ToolBarWidget() :
		m_clToolsMap{ {2, 4}, 1 }
	{
		m_clView.SetTileMap(&m_clToolsMap);

		m_clToolsMap.RegisterObject(
			std::make_unique<LitePanel::SimpleRailObject>(
				LitePanel::TileCoord_t{ 0, 0 },
				LitePanel::ObjectAngles::EAST,
				LitePanel::SimpleRailTypes::STRAIGHT
			),
			0
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<LitePanel::SimpleRailObject>(
				LitePanel::TileCoord_t{ 1, 0 },
				LitePanel::ObjectAngles::WEST,
				LitePanel::SimpleRailTypes::STRAIGHT
			),
			0
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<LitePanel::SimpleRailObject>(
				LitePanel::TileCoord_t{ 0, 1 },
				LitePanel::ObjectAngles::NORTH,
				LitePanel::SimpleRailTypes::STRAIGHT
			),
			0
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<LitePanel::SimpleRailObject>(
				LitePanel::TileCoord_t{ 1, 1 },
				LitePanel::ObjectAngles::SOUTH,
				LitePanel::SimpleRailTypes::STRAIGHT
			),
			0
		);
	}

	void ToolBarWidget::Display()
	{		
		if (ImGui::Begin("LToolBar", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{			
			ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
			ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available

			ImDrawList *draw_list = ImGui::GetWindowDrawList();			

			ImGuiTileMapRenderer renderer(*draw_list, canvas_p0);

			m_clView.SetupFrame(renderer, ImGuiVecToPoint(canvas_sz));
			m_clView.Draw(renderer);

#if 0

			draw_list->AddRectFilled(canvas_p0 + ImVec2{ 5, 5 }, canvas_p0 + ImVec2{21, 21}, IM_COL32(50, 50, 50, 255));
#endif

#if 0

			ImGui::SmallButton("X");
			ImGui::SameLine();
			ImGui::SmallButton("Y");

			ImGui::SmallButton("Z");
			ImGui::SameLine();
			ImGui::SmallButton("W");
#endif
		}				
		ImGui::End();				
	}


	void ToolBarWidget::Update()
	{

	}
}