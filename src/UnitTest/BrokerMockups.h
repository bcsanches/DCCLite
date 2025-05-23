// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include <map>
#include <vector>

#include "exec/dcc/Decoder.h"
#include "exec/dcc/IDccLiteService.h"
#include "exec/dcc/IDevice.h"

#include <dcclite/RName.h>

namespace dcclite::broker::exec::dcc
{
	class Decoder;
}


class NetworkDeviceDecoderServicesMockup: public dcclite::broker::exec::dcc::INetworkDevice_DecoderServices
{
	private:
		std::map<dcclite::RName, dcclite::broker::exec::dcc::Decoder *> m_mapDecoders;
		std::vector< dcclite::broker::exec::dcc::Decoder *> m_vecDecoders;

	public:
		void Decoder_RegisterPin(const dcclite::broker::exec::dcc::RemoteDecoder &decoder, dcclite::BasicPin pin, const char *usage) override
		{
			//empty
		}

		void Decoder_UnregisterPin(const dcclite::broker::exec::dcc::RemoteDecoder &decoder, dcclite::BasicPin pin) override
		{
			//empty
		}

		[[nodiscard]] dcclite::broker::exec::dcc::Decoder &FindDecoder(const dcclite::RName name) const override
		{
			auto it = m_mapDecoders.find(name);
			if (it == m_mapDecoders.end())
			{
				throw std::exception("Decoder not found");
			}

			return *(it->second);
		}

		[[nodiscard]] uint8_t FindDecoderIndex(const dcclite::broker::exec::dcc::Decoder &decoder) const override
		{
			auto it = std::find(m_vecDecoders.begin(), m_vecDecoders.end(), &decoder);
			if (it == m_vecDecoders.end())
				throw std::exception("Decoder not registered");

			if (m_vecDecoders.size() > 255)
				throw std::exception("too many decoders, which arduino are you using?");

			return static_cast<uint8_t>(it - m_vecDecoders.begin());
		}

		[[nodiscard]] uint16_t GetProtocolVersion() const noexcept override
		{
			return 0;
		}

		void RegisterDecoder(dcclite::broker::exec::dcc::Decoder &decoder)
		{
			if (m_mapDecoders.find(decoder.GetName()) != m_mapDecoders.end())
			{
				throw std::exception("decoder already registered");
			}

			m_mapDecoders.insert(std::make_pair(decoder.GetName(), &decoder));
			m_vecDecoders.push_back(&decoder);
		}
};

namespace dcclite::broker
{
	class Decoder;
}

class DeviceDecoderServicesMockup : public dcclite::broker::exec::dcc::IDevice_DecoderServices
{
	public:
		dcclite::RName GetDeviceName() const noexcept override
		{
			return dcclite::RName("mockup");
		}

		dcclite::broker::exec::dcc::INetworkDevice_DecoderServices *TryGetINetworkDevice() noexcept override
		{
			return &m_DecoderServices;
		}

		void Decoder_OnChangeStateRequest(const dcclite::broker::exec::dcc::Decoder &decoder) noexcept override
		{
			//empty
		}

		void RegisterDecoder(dcclite::broker::exec::dcc::Decoder &decoder)
		{
			m_DecoderServices.RegisterDecoder(decoder);			
		}

	private:
		NetworkDeviceDecoderServicesMockup m_DecoderServices;
};

class DecoderServicesMockup : public dcclite::broker::exec::dcc::IDccLite_DecoderServices
{
	public:
		void Decoder_OnStateChanged(dcclite::broker::exec::dcc::Decoder &decoder) override
		{
			//empty
		}

		dcclite::broker::exec::dcc::Decoder *TryFindDecoder(dcclite::RName id) const override
		{
			return nullptr;
		}
};
