#include "DccppService.h"

#include <FmtUtils.h>
#include <Log.h>

#include "../sys/BonjourService.h"
#include "../sys/Broker.h"
#include "../sys/EventHub.h"
#include "../sys/ZeroConfSystem.h"

#include "Decoder.h"
#include "DccLiteService.h"
#include "JsonUtils.h"
#include "NetMessenger.h"
#include "NmraUtil.h"
#include "SignalDecoder.h"
#include "SimpleOutputDecoder.h"
#include "Parser.h"
#include "SensorDecoder.h"
#include "TurnoutDecoder.h"
#include "Util.h"

using namespace std::chrono_literals;

namespace dcclite::broker
{		
	class DccppClient;

	class DccppServiceImplClientProxy
	{
		public:
			virtual void Async_ClientDisconnected(DccppClient &client) = 0;
	};

	class DccppClient: private IObjectManagerListener, public EventHub::IEventTarget
	{
		public:
			DccppClient(DccppServiceImplClientProxy &owner, DccLiteService &dccLite, const NetworkAddress address, Socket&& socket);

			DccppClient(const DccppClient& client) = delete;
			DccppClient(DccppClient&& other) = delete;

			~DccppClient() override;

			DccppClient& operator=(DccppClient&& other) noexcept
			{
				if (this != &other)
				{
					m_clMessenger = std::move(other.m_clMessenger);
				}

				return *this;
			}			

		private:			
			void OnObjectManagerEvent(const ObjectManagerEvent &event) override;

			void ParseStatusCommand(dcclite::Parser &parser, const std::string &msg);
			bool ParseSensorCommand(dcclite::Parser &parser, const std::string &msg);

			bool CreateTurnoutsStateResponse(std::stringstream &stream) const;

			bool CreateTurnoutsIdListResponse(std::stringstream &stream) const;

			bool CreateTurnoutsDefResponse(std::stringstream &stream) const;
			bool CreateOutputsDefResponse(std::stringstream &stream) const;

			void OnMessage(const std::string &msg);

			void ThreadProc();		

			class ClientEvent: public EventHub::IEvent
			{
				public:
					ClientEvent(DccppClient &target, std::string msg):
						IEvent(target),
						m_strMessage(std::move(msg))
					{
						//empty
					}

					void Fire() override
					{
						static_cast<DccppClient &>(this->GetTarget()).OnMessage(m_strMessage);
					}

				private:
					std::string m_strMessage;
			};

		private:
			NetMessenger				m_clMessenger;
			DccppServiceImplClientProxy &m_rclOwner;
			DccLiteService				&m_rclSystem;	

			sigslot::scoped_connection	m_slotSystemConnection;

			const NetworkAddress	m_clAddress;

			std::thread				m_clReceiveThread;
	};		

	class DccppServiceImpl: public DccppService, public EventHub::IEventTarget, DccppServiceImplClientProxy
	{
		public:
			DccppServiceImpl(RName name, Broker &broker, const rapidjson::Value &params, const Project &project);
			~DccppServiceImpl() override;				

		private:			
			void ListenThreadProc(const int port);

			void OnAcceptConnection(const dcclite::NetworkAddress &address, Socket s);

			void OnClientDisconnected(DccppClient &client);

			void Async_ClientDisconnected(DccppClient &client) override;		

		private:
			
			class ClientDisconnectedEvent: public EventHub::IEvent
			{
				public:
					ClientDisconnectedEvent(DccppServiceImpl &target, DccppClient &client):
						IEvent(target),
						m_rclClient(client)
					{
						//empty
					}

					void Fire() override
					{
						static_cast<DccppServiceImpl &>(this->GetTarget()).OnClientDisconnected(m_rclClient);
					}

				private:
					DccppClient &m_rclClient;
			};

			class AcceptConnectionEvent: public EventHub::IEvent
			{
				public:
					AcceptConnectionEvent(DccppServiceImpl &target, const dcclite::NetworkAddress address, dcclite::Socket socket):
						IEvent(target),
						m_clAddress(address),
						m_clSocket(std::move(socket))
					{
						//empty
					}

					void Fire() override
					{
						static_cast<DccppServiceImpl &>(this->GetTarget()).OnAcceptConnection(m_clAddress, std::move(m_clSocket));
					}

				private:
					const dcclite::NetworkAddress m_clAddress;
					dcclite::Socket m_clSocket;
			};

		private:			
			DccLiteService  &m_rclDccService;			

			//
			//Network communication
			//
			dcclite::Socket								m_clSocket;

			std::thread									m_thListenThread;

			std::mutex									m_mtxClientsLock;
			std::vector<std::unique_ptr<DccppClient>>	m_vecClients;
	};

	DccppClient::DccppClient(DccppServiceImplClientProxy &owner, DccLiteService &system, const NetworkAddress address, Socket &&socket):
		m_clMessenger(std::move(socket), ">"),
		m_rclOwner(owner),
		m_rclSystem(system),
		m_clAddress(address)
	{
		m_slotSystemConnection = m_rclSystem.m_sigEvent.connect(&DccppClient::OnObjectManagerEvent, this);

		m_clReceiveThread = std::thread{ [this] {this->ThreadProc(); } };

		dcclite::SetThreadName(m_clReceiveThread, "DccppClient::ReceiveThread");
	}
	

	DccppClient::~DccppClient()
	{
		m_clMessenger.Close();

		m_clReceiveThread.join();		
	}

	static inline std::string CreateTurnoutDecoderStateResponse(const TurnoutDecoder& decoder)
	{
		return fmt::format("<H{} {}>", decoder.GetAddress(), (decoder.GetState() == DecoderStates::ACTIVE ? 1 : 0));
	}

	static inline std::string CreateOutputDecoderStateResponse(const OutputDecoder& decoder)
	{
		if (decoder.IsTurnoutDecoder())
		{
			return CreateTurnoutDecoderStateResponse(static_cast<const TurnoutDecoder&>(decoder));
		}
		else
		{
			return fmt::format("<Y {} {}>", decoder.GetAddress(), (decoder.GetState() == DecoderStates::ACTIVE ? 1 : 0));
		}
	}

	static inline std::string CreateDecoderStateResponse(const StateDecoder &decoder)
	{	
		if (decoder.IsInputDecoder())
		{
			return fmt::format("<{} {}>", decoder.GetState() == DecoderStates::ACTIVE ? 'Q' : 'q', decoder.GetAddress());
		}	
		else
		{
			return CreateOutputDecoderStateResponse(static_cast<const OutputDecoder&>(decoder));
		}
	}

	static inline std::string CreateSensorStateResponse(const std::vector<StateDecoder *> &sensorDecoders)
	{
		std::stringstream response;
	
		if (!sensorDecoders.empty())
		{
			for (auto dec : sensorDecoders)
			{

				response <<
					"<" << (dec->GetState() == DecoderStates::ACTIVE ? 'Q' : 'q') <<
					dec->GetAddress().GetAddress() <<
					'>';
			}
		}
		else
		{
			response << "<X>";
		}

		return response.str();
	}

	static bool ParseSignalCommandM(dcclite::Parser &parser, const std::string &msg, DccLiteService &liteService)
	{	
		int num1;

		if (parser.GetHexNumber(num1) != Tokens::HEX_NUMBER)
		{
			Log::Error("[DccppClient::ParseSignalCommandM] Error parsing msg, expected TOKEN_NUMBER 0 for: {}", msg);
			return false;
		}

		if (num1 != 0)
		{
			Log::Error("[DccppClient::Update] Error parsing msg, expected TOKEN_NUMBER 0 for: {}", msg);
			return false;
		}

		if (parser.GetHexNumber(num1) != Tokens::HEX_NUMBER)
		{
			Log::Error("[DccppClient::ParseSignalCommandM] Error parsing msg, expected TOKEN_NUMBER 1 for: {}", msg);
			return false;
		}

	#if 0
		if ((0xC0 & num1) != 0x80)
		{
			Log::Error("[DccppClient::ParseSignalCommandM] Expected accessory address: {}", msg);
			goto ERROR_RESPONSE;
		}
	#endif

		int num2;
		if (parser.GetHexNumber(num2) != Tokens::HEX_NUMBER)
		{
			Log::Error("[DccppClient::ParseSignalCommandM] Error parsing msg, expected TOKEN_NUMBER 2 for: {}", msg);
			return false;
		}

		if ((num2 & 0x01) != 0x01)
		{
			Log::Error("[DccppClient::ParseSignalCommandM] Expected signal decoder address: {}", msg);
			return false;
		}

		int num3;
		if (parser.GetHexNumber(num3) != Tokens::HEX_NUMBER)
		{
			Log::Error("[DccppClient::ParseSignalCommandM] Error parsing msg, expected TOKEN_NUMBER 3 for: {}", msg);
			return false;
		}

		if ((num3 & 0xE0) != 0x00)
		{
			Log::Error("[DccppClient::ParseSignalCommandM] Expected signal decoder 2 address: {}", msg);
			return false;
		}

		uint8_t packet[3] = { static_cast<uint8_t>(num1), static_cast<uint8_t>(num2), static_cast<uint8_t>(num3) };

		uint16_t address;
		dcclite::SignalAspects packetAspect;

		std::tie(address, packetAspect) = ExtractSignalDataFromPacket(packet);

		Log::Debug("[DccppClient::Update] Signal decoder {} cmd {}", address, num3);	
		auto dec = liteService.TryFindDecoder(DccAddress(address));
		if (!dec) 
		{
			Log::Error("[DccppClient::ParseSignalCommandM] Decoder for address {} not found", address);

			return false;
		}

		auto signal = dynamic_cast<SignalDecoder *>(dec);
		if (!signal)
		{
			Log::Error("[DccppClient::ParseSignalCommandM] Decoder {} - {} is not a SignalDecoder", address, dec->GetName());

			return false;
		}

		signal->SetAspect(packetAspect, "DccppClient::ParseSignalCommandM");

		return true;
	}

	bool DccppClient::CreateTurnoutsStateResponse(std::stringstream &response) const
	{
		auto turnoutDecoders = m_rclSystem.FindAllTurnoutDecoders();
		if (turnoutDecoders.empty())
		{
			response << "<X>";
			return false;
		}		
	
		for (auto turnout : turnoutDecoders)
		{
			response << CreateTurnoutDecoderStateResponse(*turnout);
		}

		return true;
	}

	bool DccppClient::CreateTurnoutsIdListResponse(std::stringstream &response) const
	{		
		response << "<JT";

		auto turnoutDecoders = m_rclSystem.FindAllTurnoutDecoders();
		if (turnoutDecoders.empty())
		{
			response << ">";
			return false;
		}

		for (auto turnout : turnoutDecoders)
		{
			response << ' ' << turnout->GetAddress().GetAddress();
		}

		response << ">";
		return true;
	}

	bool DccppClient::CreateTurnoutsDefResponse(std::stringstream &response) const
	{
		auto turnoutDecoders = m_rclSystem.FindAllTurnoutDecoders();
		if (turnoutDecoders.empty())
		{
			response << "<X>";
			return false;
		}

		for (auto turnout : turnoutDecoders)
		{
			auto addressNum = turnout->GetAddress().GetAddress();
			auto nmraAddress = dcclite::ConvertAddressToNMRA(addressNum);

			response << "<H " << 
				addressNum << ' ' << 
				std::get<0>(nmraAddress) << ' ' << 
				std::get<1>(nmraAddress) << ' ' << 
				((turnout->GetState() == DecoderStates::ACTIVE) ? 1 : 0 ) << 
			">";
		}

		return true;
	}

	bool DccppClient::CreateOutputsDefResponse(std::stringstream &response) const
	{
		auto outputDecoders = m_rclSystem.FindAllSimpleOutputDecoders();
		if (outputDecoders.empty())
		{
			response << "<X>";
			return false;
		}
	
		for (auto dec : outputDecoders)
		{
			response << "<Y " << 
				dec->GetAddress() << ' ' << 
				dec->GetPin() << ' ' << 
				(int)dec->GetDccppFlags() << ' ' << 
				((dec->GetState() == DecoderStates::ACTIVE) ? 1 : 0) <<
			">";

			response << CreateOutputDecoderStateResponse(*dec);
		}	

		return true;
	}


	void DccppClient::ParseStatusCommand(dcclite::Parser &parser, const std::string &msg)
	{
		std::stringstream response;
		response << "<iDCC-EX V-5.0.7 / DccLite / PC G-BCS><N1: Ethernet><p0MAIN>";

		this->CreateTurnoutsStateResponse(response);
			
		auto outputDecoders = m_rclSystem.FindAllSimpleOutputDecoders();
		if (!outputDecoders.empty())
		{
			for (auto dec : outputDecoders)
			{
				response << CreateOutputDecoderStateResponse(*dec);
			}
		}
		else
		{
			response << "<X>";
		}	

		m_clMessenger.Send(m_clAddress, response.str());

		//DCCPP by default seems to do not request this, so we send so it has sensors states at load
		auto sensorDecoders = m_rclSystem.FindAllInputDecoders();
		if (!sensorDecoders.empty())		
			m_clMessenger.Send(m_clAddress, CreateSensorStateResponse(sensorDecoders));
	}

	bool DccppClient::ParseSensorCommand(dcclite::Parser &parser, const std::string &msg)
	{
		char token[128];
		if (parser.GetToken(token, sizeof(token)) != Tokens::END_OF_BUFFER)
		{
			Log::Error("[DccppClient::Update] Error parsing msg, expected TOKEN_EOF for: {}", msg);

			return false;
		}
	
		std::stringstream response;

		//Send all sensors definition
		auto sensorDecoders = m_rclSystem.FindAllInputDecoders();
		if (!sensorDecoders.empty())
		{
			for (auto dec : sensorDecoders)
			{	
				response << "<Q" << dec->GetAddress().GetAddress() << " 1 false>;";
#if 0
				response <<
					"<Q" <<
					dec->GetAddress().GetAddress() <<
					' ' <<
					static_cast<unsigned int>(dec->GetPin()) <<
					' ' <<
					dec->HasPullUp() <<
					'>';
#endif
			}
		}
		else
		{
			response << "<X>";
		}

		m_clMessenger.Send(m_clAddress, response.str());	

		return true;	
	}


	void DccppClient::OnObjectManagerEvent(const ObjectManagerEvent &event)
	{		
		if (event.m_kType != ObjectManagerEvent::ITEM_CHANGED)
			return;
	
		if(auto decoder = dynamic_cast<const StateDecoder *>(event.m_pclItem))
			m_clMessenger.Send(m_clAddress, CreateDecoderStateResponse(*decoder));					
	}

	void DccppClient::OnMessage(const std::string &msg)
	{
		dcclite::Log::Debug("[DccppClient] Received {}", msg);

		dcclite::Parser parser(msg.c_str());

		char token[128];
		if (parser.GetToken(token, sizeof(token)) != Tokens::CMD_START)
		{
			Log::Error("[DccppClient::OnMessage] Error parsing msg, expected TOKEN_CMD_START: {}", msg);

			goto ERROR_RESPONSE;
		}

		char cmd[4];

		{
			auto tokenType = parser.GetToken(cmd, sizeof(cmd));

			if (tokenType == Tokens::HASH)
			{
				//max slots, have no idea why and how it is used
				m_clMessenger.Send(m_clAddress, "<# 0>");

				return;
			}

			if (tokenType != Tokens::ID)
			{
				Log::Error("[DccppClient::OnMessage] Error parsing msg, expected TOKEN_ID for cmd identification: {}", msg);

				goto ERROR_RESPONSE;
			}
		}

		switch (cmd[0])
		{
			case 'c':
				//current
				m_clMessenger.Send(m_clAddress, "<a 0>");
				break;

			//https://dcc-ex.com/reference/software/command-summary-consolidated.html#j-t-jt-request-the-list-of-defined-turnout-point-ids
			//
			// It seems that JMRI only send this when entering the "configure base station option"
			//
			case 'J':
			{
				if (cmd[1] != 'T')
				{
					char param[10];
					const auto tokenType = parser.GetToken(param, sizeof(param));
					if (tokenType != Tokens::ID)
					{
						Log::Error("[DccppClient::Update] Unknown parameter for {}, msg>: {}", cmd, msg);

						goto ERROR_RESPONSE;
					}

					if (strcmp(param, "T"))
					{
						Log::Error("[DccppClient::Update] Unknown parameter {} for {}, msg>: {}", param, cmd, msg);

						goto ERROR_RESPONSE;
					}
				}

				int id;
				const auto tokenType = parser.GetNumber(id);
				if (tokenType == Tokens::NUMBER)
				{
					Log::Error("[DccppClient::Update] TODO HANDLE ID {} for {}, msg>: {}", id, cmd, msg);
					goto ERROR_RESPONSE;
				}
				else if (tokenType != Tokens::END_OF_BUFFER)
				{
					Log::Error("[DccppClient::Update] Unknown parameter {}, msg>: {}", cmd, msg);

					goto ERROR_RESPONSE;
				}
				
				std::stringstream response;

				this->CreateTurnoutsIdListResponse(response);
				m_clMessenger.Send(m_clAddress, response.str());				
				break;
			}

			case 'M':
				Log::Debug("[DccppClient::OnMessage] Custom packet cmd {}, msg: {}", cmd, msg);
				if (!ParseSignalCommandM(parser, msg, m_rclSystem))
					goto ERROR_RESPONSE;

				break;

			case 's':
				this->ParseStatusCommand(parser, msg);
				break;

			case 'S':
				if (!this->ParseSensorCommand(parser, msg))
					goto ERROR_RESPONSE;
				break;

			case 'Q':
				if (parser.GetToken(token, sizeof(token)) != Tokens::END_OF_BUFFER)
				{
					Log::Error("[DccppClient::OnMessage] Error parsing msg, expected TOKEN_EOF for: {}", msg);

					goto ERROR_RESPONSE;
				}
				else
				{				
					auto sensorDecoders = m_rclSystem.FindAllInputDecoders();
					m_clMessenger.Send(m_clAddress, CreateSensorStateResponse(sensorDecoders));
				}
				break;

			case 'T':
			case 'Z':
			{
				int id;
				const auto tokenType = parser.GetNumber(id);
				if (tokenType == Tokens::END_OF_BUFFER)
				{
					std::stringstream response;

					if (cmd[0] == 'T')
						this->CreateTurnoutsDefResponse(response);
					else
						this->CreateOutputsDefResponse(response);


					m_clMessenger.Send(m_clAddress, response.str());
					break;
				}

				if (tokenType != Tokens::NUMBER)
				{

					Log::Error("[DccppClient::OnMessage] Error parsing msg, expected TOKEN_NUMBER for device id: {}", msg);

					goto ERROR_RESPONSE;
				}

				int direction;
				if (parser.GetNumber(direction) != Tokens::NUMBER)
				{
					Log::Error("[DccppClient::OnMessage] Error parsing msg, expected TOKEN_NUMBER for device state: {}", msg);

					goto ERROR_RESPONSE;
				}

				auto *dec = m_rclSystem.TryFindDecoder(DccAddress(id));
				if (!dec)
				{
					Log::Error("[DccppClient::OnMessage] Error decoder {} not found", id);

					goto ERROR_RESPONSE;
				}

				auto remoteDecoder = dynamic_cast<RemoteDecoder *>(dec);				
				if ((!remoteDecoder) || (!remoteDecoder->IsOutputDecoder()))
				{
					Log::Error("[DccppClient::OnMessage] Error decoder {} - {} is not an output type", id, dec->GetName());

					goto ERROR_RESPONSE;
				}

				auto *outputDecoder = static_cast<OutputDecoder *>(remoteDecoder);

				auto newState = direction ? DecoderStates::ACTIVE : DecoderStates::INACTIVE;

				//if no state change pending and remote state is the requested one
				if (!outputDecoder->GetPendingStateChange() && outputDecoder->GetState() == newState)
				{
					//we force an output, because we should not have a incoming state, so tell JMRI that we are on requested state
					m_clMessenger.Send(m_clAddress, CreateOutputDecoderStateResponse(*outputDecoder));
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

		return;

ERROR_RESPONSE:
		m_clMessenger.Send(m_clAddress, "<X>");			
	}

	void DccppClient::ThreadProc()
	{
		for (;;)
		{
			auto [status, msg] = m_clMessenger.Poll();

			if (status != Socket::Status::OK)
				break;
			
			EventHub::PostEvent<ClientEvent>(std::ref(*this), std::move(msg));
		}

		m_rclOwner.Async_ClientDisconnected(*this);		
	}


	//
	//
	//
	//
	//


	DccppServiceImpl::DccppServiceImpl(RName name, Broker &broker, const rapidjson::Value& params, const Project& project):
		DccppService(name, broker, params, project),		
		m_rclDccService{ static_cast<DccLiteService &>(m_rclBroker.ResolveRequirement(dcclite::json::GetString(params, "requires", name.GetData().data()))) }
	{		
		//standard port used by DCC++
		const auto port = dcclite::json::TryGetDefaultInt(params, "port", 2560);		

		if (!m_clSocket.Open(port, dcclite::Socket::Type::STREAM, dcclite::Socket::Flags::FLAG_BLOCKING_MODE))
		{
			throw std::runtime_error("[DccppService] Cannot open socket");
		}

		//
		//start listening thread
		m_thListenThread = std::move(std::thread{ [this, port]() {this->ListenThreadProc(port); } });		
		dcclite::SetThreadName(m_thListenThread, "DccppServiceImpl::ListenThread");
		
		if (auto bonjourService = static_cast<BonjourService *>(m_rclBroker.TryFindService(RName{ BONJOUR_SERVICE_NAME })))
			bonjourService->Register(this->GetName().GetData(), "dccpp", NetworkProtocol::TCP, port, 36);

		ZeroConfSystem::Register(this->GetTypeName(), port);
	}

	DccppServiceImpl::~DccppServiceImpl()
	{
		//close socket
		m_clSocket.Close();

		//wait for the thread to die...
		m_thListenThread.join();

		//kill clients now, they will fire events
		m_vecClients.clear();

		//cancel any pending events, includings clients telling us that they disconnected
		EventHub::CancelEvents(*this);
	}

	void DccppServiceImpl::OnAcceptConnection(const dcclite::NetworkAddress &address, Socket s)
	{		
		std::unique_lock<std::mutex> guard{ m_mtxClientsLock };

		auto client = std::unique_ptr<DccppClient>{ new DccppClient(*this, m_rclDccService, address, std::move(s)) };

		m_vecClients.push_back(std::move(client));
	}

	void DccppServiceImpl::ListenThreadProc(const int port)
	{
		if (!m_clSocket.Listen())
		{
			throw std::runtime_error("[DccppService] Cannot put socket on listen mode");
		}

		dcclite::Log::Info("[DccppService] Started, listening on port {}", port);

		for (;;)
		{
			auto [status, socket, address] = m_clSocket.TryAccept();

			if (status == Socket::Status::OK)
			{
				dcclite::Log::Info("[DccppService] Client connected {}", address.GetIpString());								
				
				EventHub::PostEvent<AcceptConnectionEvent>(std::ref(*this), address, std::move(socket));
			}
			else if (status != Socket::Status::WOULD_BLOCK)
				break;
		}		
	}

	void DccppServiceImpl::Async_ClientDisconnected(DccppClient &client)
	{
		EventHub::PostEvent<ClientDisconnectedEvent>(std::ref(*this), std::ref(client));
	}

	void DccppServiceImpl::OnClientDisconnected(DccppClient &client)
	{
		std::unique_lock<std::mutex> guard{ m_mtxClientsLock };

		for (size_t i = 0; i < m_vecClients.size(); ++i)
		{
			auto &c = m_vecClients[i];

			if (c.get() == &client)
			{
				dcclite::Log::Info("[DccppService] Client disconnected");

				m_vecClients.erase(m_vecClients.begin() + i);

				return;
			}
		}
	}

	DccppService::DccppService(RName name, Broker &broker, const rapidjson::Value &params, const Project &project) :
		Service{ name, broker, params, project }
	{
		//empty
	}

	std::unique_ptr<Service> DccppService::Create(RName name, Broker &broker, const rapidjson::Value &params, const Project &project)
	{
		return std::make_unique<DccppServiceImpl>(name, broker, params, project);
	}

#if 1
	

#endif

}