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

#include <cstdint>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include <string_view>

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

		fd_set set;

		FD_ZERO(&set);
		FD_SET(this->m_iPortHandle, &set);

		timeval tval = { 0 };

		auto result = select(FD_SETSIZE, &set, nullptr, nullptr, &tval);

		if(result < 0)
		{
			throw std::runtime_error("[SerialPort::DataPacket::IsDataReady] select failed");
		}

		if (result == 0)
			return false;


		int numBytesTransferred = read(m_iPortHandle, m_u8Data, DATA_PACKET_SIZE);

		//should not happen... but...
		if (numBytesTransferred == 0)
			return false;

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
		m_iPortHandle = open(m_strName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
		if (m_iPortHandle < 0)
			throw std::runtime_error(fmt::format("[SerialPort] {}: Error opening port - {}", portName, strerror(errno)));

		termios options;

		//get current port settings
		tcgetattr(m_iPortHandle, &options);

		//
		//set BAUD Rate
		cfsetispeed(&options, B57600);
		cfsetospeed(&options, B57600);

		//
		//No parity
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		options.c_cflag &= ~CSIZE;
		options.c_cflag |= CS8;
		
		//
		//Disable CTS
		options.c_cflag &= ~CRTSCTS;

		//
		//Disable parity
		options.c_cflag &= ~PARENB;

		//
		//One stop bit
		options.c_cflag &= ~CSTOPB;

		options.c_cc[VMIN] = 1;
		options.c_cc[VTIME] = 0x64;

		//
		//Disable DTR, DSR, TX (TIOCM_ST)
		int status;

		ioctl(m_iPortHandle, TIOCMGET, &status);

		status &= ~(TIOCM_DTR | TIOCM_DSR | TIOCM_ST);

		ioctl(m_iPortHandle, TIOCMSET, status);
										
		//enable the changes
		if (tcsetattr(m_iPortHandle, TCSANOW, &options))
		{
			int localError = errno;
			close(m_iPortHandle);

			throw std::runtime_error(fmt::format("[SerialPort] {}: Call to tcsetattr failed - {}", portName, localError));
		}		

		//
		//make read non blocking
		fcntl(m_iPortHandle, F_SETFL, FNDELAY);
	}

	SerialPort::~SerialPort()
	{		
		close(m_iPortHandle);		
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

		auto bytesWritten = write(m_iPortHandle, packet.GetData(), packet.GetDataSize());
		packet.m_fWaiting = false;

		if (bytesWritten < 0)
		{
			if (errno != EWOULDBLOCK)
				throw std::runtime_error(fmt::format("[SerialPortal::Write] {}: Write failed, error: {}", m_strName, errno));
			
			//try later...
			packet.m_fWaiting = true;
			packet.m_iPortHandle = this->m_iPortHandle;				
		}		
	}

	void SerialPort::Read(DataPacket &packet)
	{
		if (packet.m_fWaiting)
		{
			throw std::logic_error(fmt::format("[SerialPort::AsyncRead] {}: DataPacket is on WaitingState, cannot be used for another read, WAIT!", m_strName));
		}

		packet.m_uDataSize = 0;

		auto bytesRead = read(m_iPortHandle, packet.m_u8Data, sizeof(packet.m_u8Data));
		if (bytesRead < 0)
		{
			if (errno != EWOULDBLOCK)
				throw std::runtime_error(fmt::format("[SerialPortal::Read] {}: Read failed, error: {}", m_strName, errno));
			
			//try later...
			packet.m_fWaiting = true;
			packet.m_iPortHandle = this->m_iPortHandle;			
		}
		else
		{
			packet.m_uDataSize = bytesRead;
		}		
	}
}