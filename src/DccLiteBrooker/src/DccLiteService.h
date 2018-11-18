#pragma once

#include "Service.h"

#include <map>
#include <string>

#include "Decoder.h"

#include "Socket.h"

class Device;

namespace dcclite
{
	class Packet;
}

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
		void OnNet_Hello(const dcclite::Address &senderAddress, dcclite::Packet &packet);
		void OnNet_Ping(const dcclite::Address &senderAddress, dcclite::Packet &packet);

		void SendPacket(const dcclite::Address &senderAddress, const dcclite::Packet &packet);

		Device *TryFindDeviceByName(std::string_view name);

	private:
		dcclite::Socket m_clSocket;

		FolderObject *m_pDecoders;
		FolderObject *m_pAddresses;
		FolderObject *m_pDevices;
};


