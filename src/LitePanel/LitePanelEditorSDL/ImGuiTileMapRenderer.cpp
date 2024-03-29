// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "ImGuiTileMapRenderer.h"

namespace dcclite::panel_editor
{
	inline ImVec2 PointToImGuiVec(LitePanel::FloatPoint_t p)
	{
		return ImVec2{ p.m_tX, p.m_tY };
	}
	
	ImGuiTileMapRenderer::ImGuiTileMapRenderer(ImDrawList &drawList, ImVec2 clientOrigin):
		m_clDrawList{ drawList },
		m_ptClientOrigin{ clientOrigin }
	{
		//empty
	}

	void ImGuiTileMapRenderer::DrawLine(LitePanel::FloatPoint_t p1, LitePanel::FloatPoint_t p2, LitePanel::Render::Color_t color, float thickness)
	{
		m_clDrawList.AddLine(m_ptClientOrigin + PointToImGuiVec(p1), m_ptClientOrigin + PointToImGuiVec(p2), color, thickness);
	}

	void ImGuiTileMapRenderer::DrawText(float fontSize, LitePanel::FloatPoint_t pos, LitePanel::Render::Color_t color, const char *textBegin, const char *textEnd)
	{
		m_clDrawList.AddText(nullptr, fontSize, m_ptClientOrigin + PointToImGuiVec(pos), color, textBegin, textEnd);
	}

	void ImGuiTileMapRenderer::PushClipRect(LitePanel::FloatPoint_t p1, LitePanel::FloatPoint_t p2)
	{
		m_clDrawList.PushClipRect(m_ptClientOrigin + PointToImGuiVec(p1), m_ptClientOrigin + PointToImGuiVec(p2));
	}

	void ImGuiTileMapRenderer::PopClipRect()
	{
		m_clDrawList.PopClipRect();
	}

}

