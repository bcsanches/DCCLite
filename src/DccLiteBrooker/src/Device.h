#pragma once

#include <string>

#include "json.hpp"

class DccLiteService;
class Decoder;

class Device
{
	public:
		enum class Status
		{
			OFFLINE,
			ONLINE
		};

	public:
		Device(const std::string &name, DccLiteService &dccService, const nlohmann::json &params);

		Device(const Device &) = delete;
		Device(Device &&) = delete;

	private:
		std::string m_strName;

		DccLiteService &m_clDccService;

		std::vector<Decoder *> m_vecDecoders;

		Status					m_eStatus;
};
