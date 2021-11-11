// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.
#include "SerialPort_linux.h"

#include <fmt/format.h>

namespace dcclite
{	
	bool SerialPort::DataPacket::IsDataReady()
	{
		if (!m_fWaiting)
			return true;		

		//
		// We have a pending IO operation, check what is going on
		//

		DWORD numBytesTransferred = 0;
		if (!GetOverlappedResultEx(m_hComPort, &m_stOverlapped, &numBytesTransferred, 0, FALSE))
		{
			auto error = GetLastError();

			//operation still in progress
			if (error == ERROR_IO_INCOMPLETE)
				return false;

			//push it back
			SetLastError(error);
			throw std::runtime_error(fmt::format("[SerialPort::DataPacket::IsDataReady] GetOverlappedResultEx failed: {}", GetSystemLastErrorMessage()));
		}

		//
		//we got some data
		m_fWaiting = false;
		m_uDataSize = numBytesTransferred;

		return true;
	}

	void SerialPort::DataPacket::WriteData(const uint8_t *data, unsigned int dataSize)
	{
		if(m_fWaiting)
			throw std::runtime_error("[SerialPort::DataPacket::WriteData] Packet is waiting, cannot write");

		if (dataSize > this->GetNumFreeBytes())
			throw std::overflow_error(fmt::format("[SerialPort::DataPacket::WriteData] Caller requested to write {} bytes, but only {} are left", dataSize, this->GetNumFreeBytes()));

		memcpy(m_u8Data + m_uDataSize, data, dataSize);
		m_uDataSize += dataSize;
	}

	void SerialPort::DataPacket::Clear()
	{
		if(m_fWaiting)
			throw std::runtime_error("[SerialPort::DataPacket::WriteData] Packet is waiting, cannot clear");

		m_uDataSize = 0;
	}


	SerialPort::SerialPort(std::string_view portName):
		m_strName(portName)
	{
		m_iPortHandle = open(portName, O_RDWR);
		if (m_iPortHandle < 0)
			throw std::runtime_error(fmt::format("[SerialPort] {}: Error opening port - {}", portName, strerror(errno)));

		DCB dcb;

		//  Initialize the DCB structure.
		SecureZeroMemory(&dcb, sizeof(DCB));
		dcb.DCBlength = sizeof(DCB);

		if (!GetCommState(m_hComPort, &dcb))
		{
			CloseHandle(m_hComPort);

			throw std::runtime_error(fmt::format("[SerialPort] {}: Cannot read DCB data - {}", portName, GetSystemLastErrorMessage()));
		}

		DCB old = dcb;

		dcb.BaudRate = CBR_57600;
		dcb.fBinary = TRUE;
		dcb.fParity = NOPARITY;
		dcb.fOutxCtsFlow = FALSE;
		dcb.fOutxDsrFlow = FALSE;
		dcb.fDtrControl = DTR_CONTROL_DISABLE;
		dcb.fDsrSensitivity = FALSE;
		dcb.fTXContinueOnXoff = FALSE;
		dcb.fOutX = FALSE;
		dcb.fInX = FALSE;
		dcb.fErrorChar = FALSE;
		dcb.fNull = FALSE;
		dcb.fRtsControl = RTS_CONTROL_DISABLE;	
		dcb.fAbortOnError = FALSE;
		dcb.XonLim = 0;
		dcb.XoffLim = 0;
		dcb.ByteSize = 8;
		dcb.Parity = NOPARITY;
		dcb.StopBits = ONESTOPBIT;

		/**
		dcb.XonChar = (char)0x8d;
		dcb.XoffChar = (char)0x96;
		dcb.ErrorChar = (char)0xF6;
		dcb.EofChar = (char)0x20;
		dcb.EvtChar = (char)0x00;		
		*/

		if (!SetCommState(m_hComPort, &dcb))
		{
			CloseHandle(m_hComPort);

			throw std::runtime_error(fmt::format("[SerialPort] {}: Cannot update CommState - {}", portName, GetSystemLastErrorMessage()));
		}

		COMMTIMEOUTS timeouts;
		if (!GetCommTimeouts(m_hComPort, &timeouts))
		{
			CloseHandle(m_hComPort);

			throw std::runtime_error(fmt::format("[SerialPort] {}: GetCommTimeouts failed - {}", portName, GetSystemLastErrorMessage()));
		}

		timeouts.ReadIntervalTimeout = 0x64;
		
		if (!SetCommTimeouts(m_hComPort, &timeouts))
		{
			CloseHandle(m_hComPort);

			throw std::runtime_error(fmt::format("[SerialPort] {}: SetCommTimeouts failed - {}", portName, GetSystemLastErrorMessage()));
		}
	}

	SerialPort::~SerialPort()
	{
		//Cancel any pending overlapped operation
		CancelIo(m_hComPort);		
		CloseHandle(m_hComPort);
	}

	void SerialPort::Write(DataPacket &packet)
	{
		if (packet.m_fWaiting)
		{
			throw std::logic_error(fmt::format("[SerialPort::Write] {}: DataPacket is on WaitingState, cannot be used for another write, WAIT!", m_strName));
		}
		
		const auto dataSize = packet.GetDataSize();

		//empty packet? Duh... ignore
		if (!dataSize)
			return;

		DWORD bytesWritten = 0;
		if (!WriteFile(m_hComPort, packet.GetData(), dataSize, &bytesWritten, &packet.m_stOverlapped))
		{
			auto errorCode = GetLastError();
			if (errorCode != ERROR_IO_PENDING)
			{
				SetLastError(errorCode);

				throw std::runtime_error(fmt::format("[SerialPort::Write] {}: Error ASYNC writing to port - {}", m_strName, GetSystemLastErrorMessage()));
			}

			//we must wait
			packet.m_fWaiting = true;
			packet.m_hComPort = m_hComPort;
		}		

		//if above if not entered, data went thought, nothing left to do
	}

	void SerialPort::Read(DataPacket &packet)
	{
		if (packet.m_fWaiting)
		{
			throw std::logic_error(fmt::format("[SerialPort::AsyncRead] {}: DataPacket is on WaitingState, cannot be used for another read, WAIT!", m_strName));
		}

		packet.m_uDataSize = 0;

		DWORD dwBytesRead;
		if (!ReadFile(m_hComPort, packet.m_u8Data, sizeof(packet.m_u8Data), &dwBytesRead, &packet.m_stOverlapped))
		{
			auto errorCode = GetLastError();
			if (errorCode != ERROR_IO_PENDING)
			{
				SetLastError(errorCode);

				throw std::runtime_error(fmt::format("[SerialPort::AsyncRead] {}: Error ASYNC reading from port - {}", m_strName, GetSystemLastErrorMessage()));
			}

			packet.m_fWaiting = true;
			packet.m_hComPort = m_hComPort;
		}
		else
		{
			//some data was read
			packet.m_uDataSize = dwBytesRead;
		}
	}

}