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

#include "IDccLiteService.h"
#include "IDevice.h"

namespace dcclite::broker
{
	class Decoder;
}


class NetworkDeviceDecoderServicesMockup: public dcclite::broker::INetworkDevice_DecoderServices
{
	public:
		void Decoder_RegisterPin(const dcclite::broker::RemoteDecoder &decoder, dcclite::BasicPin pin, const char *usage) override
		{

		}

		void Decoder_UnregisterPin(const dcclite::broker::RemoteDecoder &decoder, dcclite::BasicPin pin) override
		{

		}
};

namespace dcclite::broker
{
	class Decoder;
}

class DeviceDecoderServicesMockup : public dcclite::broker::IDevice_DecoderServices
{
	public:
		std::string_view GetDeviceName() const noexcept override
		{
			return "mockup";
		}

		dcclite::broker::INetworkDevice_DecoderServices *TryGetINetworkDevice() noexcept override
		{
			return &m_DecoderServices;
		}

		void Decoder_OnChangeStateRequest(const dcclite::broker::Decoder &decoder) noexcept override
		{
			//empty
		}

	private:
		NetworkDeviceDecoderServicesMockup m_DecoderServices;
};

class DecoderServicesMockup : public dcclite::broker::IDccLite_DecoderServices
{
	public:
		void Decoder_OnStateChanged(dcclite::broker::Decoder &decoder) override
		{
			//empty
		}

		dcclite::broker::Decoder *TryFindDecoder(std::string_view id) const override
		{
			return nullptr;
		}

		[[nodiscard]] std::string_view Decoder_GetSystemName() const noexcept override
		{
			return "dccsystem";
		}
};
