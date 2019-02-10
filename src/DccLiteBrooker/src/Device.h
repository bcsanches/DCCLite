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
class Project;

class Device: public dcclite::FolderObject
{
	public:
		enum class Status
		{
			OFFLINE,
			ONLINE			
		};

	public:
		Device(std::string name, DccLiteService &dccService, const nlohmann::json &params, const Project &project);
		Device(std::string name, DccLiteService &dccService);

		Device(const Device &) = delete;
		Device(Device &&) = delete;

		void Update(const dcclite::Clock &clock);

		void AcceptConnection(dcclite::Clock::TimePoint_t time, dcclite::Address remoteAddress, dcclite::Guid remoteSessionToken, dcclite::Guid remoteConfigToken);

		void OnPacket_ConfigAck(dcclite::Packet &packet, dcclite::Clock::TimePoint_t time, dcclite::Address remoteAddress);
		void OnPacket_ConfigFinished(dcclite::Packet &packet, dcclite::Clock::TimePoint_t time, dcclite::Address remoteAddress);
		void OnPacket_Ping(dcclite::Packet &packet, dcclite::Clock::TimePoint_t time, dcclite::Address remoteAddress, dcclite::Guid remoteConfigToken);		

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
		bool CheckSessionConfig(dcclite::Guid remoteConfigToken, dcclite::Address remoteAddress);
		bool CheckSession(dcclite::Address remoteAddress);

		void GoOnline(dcclite::Address remoteAddress);
		void GoOffline();

		void RefreshTimeout(dcclite::Clock::TimePoint_t time);
		bool CheckTimeout(dcclite::Clock::TimePoint_t time);

		void SendDecoderConfigPacket(size_t index) const;
		void SendConfigStartPacket() const;
		void SendConfigFinishedPacket() const;
		

	private:		
		DccLiteService			&m_clDccService;			

		bool					m_fRegistered;

		std::vector<Decoder *>	m_vecDecoders;

		//
		//
		//Remote Device Info		
		dcclite::Guid		m_ConfigToken;
		dcclite::Guid		m_SessionToken;

		dcclite::Address	m_RemoteAddress;

		Status				m_eStatus;	

		dcclite::Clock::TimePoint_t m_Timeout;

		struct ConfigInfo
		{
			std::vector<bool>	m_vecAcks;

			dcclite::Clock::TimePoint_t m_RetryTime;

			uint8_t				m_uSeqCount = { 0 };			
			bool				m_fAckReceived = { false };
		};		

		std::unique_ptr<ConfigInfo> m_upConfigState;
};
