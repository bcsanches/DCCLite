// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "ColorStyle.h"

namespace LitePanel::Render
{
	const ColorStyle g_tDarkStyle =
	{
		LP_COL32(200, 200, 200, 40),	//GridLine
		LP_COL32(0, 0, 0, 255),			//Background
		LP_COL32(139, 111, 46, 255),	//TileHighLight
		LP_COL32(255, 255, 255, 255)	//Rail
	};

	namespace detail
	{
		const ColorStyle *g_ptCurrentColorStyle = &g_tDarkStyle;
	}
}
