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

#include <string_view>

#include "BasicPin.h"

namespace dcclite::broker
{
	class Decoder;
	class RemoteDecoder;
	class INetworkDevice_DecoderServices;

	class IDevice_DecoderServices
	{
		public:
			virtual std::string_view GetDeviceName() const noexcept = 0;		

			virtual INetworkDevice_DecoderServices *TryGetINetworkDevice() noexcept
			{
				return nullptr;
			}


	};

	class INetworkDevice_DecoderServices
	{
		public:
			virtual void Decoder_RegisterPin(const RemoteDecoder &decoder, dcclite::BasicPin pin, const char *usage) = 0;
			virtual void Decoder_UnregisterPin(const RemoteDecoder &decoder, dcclite::BasicPin pin) = 0;
	};
}
