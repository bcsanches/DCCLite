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

#include "BasicPin.h"

class Decoder;
class SensorDecoder;
class ServoTurnoutDecoder;

namespace LocalDecoderManager
{

	enum ButtonActions
	{
		kTHROW = 0x01,
		kCLOSE = 0x02,
		kTOGGLE = kTHROW | kCLOSE
	};

	ServoTurnoutDecoder *CreateServoTurnout(
		uint8_t flags,
		dcclite::PinType_t pin,
		uint8_t range,
		uint8_t ticks,
		dcclite::PinType_t powerPin = dcclite::NullPin,
		dcclite::PinType_t frogPin = dcclite::NullPin
	);		

	SensorDecoder *CreateSensor(
		uint8_t flags,
		dcclite::PinType_t pin,
		uint8_t activateDelay = 0,
		uint8_t deactivateDelay = 0
	);

	void CreateButton(SensorDecoder &sensor, ServoTurnoutDecoder &target, ButtonActions actions);
	
	bool Update(const unsigned long ticks);

	Decoder *TryGetDecoder(const uint8_t slot);
}
