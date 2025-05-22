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

namespace dcclite::broker::exec::dcc
{	
	class Address
	{
		public:
			inline explicit Address(uint16_t address) noexcept :
				m_iAddress(address)
			{
				//empty
			}

			explicit Address(const rapidjson::Value &value);
		
			explicit Address(dcclite::Packet &packet);

			Address() = default;
			Address(const Address &) = default;
			Address(Address &&) = default;
			
			Address &operator=(Address &&) = default;

			inline int GetAddress() const noexcept
			{
				return m_iAddress;
			}		

			inline bool operator>=(const Address &rhs) const noexcept
			{
				return m_iAddress >= rhs.m_iAddress;
			}

			inline bool operator<(const Address &rhs) const noexcept
			{
				return m_iAddress < rhs.m_iAddress;
			}

			inline bool operator>(const Address &rhs) const noexcept
			{
				return m_iAddress > rhs.m_iAddress;
			}

			inline bool operator==(const Address &rhs) const noexcept
			{
				return m_iAddress == rhs.m_iAddress;
			}

			inline bool operator!=(const Address &rhs) const noexcept
			{
				return m_iAddress != rhs.m_iAddress;
			}

			inline Address &operator=(const Address &rhs) noexcept
			{
				m_iAddress = rhs.m_iAddress;

				return *this;
			}

			inline std::string ToString() const
			{
				return fmt::format("{:#05x}", m_iAddress);
			}

			inline std::string ToDecimalString() const
			{
				return fmt::format("{:#04}", m_iAddress);
			}

			void WriteConfig(dcclite::Packet &packet) const;		

		private:
			uint16_t m_iAddress = 0;

			friend std::ostream &operator<<(std::ostream &os, const Address &address);
	};


	inline std::ostream &operator<<(std::ostream &os, const Address &address)
	{
		os << address.m_iAddress;

		return os;
	}
}

namespace fmt
{
	template <>
	struct formatter < dcclite::broker::exec::dcc::Address >
	{
		template <typename ParseContext>
		constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const dcclite::broker::exec::dcc::Address &a, FormatContext& ctx) const
		{
			return fmt::format_to(ctx.out(), "{}", a.GetAddress());
		}
	};
} //end of namespace fmt

