#include "TerminalService.h"

#include <sstream>
#include <stdexcept>

#include <Log.h>

#include <JsonCreator/StringWriter.h>
#include <JsonCreator/Object.h>

#include "json.hpp"
#include "NetMessenger.h"
#include "TerminalCmd.h"

using json = nlohmann::json;
using namespace dcclite;

static ServiceClass terminalService("Terminal",
	[](const ServiceClass &serviceClass, const std::string &name, const nlohmann::json &params, const Project &project) -> 
	std::unique_ptr<Service> { return std::make_unique<TerminalService>(serviceClass, name, params, project); }
);

class GetChildItemCmd : public TerminalCmd
{
	public:
		GetChildItemCmd():
			TerminalCmd("Get-ChildItem")
		{
			//empty
		}

		virtual void Run(TerminalContext &context, Result_t &results, const CmdId_t id, const nlohmann::json &request)
		{
			auto item = context.GetItem();
			if (!item->IsFolder())
			{
				throw TerminalCmdException(fmt::format("Current location {} is invalid", context.GetLocation().string()), id);
			}

			results.AddStringValue("classname", "ChildItem");
			results.AddStringValue("location", item->GetPath().string());

			auto folder = static_cast<FolderObject *>(item);

			auto dataArray = results.AddArray("children");
												
			auto enumerator = folder->GetEnumerator();

			while (enumerator.MoveNext())
			{
				auto item = enumerator.TryGetCurrent();

				auto itemObject = dataArray.AddObject();
				item->Serialize(itemObject);				
			}
		}
};

static GetChildItemCmd g_GetChildItemCmd{};

class TerminalClient
{
	public:
		TerminalClient(TerminalService &owner, Socket &&socket);
		TerminalClient(const TerminalClient &client) = delete;
		TerminalClient(TerminalClient &&other);

		TerminalClient &operator=(TerminalClient &&other)
		{
			if (this != &other)
			{
				m_clMessenger = std::move(other.m_clMessenger);
			}

			return *this;
		}

		bool Update();

	private:
		static std::string CreateErrorResponse(const std::string &msg, const CmdId_t id);

	private:
		NetMessenger m_clMessenger;
		TerminalService &m_rclOwner;
		TerminalContext m_clContext;
};

TerminalClient::TerminalClient(TerminalService &owner, Socket &&socket) :
	m_rclOwner(owner),
	m_clMessenger(std::move(socket)),
	m_clContext(static_cast<dcclite::FolderObject &>(owner.GetRoot()))
{
	m_clContext.SetLocation(owner.GetPath());
}

TerminalClient::TerminalClient(TerminalClient &&other) :
	m_rclOwner(other.m_rclOwner),
	m_clMessenger(std::move(other.m_clMessenger)),
	m_clContext(std::move(other.m_clContext))
{
	//empty
}

std::string TerminalClient::CreateErrorResponse(const std::string &msg, const CmdId_t id)
{
	JsonCreator::StringWriter responseWriter;

	{
		auto responseObj = JsonCreator::MakeObject(responseWriter);

		responseObj.AddStringValue("jsonrpc", "2.0");

		if (id >= 0)
			responseObj.AddIntValue("id", id);

		{
			auto errorObj = responseObj.AddObject("error");

			errorObj.AddStringValue("message", msg);
		}
	}

	return std::string(responseWriter.GetString());
}


bool TerminalClient::Update()
{
	for (;;)
	{
		auto[status, msg] = m_clMessenger.Poll();

		if (status == Socket::Status::DISCONNECTED)
			return false;

		if (status == Socket::Status::WOULD_BLOCK)
			return true;

		if (status == Socket::Status::OK)
		{
			std::string response;

			try
			{
				dcclite::Log::Trace("Received {}", msg);

				std::stringstream stream;
				stream << msg;

				json data;

				stream >> data;

				auto jsonrpcKey = data["jsonrpc"];
				if (!jsonrpcKey.is_string() || jsonrpcKey.get<std::string>().compare("2.0"))
				{
					throw TerminalCmdException(fmt::format("Invalid rpc version: {}", msg), -1);
				}

				auto methodKey = data["method"];
				if (!methodKey.is_string())
				{
					throw TerminalCmdException(fmt::format("No method name in msg: {}", msg), -1);
				}

				const auto methodName = methodKey.get<std::string>();

				auto idKey = data["id"];
				if (!idKey.is_number_integer())
				{
					throw TerminalCmdException(fmt::format("No method id in: {}", msg), -1);

					continue;
				}

				int id = idKey.get<int>();

				auto cmd = TerminalCmd::TryFindCmd(methodName);
				if (cmd == nullptr)
				{
					throw TerminalCmdException(fmt::format("Invalid cmd name: {}", methodName), id);
					dcclite::Log::Error("Invalid cmd: {}", methodName);

					continue;
				}

				JsonCreator::StringWriter responseWriter;
				{
					auto responseObj = JsonCreator::MakeObject(responseWriter);

					responseObj.AddStringValue("jsonrpc", "2.0");
					responseObj.AddIntValue("id", id);
					auto resultObj = responseObj.AddObject("result");

					cmd->Run(m_clContext, resultObj, id, data);
				}

				dcclite::Log::Trace("response {}", responseWriter.GetString());
			}
			catch (TerminalCmdException &ex)
			{
				response = CreateErrorResponse(ex.what(), ex.GetId());
			}
			catch (std::exception &ex)
			{
				response = CreateErrorResponse(ex.what(), -1);
			}
		}
	}
	

	return true;
}

TerminalService::TerminalService(const ServiceClass &serviceClass, const std::string &name, const nlohmann::json &params, const Project &project) :
	Service(serviceClass, name, params, project)	
{	
	if (!m_clSocket.Open(params["port"].get<unsigned short>(), dcclite::Socket::Type::STREAM))
	{
		throw std::runtime_error("[TerminalService] Cannot open socket");
	}

	if (!m_clSocket.Listen())
	{
		throw std::runtime_error("[TerminalService] Cannot put socket on listen mode");
	}
}


TerminalService::~TerminalService()
{
	//empty
}

void TerminalService::Update(const dcclite::Clock &clock)
{
	auto [status, socket, address] = m_clSocket.TryAccept();

	if (status == Socket::Status::OK)
	{
		dcclite::Log::Info("[TermnialService] Client connected {}", address.GetIpString());

		m_vecClients.emplace_back(*this, std::move(socket));
	}

	for (size_t i = 0; i < m_vecClients.size(); ++i)
	{
		auto &client = m_vecClients[i];

		if (!client.Update())
		{
			dcclite::Log::Info("[TermnialService] Client disconnected");			

			m_vecClients.erase(m_vecClients.begin() + i);
		}
	}
}


