#pragma once

#include <map>

#include "Decoder.h"

class DecoderManager
{
	public:
		DecoderManager() = default;

		Decoder &Create(
			const std::string &className,
			Decoder::Address address,
			const std::string &name,
			const nlohmann::json &params
		);

	private:
		std::map<Decoder::Address, std::unique_ptr<Decoder>> m_mapDecoders;	
		std::map<std::string_view, Decoder::Address> m_mapDecodersNames;
};

