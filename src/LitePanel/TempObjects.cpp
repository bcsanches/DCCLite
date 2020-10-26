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

namespace LitePanel
{
	void QuadObject::Draw(const Gui::ViewInfo &viewInfo) const noexcept
	{
		glColor4f(m_fpColor[0], m_fpColor[1], m_fpColor[2], 1.0f);

		glBegin(GL_QUADS);

		glVertex2i(-viewInfo.m_uHalfTileScale, -viewInfo.m_uHalfTileScale);
		glVertex2i(viewInfo.m_uHalfTileScale, -viewInfo.m_uHalfTileScale);
		glVertex2i(viewInfo.m_uHalfTileScale, viewInfo.m_uHalfTileScale);
		glVertex2i(-viewInfo.m_uHalfTileScale, viewInfo.m_uHalfTileScale);

		glEnd();
	}
}