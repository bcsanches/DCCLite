#include "Node.h"

#include "Decoder.h"
#include "DccLiteService.h"

Node::Node(const std::string &name, DccLiteService &dccService, const nlohmann::json &params) :
	m_strName(name),
	m_clDccService(dccService)
{
	auto decodersData = params["decoders"];

	if (!decodersData.is_array())
		throw std::runtime_error("error: invalid config, expected decoders array inside Node");

	for (size_t i = 0, size = decodersData.size(); i < size; ++i)
	{
		auto decoderData = decodersData[i];

		auto decoderName = decoderData["name"].get<std::string>();
		auto className = decoderData["class"].get<std::string>();
		Decoder::Address address{ decoderData["address"] };

		m_vecDecoders.push_back(&m_clDccService.Create(className, address, decoderName, decodersData));
	}
}