// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include <memory>

#include "EditorWidget.h"

#include "LitePanelLib/render/TileMapRenderer.h"
#include "LitePanelLib/TileMap.h"
#include "LitePanelLib/render/IRenderer.h"

namespace dcclite::PanelEditor
{
	class ConsoleWidget;
	class KeyBindingManager;

	class ToolButton : public LitePanel::MapObject
	{
		public:
			ToolButton(const LitePanel::TileCoord_t &position) :
				MapObject{ position }
			{
				//empty
			}

			void Draw(LitePanel::Render::IRenderer &renderer, const LitePanel::Render::ViewInfo &viewInfo, const LitePanel::FloatPoint_t &tileOrigin) const override;

			virtual std::unique_ptr<LitePanel::MapObject> MakeTempObject() const = 0;

			inline void SetSelected(bool selected) noexcept
			{
				m_fSelected = selected;
			}

		private:			
			bool m_fSelected = false;
	};

	class ToolBarWidget: public EditorWidget
	{
		public:
			ToolBarWidget();

			void Display() override;
			void Update() override;			

			void RegisterCmds(dcclite::PanelEditor::ConsoleWidget &console, KeyBindingManager &bindings);

			void ClearCurrentTool() noexcept;

			inline const ToolButton *TryGetCurrentTool() noexcept
			{
				return m_pclCurrentButton;
			}

		private:
			LitePanel::Render::TileMapRenderer	m_clTileMapRenderer;

			LitePanel::TileMap					m_clToolsMap;			

			ToolButton							*m_pclCurrentButton = nullptr;
	};
}