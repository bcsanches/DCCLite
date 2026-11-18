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
		LP_COL32(050, 050, 050, 255),	//Background
		LP_COL32(139, 111, 046, 255),	//TileHighLight
		LP_COL32(255, 255, 255, 255),	//TileMapBorder
		LP_COL32(200, 200, 200, 255)	//Rail
	};

	const ColorStyle g_tLightStyle =
	{
		LP_COL32(000, 000, 000, 040),	//GridLine
		LP_COL32(255, 255, 255, 255),	//Background
		LP_COL32(255, 255, 178, 255),	//TileHighLight
		LP_COL32(000, 000, 000, 255),	//TileMapBorder
		LP_COL32(000, 000, 000, 255)	//Rail
	};

	namespace detail
	{
		const ColorStyle *g_ptCurrentColorStyle = &g_tDarkStyle;
		const ColorStyle *g_ptStyles[] = 
		{ 
			&g_tDarkStyle, 
			&g_tLightStyle 
		};
	}

	void SetColorStyle(ColorStyles style)
	{
		detail::g_ptCurrentColorStyle = detail::g_ptStyles[static_cast<int>(style)];
	}
}
