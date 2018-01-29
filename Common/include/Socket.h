#pragma once

#include <cstddef>
#include <cstdint>
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

		}

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

		inline std::uint32_t GetAddress() const { return m_uAddress; }

		inline Port_t GetPort() const { return m_uPort; }

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
				EMPTY,
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

			bool TryOpen(Port_t port, Type type);

			bool TryListen(int backlog = 8);
			bool TryConnect(const Address &server);

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
}

