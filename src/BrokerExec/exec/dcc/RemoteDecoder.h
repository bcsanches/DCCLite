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

#include "StateDecoder.h"

#include <sigslot/signal.hpp>

namespace dcclite
{
	class Packet;
}

namespace dcclite::broker::exec::dcc
{
	/**
		RemoteDecoders are decoders that represent a decoder implemented on a remote device, like an Arduino.

		So they are used to control the remote state of the "physical" decoders sending commands


	*/
	class RemoteDecoder: public StateDecoder
	{	
		public:
			RemoteDecoder(			
				const Address &address, 
				RName name,
				IDccLite_DecoderServices &owner,
				IDevice_DecoderServices &dev,
				const rapidjson::Value &params
			);		
				

			bool SyncRemoteState(dcclite::DecoderStates state);

			inline bool IsBroken() const noexcept
			{
				return m_fBroken;
			}

			virtual void WriteConfig(dcclite::Packet &packet) const;			

			//
			//IObject
			//
			//
		
			void Serialize(dcclite::JsonOutputStream_t &stream) const override;		

			sigslot::signal<RemoteDecoder &> m_sigRemoteStateSync;

		protected:
			virtual dcclite::DecoderTypes GetType() const noexcept = 0;

		private:							
			bool		m_fBroken;
	};
}


