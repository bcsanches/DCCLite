#pragma once

#include <string>

#include "Object.h"

#include "json.hpp"

class DccLiteService;
class Decoder;

class Device: public dcclite::Object
{
	public:
		enum class Status
		{
			OFFLINE,
			ONLINE
		};

	public:
		Device(std::string name, DccLiteService &dccService, const nlohmann::json &params);

		Device(const Device &) = delete;
		Device(Device &&) = delete;

	private:		
		DccLiteService &m_clDccService;

		std::vector<Decoder *> m_vecDecoders;

		Status					m_eStatus;
};
