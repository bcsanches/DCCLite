#include "Device.h"

#include "Decoder.h"
#include "DccLiteService.h"

Device::Device(std::string name, DccLiteService &dccService, const nlohmann::json &params) :
	FolderObject(std::move(name)),
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

		auto &decoder = m_clDccService.Create(className, address, decoderName, element);

		this->AddChild(std::make_unique<dcclite::Shortcut>(std::string(decoder.GetName()), decoder));
	}
}
