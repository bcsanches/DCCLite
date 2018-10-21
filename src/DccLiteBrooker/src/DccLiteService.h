#pragma once

#include "Service.h"

#include <map>
#include <string>

#include "DecoderManager.h"

#include "Socket.h"


class Device;

class DccLiteService : public Service
{
	public:
		DccLiteService(const ServiceClass &serviceClass, const std::string &name, const nlohmann::json &params);

		virtual ~DccLiteService();

		inline Decoder &Create(
			const std::string &className,
			Decoder::Address address,
			const std::string &name,
			const nlohmann::json &params
		)
		{
			return m_clDecoderManager.Create(className, address, name, params);
		}

		virtual void Update() override;

	private:		
		DecoderManager m_clDecoderManager;

		dcclite::Socket m_clSocket;
};
