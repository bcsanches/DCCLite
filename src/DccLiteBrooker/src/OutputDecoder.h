#pragma once

#include "Decoder.h"

#include "EmbeddedLibDefs.h"

class OutputDecoder : public Decoder
{
	public:
		OutputDecoder(const Class &decoderClass,
			const Address &address,
			const std::string &name,
			DccLiteService &owner,
			const nlohmann::json &params
		);

		virtual void WriteConfig(dcclite::Packet &packet) const;

		virtual dcclite::DecoderTypes GetType() const noexcept
		{
			return dcclite::DecoderTypes::OUTPUT;
		}

	private:
		dcclite::PinType_t m_iPin;
};