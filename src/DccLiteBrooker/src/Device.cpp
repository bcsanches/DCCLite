#include "Device.h"

#include <fstream>

#include "Decoder.h"
#include "DccLiteService.h"
#include "FmtUtils.h"
#include "GuidUtils.h"
#include "Log.h"
#include "Project.h"

using namespace std::chrono_literals;

static auto constexpr TIMEOUT = 10s;
static auto constexpr CONFIG_RETRY_TIME = 300ms;

Device::Device(std::string name, DccLiteService &dccService, const nlohmann::json &params, const Project &project) :
	FolderObject(std::move(name)),
	m_clDccService(dccService),
	m_eStatus(Status::OFFLINE),
	m_fRegistered(true)
{	
	const std::string deviceConfigFileName(std::string(this->GetName()) + ".decoders.json");

	const auto deviceConfigFilePath = project.GetFilePath(deviceConfigFileName);
	std::ifstream configFile(deviceConfigFilePath);
	if (!configFile)
	{
		dcclite::Log::Error("Device cannot find {}", deviceConfigFilePath.string());

		return;
	}

	m_ConfigToken = project.GetFileToken(deviceConfigFileName);	

	dcclite::Log::Trace("Device {} config token {}", this->GetName(), m_ConfigToken);
	dcclite::Log::Trace("Device {} reading config {}", this->GetName(), deviceConfigFilePath.string());

	nlohmann::json decodersData;

	configFile >> decodersData;

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

	dcclite::Log::Trace("Device {} ready.", this->GetName());
}


Device::Device(std::string name, DccLiteService &dccService):
	FolderObject(std::move(name)),
	m_clDccService(dccService),
	m_eStatus(Status::OFFLINE),
	m_fRegistered(false)
{
	//empty
}

void Device::GoOnline(dcclite::Address remoteAddress)
{	
	m_RemoteAddress = remoteAddress;
	m_SessionToken = dcclite::GuidCreate();
	m_clDccService.Device_RegisterSession(*this, m_SessionToken);

	m_eStatus = Status::ONLINE;	

	dcclite::Log::Info("[{}::Device::GoOnline] Is online", this->GetName());
}

void Device::GoOffline()
{
	m_eStatus = Status::OFFLINE;	

	m_clDccService.Device_UnregisterSession(m_SessionToken);
	m_SessionToken = dcclite::Guid{};
	m_upConfigState.reset();

	dcclite::Log::Warn("[{}::Device::GoOffline] Is OFFLINE", this->GetName());
}

void Device::SendDecoderConfigPacket(size_t index) const
{
	dcclite::Packet pkt;

	m_clDccService.Device_PreparePacket(pkt, dcclite::MsgTypes::CONFIG_DEV, m_SessionToken, m_ConfigToken);
	pkt.Write8(static_cast<uint8_t>(index));

	m_vecDecoders[index]->WriteConfig(pkt);

	m_clDccService.Device_SendPacket(m_RemoteAddress, pkt);
}

void Device::SendConfigFinishedPacket() const
{
	dcclite::Packet pkt;

	m_clDccService.Device_PreparePacket(pkt, dcclite::MsgTypes::CONFIG_FINISHED, m_SessionToken, m_ConfigToken);
	pkt.Write8(static_cast<uint8_t>(m_vecDecoders.size()));

	m_clDccService.Device_SendPacket(m_RemoteAddress, pkt);
}

void Device::SendConfigStartPacket() const
{
	dcclite::Packet pkt;

	m_clDccService.Device_PreparePacket(pkt, dcclite::MsgTypes::CONFIG_START, m_SessionToken, m_ConfigToken);

	m_clDccService.Device_SendPacket(m_RemoteAddress, pkt);
}

void Device::AcceptConnection(dcclite::Clock::TimePoint_t time, dcclite::Address remoteAddress, dcclite::Guid remoteSessionToken, dcclite::Guid remoteConfigToken)
{
	if (m_eStatus == Status::ONLINE)
	{
		dcclite::Log::Error("[{}::Device::AcceptConnection] Already connected, cannot accept request from {}", this->GetName(), remoteAddress);

		return;
	}

	this->GoOnline(remoteAddress);	

	this->RefreshTimeout(time);

	if (remoteConfigToken != m_ConfigToken)
	{
		dcclite::Log::Info("[{}::Device::AcceptConnection] Started configuring for token {}", this->GetName(), m_ConfigToken);

		m_upConfigState = std::make_unique<ConfigInfo>();
		m_upConfigState->m_vecAcks.resize(m_vecDecoders.size());

		m_upConfigState->m_RetryTime = time + CONFIG_RETRY_TIME;		
		
		this->SendConfigStartPacket();

		for(size_t i = 0, sz = m_vecDecoders.size(); i < m_vecDecoders.size(); ++i)		
		{
			this->SendDecoderConfigPacket(i);
		}		

		dcclite::Log::Info("[{}::Device::AcceptConnection] config data sent", this->GetName());		
	}
	else
	{
		dcclite::Packet pkt;
		m_clDccService.Device_PreparePacket(pkt, dcclite::MsgTypes::ACCEPTED, m_SessionToken, m_ConfigToken);
		m_clDccService.Device_SendPacket(m_RemoteAddress, pkt);		
	}	
}

void Device::OnPacket_Ping(dcclite::Packet &packet, dcclite::Clock::TimePoint_t time, dcclite::Address remoteAddress, dcclite::Guid remoteConfigToken)
{
	if (!this->CheckSessionConfig(remoteConfigToken, remoteAddress))
		return;

	dcclite::Packet pkt;
	m_clDccService.Device_PreparePacket(pkt, dcclite::MsgTypes::MSG_PONG, m_SessionToken, m_ConfigToken);
	m_clDccService.Device_SendPacket(m_RemoteAddress, pkt);	

	this->RefreshTimeout(time);

	// dcclite::Log::Trace("[{}::Device::OnPacket_Ping] ping", this->GetName());
}

void Device::OnPacket_ConfigAck(dcclite::Packet &packet, dcclite::Clock::TimePoint_t time, dcclite::Address remoteAddress)
{
	if (!this->CheckSession(remoteAddress))
		return;	

	auto seq = packet.Read<uint8_t>();

	if ((!m_upConfigState) || (seq >= m_upConfigState->m_vecAcks.size()))
	{
		dcclite::Log::Error("[{}::Device::OnPacket_ConfigAck] config out of sync, dropping connection", this->GetName());

		this->GoOffline();
	}

	//only increment seq count if m_vecAcks[seq] is not set yet, so we handle duplicate packets
	m_upConfigState->m_uSeqCount += m_upConfigState->m_vecAcks[seq] == false;
	m_upConfigState->m_vecAcks[seq] = true;	
	RefreshTimeout(time);	

	m_upConfigState->m_RetryTime = time + CONFIG_RETRY_TIME;

	dcclite::Log::Trace("[{}::Device::OnPacket_ConfigAck] Config ACK {}", this->GetName(), seq);

	if (m_upConfigState->m_uSeqCount == m_upConfigState->m_vecAcks.size())
	{
		dcclite::Log::Info("[{}::Device::OnPacket_ConfigAck] Config Finished, configured {} decoders", this->GetName(), m_upConfigState->m_uSeqCount);

		this->SendConfigFinishedPacket();
	}
}

void Device::OnPacket_ConfigFinished(dcclite::Packet &packet, dcclite::Clock::TimePoint_t time, dcclite::Address remoteAddress)
{
	if (!this->CheckSession(remoteAddress))
		return;

	m_upConfigState.reset();
	dcclite::Log::Trace("[{}::Device::OnPacket_ConfigFinished] Config Finished, device is ready", this->GetName());

	RefreshTimeout(time);
}

bool Device::CheckSessionConfig(dcclite::Guid remoteConfigToken, dcclite::Address remoteAddress)
{
	if (!this->CheckSession(remoteAddress))
		return false;

	if (remoteConfigToken != m_ConfigToken)
	{
		dcclite::Log::Warn("[{}::Device::CheckSession] Received packet from invalid config...", this->GetName());

		return false;
	}

	return true;
}



bool Device::CheckSession(dcclite::Address remoteAddress)
{
	if (m_eStatus != Status::ONLINE)
	{
		dcclite::Log::Error("[{}::Device::CheckSession] got packet from disconnected device", this->GetName());

		return false;
	}	

	if (remoteAddress != m_RemoteAddress)
	{
		dcclite::Log::Warn("[{}::Device::CheckSession] Updating remote address, session valid", this->GetName());

		m_RemoteAddress = remoteAddress;
	}

	return true;
}

void Device::RefreshTimeout(dcclite::Clock::TimePoint_t time)
{
	m_Timeout = time + TIMEOUT;
}

bool Device::CheckTimeout(dcclite::Clock::TimePoint_t time)
{	
	if (time > m_Timeout)
	{
		dcclite::Log::Warn("[{}::Device::Update] timeout", this->GetName());

		this->GoOffline();

		return false;
	}

	return true;
}

void Device::Update(const dcclite::Clock &clock)
{
	if (m_eStatus == Status::ONLINE)
	{
		auto time = clock.Now();

		if (!this->CheckTimeout(time))
			return;

		if (m_upConfigState && (m_upConfigState->m_RetryTime < time))
		{
			//we havent received ack for some time, wake up the device (timeout keeps counting)
			
			//go thought all the decoders and check what does not have an ACK
			int pos = 0, packetCount = 0;
			for(const bool &acked : m_upConfigState->m_vecAcks)
			{
				if (!acked)
				{
					if (pos == 0)
					{
						//when config 0 is not received, this could also means that CONFIG_START was not received by tge remote, so we send it again
						this->SendConfigStartPacket();
					}

					dcclite::Log::Warn("[{}::Device::Update] retrying config for device {} at {}", this->GetName(), m_vecDecoders[pos]->GetName(), pos);
					this->SendDecoderConfigPacket(pos);

					++packetCount;

					//too many packets? Wait a little bit
					if (packetCount == 3)
						break;
				}

				++pos;
			}		
						
			if (packetCount == 0)
			{
				//remote device already acked all decoders, but not acked the config finished, so, send it again
				dcclite::Log::Warn("[{}::Device::Update] retrying config finished for remote device", this->GetName());
				this->SendConfigFinishedPacket();
			}						
		}
	}
}