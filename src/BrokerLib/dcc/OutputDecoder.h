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

#include <optional>

#include "RemoteDecoder.h"

#include "SharedLibDefs.h"

namespace dcclite::broker
{

	class OutputDecoder : public RemoteDecoder
	{
		public:
			OutputDecoder(
				const DccAddress& address,
				RName name,
				IDccLite_DecoderServices & owner,
				IDevice_DecoderServices &dev,
				const rapidjson::Value& params
			) :
				RemoteDecoder(address, name, owner, dev, params)
			{
				//empty
			}		

			dcclite::DecoderTypes GetType() const noexcept override
			{
				return dcclite::DecoderTypes::DEC_OUTPUT;
			}

			void Activate(const char *requester)
			{
				this->SetState(dcclite::DecoderStates::ACTIVE, requester);			
			}

			void Deactivate(const char *requester)
			{
				this->SetState(dcclite::DecoderStates::INACTIVE, requester);			
			}

			bool SetState(const dcclite::DecoderStates newState, const char *requester);

			void ToggleState(const char *requester)
			{
				this->SetState(m_kRequestedState == dcclite::DecoderStates::ACTIVE ? dcclite::DecoderStates::INACTIVE : dcclite::DecoderStates::ACTIVE, requester);			
			}

			dcclite::DecoderStates GetRequestedState() const
			{
				return m_kRequestedState;
			}
		
			bool IsOutputDecoder() const override
			{
				return true;
			}

			bool IsInputDecoder() const override
			{
				return false;
			}

			std::optional<dcclite::DecoderStates> GetPendingStateChange() const
			{
				return m_kRequestedState != this->GetState() ? std::optional{ m_kRequestedState } : std::nullopt;
			}		

			void Serialize(dcclite::JsonOutputStream_t& stream) const override;

			//
			//IObject
			//
			//

			const char* GetTypeName() const noexcept override
			{
				return "OutputDecoder";
			}				

		private:				
			dcclite::DecoderStates m_kRequestedState = dcclite::DecoderStates::INACTIVE;
	};
}