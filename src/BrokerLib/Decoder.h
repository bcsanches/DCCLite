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

#include <optional>
#include <ostream>

#include "EmbeddedLibDefs.h"
#include "Object.h"

#include <rapidjson/document.h>

#include <fmt/format.h>

namespace dcclite
{
	class Packet;
}

namespace dcclite::broker
{
	class IDccLite_DecoderServices;
	class IDevice_DecoderServices;
	class Node;


	class DccAddress
	{
		public:
			inline explicit DccAddress(uint16_t address):
				m_iAddress(address)
			{
				//empty
			}

			explicit DccAddress(const rapidjson::Value &value);
		
			explicit DccAddress(dcclite::Packet &packet);

			DccAddress() = default;
			DccAddress(const DccAddress &) = default;
			DccAddress(DccAddress &&) = default;

			inline int GetAddress() const
			{
				return m_iAddress;
			}		

			inline bool operator>=(const DccAddress &rhs) const
			{
				return m_iAddress >= rhs.m_iAddress;
			}

			inline bool operator<(const DccAddress &rhs) const
			{
				return m_iAddress < rhs.m_iAddress;
			}

			inline bool operator>(const DccAddress &rhs) const
			{
				return m_iAddress > rhs.m_iAddress;
			}

			inline bool operator==(const DccAddress &rhs) const
			{
				return m_iAddress == rhs.m_iAddress;
			}

			inline std::string ToString() const
			{
				return fmt::format("{:#05x}", m_iAddress);
			}

			void WriteConfig(dcclite::Packet &packet) const;		

		private:
			uint16_t m_iAddress;

			friend std::ostream &operator<<(std::ostream &os, const DccAddress &address);
	};

	class Decoder: public dcclite::Object
	{	
		public:
			Decoder(			
				const DccAddress &address, 
				std::string name,
				IDccLite_DecoderServices &owner,
				IDevice_DecoderServices &dev,
				const rapidjson::Value &params
			);

			inline DccAddress GetAddress() const
			{
				return m_iAddress;
			}

			inline const std::string &GetLocationHint() const
			{
				return m_strLocationHint;
			}				

			//
			//IObject
			//
			//

			const char *GetTypeName() const noexcept override
			{
				return "Decoder";
			}

			void Serialize(dcclite::JsonOutputStream_t &stream) const override;	

		private:
			DccAddress m_iAddress;				

			std::string m_strLocationHint;		

		protected:
			IDccLite_DecoderServices &m_rclManager;
			IDevice_DecoderServices &m_rclDevice;
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
		auto format(const dcclite::broker::DccAddress &a, FormatContext& ctx)
		{
			return format_to(ctx.out(), "{}", a.GetAddress());
		}
	};
} //end of namespace fmt

