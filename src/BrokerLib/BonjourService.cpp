// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

/*
Relevants RFCs:

https://datatracker.ietf.org/doc/html/rfc6763 -> DNS-Based Service Discovery
https://datatracker.ietf.org/doc/html/rfc1035 -> DOMAIN NAMES - IMPLEMENTATION AND SPECIFICATION

https://datatracker.ietf.org/doc/html/rfc2782 -> A DNS RR for specifying the location of services (DNS SRV)

https://datatracker.ietf.org/doc/html/rfc6335 -> Internet Assigned Numbers Authority (IANA) Procedures for the Management
    of the Service Name and Transport Protocol Port Number Registry


*/


#include "BonjourService.h"

#include <magic_enum.hpp>

#include <Log.h>

#include "Packet.h"
#include "Socket.h"
#include "Util.h"

//#define NO_ENDIANNESS

namespace dcclite::broker
{
	static const NetworkAddress g_clDnsAddress{ 224, 0, 0, 251, 5353 };

	constexpr uint16_t QCLASS_IN = 0x01;
	constexpr uint16_t QTYPE_PTR = 12;

	constexpr const char *LOCAL_DOMAIN_NAME = "local";

	class NetworkPacket
	{
		public:
			NetworkPacket() noexcept = default;

			std::tuple<Socket::Status, size_t> Receive(Socket &socket)
			{
				m_clPacket.Reset();

				return socket.Receive(m_clPacket.GetRaw(), m_clPacket.GetCapacity());
			}

			void Send(const NetworkAddress &dest, Socket &socket)
			{
				socket.Send(dest, m_clPacket.GetData(), m_clPacket.GetSize());
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

			void WriteDoubleWord(uint32_t w)
			{
				m_clPacket.Write32(dcclite::htonl(w));
			}

			void WriteWord(uint16_t w)
			{
				m_clPacket.Write16(dcclite::htons(w));
			}

			void WriteByte(uint8_t b)
			{
				m_clPacket.Write8(b);
			}

			const uint8_t *GetData() const noexcept
			{
				return m_clPacket.GetData();
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
		uint8_t m_uFlags0;
		uint8_t m_uFlags1;
		uint16_t m_uQDCount;
		uint16_t m_uANCount;
		uint16_t m_uNSCount;
		uint16_t m_uARCount;

		inline bool IsResponse() const noexcept
		{
			return m_uFlags0 & 0x80;
		}

		inline void SetResponseBit() noexcept
		{
			m_uFlags0 |= 0x80; //bit 7
		}

		inline void SetAuthorityBit() noexcept 
		{
			m_uFlags0 |= 0x04; //bit 2
		}
	};
	

	typedef std::vector<std::string_view> LabelsVector_t;

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

	static std::string LabelsVector2String(const LabelsVector_t &vec)
	{
		std::string output;		
		
		for(const auto &it : vec)
		{			
			output.append(it);
			output.append(1, '.');
		}		

		return output;
	}

	static std::string LowerCase(std::string_view str) noexcept
	{
		std::string output;

		output.reserve(str.length());

		for (auto ch : str)
		{
			output.append(1, (ch >= 'A') && (ch <= 'Z') ? 'a' + (ch - 'A') : ch);
		}

		return output;
	}

	static std::string_view ProtocolCanonicalName(const NetworkProtocol protocol)
	{
		switch(protocol)
		{
			case NetworkProtocol::TCP:
				return "_tcp";

			case NetworkProtocol::UDP:
				return "_udp";

			default:
				throw std::out_of_range(fmt::format("[ProtocolCanonicalName] New protocol? What {} - {}", protocol, magic_enum::enum_name(protocol)));
		}
	}

	static std::optional<NetworkProtocol> ProtocolFromName(const std::string_view name)
	{
		if (name.compare("_udp") == 0)
			return NetworkProtocol::UDP;
		else if (name.compare("_tcp") == 0)
			return NetworkProtocol::TCP;
		else
			return std::optional<NetworkProtocol>{};
	}
}

namespace fmt
{
	template <>
	struct formatter<dcclite::broker::QSection>
	{
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const dcclite::broker::QSection &q, FormatContext &ctx)
		{		
			return format_to(ctx.out(), "{}", dcclite::broker::LabelsVector2String(q.m_vecLabels));
		}
	};

	template <>
	struct formatter<dcclite::broker::ResourceRecord>
	{
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const dcclite::broker::ResourceRecord &rr, FormatContext &ctx)
		{
			return format_to(ctx.out(), "{}", dcclite::broker::LabelsVector2String(rr.m_vecLabels));
		}
	};
}
namespace dcclite::broker
{
	///////////////////////////////////////////////////////////////////////////////
	//
	// BonjourServiceImpl
	//
	///////////////////////////////////////////////////////////////////////////////

	struct ServiceRecord
	{
		const std::string		m_strInstanceName;
		const std::string		m_strServiceName;

		const NetworkProtocol	m_tProtocol;
		const uint16_t			m_uPort;
		const uint32_t			m_uTtl;


		ServiceRecord(const std::string_view instanceName, const std::string_view serviceName, const NetworkProtocol protocol, const uint16_t port, const uint32_t ttl):
			m_strInstanceName{ instanceName },
			m_strServiceName{ serviceName },
			m_tProtocol{ protocol },
			m_uPort{ port },
			m_uTtl{ ttl }
		{
			//empty
		}
	};

	struct ServiceKey
	{
		const std::string m_strServiceName;
		const NetworkProtocol m_tProtocol;

		ServiceKey(std::string_view serviceName, NetworkProtocol protocol):
			m_strServiceName{ serviceName },
			m_tProtocol{ protocol }
		{
			//empty
		}

		inline bool operator<(const ServiceKey &rhs) const noexcept
		{
			if (m_tProtocol == rhs.m_tProtocol)
				return m_strServiceName.compare(rhs.m_strServiceName) < 0;

			return m_tProtocol < rhs.m_tProtocol;
		}
	};

	class BonjourServiceImpl : public BonjourService
	{
		public:
			BonjourServiceImpl(const std::string &name, Broker &broker, const Project &project);
			~BonjourServiceImpl() override;

			void Update(const dcclite::Clock &clock) override;			

			void Serialize(JsonOutputStream_t &stream) const override;		

			void Register(const std::string_view instanceName, const std::string_view serviceName, const NetworkProtocol protocol, const uint16_t port, const uint32_t ttl) override;

		private:
			void ParsePacket(NetworkPacket &packet);
			LabelsVector_t ParseName(NetworkPacket &packet);

			void ParseQuery(const DnsHeader &header, const QSection &qsection);

			static void CheckCNameSize(const std::string_view cname, const size_t maxLen = 63);

			static void CheckServiceName(const std::string_view serviceName);
			static void CheckInstanceName(const std::string_view instanceName);

			void SendServiceList(const DnsHeader &header, const QSection &query);

		private:		
			dcclite::Socket m_clSocket;

			std::map< ServiceKey, ServiceRecord> m_mapServices;
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

	void BonjourServiceImpl::CheckCNameSize(const std::string_view cname,const size_t maxLen)
	{
		const auto len = cname.length();
		if (len > maxLen)
		{
			throw std::length_error(fmt::format("[BonjourServiceImpl::Register] Service name cannot exceed 63 characters: {}", cname));
		}

		if (len == 0)
		{
			throw std::invalid_argument(fmt::format("[BonjourServiceImpl::Register] Service must have a name"));
		}
	}

	void BonjourServiceImpl::CheckServiceName(const std::string_view serviceName)
	{
		//should we use a regex?

		//https://datatracker.ietf.org/doc/html/rfc6335
		//MUST be at least 1 character and no more than 15 characters long
		CheckCNameSize(serviceName, 15);

		if (serviceName.find_first_of('.', 0) != std::string_view::npos)
		{
			throw std::invalid_argument(fmt::format("[BonjourServiceImpl::Register] Service name cannot contain '.' (dots): {}", serviceName));
		}

		if (serviceName.find_first_of(' ', 0) != std::string_view::npos)
		{
			throw std::invalid_argument(fmt::format("[BonjourServiceImpl::Register] Service name cannot contain ' ' (whitespace): {}", serviceName));
		}

		//MUST NOT begin or end with a hyphen
		if ((serviceName[0] == '-') || (serviceName[serviceName.length() - 1] == '-'))
		{
			throw std::invalid_argument(fmt::format("[BonjourServiceImpl::Register] Service name cannot starts or ends with '-' (hifen): {}", serviceName));
		}

		for(auto it = serviceName.cbegin(), end = serviceName.cend(); it != end; ++it)		
		{
			char ch = *it;

			//hyphens MUST NOT be adjacent to other hyphens
			if((ch == '-') && (it != end) && (*(it+1) == '-'))
				throw std::invalid_argument(fmt::format("[BonjourServiceImpl::Register] Service name cannot contains adjacent '-'.", serviceName));

			if (((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')) || ((ch >= '0') && (ch <= '9')) || (ch == '-'))
				continue;

			throw std::invalid_argument(fmt::format("[BonjourServiceImpl::Register] Service contains invalid characters, only letters and numbers allowed: {}", serviceName));
		}
	}
	void BonjourServiceImpl::CheckInstanceName(const std::string_view instanceName)
	{	
		CheckCNameSize(instanceName);

		//
		// 
		//https://datatracker.ietf.org/doc/html/rfc6763#section-4.1.1
		//
		//It MUST NOT contain ASCII control characters(byte values 0x00 - 0x1F and 0x7F)[RFC20] but otherwise is allowed to contain any characters
		//
		for (auto ch : instanceName)
		{
			if (((ch >= 0x00) && (ch <= 0x1F)) || (ch == 0x7F))
			{
				throw std::invalid_argument(fmt::format("[BonjourServiceImpl::Register] Instance name contains invalid characters: {}", instanceName));
			}
		}
	}

	void BonjourServiceImpl::Register(const std::string_view instanceName, const std::string_view serviceName, const NetworkProtocol protocol, const uint16_t port, const uint32_t ttl)
	{			
		CheckInstanceName(instanceName);
		CheckServiceName(serviceName);

		std::string localInstanceName = LowerCase(instanceName);
		std::string localServiceName = serviceName[0] == '_' ? LowerCase(serviceName) : "_" + LowerCase(serviceName);

		auto r = m_mapServices.emplace(std::make_pair(ServiceKey{ localServiceName, protocol }, ServiceRecord{ std::move(localInstanceName), std::move(localServiceName), protocol, port, ttl }));
		if (!r.second)
		{
			throw std::invalid_argument(fmt::format("[BonjourServiceImpl::Register] Service {} already exists for protocol {}", serviceName, magic_enum::enum_name(protocol)));
		}
	}

	void BonjourServiceImpl::ParseQuery(const DnsHeader &header, const QSection &qsection)
	{
		const auto numLabels = qsection.m_vecLabels.size();

		if (numLabels < 4)
		{
			Log::Debug("[BonjourServiceImpl::ParseQuery] Discarding query for: {}, it at least should contain <instanceName>.<serviceName>.<protocol>.<_local>", qsection);

			return;
		}			

		assert(numLabels);

		//Discard first bit and check class
		if ((qsection.m_uQClass & 0x7FFF) != QCLASS_IN)
		{
			Log::Debug("[BonjourServiceImpl::ParseQuery] Discarding query for: {}, qclass is not IN: {}", qsection, qsection.m_uQClass);			

			return;
		}			

		if (qsection.m_uQType != QTYPE_PTR)
		{
			Log::Debug("[BonjourServiceImpl::ParseQuery] Discarding query for: {}, qtype is not PTR: {}", qsection, qsection.m_uQType);

			return;
		}

		//not a query for local name?
		if (qsection.m_vecLabels[numLabels - 1].compare(LOCAL_DOMAIN_NAME))
			return;

		auto protocol = ProtocolFromName(qsection.m_vecLabels[numLabels - 2]);
		if (!protocol.has_value())
		{
			Log::Error("[BonjourServiceImpl::ParseQuery] Unknown protocol {}", qsection.m_vecLabels[numLabels - 2]);

			return;
		}

		//
		// detect _services._dns-sd._udp.local.
		if ((protocol.value() == NetworkProtocol::UDP) && (qsection.m_vecLabels[1].compare("_dns-sd") == 0) && (qsection.m_vecLabels[0].compare("_services") == 0))
		{
			//list all services
			Log::Trace("[[BonjourServiceImpl::ParseQuery] received a _services._dns-sd._udp.local.");

			this->SendServiceList(header, qsection);

			return;
		}

		//
		//Not a generic query, so lookup local services
		ServiceKey key{ qsection.m_vecLabels[numLabels - 3], protocol.value() };

		auto it = m_mapServices.find(key);
		if (it == m_mapServices.end())
			return;

		//
		//Service found, advertise it

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

			//
			//names are case insensitive, so make all lower case
			char *str = const_cast<char *>(reinterpret_cast<const char *>(packet.GetData())) + packet.GetSize();
			for (int i = 0; i < nameLen; ++i)
				str[i] = tolower(str[i]);

			std::string_view name{ str, nameLen };			

			results.push_back(name);
			packet.Seek(packet.GetSize() + nameLen);			

			dcclite::Log::Trace("Name: {}", results[results.size()-1]);
		}

		return results;
	}

	void BonjourServiceImpl::ParsePacket(NetworkPacket &packet)
	{
		DnsHeader header;

		header.m_uId = packet.ReadWord();
		header.m_uFlags0 = packet.ReadByte();
		header.m_uFlags1 = packet.ReadByte();

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
		
		QSection qsection;
		for (int i = 0; i < header.m_uQDCount; ++i)
		{			
			qsection.m_vecLabels.clear();

			qsection.m_vecLabels = this->ParseName(packet);

			//if labels are empty, something happened, so drop packet...
			if (qsection.m_vecLabels.empty())
				return;			

			qsection.m_uQType = packet.ReadWord();
			qsection.m_uQClass = packet.ReadWord();

			this->ParseQuery(header, qsection);
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

		auto numServices = m_mapServices.size();

		//Limit processing per frame in case something is flooding the network....
		for (auto packetCount = 0; packetCount < 8; ++packetCount)
		{
			auto [status, size] = packet.Receive(m_clSocket);

			if (status != dcclite::Socket::Status::OK)
				return;

			//corrupted packet?
			if (size < sizeof(DnsHeader))
				continue;

			//Is there any reason to parse packets if no services are registered?
			if (numServices == 0)
				continue;

			this->ParsePacket(packet);
		}								
	}

	void BonjourServiceImpl::Serialize(JsonOutputStream_t &stream) const
	{
		BonjourService::Serialize(stream);
	}

	//TODO: https://datatracker.ietf.org/doc/html/rfc6762#section-8
	//TODO: https://github.com/richardschneider/net-mdns

	class DnsMesssageWriter
	{
		public:
			DnsMesssageWriter(const DnsHeader &header)
			{
				m_clPacket.WriteWord(header.m_uId);
				m_clPacket.WriteByte(header.m_uFlags0);
				m_clPacket.WriteByte(header.m_uFlags1);
				m_clPacket.WriteWord(header.m_uQDCount);
				m_clPacket.WriteWord(header.m_uANCount);
				m_clPacket.WriteWord(header.m_uNSCount);
				m_clPacket.WriteWord(header.m_uARCount);
			}

			void WriteNames(const LabelsVector_t &names)
			{
				for (auto it : names)
				{
					this->WriteName(it);
				}

				m_clPacket.WriteByte(0);
			}

			void WriteName(const std::string_view name)
			{
#ifdef COMPRESS_NAME
				auto it = m_mapNameCache.find(name);
				if (it != m_mapNameCache.end())
				{
					//
					//make a name pointer
					uint16_t pointer = it->second;
					pointer |= 0xC0;

					m_clPacket.WriteWord(pointer);
				}
				else
				{			
					const uint16_t pos = m_clPacket.GetSize();

					//should never happen...
					if (name.length() > 63)
						throw std::out_of_range(fmt::format("[DnsMesssageWriter::WriteName] Name cannot exceed 63 bytes - {}", name));

					auto len = name.length();
					m_clPacket.WriteByte(len);
					
					for (auto ch : name)
					{
						m_clPacket.WriteByte(ch);
					}				
					
					m_mapNameCache.emplace(
						std::string_view{ m_clPacket.GetData() + pos, len }, 
						pos
					);
				}
#else
				//should never happen...
				if (name.length() > 63)
					throw std::out_of_range(fmt::format("[DnsMesssageWriter::WriteName] Name cannot exceed 63 bytes - {}", name));

				auto len = static_cast<uint8_t>(name.length());
				m_clPacket.WriteByte(len);

				for (auto ch : name)
				{
					m_clPacket.WriteByte(ch);
				}
#endif				
			}		

			inline void WriteByte(const uint8_t byte) noexcept
			{
				m_clPacket.WriteByte(byte);
			}

			inline void WriteWord(const uint16_t word) noexcept
			{
				m_clPacket.WriteWord(word);
			}

			inline void WriteDoubleWord(const uint32_t dword) noexcept
			{
				m_clPacket.WriteDoubleWord(dword);
			}

			inline uint16_t GetSize() const noexcept
			{
				return m_clPacket.GetSize();
			}

			inline void Seek(uint16_t newPos)
			{
				m_clPacket.Seek(newPos);
			}

			inline void Send(Socket &socket)
			{
				m_clPacket.Send(g_clDnsAddress, socket);				
			}

		private:
			NetworkPacket m_clPacket;			

#ifdef COMPRESS_NAME
			std::map<std::string_view, uint16_t> m_mapNameCache;
#endif
	};

	void BonjourServiceImpl::SendServiceList(const DnsHeader &header, const QSection &query)
	{
		DnsHeader rheader = { 0 };

		rheader.m_uId = header.m_uId;
		rheader.SetResponseBit();
		rheader.SetAuthorityBit();

		auto numServices = m_mapServices.size();
		
		rheader.m_uANCount = static_cast<uint16_t>(numServices);

		DnsMesssageWriter writer{ rheader };

		for (auto &service : m_mapServices)
		{
			writer.WriteNames(query.m_vecLabels);

			writer.WriteWord(QTYPE_PTR);
			writer.WriteWord(QCLASS_IN);
			writer.WriteDoubleWord(service.second.m_uTtl);

			//write a zero length, we will update it later with the position
			auto lengthPos = writer.GetSize();
			writer.WriteWord(0);

			writer.WriteName(service.second.m_strServiceName);
			writer.WriteName(ProtocolCanonicalName(service.second.m_tProtocol));
			writer.WriteName(LOCAL_DOMAIN_NAME);

			//mark end of names
			writer.WriteByte(0);
			
			auto finalPos = writer.GetSize();

			//update length and return to current position
			writer.Seek(lengthPos);
			writer.WriteWord(finalPos - lengthPos - 2);
			writer.Seek(finalPos);
		}

		writer.Send(m_clSocket);
	}
	
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
