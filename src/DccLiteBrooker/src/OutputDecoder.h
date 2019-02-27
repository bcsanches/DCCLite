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
			return dcclite::DecoderTypes::DEC_OUTPUT;
		}

		//
		//IObject
		//
		//

		virtual const char *GetTypeName() const noexcept
		{
			return "Device";
		}

		virtual void Serialize(dcclite::JsonOutputStream_t &stream) const
		{
			Decoder::Serialize(stream);

			stream.AddIntValue ("pin", m_iPin);
			stream.AddBool("invertedOperation", m_fInvertedOperation);
			stream.AddBool("ignoreSaveState", m_fIgnoreSavedState);
			stream.AddBool("activateOnPowerUp", m_fActivateOnPowerUp);
		}

	private:
		dcclite::PinType_t m_iPin;

		bool m_fInvertedOperation = false;
		bool m_fIgnoreSavedState = false;
		bool m_fActivateOnPowerUp = false;
};