#include "DccppService.h"

#include <Log.h>

#include "Broker.h"
#include "Decoder.h"
#include "DccLiteService.h"
#include "NetMessenger.h"
#include "OutputDecoder.h"
#include "Parser.h"
#include "SensorDecoder.h"
#include "TurnoutDecoder.h"


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

		~DccppClient() override;

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
	m_clMessenger(std::move(socket), ">"),
	m_rclSystem(system),
	m_clAddress(address)
{
	m_rclSystem.AddListener(*this);
}

DccppClient::DccppClient(DccppClient&& other) noexcept :	
	m_clMessenger(std::move(other.m_clMessenger)),
	m_rclSystem(other.m_rclSystem),
	m_clAddress(std::move(other.m_clAddress))
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

static inline std::string CreateTurnoutDecoderResponse(const TurnoutDecoder& decoder)
{
	return fmt::format("<H{} {}>", decoder.GetAddress(), (decoder.GetRemoteState() == DecoderStates::ACTIVE ? 1 : 0));
}

static inline std::string CreateOutputDecoderResponse(const OutputDecoder& decoder)
{
	if (decoder.IsTurnoutDecoder())
	{
		return CreateTurnoutDecoderResponse(static_cast<const TurnoutDecoder&>(decoder));
	}
	else
	{
		return fmt::format("<Y {} {}>", decoder.GetAddress(), (decoder.GetRemoteState() == DecoderStates::ACTIVE ? 1 : 0));
	}
}

static inline std::string CreateDecoderResponse(const Decoder& decoder)
{	
	if (decoder.IsInputDecoder())
	{
		return fmt::format("<{} {}>", decoder.GetRemoteState() == DecoderStates::ACTIVE ? 'Q' : 'q', decoder.GetAddress());		
	}	
	else
	{
		return CreateOutputDecoderResponse(static_cast<const OutputDecoder&>(decoder));
	}
}


void DccppClient::OnDecoderStateChange(Decoder& decoder)
{
	m_clMessenger.Send(m_clAddress, CreateDecoderResponse(decoder));	
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
			dcclite::Log::Debug("[DccppClient] Received {}", msg);

			dcclite::Parser parser(msg.c_str());

			char token[128];
			if (parser.GetToken(token, sizeof(token)) != TOKEN_CMD_START)
			{
				Log::Error("[DccppClient::Update] Error parsing msg, expected TOKEN_CMD_START: {}", msg);

				goto ERROR_RESPONSE;
			}

			char cmd[4];
			if (parser.GetToken(cmd, sizeof(cmd)) != TOKEN_ID)
			{
				Log::Error("[DccppClient::Update] Error parsing msg, expected TOKEN_ID for cmd identification: {}", msg);

				goto ERROR_RESPONSE;
			}

			switch (cmd[0])
			{
				case 'c':
					//current
					m_clMessenger.Send(m_clAddress, "<a 0>");
					break;

				case 's':
				{
					std::stringstream response;
					response << "<p0><iDCC++ DccLite><N1: Ethernet><H 100 0>";

					auto turnoutDecoders = m_rclSystem.FindAllTurnoutDecoders();
					if (!turnoutDecoders.empty())
					{
						for (auto turnout : turnoutDecoders)
						{
							response << CreateTurnoutDecoderResponse(*turnout);
						}
					}
					else
					{
						response << "<X>";
					}

					auto outputDecoders = m_rclSystem.FindAllOutputDecoders();
					if (!outputDecoders.empty())
					{
						for (auto dec : outputDecoders)
						{
							response << CreateOutputDecoderResponse(*dec);							
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
				case 'Q':
					if (parser.GetToken(token, sizeof(token)) != TOKEN_EOF)
					{
						Log::Error("[DccppClient::Update] Error parsing msg, expected TOKEN_EOF for: {}", msg);

						goto ERROR_RESPONSE;
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
									static_cast<unsigned int>(dec->GetPin()) <<
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

				case 'T':
				case 'Z':
					{
						int id;
						if (parser.GetNumber(id) != TOKEN_NUMBER)
						{
							Log::Error("[DccppClient::Update] Error parsing msg, expected TOKEN_NUMBER for device id: {}", msg);

							goto ERROR_RESPONSE;
						}

						int direction;
						if (parser.GetNumber(direction) != TOKEN_NUMBER)
						{
							Log::Error("[DccppClient::Update] Error parsing msg, expected TOKEN_NUMBER for device state: {}", msg);

							goto ERROR_RESPONSE;
						}

						auto *dec = m_rclSystem.TryFindDecoder(Decoder::Address(id));
						if (!dec)
						{
							Log::Error("[DccppClient::Update] Error decoder {} not found", id);

							goto ERROR_RESPONSE;
						}

						if (!dec->IsOutputDecoder())
						{
							Log::Error("[DccppClient::Update] Error decoder {} - {} is not an output type", id, dec->GetName());

							goto ERROR_RESPONSE;
						}

						auto* outputDecoder = static_cast<OutputDecoder*>(dec);

						auto newState = direction ? DecoderStates::ACTIVE : DecoderStates::INACTIVE;

						//if no state change pending and remote state is the requested one
						if (!outputDecoder->GetPendingStateChange() && outputDecoder->GetRemoteState() == newState)
						{
							//we force an output, because we should not have a incoming state, so tell JMRI that we are on requested state
							m_clMessenger.Send(m_clAddress, CreateOutputDecoderResponse(*outputDecoder));
						}
						else
						{
							//now we set the new state and later, the decoder will update it and send a response to JMRI
							outputDecoder->SetState(newState, "DccppClient");
						}						
					}
					break;

				default:
					Log::Error("[DccppClient::Update] Unknown cmd {}, msg>: {}", cmd, msg);
					goto ERROR_RESPONSE;
					break;
			}		
		}
	}

	return true;

ERROR_RESPONSE:
	m_clMessenger.Send(m_clAddress, "<X>");
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
		throw std::runtime_error("[DccppService] Cannot open socket");
	}

	if (!m_clSocket.Listen())
	{
		throw std::runtime_error("[DccppService] Cannot put socket on listen mode");
	}

	dcclite::Log::Info("[DccppService] Started, listening on port {}", port);
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
