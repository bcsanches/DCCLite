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

#include "ConsoleWidget.h"
#include "KeyBindingManager.h"

#include "LitePanelLib/Panel.h"
#include "LitePanelLib/RailObject.h"
#include "LitePanelLib/render/ColorStyle.h"

enum ToolsLayers
{
	kLAYER_BUTTON = 0,
	kLAYER_OBJECTS = 1
};

namespace dcclite::PanelEditor
{
	class EditCommandManager;

	class IEditCmd
	{
		public:
			IEditCmd() = default;
			virtual ~IEditCmd() = 0;

			virtual void Do(LitePanel::Panel &panel, EditCommandManager &cmdManager) = 0;
			virtual void Undo(LitePanel::Panel &panel, EditCommandManager &cmdManager) = 0;
	};

	typedef std::unique_ptr<IEditCmd> IEditCmdPtr_t;

	class EditCommandManager
	{
		public:			
			EditCommandManager(LitePanel::Panel &panel):
				m_rclPanel{panel}
			{
				//empty
			}

			void RunCmd(IEditCmdPtr_t cmd)
			{
				cmd->Do(m_rclPanel, *this);

				m_vecUndoStack.push_back(std::move(cmd));
			}

		private:
			void PushUndo(IEditCmdPtr_t cmd)
			{
				m_vecUndoStack.push_back(std::move(cmd));
			}

		private:
			std::vector<IEditCmdPtr_t> m_vecUndoStack;

			LitePanel::Panel &m_rclPanel;

	};

	class DeleteRailCmd : public IEditCmd
	{
		public:
			DeleteRailCmd(const LitePanel::TileCoord_t &position):
				m_tPosition{position}
			{
				//empty
			}

			void Do(LitePanel::Panel &panel, EditCommandManager &manager) override
			{
				if (!panel.IsRailTileOccupied(m_tPosition))
				{
					throw std::runtime_error(fmt::format("[DeleteRailCmd::Do] Rail tile is empty: {}-{}", m_tPosition.m_tX, m_tPosition.m_tY));
				}

				m_upDeletedObject = panel.TryUnregisterRail(m_tPosition);
			}

			void Undo(LitePanel::Panel &panel, EditCommandManager &manager) override
			{
				panel.RegisterRail(std::move(m_upDeletedObject));
			}

		private:
			LitePanel::TileCoord_t m_tPosition;

			std::unique_ptr<LitePanel::RailObject> m_upDeletedObject;
	};

	class InsertSimpleRailCmd: public IEditCmd
	{
		public:
			InsertSimpleRailCmd(
				const LitePanel::TileCoord_t &position,
				LitePanel::ObjectAngles angle,
				const LitePanel::SimpleRailTypes type,
				const LitePanel::BlockSplitTypes splitTypes = LitePanel::kBLOCK_SPLIT_NONE):
				m_tPosition{ position },
				m_kAngle{angle},
				m_kType{ type },
				m_kSplit{ splitTypes }
			{
				//empty
			}

			void Do(LitePanel::Panel &panel, EditCommandManager &manager) override
			{
				if (panel.IsRailTileOccupied(m_tPosition))
				{
					manager.RunCmd(std::make_unique<DeleteRailCmd>(m_tPosition));
				}

				panel.RegisterRail(std::make_unique<LitePanel::SimpleRailObject>(m_tPosition, m_kAngle, m_kType, m_kSplit));
			}

			void Undo(LitePanel::Panel &panel, EditCommandManager &manager) override
			{
				panel.TryUnregisterRail(m_tPosition);
			}

		private:
			LitePanel::TileCoord_t m_tPosition;
			LitePanel::ObjectAngles m_kAngle;
			LitePanel::SimpleRailTypes m_kType;
			LitePanel::BlockSplitTypes m_kSplit;
	};

	class SimpleRailObjectToolButton : public ToolButton
	{
		public:
			SimpleRailObjectToolButton(const LitePanel::TileCoord_t &position, LitePanel::ObjectAngles angle, LitePanel::SimpleRailTypes railType) :
				ToolButton{ position },
				m_kAngle{ angle },
				m_kType{ railType }
			{
				//empty
			}			

			std::unique_ptr<LitePanel::MapObject> MakeTempObject() const override
			{
				return std::make_unique<LitePanel::SimpleRailObject>(
					LitePanel::TileCoord_t{ 0, 0 },
					m_kAngle,
					m_kType
				);
			}

			IEditCmdPtr_t MakeCmd(const LitePanel::TileCoord_t &position) const
			{
				return std::make_unique<InsertSimpleRailCmd>(
					position,
					m_kAngle,
					m_kType,
					LitePanel::kBLOCK_SPLIT_NONE
				);
			}

		private:
			LitePanel::ObjectAngles		m_kAngle;
			LitePanel::SimpleRailTypes	m_kType;
	};

	class JunctionRailObjectToolButton : public ToolButton
	{
		public:
			JunctionRailObjectToolButton(const LitePanel::TileCoord_t &position, LitePanel::ObjectAngles angle, LitePanel::JunctionTypes junctionType) :
				ToolButton{ position },
				m_kAngle{ angle },
				m_kType{ junctionType }
			{
				//empty
			}

			std::unique_ptr<LitePanel::MapObject> MakeTempObject() const override
			{
				return std::make_unique<LitePanel::JunctionRailObject>(
					LitePanel::TileCoord_t{ 0, 0 },
					m_kAngle,
					m_kType
				);
			}

		private:
			LitePanel::ObjectAngles		m_kAngle;
			LitePanel::JunctionTypes	m_kType;
	};


	void ToolButton::Draw(LitePanel::Render::IRenderer &renderer, const LitePanel::Render::ViewInfo &viewInfo, const LitePanel::FloatPoint_t &tileOrigin) const
	{
		if (!m_fSelected)
			return;

		renderer.DrawFilledRect(
			tileOrigin,
			tileOrigin + LitePanel::FloatPoint_t{ static_cast<float>(viewInfo.m_uTileSize), static_cast<float>(viewInfo.m_uTileSize)},
			LitePanel::Render::GetCurrentColorStyle().m_tTileHighLight
		);
	}

	ToolBarWidget::ToolBarWidget() :
		m_clToolsMap{ {2, 6}, 2 }
	{		
		m_clTileMapRenderer.SetTileMap(&m_clToolsMap);
		m_clTileMapRenderer.SetRenderFlags(LitePanel::Render::TileMapRendererFlags_DrawBorderTileMapRect);

		m_clToolsMap.RegisterObject(
			std::make_unique<SimpleRailObjectToolButton>(
				LitePanel::TileCoord_t{ 0, 0 },
				LitePanel::ObjectAngles::EAST,
				LitePanel::SimpleRailTypes::STRAIGHT
			),
			kLAYER_BUTTON
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<SimpleRailObjectToolButton>(
				LitePanel::TileCoord_t{ 1, 0 },
				LitePanel::ObjectAngles::NORTH,
				LitePanel::SimpleRailTypes::STRAIGHT
			),
			kLAYER_BUTTON
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<SimpleRailObjectToolButton>(
				LitePanel::TileCoord_t{ 0, 1 },
				LitePanel::ObjectAngles::SOUTHEAST,
				LitePanel::SimpleRailTypes::STRAIGHT
			),
			kLAYER_BUTTON
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<SimpleRailObjectToolButton>(
				LitePanel::TileCoord_t{ 1, 1 },
				LitePanel::ObjectAngles::NORTHEAST,
				LitePanel::SimpleRailTypes::STRAIGHT
			),
			kLAYER_BUTTON
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<SimpleRailObjectToolButton>(
				LitePanel::TileCoord_t{ 0, 2 },
				LitePanel::ObjectAngles::WEST,
				LitePanel::SimpleRailTypes::CURVE_RIGHT
			),
			kLAYER_BUTTON
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<SimpleRailObjectToolButton>(
				LitePanel::TileCoord_t{ 1, 2 },
				LitePanel::ObjectAngles::EAST,
				LitePanel::SimpleRailTypes::CURVE_LEFT
			),
			kLAYER_BUTTON
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<SimpleRailObjectToolButton>(
				LitePanel::TileCoord_t{ 0, 3 },
				LitePanel::ObjectAngles::WEST,
				LitePanel::SimpleRailTypes::CURVE_LEFT
			),
			kLAYER_BUTTON
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<SimpleRailObjectToolButton>(
				LitePanel::TileCoord_t{ 1, 3 },
				LitePanel::ObjectAngles::EAST,
				LitePanel::SimpleRailTypes::CURVE_RIGHT
			),
			kLAYER_BUTTON
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<JunctionRailObjectToolButton>(
				LitePanel::TileCoord_t{ 0, 4 },
				LitePanel::ObjectAngles::WEST,
				LitePanel::JunctionTypes::RIGHT_TURNOUT
			),
			kLAYER_BUTTON
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<JunctionRailObjectToolButton>(
				LitePanel::TileCoord_t{ 1, 4 },
				LitePanel::ObjectAngles::EAST,
				LitePanel::JunctionTypes::LEFT_TURNOUT
			),
			kLAYER_BUTTON
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<JunctionRailObjectToolButton>(
				LitePanel::TileCoord_t{ 0, 5 },
				LitePanel::ObjectAngles::WEST,
				LitePanel::JunctionTypes::LEFT_TURNOUT
			),
			kLAYER_BUTTON
		);

		m_clToolsMap.RegisterObject(
			std::make_unique<JunctionRailObjectToolButton>(
				LitePanel::TileCoord_t{ 1, 5 },
				LitePanel::ObjectAngles::EAST,
				LitePanel::JunctionTypes::RIGHT_TURNOUT
			),
			kLAYER_BUTTON
		);			

		auto &mapSize = m_clToolsMap.GetSize();

		for (uint16_t i = 0; i < mapSize.m_tX; ++i)
		{
			for (uint16_t j = 0; j < mapSize.m_tY; ++j)
			{
				LitePanel::TileCoord_t pos{ i, j };

				auto button = static_cast<const ToolButton *>(m_clToolsMap.TryGetMapObject(pos, kLAYER_BUTTON));

				auto obj = button->MakeTempObject();
				obj->SetPosition(pos);

				m_clToolsMap.RegisterObject(std::move(obj), kLAYER_OBJECTS);
			}
		}
	}

	void ToolBarWidget::RegisterCmds(dcclite::PanelEditor::ConsoleWidget &console, KeyBindingManager &bindings)
	{
		RName clearCurrentToolCmdName{ "ToolBar.ClearCurrentTool" };
	
		console.RegisterCommand(clearCurrentToolCmdName, [this](const ConsoleCmdParams &params)
			{
				this->ClearCurrentTool();
			}
		);

		bindings.Bind(clearCurrentToolCmdName.GetData().data(), SDL_SCANCODE_ESCAPE, 0);		
	}

	void ToolBarWidget::ClearCurrentTool() noexcept
	{
		if (!m_pclCurrentButton)
			return;

		m_pclCurrentButton->SetSelected(false);
		m_pclCurrentButton = nullptr;
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
#if 1
				if (m_clToolsMap.IsInside(tilePos))
				{
					auto button = const_cast<ToolButton *>(static_cast<const ToolButton *>(m_clToolsMap.TryGetMapObject(tilePos, kLAYER_BUTTON)));					

					if (m_pclCurrentButton)
						m_pclCurrentButton->SetSelected(false);

					if (button == m_pclCurrentButton)
					{						
						m_pclCurrentButton = nullptr;
					}
					else
					{
						button->SetSelected(true);
						m_pclCurrentButton = button;
					}
				}
#endif
			}

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