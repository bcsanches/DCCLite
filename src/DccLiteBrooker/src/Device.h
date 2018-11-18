#pragma once

#include <string>

#include "Guid.h"
#include "json.hpp"
#include "Object.h"
#include "Socket.h"

class DccLiteService;
class Decoder;

class Device: public dcclite::FolderObject
{
	public:
		enum class Status
		{
			OFFLINE,
			ONLINE,
			CONFIGURING
		};

	public:
		Device(std::string name, DccLiteService &dccService, const nlohmann::json &params);
		Device(std::string name, DccLiteService &dccService);

		Device(const Device &) = delete;
		Device(Device &&) = delete;

		inline void SetStatus(Status status) noexcept
		{
			m_eStatus = status;
		}

		inline const dcclite::Guid &GetSessionToken() noexcept 
		{
			return m_SessionToken;
		}

		inline void SetSessionToken(const dcclite::Guid &guid) noexcept
		{
			m_SessionToken = guid;
		}

		inline const dcclite::Guid &GetConfigToken() noexcept
		{
			return m_ConfigToken;
		}

		inline void SetConfigToken(const dcclite::Guid &guid) noexcept
		{
			m_ConfigToken = guid;
		}

		inline void SetRemoteAddress(dcclite::Address address) noexcept
		{
			m_RemoteAddress = address;
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

		Status				m_eStatus;
};
