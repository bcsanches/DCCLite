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

#include <Log.h>

#include <mutex>

#include "Clock.h"
#include "Packet.h"
#include "Util.h"

using namespace std::chrono_literals;

namespace dcclite::broker::ZeroconfService
{		
	static std::map<std::string_view, uint16_t> g_mapServices;
	static dcclite::Socket						g_clSocket;	
	static std::string							g_strProjectName;
	static std::mutex							g_lckMapServicesMutex;
	
	static void OpenSocket()
	{
		if (!g_clSocket.Open(9381, dcclite::Socket::Type::DATAGRAM, dcclite::Socket::FLAG_ADDRESS_REUSE | dcclite::Socket::FLAG_BLOCKING_MODE))
		{
			throw std::runtime_error("[ZeroconfService] [OpenSocket] Cannot open port 9381 for listening");
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

	static void SendReply(const NetworkAddress &destination, const std::string_view serviceName, const uint16_t port)
	{
		dcclite::BasePacket<MAX_PACKET_SIZE> packet;

		packet.Write32(PACKET_MAGIC_NUMBER);
		packet.Write8(PACKET_VERSION);
		packet.Write8(0);

		for (auto ch : serviceName)
			packet.Write8(ch);

		packet.Write8(0);
		packet.Write16(port);		

		auto projectNameLen = std::min((size_t)(MAX_SERVICE_NAME - 1), g_strProjectName.length());
		for (auto ch : g_strProjectName)
		{
			packet.Write8(ch);

			//avoid giant names
			--projectNameLen;
			if (!projectNameLen)
				break;

		}

		packet.Write8(0);

		g_clSocket.Send(destination, packet.GetData(), packet.GetSize());
	}

	static void WorkerThreadProc()
	{			
		OpenSocket();

		dcclite::BasePacket<MAX_PACKET_SIZE> packet;
		NetworkAddress sender;
		
		for (;;)
		{		
			auto startTime = dcclite::Clock::DefaultClock_t::now();

			//
			//Process 16 requests per frame, to avoid flooding
			for (int packetCount = 0; packetCount < 16; ++packetCount)
			{
				packet.Reset();

				auto [status, size] = g_clSocket.Receive(sender, packet.GetRaw(), packet.GetCapacity(), true);
				if (status != Socket::Status::OK)
				{
					//Not sure why, but when SharpTerminal stops querying, this error may happen (just wait the countdown message to see it)
					//The socket here is lost... so we reset it
					if (status == Socket::Status::CONNRESET)
					{
						OpenSocket();
					}

					continue;
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

				{
					std::lock_guard guard{ g_lckMapServicesMutex };

					auto it = g_mapServices.find(serviceName);
					if (it == g_mapServices.end())
						continue;

					//
					//found a service, send data...
					SendReply(sender, it->first, it->second);
				}				
			}

			auto endTime = dcclite::Clock::DefaultClock_t::now();
			if ((endTime - startTime) < 50ms)
			{
				//we are going too fast.... wait a bit so we do not flood if someone is attacking
				std::this_thread::sleep_for(100ms);
			}
		}			
	}		

	void Register(const std::string_view serviceName, const uint16_t port)
	{		
		if (serviceName.length() > MAX_SERVICE_NAME)
			throw std::invalid_argument(fmt::format("[ZeroconfServiceImpl] [Register] Name length cannot be greater than {} - {}", MAX_SERVICE_NAME, serviceName));		

		std::lock_guard guard{ g_lckMapServicesMutex };

		auto result = g_mapServices.emplace(std::make_pair(serviceName, port));

		if (!result.second)
			throw std::runtime_error(fmt::format("[ZeroconfServiceImpl] [Register] Service {} already registered with port {}", serviceName, result.first->second));				
	}

	
	void Start(std::string_view projectName)
	{
		if (g_clSocket.IsOpen())
			throw std::logic_error("[ZeroconfService] [Start] already called??");

		g_strProjectName = projectName;

		std::thread worker{ WorkerThreadProc };
		dcclite::SetThreadName(worker, "ZeroConfService::WorkerThread");

		worker.detach();
	}
}
