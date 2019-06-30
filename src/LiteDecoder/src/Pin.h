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

#include <BasicPin.h>

#include <Arduino.h>

class Pin : public dcclite::BasicPin
{
	public:
		enum Modes
		{
			MODE_OUTPUT = OUTPUT,
			MODE_INPUT = INPUT,
			MODE_INPUT_PULLUP = INPUT_PULLUP
		};

		enum Voltage
		{
			VLOW = LOW,
			VHIGH = HIGH
		};

	public:
		Pin()
		{
			//empty
		}

		Pin(const dcclite::PinType_t pin, const Modes mode) :
			BasicPin(pin)
		{
			this->Attach(pin, mode);
		}

		void Attach(const dcclite::PinType_t pin, const Modes mode)
		{
			assert(!dcclite::IsPinNull(pin));

			BasicPin::Attach(pin);

			pinMode(this->Raw(), mode == MODE_OUTPUT ? OUTPUT : (mode == MODE_INPUT ? INPUT : INPUT_PULLUP));
		}

		void DigitalWrite(const Voltage value)
		{
			assert(*this);

			digitalWrite(this->Raw(), value == VHIGH ? HIGH : LOW);
		}

		Voltage DigitalRead()
		{
			assert(*this);

			return digitalRead(this->Raw()) == HIGH ? VHIGH : VLOW;
		}
};


