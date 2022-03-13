// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include "ZeroconfService.h"

#include <magic_enum.hpp>

#include <Log.h>

#include "Packet.h"
#include "Project.h"
#include "Thinker.h"

using namespace std::chrono_literals;

namespace dcclite::broker
{		
	class ZeroconfServiceImpl : public ZeroconfService
	{
		public:
			ZeroconfServiceImpl(const std::string &name, Broker &broker, const Project &project);
			~ZeroconfServiceImpl() override;			

			void Serialize(JsonOutputStream_t &stream) const override;		

			void Register(const std::string_view serviceName, const uint16_t port) override;

		private:
			void SendReply(const NetworkAddress &destination, const std::string_view serviceName, const uint16_t port);

			void Think(const dcclite::Clock::TimePoint_t ticks);

			void OpenSocket();

		private:		
			Thinker m_tThinker;

			std::map<std::string_view, uint16_t> m_mapServices;

			dcclite::Socket m_clSocket;
	};


	ZeroconfServiceImpl::ZeroconfServiceImpl(const std::string& name, Broker &broker, const Project& project):
		ZeroconfService(name, broker, project),
		m_tThinker{ {}, THINKER_MF_LAMBDA(Think) }
	{				
		this->OpenSocket();
	}
	

	ZeroconfServiceImpl::~ZeroconfServiceImpl()
	{
		//empty
	}

	void ZeroconfServiceImpl::OpenSocket()
	{
		if (!m_clSocket.Open(9381, dcclite::Socket::Type::DATAGRAM, dcclite::Socket::FLAG_ADDRESS_REUSE))
		{
			throw std::runtime_error("[ZeroconfServiceImpl] Cannot open port 9381 for listening");
		}
	}


	/**
	* Packet format

									1  1  1  1  1  1
	  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	|                     MAGIC                     |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+			
	|                     MAGIC                     |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	|        VERSION        |         FLAGS         |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+		
	|                                               |
	//                 SERVICE NAME                 //
	|                                               |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
	|                      PORT                     |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+	
	|                                               |
	//                 PROJECT NAME                 //
	|                                               |
	+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+	

	*/
	constexpr uint32_t PACKET_MAGIC_NUMBER = 0xABCDDCCA;
	constexpr uint16_t PACKET_VERSION = 0x01;
	constexpr auto PACKET_MINIMUM_SIZE = 10;

	constexpr auto MAX_SERVICE_NAME = 64;

	constexpr auto PACKET_SIZE = 80;

	constexpr auto MAX_PACKET_SIZE = 192;

	enum PacketFlags
	{
		FLAG_QUERY = 0x01
	};

	void ZeroconfServiceImpl::Think(const dcclite::Clock::TimePoint_t ticks)
	{	
		m_tThinker.SetNext(ticks + 500ms);

		dcclite::BasePacket<MAX_PACKET_SIZE> packet;
		NetworkAddress sender;

		//
		//Process 16 requests per frame, to avoid flooding
		for (int packetCount = 0; packetCount < 16; ++packetCount)
		{
			packet.Reset();

			auto [status, size] = m_clSocket.Receive(sender, packet.GetRaw(), packet.GetCapacity(), true);
			if (status != Socket::Status::OK)
			{
				//Not sure why, but when SharpTerminal stops querying, this error may happen (just wait the countdown message to see it)
				//The socket here is lost... so we reset it
				if (status == Socket::Status::CONNRESET)
				{
					this->OpenSocket();
				}

				break;
			}
				

			//Packet too small ?
			if (size < PACKET_MINIMUM_SIZE)
				continue;

			//Invalid magic number?
			if (packet.Read<uint32_t>() != PACKET_MAGIC_NUMBER)
				continue;

			//Invalid version?
			if (packet.Read<uint8_t>() != PACKET_VERSION)
				continue;

			//not a query?
			if (!(packet.Read<uint8_t>() & FLAG_QUERY))
				continue;

			char serviceName[MAX_SERVICE_NAME];			

			bool finished = false;
			for (int i = 0; i < MAX_SERVICE_NAME; ++i)
			{
				if (packet.GetSize() == size)
				{
					//overflow? Drop packet
					break;
				}

				serviceName[i] = packet.ReadByte();

				if (serviceName[i] == '\0')
				{
					finished = true;
					break;
				}
					
			}

			if (!finished)
				continue;

			auto it = m_mapServices.find(serviceName);
			if (it == m_mapServices.end())
				continue;

			//
			//found a service, send data...
			this->SendReply(sender, it->first, it->second);		
		}
		

	}

	void ZeroconfServiceImpl::SendReply(const NetworkAddress &destination, const std::string_view serviceName, const uint16_t port)
	{
		dcclite::BasePacket<MAX_PACKET_SIZE> packet;

		packet.Write32(PACKET_MAGIC_NUMBER);
		packet.Write8(PACKET_VERSION);
		packet.Write8(0);
		
		for (auto ch : serviceName)
			packet.Write8(ch);

		packet.Write8(0);
		packet.Write16(port);

		auto &projectName = this->m_rclProject.GetName();
		
		auto projectNameLen = std::min((size_t)(MAX_SERVICE_NAME - 1), projectName.length());
		for (auto ch : projectName)
		{
			packet.Write8(ch);
			
			//avoid giant names
			--projectNameLen;
			if (!projectNameLen)
				break;

		}

		packet.Write8(0);

		m_clSocket.Send(destination, packet.GetData(), packet.GetSize());
	}

	void ZeroconfServiceImpl::Serialize(JsonOutputStream_t &stream) const
	{
		ZeroconfService::Serialize(stream);
	}

	void ZeroconfServiceImpl::Register(const std::string_view serviceName, const uint16_t port)
	{		
		if (serviceName.length() > MAX_SERVICE_NAME)
			throw std::invalid_argument(fmt::format("[ZeroconfServiceImpl::Register] Name length cannot be greater than {} - {}", MAX_SERVICE_NAME, serviceName));		

		auto result = m_mapServices.emplace(std::make_pair(serviceName, port));

		if (!result.second)
			throw std::runtime_error(fmt::format("[ZeroconfServiceImpl::Register] Service {} already registered with port {}", serviceName, result.first->second));				
	}

	
	///////////////////////////////////////////////////////////////////////////////
	//
	// ZeroconfService
	//
	///////////////////////////////////////////////////////////////////////////////

	ZeroconfService::ZeroconfService(const std::string &name, Broker &broker, const Project &project) :
		Service(name, broker, project)
	{
		//empty
	}

	std::unique_ptr<Service> ZeroconfService::Create(const std::string &name, Broker &broker, const Project &project)
	{
		return std::make_unique<ZeroconfServiceImpl>(name, broker, project);
	}
}
