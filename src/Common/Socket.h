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

	class Address
	{
		public:
			Address() :
				m_uAddress{ 0 },
				m_uPort{ 0 }
			{
				//empty
			}

			Address(const Address &rhs) = default;			

			inline Address(std::uint_fast8_t a, std::uint_fast8_t b, std::uint_fast8_t c, std::uint_fast8_t d, Port_t port):
				m_uAddress{ (std::uint32_t{ a } << 24) | (std::uint32_t{ b } << 16) | (std::uint32_t{ c } << 8) | std::uint32_t{ d } },
				m_uPort{ port }
			{
				//empty
			}

			Address(std::uint_fast32_t address, Port_t port) :
				m_uAddress{ address },
				m_uPort{ port }
			{
				//emtpy
			}

			Address &operator=(const Address &rhs) = default;			

			inline std::uint32_t GetAddress() const { return m_uAddress; }

			inline std::string GetIpString() const
			{
				std::stringstream stream;

				stream <<
					((m_uAddress >> 24) & 0x000000FF) << "." <<
					((m_uAddress >> 16) & 0x000000FF) << "." <<
					((m_uAddress >> 8) & 0x000000FF) << "." <<
					((m_uAddress >> 0) & 0x000000FF);

				return stream.str();
			}

			inline std::uint_fast8_t GetA() const { return static_cast<std::uint_fast8_t>((m_uAddress >> 24) & 0x000000FF); }
			inline std::uint_fast8_t GetB() const { return static_cast<std::uint_fast8_t>((m_uAddress >> 16) & 0x000000FF); }
			inline std::uint_fast8_t GetC() const { return static_cast<std::uint_fast8_t>((m_uAddress >> 8) & 0x000000FF); }
			inline std::uint_fast8_t GetD() const { return static_cast<std::uint_fast8_t>((m_uAddress >> 0) & 0x000000FF); }

			inline Port_t GetPort() const { return m_uPort; }

			inline bool operator==(const Address &rhs) const noexcept
			{
				return (m_uAddress == rhs.m_uAddress) && (m_uPort == rhs.m_uPort);
			}

			inline bool operator!=(const Address &rhs) const noexcept
			{
				return (m_uAddress != rhs.m_uAddress) || (m_uPort != rhs.m_uPort);
			}

		private:
			std::uint_fast32_t m_uAddress;
			Port_t m_uPort;
	};

	class Socket
	{
		public:
			enum class Status
			{
				OK = 0,
				WOULD_BLOCK,
				DISCONNECTED
			};

			enum class Type
			{
				STREAM,
				DATAGRAM
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
			Socket(Socket &&other);
			Socket(const Socket &other) = delete;
			~Socket();

			Socket &operator=(Socket &&other);

			bool Open(Port_t port, Type type);

			bool Listen(int backlog = 8);
			bool StartConnection(const Address &server);

			Status GetConnectionProgress();

			std::tuple<Status, Socket, Address> TryAccept();

			void Close();

			bool IsOpen() const;

			bool Send(const Address &destination, const void *data, size_t size);

			std::tuple<Status, size_t> Receive(Address &sender, void *data, size_t size);
			std::tuple<Status, size_t> Receive(void *data, size_t size);

		private:
			Handler_t m_hHandle;

			static inline size_t g_iCount = 0;
	};
} //end of namespace dcclite

