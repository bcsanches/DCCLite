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

#include <ostream>

#include <fmt/format.h>
#include <rapidjson/document.h>

namespace dcclite
{
	class Packet;
}

namespace dcclite::broker
{	
	class DccAddress
	{
		public:
			inline explicit DccAddress(uint16_t address) noexcept :
				m_iAddress(address)
			{
				//empty
			}

			explicit DccAddress(const rapidjson::Value &value);
		
			explicit DccAddress(dcclite::Packet &packet);

			DccAddress() = default;
			DccAddress(const DccAddress &) = default;
			DccAddress(DccAddress &&) = default;
			
			DccAddress &operator=(DccAddress &&) = default;

			inline int GetAddress() const noexcept
			{
				return m_iAddress;
			}		

			inline bool operator>=(const DccAddress &rhs) const noexcept
			{
				return m_iAddress >= rhs.m_iAddress;
			}

			inline bool operator<(const DccAddress &rhs) const noexcept
			{
				return m_iAddress < rhs.m_iAddress;
			}

			inline bool operator>(const DccAddress &rhs) const noexcept
			{
				return m_iAddress > rhs.m_iAddress;
			}

			inline bool operator==(const DccAddress &rhs) const noexcept
			{
				return m_iAddress == rhs.m_iAddress;
			}

			inline bool operator!=(const DccAddress &rhs) const noexcept
			{
				return m_iAddress != rhs.m_iAddress;
			}

			inline DccAddress &operator=(const DccAddress &rhs) noexcept
			{
				m_iAddress = rhs.m_iAddress;

				return *this;
			}

			inline std::string ToString() const
			{
				return fmt::format("{:#05x}", m_iAddress);
			}

			void WriteConfig(dcclite::Packet &packet) const;		

		private:
			uint16_t m_iAddress = 0;

			friend std::ostream &operator<<(std::ostream &os, const DccAddress &address);
	};


	inline std::ostream &operator<<(std::ostream &os, const dcclite::broker::DccAddress &address)
	{
		os << address.m_iAddress;

		return os;
	}
}

namespace fmt
{
	template <>
	struct formatter<dcclite::broker::DccAddress>
	{
		template <typename ParseContext>
		constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const dcclite::broker::DccAddress &a, FormatContext& ctx) const
		{
			return fmt::format_to(ctx.out(), "{}", a.GetAddress());
		}
	};
} //end of namespace fmt

