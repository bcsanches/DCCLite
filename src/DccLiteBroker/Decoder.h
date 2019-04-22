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

#include "ClassInfo.h"
#include "EmbeddedLibDefs.h"
#include "Object.h"

#include <rapidjson/document.h>

#include <fmt/format.h>

class DccLiteService;
class Node;

namespace dcclite
{
	class Packet;
}

class Decoder: public dcclite::Object
{
	public:
		class Address
		{
			public:
				inline Address(int address) :
					m_iAddress(address)
				{
					//empty
				}

				Address(const rapidjson::Value &value);

				Address() = default;
				Address(const Address &) = default;
				Address(Address &&) = default;		

				inline int GetAddress() const
				{
					return m_iAddress;
				}

				inline bool operator<(const Address &rhs) const
				{
					return m_iAddress < rhs.m_iAddress;
				}

				std::string ToString() const
				{
					return fmt::format("{:#05x}", m_iAddress);					
				}

				void WriteConfig(dcclite::Packet &packet) const;

			private:
				int16_t m_iAddress;

				friend std::ostream& operator<<(std::ostream& os, const Address& address);
		};

		typedef dcclite::ClassInfo<Decoder, const Address &, const std::string &, DccLiteService &, const rapidjson::Value &> Class;		

	public:
		Decoder(
			const Class &decoderClass, 
			const Address &address, 
			std::string name,
			DccLiteService &owner,
			const rapidjson::Value &params
		);

		inline Address GetAddress() const
		{
			return m_iAddress;
		}

		virtual void WriteConfig(dcclite::Packet &packet) const;

		virtual dcclite::DecoderTypes GetType() const noexcept = 0;

		virtual bool IsOutputDecoder() const = 0;
		virtual bool IsInputDecoder() const = 0;

		virtual std::optional<dcclite::DecoderStates> GetPendingStateChange() const
		{
			return std::nullopt;
		}

		virtual void SyncRemoteState(dcclite::DecoderStates state) = 0;

		//
		//IObject
		//
		//

		virtual const char *GetTypeName() const noexcept
		{
			return "Decoder";
		}

		virtual void Serialize(dcclite::JsonOutputStream_t &stream) const
		{
			Object::Serialize(stream);

			stream.AddIntValue("address", m_iAddress.GetAddress());
		}

	private:
		Address m_iAddress;		

		DccLiteService &m_rclManager;
};

inline std::ostream &operator<<(std::ostream& os, const Decoder::Address &address)
{
	os << address.m_iAddress;

	return os;
}

