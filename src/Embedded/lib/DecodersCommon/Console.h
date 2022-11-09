// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#ifndef CONSOLE_NET_H
#define CONSOLE_NET_H

#include "Strings.h"
#include <Arduino.h>

namespace Console
{
	typedef __FlashStringHelper ConsoleFlashStringHelper_t;

	class EndLineMarkerTag
	{

	};

	class OutputStream
	{
		public:
			OutputStream()
			{
				//empty
			}

			OutputStream &operator<<(const char *str)
			{
				Serial.print(str);

				return *this;
			}		

			OutputStream &operator<<(const ConsoleFlashStringHelper_t *fstr)
			{
				Serial.print(fstr);

				return *this;
			}

			OutputStream &operator<<(char value)
			{
				Serial.print(value);

				return *this;
			}

			OutputStream &operator<<(unsigned char value)
			{
				Serial.print(value);

				return *this;
			}

			OutputStream &operator<<(int value)
			{
				Serial.print(value);

				return *this;
			}

			OutputStream &operator<<(unsigned int value)
			{
				Serial.print(value);

				return *this;
			}

			OutputStream &operator<<(unsigned long int value)
			{
				Serial.print(value);

				return *this;
			}
			
			OutputStream &HexNumber(int value)
			{
				Serial.print(value, HEX);

				return *this;
			}

			OutputStream &HexNumber(unsigned int value)
			{
				Serial.print(value, HEX);

				return *this;
			}

			OutputStream &HexNumber(unsigned long int value)			
			{
				Serial.print(value, HEX);

				return *this;
			}			

			OutputStream &operator<<(const EndLineMarkerTag &tag)
			{
				Serial.println();

				return *this;
			}

			OutputStream &IpNumber(const unsigned char src_ip[4])
			{
				*this << src_ip[0] << '.' << src_ip[1] << '.' << src_ip[2] << '.' << src_ip[3];

				return *this;
			}
	};	

	extern void Printf(ConsoleFlashStringHelper_t *format, ...);
	
	extern void Init();		

	extern void Update();

	extern bool Custom_ParseCommand(const char *command);
};

#define DCCLITE_LOG Console::OutputStream{}

#define DCCLITE_ENDL Console::EndLineMarkerTag{}

#define DCCLITE_LOG_MODULE_EX(MC_stream) MC_stream << '[' << MODULE_NAME << F("] ")

#define DCCLITE_LOG_MODULE_LN_EX(MC_stream, x) DCCLITE_LOG_MODULE_EX(MC_stream) << x << DCCLITE_ENDL

#define DCCLITE_LOG_MODULE_LN(x) DCCLITE_LOG_MODULE_LN_EX(Console::OutputStream{} , x)



#endif
