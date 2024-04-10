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
#include "LitePanelLib/render/ColorStyle.h"

enum ToolsLayers
{
	kLAYER_BUTTON = 0,
	kLAYER_OBJECTS = 1
};

namespace dcclite::PanelEditor
{

	void ToolButton::Draw(LitePanel::Render::IRenderer &renderer, const LitePanel::Render::ViewInfo &viewInfo, const LitePanel::FloatPoint_t &tileOrigin) const
	{
		renderer.DrawRect(
			tileOrigin,
			tileOrigin + LitePanel::FloatPoint_t{ static_cast<float>(viewInfo.m_uTileSize), static_cast<float>(viewInfo.m_uTileSize)},
			LitePanel::Render::GetCurrentColorStyle().m_tTileHighLight
		);
	}

	ToolBarWidget::ToolBarWidget() :
		m_clToolsMap{ {2, 6}, 2 },
		m_upButton{std::make_unique<ToolButton>(LitePanel::TileCoord_t{ 0, 0 })}
	{
		m_pclButton = m_upButton.get();

		m_clTileMapRenderer.SetTileMap(&m_clToolsMap);

		m_clToolsMap.RegisterObject(
			std::make_unique<LitePanel::SimpleRailObject>(
				LitePanel::TileCoord_t{ 0, 0 },
				LitePanel::ObjectAngles::EAST,
				LitePanel::SimpleRailTypes::STRAIGHT
			),
			kLAYER_OBJECTS
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<LitePanel::SimpleRailObject>(
				LitePanel::TileCoord_t{ 1, 0 },
				LitePanel::ObjectAngles::NORTH,
				LitePanel::SimpleRailTypes::STRAIGHT
			),
			kLAYER_OBJECTS
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<LitePanel::SimpleRailObject>(
				LitePanel::TileCoord_t{ 0, 1 },
				LitePanel::ObjectAngles::SOUTHEAST,
				LitePanel::SimpleRailTypes::STRAIGHT
			),
			kLAYER_OBJECTS
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<LitePanel::SimpleRailObject>(
				LitePanel::TileCoord_t{ 1, 1 },
				LitePanel::ObjectAngles::NORTHEAST,
				LitePanel::SimpleRailTypes::STRAIGHT
			),
			1
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<LitePanel::SimpleRailObject>(
				LitePanel::TileCoord_t{ 0, 2 },
				LitePanel::ObjectAngles::WEST,
				LitePanel::SimpleRailTypes::CURVE_RIGHT
			),
			kLAYER_OBJECTS
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<LitePanel::SimpleRailObject>(
				LitePanel::TileCoord_t{ 1, 2 },
				LitePanel::ObjectAngles::EAST,
				LitePanel::SimpleRailTypes::CURVE_LEFT
			),
			kLAYER_OBJECTS
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<LitePanel::SimpleRailObject>(
				LitePanel::TileCoord_t{ 0, 3 },
				LitePanel::ObjectAngles::WEST,
				LitePanel::SimpleRailTypes::CURVE_LEFT
			),
			kLAYER_OBJECTS
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<LitePanel::SimpleRailObject>(
				LitePanel::TileCoord_t{ 1, 3 },
				LitePanel::ObjectAngles::EAST,
				LitePanel::SimpleRailTypes::CURVE_RIGHT
			),
			kLAYER_OBJECTS
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<LitePanel::JunctionRailObject>(
				LitePanel::TileCoord_t{ 0, 4 },
				LitePanel::ObjectAngles::WEST,
				LitePanel::JunctionTypes::LEFT_TURNOUT
			),
			kLAYER_OBJECTS
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<LitePanel::JunctionRailObject>(
				LitePanel::TileCoord_t{ 1, 4 },
				LitePanel::ObjectAngles::EAST,
				LitePanel::JunctionTypes::RIGHT_TURNOUT
			),
			kLAYER_OBJECTS
		);		

		m_clToolsMap.RegisterObject(
			std::make_unique<LitePanel::JunctionRailObject>(
				LitePanel::TileCoord_t{ 0, 5 },
				LitePanel::ObjectAngles::EAST,
				LitePanel::JunctionTypes::LEFT_TURNOUT
			),
			kLAYER_OBJECTS
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<LitePanel::JunctionRailObject>(
				LitePanel::TileCoord_t{ 1, 5 },
				LitePanel::ObjectAngles::WEST,
				LitePanel::JunctionTypes::RIGHT_TURNOUT
			),
			kLAYER_OBJECTS
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

			m_clTileMapRenderer.SetupFrame(renderer, ImGuiVecToPoint(canvas_sz));
			m_clTileMapRenderer.Draw(renderer);

			ImGui::InvisibleButton("toolbar_invisible_button", canvas_sz, ImGuiButtonFlags_MouseButtonLeft);			
			const bool is_active = ImGui::IsItemActive();   // Held

			if (is_active && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				auto pos = ImGui::GetMousePos() - canvas_p0;

				auto tilePos = m_clTileMapRenderer.WorldToTileFloor(ImGuiVecToPoint(pos));

				if (m_clToolsMap.IsInside(tilePos))
				{
					if (m_upButton)
					{
						m_clToolsMap.RegisterObject(std::move(m_upButton), kLAYER_BUTTON);
					}
					else if (tilePos == m_pclButton->GetPosition())
					{
						m_upButton.reset(static_cast<ToolButton *>(m_clToolsMap.UnregisterObject(*m_pclButton, kLAYER_BUTTON).release()));
					}
					else
					{
						auto button = m_clToolsMap.UnregisterObject(*m_pclButton, kLAYER_BUTTON);
						button->SetPosition(tilePos);
						m_clToolsMap.RegisterObject(std::move(button), kLAYER_BUTTON);
					}
				}
			}
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