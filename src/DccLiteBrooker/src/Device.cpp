#include "Device.h"

#include "Decoder.h"
#include "DccLiteService.h"

Device::Device(std::string name, DccLiteService &dccService, const nlohmann::json &params) :
	Object(std::move(name)),
	m_clDccService(dccService),
	m_eStatus(Status::OFFLINE)
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
