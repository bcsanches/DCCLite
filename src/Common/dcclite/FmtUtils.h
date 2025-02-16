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

#include <fmt/format.h>

#include "GuidDefs.h"
#include "RName.h"
#include "Socket.h"

namespace fmt 
{
	template <>
	struct formatter<dcclite::Guid> 
	{
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const dcclite::Guid &g, FormatContext &ctx) 
		{
			return fmt::format_to(ctx.out(), "{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}", 
				g.m_bId[0], g.m_bId[1], g.m_bId[2], g.m_bId[3],
				g.m_bId[4], g.m_bId[5],
				g.m_bId[6], g.m_bId[7],
				g.m_bId[8], g.m_bId[9],
				g.m_bId[10], g.m_bId[11], g.m_bId[12], g.m_bId[13], g.m_bId[14], g.m_bId[15]
			);
		}
	};

	template <>
	struct formatter<dcclite::NetworkAddress>
	{
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const dcclite::NetworkAddress &a, FormatContext &ctx)
		{
			return fmt::format_to(ctx.out(), "{:03d}.{:03d}.{:03d}.{:03d}",
				a.GetA(),
				a.GetB(),
				a.GetC(),
				a.GetD()
			);
		}
	};

	template <>
	struct formatter<dcclite::RName>
	{
		template <typename ParseContext>
		constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const dcclite::RName &n, FormatContext &ctx) const
		{
			return fmt::format_to(ctx.out(), "{}", n.GetData());			
		}
	};
}
