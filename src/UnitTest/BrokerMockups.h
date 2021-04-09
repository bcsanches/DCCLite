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


class NetworkDeviceDecoderServicesMockup: public INetworkDevice_DecoderServices
{
	public:
		void Decoder_RegisterPin(const RemoteDecoder &decoder, dcclite::BasicPin pin, const char *usage) override
		{

		}

		void Decoder_UnregisterPin(const RemoteDecoder &decoder, dcclite::BasicPin pin) override
		{

		}
};

class DeviceDecoderServicesMockup : public IDevice_DecoderServices
{
	public:
		std::string_view GetDeviceName() const noexcept override
		{
			return "mockup";
		}

		INetworkDevice_DecoderServices *TryGetINetworkDevice() noexcept override
		{
			return &m_DecoderServices;
		}

	private:
		NetworkDeviceDecoderServicesMockup m_DecoderServices;
};

class DecoderServicesMockup : public IDccLite_DecoderServices
{
	public:
		void Decoder_OnStateChanged(Decoder &decoder) override
		{
			//empty
		}
};
