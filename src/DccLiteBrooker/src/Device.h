#pragma once

#include <string>

#include "Clock.h"
#include "Guid.h"
#include "json.hpp"
#include "Object.h"
#include "Packet.h"
#include "Socket.h"

class DccLiteService;
class Decoder;

class Device: public dcclite::FolderObject
{
	public:
		enum class Status
		{
			OFFLINE,
			ONLINE			
		};

	public:
		Device(std::string name, DccLiteService &dccService, const nlohmann::json &params);
		Device(std::string name, DccLiteService &dccService);

		Device(const Device &) = delete;
		Device(Device &&) = delete;

		void AcceptConnection(dcclite::Clock::TimePoint_t time, dcclite::Address remoteAddress, dcclite::Guid remoteSessionToken, dcclite::Guid remoteConfigToken);

		dcclite::Packet &ProducePacket(dcclite::Address destination, dcclite::MsgTypes msgType);

		void SendPackets(dcclite::Socket &m_clSocket);		
		
		inline const dcclite::Guid &GetConfigToken() noexcept
		{
			return m_ConfigToken;
		}

		inline bool IsOnline() const noexcept
		{
			return m_eStatus == Status::ONLINE;
		}
		

	private:		
		DccLiteService			&m_clDccService;			

		bool					m_fRegistered;

		std::vector<Decoder *>	m_vecDecoders;

		//
		//
		//Remote Device Info		
		dcclite::Guid		m_ConfigToken;

		dcclite::Address	m_RemoteAddress;

		Status						m_eStatus;				
};
