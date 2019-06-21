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

#include <Arduino.h>

#include <EmbeddedLibDefs.h>

namespace dcclite
{
	class Pin
	{
		public:
			enum Modes
			{
				MODE_OUTPUT = OUTPUT,
				MODE_INPUT = INPUT,
				MODE_INPUT_PULLUP = INPUT_PULLUP
			};

		public:
			Pin() :
				m_tPin{ 0 },
				m_tNull{ true }
			{
				//empty
			}

			Pin(unsigned char pin, Modes mode):
				Pin()
			{
				this->Attach(pin, mode);
			}
			
			operator bool() const
			{
				return !m_tNull;
			}			

			void Attach(unsigned char pin, Modes mode)
			{
				m_tNull = false;
				m_tPin = pin;

				pinMode(m_tPin, mode == MODE_OUTPUT ? OUTPUT : (mode == MODE_INPUT ? INPUT : INPUT_PULLUP));
			}

			void Detach()
			{
				m_tNull = true;
			}

			void DigitalWrite(bool value)
			{
				assert(!m_tNull);

				digitalWrite(m_tPin, value ? HIGH : LOW);
			}

			unsigned char Num() const
			{
				return m_tPin;
			}		

		private:
			unsigned char m_tPin:7;
			bool m_tNull : 1;
	};
}
