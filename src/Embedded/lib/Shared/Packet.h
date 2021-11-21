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

#include "BitPack.h"
#include "Guid.h"

namespace dcclite
{
	enum class MsgTypes: uint8_t
	{
		RESERVED0,

		DISCOVERY,
		HELLO,				
		ACCEPTED,
		CONFIG_START,
		CONFIG_DEV,
		CONFIG_FINISHED,
		CONFIG_ACK,
		MSG_PING,
		MSG_PONG,			
		STATE,
		SYNC,
		DISCONNECT
	};

	constexpr uint32_t PACKET_ID = 0xBEEFFEED;

	constexpr uint8_t PACKET_MAX_SIZE = 128;

	constexpr uint8_t MAX_DECODERS_STATES_PER_PACKET = 64;

	constexpr uint16_t PROTOCOL_VERSION = 4;

	typedef BitPack<MAX_DECODERS_STATES_PER_PACKET> StatesBitPack_t;

	inline const char *MsgName(const MsgTypes type)
	{
		switch (type)
		{
			case MsgTypes::RESERVED0:
				return "reserved0";

			case MsgTypes::DISCOVERY:
				return "discovery";

			case MsgTypes::HELLO:
				return "hello";

			case MsgTypes::ACCEPTED:
				return "accepted";

			case MsgTypes::CONFIG_START:
				return "config_start";

			case MsgTypes::CONFIG_DEV:
				return "config_dev";

			case MsgTypes::CONFIG_FINISHED:
				return "config_finished";

			case MsgTypes::CONFIG_ACK:
				return "config_ack";

			case MsgTypes::MSG_PING:
				return "msg_ping";

			case MsgTypes::MSG_PONG:
				return "msg_pong";

			case MsgTypes::STATE:
				return "state";

			case MsgTypes::SYNC:
				return "sync";

			case MsgTypes::DISCONNECT:
				return "disconnect";

			default:
				return nullptr;
		}
	}	

	template <uint32_t SIZE, typename LENSIZE = uint8_t >
	class BasePacket
	{
		public:
			inline BasePacket() noexcept = default;

			inline BasePacket(const uint8_t *data, LENSIZE len) noexcept
			{
				assert(len < SIZE);
				memcpy(&m_arData[0], data, len);
			}

			BasePacket(const BasePacket &rhs) noexcept:
				m_uIndex{ rhs.m_uIndex }
			{
				memcpy(m_arData, rhs.m_arData, sizeof(m_arData));
			}

			BasePacket(BasePacket &&) = delete;

			BasePacket &operator=(const BasePacket &) = delete;

			inline void Write8(uint8_t byte) noexcept
			{
				assert(m_uIndex + 1 < SIZE);

				m_arData[m_uIndex++] = byte;
			}

			inline void Write16(uint16_t data) noexcept
			{
				assert(m_uIndex + 2 < SIZE);

				m_arData[m_uIndex++] = ((uint8_t *)(&data))[0];
				m_arData[m_uIndex++] = ((uint8_t *)(&data))[1];
			}
			
			inline void Write32(const uint32_t data) noexcept
			{
				assert(m_uIndex + sizeof(data) < SIZE);

				m_arData[m_uIndex++] = ((uint8_t *)(&data))[0];
				m_arData[m_uIndex++] = ((uint8_t *)(&data))[1];
				m_arData[m_uIndex++] = ((uint8_t *)(&data))[2];
				m_arData[m_uIndex++] = ((uint8_t *)(&data))[3];				
			}

			inline void Write64(uint64_t data) noexcept
			{
				assert(m_uIndex + sizeof(data) < SIZE);

				m_arData[m_uIndex++] = ((uint8_t *)(&data))[0];
				m_arData[m_uIndex++] = ((uint8_t *)(&data))[1];
				m_arData[m_uIndex++] = ((uint8_t *)(&data))[2];
				m_arData[m_uIndex++] = ((uint8_t *)(&data))[3];
				m_arData[m_uIndex++] = ((uint8_t *)(&data))[4];
				m_arData[m_uIndex++] = ((uint8_t *)(&data))[5];
				m_arData[m_uIndex++] = ((uint8_t *)(&data))[6];
				m_arData[m_uIndex++] = ((uint8_t *)(&data))[7];
			}

			inline void Write(const Guid &guid) noexcept
			{
				assert(m_uIndex + sizeof(guid.m_bId) < SIZE);

				memcpy(&m_arData[m_uIndex], guid.m_bId, sizeof(guid.m_bId));
				m_uIndex += sizeof(guid.m_bId);
			}

			template <size_t BITS>
			inline void Write(const BitPack<BITS> &bitPack) noexcept
			{
				assert(m_uIndex + bitPack.GetNumBytes() < SIZE);

				memcpy(&m_arData[m_uIndex], bitPack.GetRaw(), bitPack.GetNumBytes());
				m_uIndex += bitPack.GetNumBytes();
			}

			inline Guid ReadGuid() noexcept
			{
				Guid guid;

				assert(m_uIndex + sizeof(guid.m_bId) < SIZE);

				memcpy(guid.m_bId, &m_arData[m_uIndex], sizeof(guid.m_bId));
				m_uIndex += sizeof(guid.m_bId);

				return guid;
			}

			template <size_t NBITS>
			inline void ReadBitPack(dcclite::BitPack<NBITS> &dest) noexcept
			{
				assert(m_uIndex + sizeof(dest.GetNumBytes()) < SIZE);

				dest.Set(m_arData + m_uIndex);
				m_uIndex += dest.GetNumBytes();
			}

			template <typename T>
			inline T Read() noexcept
			{				
				assert(m_uIndex + sizeof(T) < SIZE);
				
				T num;

				uint8_t *dst = (uint8_t *) &num;

				for (int i = 0; i < sizeof(num); ++i)
					dst[i] = m_arData[m_uIndex + i];
				
				m_uIndex += sizeof(T);

				return num;
			}

			inline uint8_t ReadByte() noexcept
			{
				return Read<uint8_t>();
			}						

			inline LENSIZE GetSize() const noexcept
			{
				return m_uIndex;
			}

			inline const uint8_t *GetData() const noexcept
			{
				return m_arData;
			}

			inline void Seek(LENSIZE pos) noexcept
			{
				assert(pos < SIZE);

				m_uIndex = pos;
			}

			/// <summary>
			/// Go to starting index
			/// </summary>						
			inline void Reset() noexcept
			{
				m_uIndex = 0;
			}

			inline uint8_t *GetRaw() noexcept
			{
				return m_arData;
			}

			inline LENSIZE GetCapacity() const noexcept
			{
				return SIZE;
			}

		private:
			uint8_t m_arData[SIZE];

			LENSIZE				m_uIndex = 0;
	};

	class Packet : public BasePacket<PACKET_MAX_SIZE>
	{
		public:
			inline Packet() = default;

			inline Packet(const uint8_t *data, uint8_t len):
				BasePacket{data, len}
			{
				//empty
			}

			Packet(const Packet & rhs) :
				BasePacket{rhs}				
			{
				//empty
			}
	};

	/**
		Basic packet format:

		ID MSG_TYPE SESSION_TOKEN CONFIG_TOKEN MSG


		*/
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
			inline explicit PacketReader(Packet &pkt):
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
} //end of namespace dcclite
