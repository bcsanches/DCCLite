#include "Device.h"

#include "Decoder.h"
#include "DccLiteService.h"
#include "FmtUtils.h"
#include "GuidUtils.h"
#include "Log.h"

Device::Device(std::string name, DccLiteService &dccService, const nlohmann::json &params) :
	FolderObject(std::move(name)),
	m_clDccService(dccService),
	m_eStatus(Status::OFFLINE),
	m_fRegistered(true)
{	
	auto it = params.find("decoders");
	if (it == params.end())
		return;

	auto decodersData = *it;

	if (!decodersData.is_array())
		throw std::runtime_error("error: invalid config, expected decoders array inside Node");

	for (auto &element : decodersData)
	{
		auto decoderName = element["name"].get<std::string>();
		auto className = element["class"].get<std::string>();
		Decoder::Address address{ element["address"] };

		auto &decoder = m_clDccService.Create(className, address, decoderName, element);

		m_vecDecoders.push_back(&decoder);

		this->AddChild(std::make_unique<dcclite::Shortcut>(std::string(decoder.GetName()), decoder));
	}
}


Device::Device(std::string name, DccLiteService &dccService):
	FolderObject(std::move(name)),
	m_clDccService(dccService),
	m_eStatus(Status::OFFLINE),
	m_fRegistered(false)
{
	//empty
}

void Device::AcceptConnection(dcclite::Clock::TimePoint_t time, dcclite::Address remoteAddress, dcclite::Guid remoteSessionToken, dcclite::Guid remoteConfigToken)
{
	m_eStatus = Status::ONLINE;
	m_RemoteAddress = remoteAddress;
		
	if(m_ConfigToken.IsNull())
	{
		m_ConfigToken = dcclite::GuidCreate();		

		dcclite::Log::Info("[{}::Device::AcceptConnection] Created config token {} for {}", this->GetName(), m_ConfigToken, remoteAddress);

		m_clDccService.Device_RegisterConfig(*this, m_ConfigToken);
	}

	if (remoteConfigToken != m_ConfigToken)
	{
		dcclite::Log::Info("[{}::Device::AcceptConnection] Started configuring", this->GetName());

		dcclite::Packet pkt;
		{
			m_clDccService.Device_ConfigurePacket(pkt, dcclite::MsgTypes::CONFIG_START, m_ConfigToken);

			m_clDccService.Device_SendPacket(m_RemoteAddress, pkt);
		}

		for(size_t i = 0, sz = m_vecDecoders.size(); i < m_vecDecoders.size(); ++i)		
		{
			pkt.Reset();

			m_clDccService.Device_ConfigurePacket(pkt, dcclite::MsgTypes::CONFIG_DEV, m_ConfigToken);
			pkt.Write8(static_cast<uint8_t>(i));

			m_vecDecoders[i]->WriteConfig(pkt);

			m_clDccService.Device_SendPacket(m_RemoteAddress, pkt);
		}

		{
			pkt.Reset();

			m_clDccService.Device_ConfigurePacket(pkt, dcclite::MsgTypes::CONFIG_FINISHED, m_ConfigToken);	
			pkt.Write8(static_cast<uint8_t>(m_vecDecoders.size()));

			m_clDccService.Device_SendPacket(m_RemoteAddress, pkt);
		}

		dcclite::Log::Info("[{}::Device::AcceptConnection] configured", this->GetName());		
	}
	else
	{
		dcclite::Packet pkt;
		m_clDccService.Device_ConfigurePacket(pkt, dcclite::MsgTypes::ACCEPTED, m_ConfigToken);
		m_clDccService.Device_SendPacket(m_RemoteAddress, pkt);		
	}	
}

