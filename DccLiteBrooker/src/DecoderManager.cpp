#include "DecoderManager.h"

Decoder &DecoderManager::Create(
	const std::string &className,
	Decoder::Address address,
	const std::string &name,
	const nlohmann::json &params
)
{
	auto it = m_mapDecoders.find(address);
	if (it != m_mapDecoders.end())
	{
		std::stringstream stream;

		stream << "error: cannot create decoder with address " << address << " named " << name << " because it already exists";

		throw std::runtime_error(stream.str());
	}

	auto decoder = Decoder::Class::TryProduce(className.c_str(), address, name, *this, params);
	if (!decoder)
	{
		std::stringstream stream;

		stream << "error: failed to instantiate decoder " << address << " named " << name;

		throw std::runtime_error(stream.str());
	}

	auto decoderPtr = decoder.get();
	
	m_mapDecodersNames.insert(std::make_pair(name, address));
	m_mapDecoders.insert(it, std::make_pair(address, std::move(decoder)));

	return *decoderPtr;
}
