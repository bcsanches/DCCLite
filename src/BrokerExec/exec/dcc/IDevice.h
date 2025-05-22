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

#include <dcclite_shared/BasicPin.h>

#include <dcclite/RName.h>

namespace dcclite::broker::exec::dcc
{
	class Decoder;
	class RemoteDecoder;
	class INetworkDevice_DecoderServices;
	class INetworkDevice_TaskProvider;

	class IDevice_DecoderServices
	{
		public:
			virtual RName GetDeviceName() const noexcept = 0;		

			virtual INetworkDevice_DecoderServices *TryGetINetworkDevice() noexcept
			{
				return nullptr;
			}			

			virtual INetworkDevice_TaskProvider *TryGetINetworkTaskProvider() noexcept
			{
				return nullptr;
			}

			virtual void Decoder_OnChangeStateRequest(const Decoder &decoder) noexcept = 0;			
	};

	class INetworkDevice_DecoderServices
	{
		public:
			virtual void Decoder_RegisterPin(const RemoteDecoder &decoder, dcclite::BasicPin pin, const char *usage) = 0;
			virtual void Decoder_UnregisterPin(const RemoteDecoder &decoder, dcclite::BasicPin pin) = 0;			

			[[nodiscard]] virtual Decoder &FindDecoder(RName name) const = 0;
			[[nodiscard]] virtual uint8_t FindDecoderIndex(const Decoder &decoder) const = 0;

			[[nodiscard]] virtual std::uint16_t GetProtocolVersion() const noexcept = 0;
	};
}
