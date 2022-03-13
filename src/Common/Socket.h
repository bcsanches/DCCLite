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

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>

#include "defs.h"

//
//
// Based on: https://gafferongames.com/post/sending_and_receiving_packets/
//
//

namespace dcclite
{
	typedef std::uint_fast16_t Port_t;

	class IpAddress
	{
		public:
			inline IpAddress() noexcept:
				m_uAddress{ 0 }
			{
				//empty
			}

			IpAddress(const IpAddress& rhs) noexcept = default;
			inline IpAddress(std::uint_fast8_t a, std::uint_fast8_t b, std::uint_fast8_t c, std::uint_fast8_t d) noexcept :
				m_uAddress{ (std::uint32_t{ a } << 24) | (std::uint32_t{ b } << 16) | (std::uint32_t{ c } << 8) | std::uint32_t{ d } }
			{
				//empty
			}

			inline IpAddress(std::uint_fast32_t address) noexcept:
				m_uAddress{ address }
			{

			}

			IpAddress& operator=(const IpAddress& rhs) noexcept = default;

			inline std::uint32_t GetAddress() const noexcept { return m_uAddress; }

			inline std::string GetIpString() const noexcept
			{
				std::stringstream stream;

				stream <<
					((m_uAddress >> 24) & 0x000000FF) << "." <<
					((m_uAddress >> 16) & 0x000000FF) << "." <<
					((m_uAddress >> 8) & 0x000000FF) << "." <<
					((m_uAddress >> 0) & 0x000000FF);

				return stream.str();
			}

			inline std::uint_fast8_t GetA() const noexcept { return static_cast<std::uint_fast8_t>((m_uAddress >> 24) & 0x000000FF); }
			inline std::uint_fast8_t GetB() const noexcept { return static_cast<std::uint_fast8_t>((m_uAddress >> 16) & 0x000000FF); }
			inline std::uint_fast8_t GetC() const noexcept { return static_cast<std::uint_fast8_t>((m_uAddress >> 8) & 0x000000FF); }
			inline std::uint_fast8_t GetD() const noexcept { return static_cast<std::uint_fast8_t>((m_uAddress >> 0) & 0x000000FF); }

			inline bool operator==(const IpAddress& rhs) const noexcept
			{
				return (m_uAddress == rhs.m_uAddress);
			}

			inline bool operator!=(const IpAddress& rhs) const noexcept
			{
				return (m_uAddress != rhs.m_uAddress);
			}			

		private:
			std::uint_fast32_t m_uAddress;

	};

	class NetworkAddress: public IpAddress
	{
		public:
			inline NetworkAddress() noexcept :				
				m_uPort{ 0 }
			{
				//empty
			}

			NetworkAddress(const NetworkAddress &rhs) = default;			

			inline NetworkAddress(std::uint_fast8_t a, std::uint_fast8_t b, std::uint_fast8_t c, std::uint_fast8_t d, Port_t port):
				IpAddress{a, b, c, d},				
				m_uPort{ port }
			{
				//empty
			}

			inline NetworkAddress(std::uint_fast32_t address, Port_t port) noexcept:
				IpAddress{ address },				
				m_uPort{ port }
			{
				//emtpy
			}

			NetworkAddress &operator=(const NetworkAddress &rhs) = default;			

			inline Port_t GetPort() const { return m_uPort; }

			inline bool operator==(const NetworkAddress &rhs) const noexcept
			{
				const IpAddress &a = *this, &b = rhs;

				return (a == b) && (m_uPort == rhs.m_uPort);
			}

			inline bool operator!=(const NetworkAddress &rhs) const noexcept
			{
				const IpAddress& a = *this, & b = rhs;

				return (a != b) || (m_uPort != rhs.m_uPort);				
			}

			static NetworkAddress ParseAddress(const std::string_view address);

		private:			
			Port_t m_uPort;
	};

	class Socket
	{
		public:
			enum class Status
			{
				OK = 0,
				WOULD_BLOCK,
				DISCONNECTED,
				CONNRESET
			};

			enum class Type
			{
				STREAM,
				DATAGRAM
			};

			enum Flags
			{
				FLAG_ADDRESS_REUSE = 0x01
			};

#ifdef DCCLITE64
			typedef std::int64_t Handler_t;
#else
			typedef std::int32_t Handler_t;
#endif

		private:
			Socket(Handler_t validHandle);

		public:
			Socket();
			Socket(Socket &&other) noexcept;
			Socket(const Socket &other) = delete;
			~Socket();

			Socket &operator=(Socket &&other) noexcept;

			bool Open(Port_t port, Type type, uint32_t flags = 0);

			bool Listen(int backlog = 8);
			bool StartConnection(const NetworkAddress &server);
			bool StartConnection(Port_t port, Type type, const NetworkAddress &server);

			Status GetConnectionProgress();

			std::tuple<Status, Socket, NetworkAddress> TryAccept();

			void Close();

			bool IsOpen() const;

			bool Send(const NetworkAddress &destination, const void *data, size_t size);
			std::tuple<Status, size_t> Send(const void *data, size_t size);

			std::tuple<Status, size_t> Receive(NetworkAddress &sender, void *data, const size_t size, const bool truncate = false);
			std::tuple<Status, size_t> Receive(void *data, size_t size);

			bool JoinMulticastGroup(const IpAddress &address);		

		private:
			Handler_t m_hHandle;

			static inline size_t g_iCount = 0;
	};
} //end of namespace dcclite

