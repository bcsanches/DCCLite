#include "DccppService.h"

#include <Log.h>

#include "Broker.h"
#include "DccLiteService.h"
#include "NetMessenger.h"
#include "OutputDecoder.h"
#include "SensorDecoder.h"
#include "Parser.h"

using namespace dcclite;

static ServiceClass dccppService("DccppService",
	[](const ServiceClass& serviceClass, const std::string& name, Broker& broker, const rapidjson::Value& params, const Project& project) ->
	std::unique_ptr<Service> { return std::make_unique<DccppService>(serviceClass, name, broker, params, project); }
);

class DccppClient: private IDccLiteServiceListener
{
	public:
		DccppClient(DccLiteService& dccLite, const Address address, Socket&& socket);
		DccppClient(const DccppClient& client) = delete;
		DccppClient(DccppClient&& other) noexcept;

		~DccppClient();

		DccppClient& operator=(DccppClient&& other) noexcept
		{
			if (this != &other)
			{
				m_clMessenger = std::move(other.m_clMessenger);
			}

			return *this;
		}

		bool Update();

	private:
		void OnDeviceConnected(Device& device) override;
		void OnDeviceDisconnected(Device& device) override;

		void OnDecoderStateChange(Decoder& decoder) override;

	private:
		NetMessenger m_clMessenger;
		DccLiteService& m_rclSystem;

		const Address	m_clAddress;
};

DccppClient::DccppClient(DccLiteService& system, const Address address, Socket&& socket) :
	m_rclSystem(system),
	m_clAddress(address),
	m_clMessenger(std::move(socket), ">")
{
	m_rclSystem.AddListener(*this);
}

DccppClient::DccppClient(DccppClient&& other) noexcept :
	m_rclSystem(other.m_rclSystem),
	m_clAddress(std::move(other.m_clAddress)),
	m_clMessenger(std::move(other.m_clMessenger))
{
	//empty
}

DccppClient::~DccppClient()
{
	m_rclSystem.RemoveListener(*this);
}

void DccppClient::OnDeviceConnected(Device& device)
{
	//ignore
}

void DccppClient::OnDeviceDisconnected(Device& device)
{
	//ignore
}

void DccppClient::OnDecoderStateChange(Decoder& decoder)
{
	if (decoder.IsInputDecoder())
	{
		std::stringstream msg;

		if (decoder.GetRemoteState() == DecoderStates::ACTIVE)
		{
			//from HIGH to LOW
			msg << "<Q ";
		}
		else
		{
			msg << "<q ";
		}
		
		msg << decoder.GetAddress() << '>';

		m_clMessenger.Send(m_clAddress, msg.str());
	}
	else
	{
		std::stringstream msg;

		msg << "<Y " << decoder.GetAddress() << ' ' << (decoder.GetRemoteState() == DecoderStates::ACTIVE ? 1 : 0) << '>';

		m_clMessenger.Send(m_clAddress, msg.str());
	}
}

bool DccppClient::Update()
{
	for (;;)
	{
		auto [status, msg] = m_clMessenger.Poll();

		if (status == Socket::Status::DISCONNECTED)
			return false;

		if (status == Socket::Status::WOULD_BLOCK)
			return true;

		if (status == Socket::Status::OK)
		{
			std::string response;

			dcclite::Log::Trace("[DccppClient] Received {}", msg);

			dcclite::Parser parser(msg.c_str());

			char token[128];
			if (parser.GetToken(token, sizeof(token)) != TOKEN_CMD_START)
			{
				Log::Error("[DccppClient::Update] Error parsing msg, expected TOKEN_CMD_START: {}", msg);

				m_clMessenger.Send(m_clAddress, "<X>");
			}

			char cmd[4];
			if (parser.GetToken(cmd, sizeof(cmd)) != TOKEN_ID)
			{
				Log::Error("[DccppClient::Update] Error parsing msg, expected TOKEN_ID for cmd identification: {}", msg);

				m_clMessenger.Send(m_clAddress, "<X>");
			}

			switch (cmd[0])
			{
				case 's':
				{
					std::stringstream response;
					response << "<p0><iDCC++ DccLite><N1: Ethernet><H 100 0><X>";

					auto outputDecoders = m_rclSystem.FindAllOutputDecoders();
					if (!outputDecoders.empty())
					{
						for (auto dec : outputDecoders)
						{
							response <<
								"<Y" <<
								dec->GetAddress().GetAddress() <<
								' ' <<
								(dec->GetRemoteState() == dcclite::DecoderStates::ACTIVE ? 1 : 0) <<
								'>'
								;
						}
					}
					else
					{
						response << "<X>";
					}

					m_clMessenger.Send(m_clAddress, response.str());
				}
				break;

				case 'S':					
					if (parser.GetToken(token, sizeof(token)) != TOKEN_EOF)
					{
						Log::Error("[DccppClient::Update] Error parsing msg, expected TOKEN_EOF for: {}", msg);

						m_clMessenger.Send(m_clAddress, "<X>");
					}
					else
					{
						std::stringstream response;

						auto sensorDecoders = m_rclSystem.FindAllSensorDecoders();
						if (!sensorDecoders.empty())
						{
							for (auto dec : sensorDecoders)
							{
								response <<
									"<Q" <<
									dec->GetAddress().GetAddress() <<
									' ' <<
									dec->GetPin() <<
									' ' <<
									dec->HasPullUp() <<
									'>';
							}
						}
						else
						{
							response << "<X>";
						}

						m_clMessenger.Send(m_clAddress, response.str());
					}															
					break;

				case 'Z':
					{
						int id;
						if (parser.GetNumber(id) != TOKEN_NUMBER)
						{
							Log::Error("[DccppClient::Update] Error parsing msg, expected TOKEN_NUMBER for device id: {}", msg);

							m_clMessenger.Send(m_clAddress, "<X>");
						}

						int direction;
						if (parser.GetNumber(direction) != TOKEN_NUMBER)
						{
							Log::Error("[DccppClient::Update] Error parsing msg, expected TOKEN_NUMBER for device state: {}", msg);

							m_clMessenger.Send(m_clAddress, "<X>");
						}

						auto *dec = m_rclSystem.TryFindDecoder(Decoder::Address(id));
						if (!dec)
						{
							Log::Error("[DccppClient::Update] Error decoder {} not found", id);

							m_clMessenger.Send(m_clAddress, "<X>");
						}

						if (!dec->IsOutputDecoder())
						{
							Log::Error("[DccppClient::Update] Error decoder {} - {} is not an output type", id, dec->GetName());

							m_clMessenger.Send(m_clAddress, "<X>");
						}

						auto* outputDecoder = static_cast<OutputDecoder*>(dec);

						std::stringstream response;
						response << "<Y" << id << ' ';
						
						if (direction)
						{
							outputDecoder->Activate();
							response << '1';
						}
						else
						{
							outputDecoder->Deactivate();
							response << '0';
						}
						response << '>';

						m_clMessenger.Send(m_clAddress, response.str());
					}
					break;

				default:
					Log::Error("[DccppClient::Update] Unknown cmd {}, msg>: {}", cmd, msg);
					m_clMessenger.Send(m_clAddress, "<X>");
					break;
			}		
		}
	}

	return true;
}


DccppService::DccppService(const ServiceClass& serviceClass, const std::string& name, Broker &broker, const rapidjson::Value& params, const Project& project):
	Service(serviceClass, name, broker, params, project),
	m_strDccServiceName(params["system"].GetString())
{
	//standard port used by DCC++
	int port = 2560;

	auto it = params.FindMember("port");
	if (it != params.MemberEnd())
		port = it->value.GetInt();

	if (!m_clSocket.Open(port, dcclite::Socket::Type::STREAM))
	{
		throw std::runtime_error("[TerminalService] Cannot open socket");
	}

	if (!m_clSocket.Listen())
	{
		throw std::runtime_error("[TerminalService] Cannot put socket on listen mode");
	}
}

void DccppService::Initialize()
{
	m_pclDccService = static_cast<DccLiteService *>(m_rclBroker.TryFindService(m_strDccServiceName));

	if (!m_pclDccService)
		throw std::runtime_error(fmt::format("[DccppService::Initialize] Cannot find dcc service: {}", m_strDccServiceName));
}

void DccppService::Update(const dcclite::Clock& clock)
{
	auto [status, socket, address] = m_clSocket.TryAccept();

	if (status == Socket::Status::OK)
	{
		dcclite::Log::Info("[DccppService] Client connected {}", address.GetIpString());

		assert(m_pclDccService);

		m_vecClients.emplace_back(*m_pclDccService, address, std::move(socket));
	}

	for (size_t i = 0; i < m_vecClients.size(); ++i)
	{
		auto& client = m_vecClients[i];

		if (!client.Update())
		{
			dcclite::Log::Info("[DccppService] Client disconnected");

			m_vecClients.erase(m_vecClients.begin() + i);
		}
	}
}
