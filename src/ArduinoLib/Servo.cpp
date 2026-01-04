// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Servo.h"

#include <dcclite/Log.h>

void Servo::attach(int pin)
{
	m_iPin = pin;

	dcclite::Log::Trace("[Servo::attach] Pin {}", m_iPin);
}


void Servo::write(int angle)
{
	m_iAngle = angle;

	dcclite::Log::Trace("[Servo::write] Pin {} moved to {}", m_iPin, m_iAngle);
}

int Servo::read() const
{
	return m_iAngle;
}

void Servo::detach()
{
	//already detached? Ignore...
	if(m_iPin < 0)
		return;

	dcclite::Log::Trace("[Servo::detach] Pin {}", m_iPin);

	m_iPin = -1;
}

bool Servo::attached() const noexcept
{
	return m_iPin >= 0;
}
