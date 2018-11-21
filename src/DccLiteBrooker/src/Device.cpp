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

	m_uLastReceivedPacket = 0;
	m_uSentPacketsAck = 0;
	m_uSequence = 0;
	m_uNextPacket = 0;

	m_vecPendingPackets.clear();

	m_SessionToken = dcclite::GuidCreate();	

	if(m_ConfigToken.IsNull())
	{
		m_ConfigToken = dcclite::GuidCreate();		

		dcclite::Log::Info("[{}::Device::AcceptConnection] Created config token {} for {}", this->GetName(), m_ConfigToken, remoteAddress);
	}

	if (remoteConfigToken != m_ConfigToken)
	{
		dcclite::Log::Info("[{}::Device::AcceptConnection] Started configuring", this->GetName());

		{
			this->ProducePacket(time, dcclite::MsgTypes::CONFIG_START);
		}

		{
			this->ProducePacket(time, dcclite::MsgTypes::CONFIG_FINISHED);
		}

		dcclite::Log::Info("[{}::Device::AcceptConnection] configured", this->GetName());
	}
	else
	{
		this->ProducePacket(time, dcclite::MsgTypes::ACCEPTED);
	}	
}

dcclite::Packet &Device::ProducePacket(dcclite::Clock::TimePoint_t time, dcclite::MsgTypes msgType)
{
	++m_uSequence;

	m_vecPendingPackets.push_back(Message(time, m_uSequence));

	auto &pkt = m_vecPendingPackets.back().m_Packet;

	dcclite::PacketBuilder builder{ pkt, msgType, m_uSequence, m_SessionToken, m_ConfigToken };

	return pkt;
}

void Device::SendPackets(dcclite::Socket &m_clSocket)
{
	for(;m_uNextPacket < m_vecPendingPackets.size(); ++m_uNextPacket)
	{
		const auto &pkt = m_vecPendingPackets[m_uNextPacket].m_Packet;

		if (!m_clSocket.Send(m_RemoteAddress, pkt.GetData(), pkt.GetSize()))
		{
			dcclite::Log::Error("[{}::Device::SendPackets] Failed to send answer to device {}", this->GetName(), m_RemoteAddress);
		}
	}
}
