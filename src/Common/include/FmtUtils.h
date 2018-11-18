#pragma once

#include <fmt/format.h>

#include "Guid.h"
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
			return format_to(ctx.begin(), "{:08x}-{:04x}-{:04x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}", 
				*reinterpret_cast<const uint32_t*>(&g.m_bId[0]),
				*reinterpret_cast<const uint16_t*>(&g.m_bId[4]),
				*reinterpret_cast<const uint16_t*>(&g.m_bId[6]),
				g.m_bId[8], g.m_bId[9],
				g.m_bId[10], g.m_bId[11], g.m_bId[12], g.m_bId[13], g.m_bId[14], g.m_bId[15]
			);
		}
	};

	template <>
	struct formatter<dcclite::Address>
	{
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const dcclite::Address &a, FormatContext &ctx)
		{
			return format_to(ctx.begin(), "{:03d}.{:03d}.{:03d}.{:03d}",
				a.GetA(),
				a.GetB(),
				a.GetC(),
				a.GetD()
			);
		}
	};
}
