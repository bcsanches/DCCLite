// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include "BonjourService.h"

#include <Log.h>

#include "Packet.h"
#include "Socket.h"
#include "Util.h"

namespace dcclite::broker
{
	static const NetworkAddress g_clDnsAddress{ 224, 0, 0, 251, 5353 };

	class NetworkPacket
	{
		public:
			NetworkPacket() noexcept = default;

			std::tuple<Socket::Status, size_t> Receive(Socket &socket)
			{
				m_clPacket.Reset();

				return socket.Receive(m_clPacket.GetRaw(), m_clPacket.GetCapacity());				
			}

			inline uint16_t ReadWord() noexcept
			{
				return dcclite::ntohs(m_clPacket.Read<uint16_t>());
			}

			inline uint8_t ReadByte() noexcept
			{
				return m_clPacket.ReadByte();
			}

			inline void Seek(uint16_t pos) noexcept
			{
				m_clPacket.Seek(pos);
			}

			inline uint16_t GetSize() const noexcept
			{
				return m_clPacket.GetSize();
			}

		private:
			dcclite::BasePacket<512, uint16_t> m_clPacket;
	};

	/**
	* The header contains the following fields:

                                    1  1  1  1  1  1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      ID                       |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QDCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ANCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    NSCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ARCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	
	*/
	struct DnsHeader
	{
		uint16_t m_uId;
		uint16_t m_uFlags;
		uint16_t m_uQDCount;
		uint16_t m_uANCount;
		uint16_t m_uNSCount;
		uint16_t m_uARCount;

		inline bool IsResponse() const noexcept
		{
			return m_uFlags & 0x01;
		}
	};	

	struct QName
	{		
		char m_szStr[256];
	};

	struct QSection
	{
		std::vector<QName> m_vecNames;

		uint16_t m_uQType;
		uint16_t m_uQClass;
	};


	///////////////////////////////////////////////////////////////////////////////
	//
	// BonjourServiceImpl
	//
	///////////////////////////////////////////////////////////////////////////////

	class BonjourServiceImpl : public BonjourService
	{
		public:
			BonjourServiceImpl(const std::string &name, Broker &broker, const Project &project);
			~BonjourServiceImpl() override;

			void Update(const dcclite::Clock &clock) override;			

			void Serialize(JsonOutputStream_t &stream) const override;				

		private:		
			dcclite::Socket m_clSocket;
	};


	BonjourServiceImpl::BonjourServiceImpl(const std::string& name, Broker &broker, const Project& project):
		BonjourService(name, broker, project)
	{				
		if (!m_clSocket.Open(g_clDnsAddress.GetPort(), dcclite::Socket::Type::DATAGRAM, dcclite::Socket::FLAG_ADDRESS_REUSE))
		{
			throw std::runtime_error("[BonjourServiceImpl] Cannot open port 5353 for listening");
		}

		if (!m_clSocket.JoinMulticastGroup(g_clDnsAddress))
		{
			throw std::runtime_error("[BonjourServiceImpl] Cannot join multicast group");
		}
	}
	

	BonjourServiceImpl::~BonjourServiceImpl()
	{
		//empty
	}

	void BonjourServiceImpl::Update(const dcclite::Clock& clock)
	{	
		NetworkPacket packet;		
				
		auto [status, size] = packet.Receive(m_clSocket);

		if (status != dcclite::Socket::Status::OK)
			return;

		//corrupted packet?
		if (size < sizeof(DnsHeader))
			return;

		DnsHeader header;

		header.m_uId = packet.ReadWord();
		header.m_uFlags = packet.ReadWord();
		header.m_uQDCount = packet.ReadWord();
		header.m_uANCount = packet.ReadWord();
		header.m_uNSCount = packet.ReadWord();
		header.m_uARCount = packet.ReadWord();

		dcclite::Log::Trace("[BonjourServiceImpl::Update] got packet");

		/**
		* The compression scheme allows a domain name in a message to be
		*	represented as either:
		*
		*	- a sequence of labels ending in a zero octet
		*
		*	- a pointer
		*
		*	- a sequence of labels ending with a pointer
		*/		

		int previousIndex = -1;		
		for(int i = 0;i < header.m_uQDCount; ++i)
		{
			dcclite::Log::Trace("[BonjourServiceImpl::Update] QSection");
			QSection qsection;

			for (int nameCount = 0;;++nameCount)
			{
READ_NAME_AGAIN:
				uint16_t nameLen = packet.ReadByte();
				if (0 == nameLen)
				{
					//restore position if we were following a pointer
					if (previousIndex >= 0)
					{
						packet.Seek(previousIndex);
						previousIndex = -1;
					}
					break;
				}

				/**
				* The pointer takes the form of a two octet sequence:
				*
				*	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
				*	| 1  1|                OFFSET                   |
				*	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
				*
				*   is that a pointer? 
				*/								
				if (nameLen & 0xC0)
				{										
					//strip first two bits out and shift high byte
					nameLen = (~0xC0) & nameLen;
					nameLen <<= 8;

					//read next byte and concatenate it
					nameLen = nameLen | packet.ReadByte();
					
					//insane check and does not trust network, does it make sense?
					if (nameLen >= packet.GetSize())
					{
						Log::Error("[BonjourServiceImpl::Update] Got packet with pointer jumping forward, does it makes sense? Packet dropped");

						//drop packet
						return;
					}					

					//if first pointer, save current position
					if(previousIndex < 0)
						previousIndex = packet.GetSize();

					packet.Seek(nameLen);

					goto READ_NAME_AGAIN;
				}

				qsection.m_vecNames.resize(nameCount + 1);

				char *name = qsection.m_vecNames[nameCount].m_szStr;

				for (int j = 0; j < nameLen; ++j)
					name[j] = packet.ReadByte();

				name[nameLen] = '\0';				

				dcclite::Log::Trace("Name: {}", name);
			}

			uint16_t qtype = packet.ReadWord();
			uint16_t qclass = packet.ReadWord();			
		}

		if (header.m_uARCount)
		{
			dcclite::Log::Trace("[BonjourServiceImpl::Update] m_uARCount");
			uint8_t nameLen = packet.ReadByte();

#if 0
			std::string name;
			name.reserve(nameLen);

			for (int i = 0; i < nameLen; ++i)
				name[0] = packet.ReadByte();

			dcclite::Log::Trace("Got Answer: {}", name);
#endif
		}
	}

	void BonjourServiceImpl::Serialize(JsonOutputStream_t &stream) const
	{
		BonjourService::Serialize(stream);
	}

	//TODO: https://datatracker.ietf.org/doc/html/rfc6762#section-8

	
	///////////////////////////////////////////////////////////////////////////////
	//
	// ThrottleServiceImpl
	//
	///////////////////////////////////////////////////////////////////////////////

	BonjourService::BonjourService(const std::string &name, Broker &broker, const Project &project) :
		Service(name, broker, project)
	{
		//empty
	}

	std::unique_ptr<Service> BonjourService::Create(const std::string &name, Broker &broker, const Project &project)
	{
		return std::make_unique<BonjourServiceImpl>(name, broker, project);
	}
}