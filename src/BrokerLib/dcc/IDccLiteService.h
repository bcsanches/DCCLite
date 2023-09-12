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

#include <string>

#include <rapidjson/document.h>

#include <RName.h>

namespace dcclite
{
	class Guid;
	class IObject;
	class NetworkAddress;
	class Packet;
}

namespace dcclite::broker
{

	class DccAddress;
	class Decoder;
	class NetworkDevice;
	class IDevice_DecoderServices;
	class LocationManager;

	class DccLiteEvent
	{
		public:
			struct DeviceEvent
			{
				const NetworkDevice *m_pclDevice;
			};

			struct ItemEvent
			{
				const dcclite::IObject *m_pclItem;
			};

			struct DecoderEvent
			{
				const Decoder *m_pclDecoder;
			};		

			enum EventType
			{
				DEVICE_CONNECTED,
				DEVICE_DISCONNECTED,

				ITEM_CREATED,
				ITEM_DESTROYED,

				DECODER_STATE_CHANGE
			};

			EventType m_tType;

			union
			{
				DeviceEvent		m_stDevice;
				ItemEvent		m_stItem;
				DecoderEvent	m_stDecoder;
			};
	};

	class IDccLiteServiceListener
	{
		public:	
			virtual void OnDccLiteEvent(const DccLiteEvent &event) = 0;

			virtual ~IDccLiteServiceListener()
			{
				//empty
			}
	};

	class IDccLite_DecoderServices
	{
		public:
			virtual void Decoder_OnStateChanged(Decoder& decoder) = 0;

			virtual Decoder *TryFindDecoder(RName id) const = 0;

			[[nodiscard]] virtual RName Decoder_GetSystemName() const noexcept = 0;
	};

	class IDccLite_DeviceServices
	{
		public:
			virtual Decoder &Device_CreateDecoder(
				IDevice_DecoderServices &dev,
				const std::string &className,
				DccAddress address,
				RName name,
				const rapidjson::Value &params
			) = 0;

			virtual void Device_DestroyDecoder(Decoder &) = 0;
		
			virtual void Device_SendPacket(const dcclite::NetworkAddress destination, const dcclite::Packet& packet) = 0;

			virtual void Device_RegisterSession(NetworkDevice &dev, const dcclite::Guid &configToken) = 0;
			virtual void Device_UnregisterSession(NetworkDevice &dev, const dcclite::Guid &sessionToken) = 0;				

			virtual void Device_NotifyInternalItemCreated(dcclite::IObject &item) const = 0;
			virtual void Device_NotifyInternalItemDestroyed(dcclite::IObject &item) const = 0;
			virtual void Device_NotifyStateChange(NetworkDevice &device) const = 0;
	};
}