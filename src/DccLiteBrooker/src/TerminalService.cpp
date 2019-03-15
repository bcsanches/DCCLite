#include "TerminalService.h"

#include <sstream>
#include <stdexcept>

#include <Log.h>

#include <JsonCreator/StringWriter.h>
#include <JsonCreator/Object.h>

#include <rapidjson/document.h>

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

		virtual void Run(TerminalContext &context, Result_t &results, const CmdId_t id, const rapidjson::Document &request)
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

class SetLocationCmd : public TerminalCmd
{
	public:
		SetLocationCmd() :
			TerminalCmd("Set-Location")
		{
			//empty
		}

		virtual void Run(TerminalContext &context, Result_t &results, const CmdId_t id, const rapidjson::Document &request)
		{
			auto item = context.GetItem();
			if (!item->IsFolder())
			{
				throw TerminalCmdException(fmt::format("Current location {} is invalid", context.GetLocation().string()), id);
			}

			auto folder = static_cast<FolderObject *>(item);

			auto paramsIt = request.FindMember("params");			
			if (paramsIt != request.MemberEnd())
			{							
				if (!paramsIt->value.IsArray())
				{
					throw TerminalCmdException("Expected positional parameters", id);
				}

				auto destinationPath = paramsIt->value[0].GetString();

				auto destinationObj = folder->TryNavigate(Path_t(destinationPath));
				if (!destinationObj)
				{
					throw TerminalCmdException(fmt::format("Invalid path {}", destinationPath), id);
				}

				if (!destinationObj->IsFolder())
				{
					throw TerminalCmdException(fmt::format("Path {} led to an IObject, not IFolder", destinationPath), id);
				}


				context.SetLocation(destinationObj->GetPath());
				item = destinationObj;
			}
		
			results.AddStringValue("classname", "Location");
			results.AddStringValue("location", item->GetPath().string());
		}
};

class TerminalClient
{
	public:
		TerminalClient(TerminalService &owner, Address address, Socket &&socket);
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

		const Address	m_clAddress;
};

TerminalClient::TerminalClient(TerminalService &owner, Address address, Socket &&socket) :
	m_rclOwner(owner),
	m_clAddress(address),
	m_clMessenger(std::move(socket)),
	m_clContext(static_cast<dcclite::FolderObject &>(owner.GetRoot()))
{
	m_clContext.SetLocation(owner.GetPath());
}

TerminalClient::TerminalClient(TerminalClient &&other) :
	m_rclOwner(other.m_rclOwner),
	m_clAddress(std::move(other.m_clAddress)),
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

				rapidjson::Document doc;
				doc.Parse(msg.c_str());

				if (doc.HasParseError())
				{
					throw TerminalCmdException(fmt::format("Invalid json: {}", msg), -1);
				}

				auto jsonrpcKey = doc.FindMember("jsonrpc");
				if ((jsonrpcKey == doc.MemberEnd()) || (!jsonrpcKey->value.IsString()) || (strcmp(jsonrpcKey->value.GetString(), "2.0")))
				{
					throw TerminalCmdException(fmt::format("Invalid rpc version or was not set: {}", msg), -1);
				}

				auto methodKey = doc.FindMember("method");
				if ((methodKey == doc.MemberEnd()) || (!methodKey->value.IsString()))
				{
					throw TerminalCmdException(fmt::format("Invalid method name in msg: {}", msg), -1);
				}

				const auto methodName = methodKey->value.GetString();

				auto idKey = doc.FindMember("id");
				if ((idKey == doc.MemberEnd()) || (!idKey->value.IsInt()))
				{
					throw TerminalCmdException(fmt::format("No method id in: {}", msg), -1);
				}				

				int id = idKey->value.GetInt();

				auto cmdHost = TerminalCmdHost::Instance();
				assert(cmdHost);

				auto cmd = cmdHost->TryFindCmd(methodName);
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

					cmd->Run(m_clContext, resultObj, id, doc);
				}

				response = responseWriter.GetString();

				dcclite::Log::Trace("response {}", response);
			}
			catch (TerminalCmdException &ex)
			{
				response = CreateErrorResponse(ex.what(), ex.GetId());
			}
			catch (std::exception &ex)
			{
				response = CreateErrorResponse(ex.what(), -1);
			}

			if (!m_clMessenger.Send(m_clAddress, response))
			{
				dcclite::Log::Error("message for {} not sent, contents: {}", m_clAddress.GetIpString(), response);
			}
		}
	}
	

	return true;
}

TerminalService::TerminalService(const ServiceClass &serviceClass, const std::string &name, const nlohmann::json &params, const Project &project) :
	Service(serviceClass, name, params, project)	
{	
	auto cmdHost = TerminalCmdHost::Instance();

	assert(cmdHost);

	cmdHost->AddCmd(std::make_unique<GetChildItemCmd>());
	cmdHost->AddCmd(std::make_unique<SetLocationCmd>());	

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

		m_vecClients.emplace_back(*this, address, std::move(socket));
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


