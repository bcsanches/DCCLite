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

#include <dcclite/Object.h>

#include "Address.h"
#include "IDevice.h"

namespace dcclite
{
	class Packet;
}

namespace dcclite::broker::exec::dcc
{	
	class IDccLite_DecoderServices;
	class Node;	

	class Decoder: public dcclite::Object
	{	
		public:
			Decoder(			
				const Address &address, 
				RName name,
				IDccLite_DecoderServices &owner,
				IDevice_DecoderServices &dev,
				const rapidjson::Value &params
			);

			inline Address GetAddress() const
			{
				return m_iAddress;
			}

			inline RName GetLocationHint() const
			{
				return m_rnLocationHint;
			}		

			virtual void InitAfterDeviceLoad()
			{
				//empty
			}

			inline RName GetDeviceName() const noexcept
			{
				return m_rclDevice.GetDeviceName();
			}

			inline IDevice_DecoderServices &GetDevice() const noexcept
			{
				return m_rclDevice;
			}

			//
			//IObject
			//
			//

			const char *GetTypeName() const noexcept override
			{
				return "Decoder";
			}

			void Serialize(dcclite::JsonOutputStream_t &stream) const override;

		private:
			Address m_iAddress;				

			RName m_rnLocationHint;

		protected:
			IDccLite_DecoderServices &m_rclManager;
			IDevice_DecoderServices &m_rclDevice;
	};
}

