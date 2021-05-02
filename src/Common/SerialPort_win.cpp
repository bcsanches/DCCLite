// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.
#include "SerialPort_win.h"

#include <fmt/format.h>

#include "Util.h"

namespace dcclite
{
	SerialPort::SerialPort(std::string_view portName):
		m_strName(portName)
	{
		m_hComPort = ::CreateFile(
			portName.data(),
			GENERIC_READ | GENERIC_WRITE,
			0,
			0,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			0
		);

		if (m_hComPort == INVALID_HANDLE_VALUE)
			throw std::runtime_error(fmt::format("[SerialPort] {}: Error opening port - {}", portName, GetSystemLastErrorMessage()));

		DCB dcb;

		if (!GetCommState(m_hComPort, &dcb))
		{
			CloseHandle(m_hComPort);

			throw std::runtime_error(fmt::format("[SerialPort] {}: Cannot read DCB data - {}", portName, GetSystemLastErrorMessage()));
		}

		dcb.fDtrControl = DTR_CONTROL_ENABLE;
		dcb.fRtsControl = RTS_CONTROL_ENABLE;
		dcb.fOutxDsrFlow = FALSE;
		dcb.fOutxCtsFlow = FALSE;
		dcb.StopBits = ONESTOPBIT;
		dcb.Parity = NOPARITY;
		dcb.BaudRate = CBR_57600;
		dcb.fDtrControl = DTR_CONTROL_ENABLE;

		if (!SetCommState(m_hComPort, &dcb))
		{
			CloseHandle(m_hComPort);

			throw std::runtime_error(fmt::format("[SerialPort] {}: Cannot update CommState - {}", portName, GetSystemLastErrorMessage()));
		}
	}

	SerialPort::~SerialPort()
	{
		CloseHandle(m_hComPort);
	}

	size_t SerialPort::Write(void *buffer, std::size_t numOfBytes)
	{
		DWORD bytesWritten = 0;
		if (!WriteFile(m_hComPort, buffer, numOfBytes, &bytesWritten, nullptr))
		{
			throw std::runtime_error(fmt::format("[SerialPort] {}: Error writing {} bytes to port - {}", m_strName, numOfBytes, GetSystemLastErrorMessage()));			
		}

		return bytesWritten;
	}

	size_t SerialPort::Read(void *buffer, std::size_t bufferSize)
	{
		DWORD dwBytesRead;

		if (!ReadFile(m_hComPort, buffer, bufferSize, &dwBytesRead, nullptr))
		{
			throw std::runtime_error(fmt::format("[SerialPort] {}: Error reading {} bytes from port - {}", m_strName, bufferSize, GetSystemLastErrorMessage()));
		}

		return dwBytesRead;
	}

}