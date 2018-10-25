#pragma once

#include "Service.h"

#include <map>
#include <string>

#include "Decoder.h"

#include "Socket.h"


class Device;

class DccLiteService : public Service
{
	public:
		DccLiteService(const ServiceClass &serviceClass, const std::string &name, const nlohmann::json &params);

		virtual ~DccLiteService();

		Decoder &Create(
			const std::string &className,
			Decoder::Address address,
			const std::string &name,
			const nlohmann::json &params
		);

		virtual void Update() override;

	private:
		dcclite::Socket m_clSocket;

		FolderObject *m_pDecoders;
		FolderObject *m_pAddresses;
		FolderObject *m_pDevices;
};


