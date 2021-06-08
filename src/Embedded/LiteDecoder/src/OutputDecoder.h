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

#include "Decoder.h"

#include "Pin.h"

class OutputDecoder : public Decoder
{	
	private:
		uint16_t		m_uFlagsStorageIndex = 0;
		Pin				m_clPin;
		uint8_t			m_fFlags = 0;

	public:
		explicit OutputDecoder(dcclite::Packet &packet);
		explicit OutputDecoder(Storage::EpromStream &stream);

		void SaveConfig(Storage::EpromStream &stream) override;

		bool IsOutputDecoder() const override
		{
			return true;
		}

		dcclite::DecoderTypes GetType() const override
		{
			return dcclite::DecoderTypes::DEC_OUTPUT;
		};

		bool AcceptServerState(dcclite::DecoderStates state) override;

		bool IsActive() const override
		{
			return m_fFlags & dcclite::OUTD_ACTIVE;
		}

		bool IsSyncRequired() const override
		{
			return false;
		}

	private:
		void Init();

		void OperatePin();
};
