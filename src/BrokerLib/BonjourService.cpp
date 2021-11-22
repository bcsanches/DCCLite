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

			inline uint32_t ReadDoubleWord() noexcept
			{
				return dcclite::ntohl(m_clPacket.Read<uint32_t>());
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

	struct Label
	{		
		char m_szStr[64];
	};

	typedef std::vector<Label> LabelsVector_t;

	struct QSection
	{
		LabelsVector_t m_vecLabels;

		uint16_t m_uQType;
		uint16_t m_uQClass;
	};

	struct ResourceRecord
	{
		LabelsVector_t m_vecLabels;

		uint16_t m_uType;
		uint16_t m_uClass;
		uint32_t m_uTTL;
		uint16_t m_uLength;
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

			void Register(std::string_view serviceName, NetworkProtocol protocol, uint16_t port) override;

		private:
			void ParsePacket(NetworkPacket &packet);
			LabelsVector_t ParseName(NetworkPacket &packet);

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

	void BonjourServiceImpl::Register(std::string_view serviceName, NetworkProtocol protocol, uint16_t port)
	{			
		//should we use a regex?
		const auto len = serviceName.length();
		if (len > 63)
		{
			throw std::length_error(fmt::format("[BonjourServiceImpl::Register] Service name cannot exceed 63 characters: {}", serviceName));
		}

		if (len == 0)
		{
			throw std::invalid_argument(fmt::format("[BonjourServiceImpl::Register] Service must have a name"));
		}

		if (serviceName.find_first_of('.', 0) != std::string_view::npos)
		{
			throw std::invalid_argument(fmt::format("[BonjourServiceImpl::Register] Service name cannot contain '.' (dots): {}", serviceName));
		}

		if (serviceName.find_first_of(' ', 0) != std::string_view::npos)
		{
			throw std::invalid_argument(fmt::format("[BonjourServiceImpl::Register] Service name cannot contain ' ' (whitespace): {}", serviceName));
		}

		if ((serviceName[0] == '-') || (serviceName[len - 1] == '-'))
		{
			throw std::invalid_argument(fmt::format("[BonjourServiceImpl::Register] Service name cannot starts or ends with '-' (hifen): {}", serviceName));
		}
		
		for (auto ch : serviceName)
		{
			if (((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')) || ((ch >= '0') && (ch <= '9')) || (ch == '-'))
				continue;

			throw std::invalid_argument(fmt::format("[BonjourServiceImpl::Register] Service contains invalid characters, only letters and numbers allowed: {}", serviceName));
		}
	}

	LabelsVector_t BonjourServiceImpl::ParseName(NetworkPacket &packet)
	{
		LabelsVector_t results;

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
		
		for (int nameCount = 0;; ++nameCount)
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

					//drop packet, so return a empty result
					return LabelsVector_t{};
				}

				//if first pointer, save current position
				if (previousIndex < 0)
					previousIndex = packet.GetSize();

				packet.Seek(nameLen);

				goto READ_NAME_AGAIN;
			}

			results.resize(nameCount + 1);

			char *name = results[nameCount].m_szStr;

			for (int j = 0; j < nameLen; ++j)
				name[j] = packet.ReadByte();

			name[nameLen] = '\0';

			dcclite::Log::Trace("Name: {}", name);
		}

		return results;
	}

	void BonjourServiceImpl::ParsePacket(NetworkPacket &packet)
	{
		DnsHeader header;

		header.m_uId = packet.ReadWord();
		header.m_uFlags = packet.ReadWord();

#if 0
		//we do not care about responses...
		if (header.IsResponse())
			return;
#endif

		header.m_uQDCount = packet.ReadWord();
		header.m_uANCount = packet.ReadWord();
		header.m_uNSCount = packet.ReadWord();
		header.m_uARCount = packet.ReadWord();

		dcclite::Log::Trace("[BonjourServiceImpl::Update] got packet");
		
		for (int i = 0; i < header.m_uQDCount; ++i)
		{
			dcclite::Log::Trace("[BonjourServiceImpl::Update] QSection");
			QSection qsection;

			qsection.m_vecLabels = this->ParseName(packet);

			//if labels are empty, something happened, so drop packet...
			if (qsection.m_vecLabels.empty())
				return;			

			uint16_t qtype = packet.ReadWord();
			uint16_t qclass = packet.ReadWord();
		}

		while (header.m_uANCount)
		{
			ResourceRecord record;

			dcclite::Log::Trace("[BonjourServiceImpl::Update] RRecord - m_uANCount");

			record.m_vecLabels = this->ParseName(packet);

			record.m_uType = packet.ReadWord();
			record.m_uClass = packet.ReadWord();
			record.m_uTTL = packet.ReadDoubleWord();
			record.m_uLength = packet.ReadWord();

			//PTR?
			if (record.m_uType == 12)
			{
				dcclite::Log::Trace("[BonjourServiceImpl::Update] Answer names:");

				LabelsVector_t labels = this->ParseName(packet);
			}	
			//https://www.rfc-editor.org/rfc/rfc2782.html
			else if (record.m_uType == 33)
			{
				//skip data
				packet.Seek(packet.GetSize() + record.m_uLength);
			}
			else
			{
				//skip data
				packet.Seek(packet.GetSize() + record.m_uLength);
			}

			--header.m_uANCount;
		}

		while (header.m_uNSCount)
		{
			ResourceRecord record;

			dcclite::Log::Trace("[BonjourServiceImpl::Update] RRecord - NSCount");

			record.m_vecLabels = this->ParseName(packet);

			record.m_uType = packet.ReadWord();
			record.m_uClass = packet.ReadWord();
			record.m_uTTL = packet.ReadDoubleWord();
			record.m_uLength = packet.ReadWord();

			//skip data
			packet.Seek(packet.GetSize() + record.m_uLength);

			--header.m_uNSCount;
		}

		while (header.m_uARCount)
		{
			ResourceRecord record;

			dcclite::Log::Trace("[BonjourServiceImpl::Update] RRecord - m_uARCount");

			record.m_vecLabels = this->ParseName(packet);

			record.m_uType = packet.ReadWord();
			record.m_uClass = packet.ReadWord();
			record.m_uTTL = packet.ReadDoubleWord();
			record.m_uLength = packet.ReadWord();

			//skip data
			packet.Seek(packet.GetSize() + record.m_uLength);

			--header.m_uARCount;
		}
	}

	void BonjourServiceImpl::Update(const dcclite::Clock& clock)
	{	
		NetworkPacket packet;

		//Limit processing per frame in case something is flooding the network....
		for (auto packetCount = 0; packetCount < 8; ++packetCount)
		{
			auto [status, size] = packet.Receive(m_clSocket);

			if (status != dcclite::Socket::Status::OK)
				return;

			//corrupted packet?
			if (size < sizeof(DnsHeader))
				continue;

			this->ParsePacket(packet);
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