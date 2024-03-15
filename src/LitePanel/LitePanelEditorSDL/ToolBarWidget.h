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

#include "EditorWidget.h"

#include "LitePanelLib/render/TileMapView.h"
#include "LitePanelLib/TileMap.h"

namespace dcclite::panel_editor
{
	class ToolBarWidget: public EditorWidget
	{
		public:
			ToolBarWidget();

			void Display() override;
			void Update() override;			

		private:
			LitePanel::Render::TileMapView m_clView;

			LitePanel::TileMap m_clToolsMap;
	};
}