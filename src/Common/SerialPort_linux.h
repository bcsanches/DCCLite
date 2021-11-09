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

#include <string_view>

namespace dcclite
{
	constexpr auto DATA_PACKET_SIZE = 512;

	class SerialPort
	{
		public:
			class DataPacket
			{
				public:
					DataPacket();
					~DataPacket();

					bool IsDataReady();

					unsigned int GetDataSize() const
					{
						return m_uDataSize;
					}

					const uint8_t* GetData() const
					{
						return m_u8Data;
					}

					unsigned int GetNumFreeBytes() const
					{
						return DATA_PACKET_SIZE - m_uDataSize;
					}

					/**
						This actually reset data size counter

						Usually is only called after a Write is complete, so you can re-use the packet for writing more data

					*/
					void Clear();

					void WriteData(const uint8_t* data, unsigned int dataSize);

				private:
					uint8_t m_u8Data[DATA_PACKET_SIZE];
					unsigned int m_uDataSize = { 0 };					

					bool m_fWaiting = false;

					friend class SerialPort;
				};

		public:
			SerialPort(std::string_view portName);
			~SerialPort();

			void Read(DataPacket& packet);
			void Write(DataPacket& packet);

		private:			
			std::string m_strName;
	};
} //end of namespace dcclite

