#include "DccppService.h"

#include <Log.h>

#include "Broker.h"
#include "DccLiteService.h"
#include "NetMessenger.h"
#include "OutputDecoder.h"

using namespace dcclite;

static ServiceClass dccppService("DccppService",
	[](const ServiceClass& serviceClass, const std::string& name, Broker &broker, const rapidjson::Value& params, const Project& project) ->
	std::unique_ptr<Service> { return std::make_unique<DccppService>(serviceClass, name, broker, params, project); }
);

class DccppClient
{
	public:
		DccppClient(DccLiteService &dccLite, const Address address, Socket&& socket);
		DccppClient(const DccppClient& client) = delete;
		DccppClient(DccppClient&& other) noexcept;

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
		NetMessenger m_clMessenger;
		DccLiteService &m_rclSystem;		

		const Address	m_clAddress;
};

DccppClient::DccppClient(DccLiteService &system, const Address address, Socket&& socket) :
	m_rclSystem(system),
	m_clAddress(address),
	m_clMessenger(std::move(socket), ">")
{
	//empty
}

DccppClient::DccppClient(DccppClient&& other) noexcept:
	m_rclSystem(other.m_rclSystem),
	m_clAddress(std::move(other.m_clAddress)),
	m_clMessenger(std::move(other.m_clMessenger))
{
	//empty
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

			if (msg.compare(0, 2, "<T") == 0)
			{
				if(msg.compare("<T 100 0"))
					m_clMessenger.Send(m_clAddress, "<H 100 1>");
				else if(msg.compare("<T 100 1"))
					m_clMessenger.Send(m_clAddress, "<H 100 1>");
			}
			else if (msg.compare("<s") == 0)
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
							(dec->GetCurrentState() == dcclite::DecoderStates::ACTIVE ? 1 : 0) << 
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
			else if (msg.compare("<S") == 0)
			{
				m_clMessenger.Send(m_clAddress, "<X>");
			}
			else
			{
				m_clMessenger.Send(m_clAddress, "<X>");
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
