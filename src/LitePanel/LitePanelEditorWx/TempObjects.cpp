// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include "TempObjects.h"

#include <wx/glcanvas.h>

#include "TileMapCanvas.h"

#include <LitePanelLib/Render/IRenderer.h>
#include <LitePanelLib/Render/TileMapRenderer.h>

namespace LitePanel
{
	void QuadObject::Draw(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const
	{
		auto sz = static_cast<FloatPoint_t::Type_t>(viewInfo.m_uTileSize);

		renderer.DrawFilledRect(tileOrigin, FloatPoint_t{ sz, sz }, m_tColor);
	}
}