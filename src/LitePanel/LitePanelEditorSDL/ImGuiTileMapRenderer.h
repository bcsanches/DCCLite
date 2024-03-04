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

#include "imgui.h"

#include "LitePanelLib/render/IRenderer.h"

namespace dcclite::panel_editor
{
	inline LitePanel::FloatPoint_t ImGuiVecToPoint(ImVec2 vec)
	{
		return LitePanel::FloatPoint_t{ vec.x, vec.y };
	}

	class ImGuiTileMapRenderer : public LitePanel::Render::IRenderer
	{
		public:
			ImGuiTileMapRenderer(ImDrawList &drawList, ImVec2 clientOrigin);

			void DrawLine(LitePanel::FloatPoint_t p1, LitePanel::FloatPoint_t p2, LitePanel::Render::Color_t color, float thickness = 1.0f) override;
			void DrawText(float fontSize, LitePanel::FloatPoint_t pos, LitePanel::Render::Color_t color, const char *textBegin, const char *textEnd) override;			

		private:
			ImDrawList &m_clDrawList;

			ImVec2		m_ptClientOrigin;
	};
}
