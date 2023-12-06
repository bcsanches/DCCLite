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

//straight from ImGui
#define LP_COL32_R_SHIFT    0
#define LP_COL32_G_SHIFT    8
#define LP_COL32_B_SHIFT    16
#define LP_COL32_A_SHIFT    24

#define LP_COL32(R,G,B,A)    (((Uint32_t)(A)<<LP_COL32_A_SHIFT) | ((Uint32_t)(B)<<LP_COL32_B_SHIFT) | ((Uint32_t)(G)<<LP_COL32_G_SHIFT) | ((Uint32_t)(R)<<LP_COL32_R_SHIFT))

namespace LitePanel::Render
{	
	typedef uint32_t Color_t;

	class IRenderer
	{
		public:
			virtual void DrawLine(FloatPoint_t p1, FloatPoint_t p2, Color_t color, float thickness = 1.0f) = 0;

	};
}
