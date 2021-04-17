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

#include <SharedLibDefs.h>

namespace dcclite
{
	typedef unsigned char PinType_t;

	constexpr PinType_t PIN_NULL_BIT_MASK = 0x80;
	constexpr PinType_t PIN_DATA_BIT_MASK = 0x7F;

	inline bool IsPinNull(const PinType_t pin)
	{
		return pin & PIN_NULL_BIT_MASK;
	}

	constexpr PinType_t NullPin = PIN_NULL_BIT_MASK;	

	class BasicPin
	{	
		public:
			BasicPin() :
				m_tPin{ NullPin }
			{
				//empty
			}

			explicit BasicPin(const PinType_t pin) :
				m_tPin{ pin }
			{				
				//empty
			}			

			void Detach()
			{
				m_tPin = NullPin;
			}

			operator bool() const
			{
				return !IsPinNull(m_tPin);
			}		

			PinType_t Raw() const
			{
				return m_tPin;
			}

		protected:
			void Attach(const PinType_t pin)
			{
				m_tPin = pin;
			}

		private:
			PinType_t m_tPin;			
	};	
}
