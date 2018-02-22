#include "DccLiteService.h"

#include "Node.h"

static ServiceClass dccLiteService("DccLite", 
	[](const ServiceClass &serviceClass, const std::string &name, const nlohmann::json &params) -> std::unique_ptr<Service> { return std::make_unique<DccLiteService>(serviceClass, name, params); }
);

DccLiteService::DccLiteService(const ServiceClass &serviceClass, const std::string &name, const nlohmann::json &params) :
	Service(serviceClass, name, params)
{
	auto nodesData = params["nodes"];

	if (!nodesData.is_array())
		throw std::runtime_error("error: invalid config, expected nodes array inside DccLiteService");

	for (unsigned i = 0, size = nodesData.size(); i < size; ++i)
	{
		auto node = nodesData[i];

		auto nodeName = node["name"].get<std::string>();

		auto existingNodeIt = m_mapNodes.find(nodeName);
		if (existingNodeIt != m_mapNodes.end())
		{
			std::stringstream stream;

			stream << "error: node " << nodeName << " already exists on this service (" << this->GetName() << ").";

			throw std::runtime_error(stream.str());
		}		

		auto pair = m_mapNodes.insert(
			existingNodeIt,
			std::make_pair(
				nodeName,
				std::make_unique<Node>(nodeName, *this, node)
			)
		);
	}
}

DccLiteService::~DccLiteService()
{
	//empty
}


