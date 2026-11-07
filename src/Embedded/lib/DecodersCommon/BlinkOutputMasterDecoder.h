// Copyright (C) 2026 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include "Decoder.h"

#include "OutputDecoderFlags.h"

using BlinkOutputMasterDecoderFlags = OutputDecoderFlags<
	dcclite::BOUTMD_IGNORE_SAVED_STATE,
	0,																									//no inverted operation for master decoder
	dcclite::BOUTMD_ACTIVATE_ON_POWER_UP,	
	dcclite::BOUTMD_ACTIVE,																				//active mask
	0,																									//no user bits
	dcclite::BOUTMD_IGNORE_SAVED_STATE | dcclite::BOUTMD_ACTIVATE_ON_POWER_UP,							//packet mask
	dcclite::BOUTMD_IGNORE_SAVED_STATE | dcclite::BOUTMD_ACTIVATE_ON_POWER_UP | dcclite::BOUTMD_ACTIVE	//stream mask
>;

class BlinkOutputMasterDecoder : public Decoder
{
	private:
		BlinkOutputMasterDecoderFlags	m_fFlags;
		uint8_t							m_u8BlinkDecoderIndex;

	public:
		explicit BlinkOutputMasterDecoder(dcclite::Packet &packet);
		explicit BlinkOutputMasterDecoder(Storage::EpromStream &stream);

		void SaveConfig(Storage::EpromStream &stream) override;

		bool IsOutputDecoder() const override
		{
			return true;
		}

		dcclite::DecoderTypes GetType() const override
		{
			return dcclite::DecoderTypes::DEC_BLINK_OUTPUT_MASTER;
		};

		bool IsActive() const override
		{
			return m_fFlags.IsActive();
		}

		bool IsSyncRequired() const override
		{
			return false;
		}

		bool AcceptServerState(dcclite::DecoderStates state, const unsigned long time) override;
};
