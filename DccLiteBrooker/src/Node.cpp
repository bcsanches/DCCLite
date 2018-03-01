#include "Node.h"

#include "Decoder.h"
#include "DccLiteService.h"

Node::Node(const std::string &name, DccLiteService &dccService, const nlohmann::json &params) :
	m_strName(name),
	m_clDccService(dccService)
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

		m_vecDecoders.push_back(&m_clDccService.Create(className, address, decoderName, decodersData));
	}
}