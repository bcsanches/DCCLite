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

#include "ArduinoLibDefs.h"

class ARDUINO_API Servo
{
	public:
		void attach(int pin);

		void write(int angle);

		int read() const;

		void detach();

		bool attached() const noexcept;

	private:
		int m_iPin = -1;

		int m_iAngle = 0;		
};
