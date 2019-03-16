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

#include <assert.h>
#include <string.h>
#include <stdint.h>

#include "Guid.h"

namespace dcclite
{
	enum class MsgTypes: uint8_t
	{
		RESERVED0,

		HELLO,				
		ACCEPTED,
		CONFIG_START,
		CONFIG_DEV,
		CONFIG_FINISHED,
		CONFIG_ACK,
		MSG_PING,
		MSG_PONG,		
	};	

	constexpr uint32_t PACKET_ID = 0xBEEFFEED;

	constexpr uint8_t PACKET_MAX_SIZE = 128;

	/**
	Basic packet format:

	ID MSG_TYPE SESSION_TOKEN CONFIG_TOKEN MSG
	
	
	*/

	class Packet
	{
		public:
			inline Packet() = default;

			inline Packet(const uint8_t *data, uint8_t len)
			{
				assert(len < PACKET_MAX_SIZE);
				memcpy(&m_arData[0], data, len);
			}

			Packet(const Packet &rhs):				
				m_iIndex{ rhs.m_iIndex }
			{
				memcpy(m_arData, rhs.m_arData, sizeof(m_arData));
			}

			Packet(Packet &&) = delete;

			Packet &operator=(const Packet &) = delete;			

			inline void Write8(uint8_t byte) noexcept
			{
				assert(m_iIndex + 1 < PACKET_MAX_SIZE);

				m_arData[m_iIndex++] = byte;
			}

			inline void Write16(uint16_t data) noexcept
			{
				assert(m_iIndex + 2 < PACKET_MAX_SIZE);

				memcpy(&m_arData[m_iIndex], &data, sizeof(data));				
				m_iIndex += sizeof(data);
			}
			
			inline void Write32(uint32_t data)
			{
				assert(m_iIndex + sizeof(data) < PACKET_MAX_SIZE);

				memcpy(&m_arData[m_iIndex], &data, sizeof(data));
				m_iIndex += sizeof(data);
			}

			inline void Write(const Guid &guid) noexcept
			{
				assert(m_iIndex + sizeof(guid.m_bId) < PACKET_MAX_SIZE);

				memcpy(&m_arData[m_iIndex], guid.m_bId, sizeof(guid.m_bId));
				m_iIndex += sizeof(guid.m_bId);				
			}

			inline Guid ReadGuid()
			{
				Guid guid;

				assert(m_iIndex + sizeof(guid.m_bId) < PACKET_MAX_SIZE);

				memcpy(guid.m_bId, &m_arData[m_iIndex], sizeof(guid.m_bId));
				m_iIndex += sizeof(guid.m_bId);

				return guid;
			}

			template <typename T>
			inline T Read()
			{				
				assert(m_iIndex + sizeof(T) < PACKET_MAX_SIZE);

				T num;

				memcpy(&num, &m_arData[m_iIndex], sizeof(T));
				m_iIndex += sizeof(T);

				return num;
			}

			inline unsigned int GetSize() const noexcept
			{
				return m_iIndex;
			}

			inline const uint8_t *GetData() const noexcept
			{
				return &m_arData[0];
			}

			inline void Reset() noexcept
			{
				m_iIndex = 0;
			}

		private:
			uint8_t m_arData[PACKET_MAX_SIZE];

			unsigned int				m_iIndex = 0;
	};

	class PacketBuilder
	{
		public:
			inline PacketBuilder(Packet &pkt, MsgTypes msgType, const Guid &sessionToken, const Guid &configToken):
				m_Packet(pkt)
			{
				pkt.Write32(PACKET_ID);				
				pkt.Write8(static_cast<uint8_t>(msgType));
				pkt.Write(sessionToken);
				pkt.Write(configToken);
			}

			inline void WriteStr(const char *str)
			{
				size_t len = strlen(str);
				
				assert(len < 255);

				m_Packet.Write8(static_cast<uint8_t>(len));
				for (size_t i = 0; i < len; ++i)
				{
					m_Packet.Write8(str[i]);
				}
			}

		private:
			Packet &m_Packet;
	};

	class PacketReader
	{
		public:
			inline PacketReader(Packet &pkt):
				m_Packet(pkt)
			{
				//empty
			}

			inline void ReadStr(char *str, size_t max)
			{
				size_t len = m_Packet.Read<uint8_t>();

				size_t bytesToRead = len < max ? len : max;
				for (size_t i = 0; i < bytesToRead; ++i)
				{
					str[i] = m_Packet.Read<uint8_t>();
				}

				str[bytesToRead] = '\0';

				//make sure we read everything
				while (bytesToRead < len)
					m_Packet.Read<uint8_t>();
			}

		private:
			Packet &m_Packet;
	};
}
