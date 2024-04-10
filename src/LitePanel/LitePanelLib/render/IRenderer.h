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

#include "../Point.h"

#include "Color.h"

namespace LitePanel::Render
{		
	class IRenderer
	{
		public:
			virtual void DrawLine(FloatPoint_t p1, FloatPoint_t p2, Color_t color, float thickness = 1.0f) = 0;

			virtual void DrawRect(LitePanel::FloatPoint_t p1, LitePanel::FloatPoint_t p2, LitePanel::Render::Color_t color) = 0;

			virtual void DrawText(float fontSize, LitePanel::FloatPoint_t pos, LitePanel::Render::Color_t color, const char *textBegin, const char *textEnd) = 0;

			virtual void PushClipRect(FloatPoint_t p1, FloatPoint_t p2) = 0;
			virtual void PopClipRect() = 0;
	};
}
