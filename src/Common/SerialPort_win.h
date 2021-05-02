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

//may god forgive us for putting this on a header...
#include <windows.h>

#include <string_view>

namespace dcclite
{
	class SerialPort
	{
		public:
			SerialPort(std::string_view portName);
			~SerialPort();

			/**
				Write numOfBytes bytes from buffer to the port

				Returns number of bytes written

				This throws if the write fails
			
			*/
			size_t Write(void *buffer, std::size_t numOfBytes);

			size_t Read(void *buffer, std::size_t bufferSize);

		private:
			HANDLE m_hComPort;

			std::string m_strName;
	};	

} //end of namespace dcclite

