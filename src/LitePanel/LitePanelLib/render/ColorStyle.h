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

#include "Color.h"

namespace LitePanel::Render
{	
	struct ColorStyle
	{
		Color_t m_tGridLine;
		Color_t m_tBackground;
		Color_t m_tTileHighLight;

		Color_t m_tRail;
	};

	enum class ColorStyles
	{
		DARK
	};

	namespace detail
	{
		extern const ColorStyle *g_ptCurrentColorStyle;
	}

	inline const ColorStyle &GetCurrentColorStyle() noexcept
	{
		return *detail::g_ptCurrentColorStyle;
	}
}
