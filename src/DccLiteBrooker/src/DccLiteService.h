#pragma once

#include "Service.h"

#include <map>
#include <string>

#include "Decoder.h"
#include "Guid.h"
#include "Packet.h"

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

		virtual void Update(const dcclite::Clock &clock) override;		

		//
		// To be used only by Devices
		//
		//
		void Device_ConfigurePacket(dcclite::Packet &packet, dcclite::MsgTypes msgType, const dcclite::Guid &configToken);
		void Device_SendPacket(const dcclite::Address destination, const dcclite::Packet &packet);

		void Device_RegisterConfig(Device &dev, const dcclite::Guid &configToken);

	private:
		void OnNet_Hello(const dcclite::Clock &clock, const dcclite::Address &senderAddress, dcclite::Packet &packet);
		void OnNet_Ping(const dcclite::Clock &clock, const dcclite::Address &senderAddress, dcclite::Packet &packet);		

		Device *TryFindDeviceByName(std::string_view name);

		Device *TryFindDeviceByConfig(const dcclite::Guid &guid);

	private:
		dcclite::Socket m_clSocket;

		dcclite::Guid m_SessionToken;

		FolderObject *m_pDecoders;
		FolderObject *m_pAddresses;
		FolderObject *m_pDevices;
		FolderObject *m_pConfigs;
};


