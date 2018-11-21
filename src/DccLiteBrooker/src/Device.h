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

		struct Message
		{
			dcclite::Clock::TimePoint_t m_Time;

			dcclite::Packet m_Packet;

			inline Message(dcclite::Clock::TimePoint_t time) :
				m_Time{time}
			{
				//empty
			}
		};

	public:
		Device(std::string name, DccLiteService &dccService, const nlohmann::json &params);
		Device(std::string name, DccLiteService &dccService);

		Device(const Device &) = delete;
		Device(Device &&) = delete;

		void AcceptConnection(dcclite::Clock::TimePoint_t time, dcclite::Address remoteAddress, dcclite::Guid remoteSessionToken, dcclite::Guid remoteConfigToken);

		dcclite::Packet &ProducePacket(dcclite::Clock::TimePoint_t time, dcclite::MsgTypes msgType);

		void SendPackets(dcclite::Socket &m_clSocket);

		inline const dcclite::Guid &GetSessionToken() noexcept 
		{
			return m_SessionToken;
		}
		
		inline const dcclite::Guid &GetConfigToken() noexcept
		{
			return m_ConfigToken;
		}

		inline bool IsOnline() const noexcept
		{
			return m_eStatus == Status::ONLINE;
		}
		

	private:		
		DccLiteService &m_clDccService;			

		bool			m_fRegistered;

		//
		//
		//Remote Device Info
		dcclite::Guid		m_SessionToken;
		dcclite::Guid		m_ConfigToken;

		dcclite::Address	m_RemoteAddress;

		Status						m_eStatus;		

		std::vector<Message>		m_vecPendingPackets;
		size_t						m_uNextPacket = 0;
};
