// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Socket.h"

#include "LogUtils.h"
#include "Parser.h"

#include <cassert>
#include <stdexcept>

#include <spdlog/logger.h>

#define PLATFORM_WINDOWS  1
#define PLATFORM_MAC      2
#define PLATFORM_UNIX     3

#if defined(_WIN32)
#define PLATFORM PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM PLATFORM_MAC
#else
#define PLATFORM PLATFORM_UNIX
#endif

#if PLATFORM == PLATFORM_WINDOWS

#define FD_SETSIZE 1

#include <winsock2.h>
#include <Ws2tcpip.h>

static const dcclite::Socket::Handler_t NULL_SOCKET = INVALID_SOCKET;

typedef int socklen_t;

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

static const dcclite::Socket::Handler_t NULL_SOCKET = -1;

#endif

#if PLATFORM == PLATFORM_WINDOWS
#pragma comment( lib, "wsock32.lib" )
#pragma comment( lib, "Ws2_32.lib" )
#endif

namespace dcclite
{
	NetworkAddress NetworkAddress::ParseAddress(const std::string_view address)
	{
		Parser parser(address.data());

		uint8_t numbers[4];

		for (int i = 0; i < 4; ++i)
		{
			int a;
			if (parser.GetNumber(a) != Tokens::NUMBER)
				throw std::invalid_argument(fmt::format("[NetworkAddress::ParseAddress] Error parsing address {}, expected number", address));

			numbers[i] = a;

			if (i == 3)
				break;
			
			if (parser.GetToken(nullptr, 0) != Tokens::DOT)			
				throw std::invalid_argument(fmt::format("[NetworkAddress::ParseAddress] Error parsing address {}, expected dot", address));			
		}

		if(parser.GetToken(nullptr, 0) != Tokens::COLON)
			throw std::invalid_argument(fmt::format("[NetworkAddress::ParseAddress] Error parsing address {}, expected colon", address));

		int port;
		if(parser.GetNumber(port) != Tokens::NUMBER)
			throw std::invalid_argument(fmt::format("[NetworkAddress::ParseAddress] Error parsing address {}, expected port number", address));

		return NetworkAddress{ numbers[0], numbers[1], numbers[2], numbers[3], static_cast<uint16_t>(port) };
	}

	static sockaddr_in MakeAddr(const NetworkAddress &address)
	{						
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(address.GetAddress());
		addr.sin_port = htons(address.GetPort());

		return addr;
	}

	Socket::Socket() :
		m_hHandle(NULL_SOCKET)
	{
		if (g_iCount == 0)
		{
			#if PLATFORM == PLATFORM_WINDOWS
			WSADATA WsaData;
			if (WSAStartup(MAKEWORD(2, 2), &WsaData) != NO_ERROR)
			{
				throw std::runtime_error("WSAStartup failed.");
			}
			#endif
		}

		++g_iCount;
	}

	Socket::Socket(Handler_t validHandle):
		m_hHandle(validHandle)
	{
		assert(g_iCount);

		++g_iCount;
	}

	Socket::Socket(Socket &&other) noexcept:
		m_hHandle(std::move(other.m_hHandle))
	{
		assert(g_iCount > 0);

		++g_iCount;

		other.m_hHandle = NULL_SOCKET;
	}

	Socket::~Socket()
	{
		--g_iCount;

		this->Close();

		#if PLATFORM == PLATFORM_WINDOWS
		if(g_iCount == 0)
			WSACleanup();
		#endif
	}

	Socket &Socket::operator=(Socket &&other) noexcept
	{
		if (this != &other)
		{
			m_hHandle = other.m_hHandle;

			other.m_hHandle = NULL_SOCKET;
		}

		return *this;
	}

	bool Socket::Open(Port_t port, Type type, uint32_t flags)
	{
		assert(g_iCount > 0);

		if (m_hHandle != NULL_SOCKET)
			this->Close();

		auto intType = type == Type::DATAGRAM ? SOCK_DGRAM : SOCK_STREAM;
		auto intProto = type == Type::DATAGRAM ? IPPROTO_UDP : IPPROTO_TCP;

		m_hHandle = socket(AF_INET, intType, intProto);		

		if (m_hHandle == NULL_SOCKET)
		{
			LogGetDefault()->error("[Socket::Open] Failed to create socket.");
			return false;
		}

		#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

		int nonBlocking = 1;
		if (fcntl(m_hHandle, F_SETFL, O_NONBLOCK, nonBlocking) == -1)
		{
			LogGetDefault()->error("[Socket::Open] Failed to set socket to non-blocking mode.");			
			return false;
		}

		#elif PLATFORM == PLATFORM_WINDOWS

		DWORD nonBlocking = 1;
		if (ioctlsocket(m_hHandle, FIONBIO, &nonBlocking) != 0)
		{
			this->Close();

			LogGetDefault()->error("[Socket::Open] Failed to set socket to non-blocking mode.");
			return false;
		}

		int noDelay = 1;
		if ((type == Type::STREAM) && (setsockopt(m_hHandle, IPPROTO_TCP, TCP_NODELAY, (const char *)&noDelay, sizeof(int)) != 0))
		{
			this->Close();

			LogGetDefault()->error("[Socket::Open] Failed to enable NO_DELAY.");
			return false;
		}

		const char broadcast = 1;
		if ((type == Type::DATAGRAM) && (setsockopt(m_hHandle, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) != 0))
		{
			this->Close();

			LogGetDefault()->error("[Socket::Open] Failed to enable SO_BROADCAST.");
			return false;
		}

		if (flags & Flags::FLAG_ADDRESS_REUSE)
		{
			const char reuseaddr = 1;
			if (setsockopt(m_hHandle, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) != 0)
			{
				this->Close();

				LogGetDefault()->error("[Socket::Open] Failed to enable SO_REUSEADDR.");
				return false;
			}
		}

		#else
		#error "plataform"
		#endif

		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons((unsigned short)port);

		if (bind(m_hHandle, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0)
		{
			this->Close();

#if PLATFORM == PLATFORM_WINDOWS
			auto error = WSAGetLastError();
			LogGetDefault()->error("[Socket::Open] Failed to bind socket {}.", error);
#else
			LogGetDefault()->error("[Socket::Open] Failed to bind socket.");
#endif
			return false;
		}

		return true;
	}

	void Socket::Close()
	{		
		if (m_hHandle == NULL_SOCKET)
			return;

#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
		close(m_hHandle);
#elif PLATFORM == PLATFORM_WINDOWS
		closesocket(m_hHandle);
#endif

		m_hHandle = NULL_SOCKET;
	}

	bool Socket::Listen(int backlog)
	{
		assert(m_hHandle != NULL_SOCKET);

		return listen(m_hHandle, backlog) == 0;
	}

	bool Socket::StartConnection(Port_t port, Type type, const NetworkAddress &server)
	{
		if (!this->Open(port, type))
			return false;

		return this->StartConnection(server);
	}

	bool Socket::StartConnection(const NetworkAddress &serverAddress)
	{
		if (m_hHandle == NULL_SOCKET)
		{
			throw std::logic_error("[Socket::StartConnection] m_hHandler == NULL_SOCKET, call open first!!!");
		}

		auto addr = MakeAddr(serverAddress);

		auto rc = connect(m_hHandle, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr));

#if PLATFORM == PLATFORM_WINDOWS
		if ((rc == SOCKET_ERROR) && (WSAGetLastError() != WSAEWOULDBLOCK))
#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
		if((rc < 0) && (errno != EINPROGRESS))
#endif
		{			
			LogGetDefault()->error("Unknown connect error");			

			return false;
		}		

		//connection in progress, check it
		return true;
	}

	Socket::Status Socket::GetConnectionProgress()
	{		
		fd_set set;

#if PLATFORM == PLATFORM_WINDOWS
		set.fd_array[0] = this->m_hHandle;
		set.fd_count = 1;
#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
		FD_ZERO(&set);
		FD_SET(this->m_hHandle, &set);
#endif

		timeval tval = { 0 };

		int rc = select(FD_SETSIZE, nullptr, &set, nullptr, &tval);
		if (rc < 0)
			throw std::runtime_error("select (write test) failed for GetConnectionProgress");

		if (rc > 0)
			return Status::OK;

		//rc == 0, this means that socket is not ready, but it can have failed, so try again and check error state
#if PLATFORM == PLATFORM_WINDOWS
		set.fd_array[0] = this->m_hHandle;
		set.fd_count = 1;
#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
		FD_ZERO(&set);
		FD_SET(this->m_hHandle, &set);
#endif		

		rc = select(FD_SETSIZE, nullptr, nullptr, &set, &tval);
		if (rc < 0)
			throw std::runtime_error("select (error test) failed for GetConnectionProgress");

		if (rc > 0)
		{
			//sorry dude, failed
			return Status::DISCONNECTED;
		}

		//not ready, try later
		return Status::WOULD_BLOCK;
	}

	std::tuple<Socket::Status, Socket, NetworkAddress> Socket::TryAccept()
	{
		sockaddr_in addr;

		socklen_t addrSize = sizeof(addr);

		auto s = accept(m_hHandle, (sockaddr*)&addr, &addrSize);

		if (s == NULL_SOCKET)
		{
			return std::make_tuple(Status::WOULD_BLOCK, Socket(), NetworkAddress());
		}

		unsigned int from_address = ntohl(addr.sin_addr.s_addr);

		unsigned int from_port = ntohs(addr.sin_port);		

		return std::make_tuple(Status::OK, Socket(s), NetworkAddress(from_address, from_port));
	}

	bool Socket::IsOpen() const
	{
		return m_hHandle != NULL_SOCKET;
	}

	bool Socket::Send(const NetworkAddress &destination, const void *data, size_t size)
	{
		assert(m_hHandle != NULL_SOCKET);

		auto saddr = MakeAddr(destination);

		auto sent_bytes = sendto(
			m_hHandle, 
			(const char*)data, 
			static_cast<int>(size), 
			0, 
			(const sockaddr *)&saddr, 
			sizeof(saddr)
		);

		if (sent_bytes != size)
		{
			LogGetDefault()->error("Failed to send packet.");
			return false;
		}

		return true;
	}

	std::tuple<Socket::Status, size_t> Socket::Send(const void *data, size_t size)
	{
		assert(m_hHandle != NULL_SOCKET);

		auto bytesSent = send(m_hHandle, (const char *)data, size, 0);
		
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
		if (bytesSent < 0)
		{
			switch (errno)
			{
				//case EAGAIN:
				case EWOULDBLOCK:
					return std::make_tuple(Status::WOULD_BLOCK, 0);

				case ECONNRESET:
					return std::make_tuple(Status::DISCONNECTED, 0);

				default:
					throw std::logic_error(fmt::format("[Socket::Send] Failed to send: {}", errno));
			}			
		}
			
#elif PLATFORM == PLATFORM_WINDOWS
		if (bytesSent == SOCKET_ERROR)
		{
			auto error = WSAGetLastError();
			switch (error)
			{
				case WSAEWOULDBLOCK:
					return std::make_tuple(Status::WOULD_BLOCK, 0);

				case WSAECONNRESET:
					return std::make_tuple(Status::DISCONNECTED, 0);

				default:
					throw std::logic_error(fmt::format("[Socket::Send] Failed to send: {}", error));
			}
		}
#endif			
		
		return std::make_tuple(Status::OK, bytesSent);
	}


	std::tuple<Socket::Status, size_t> Socket::Receive(NetworkAddress &sender, void *data, size_t size)
	{	
		assert(m_hHandle != NULL_SOCKET);

		sockaddr_in from;
		socklen_t fromLength = sizeof(from);

		auto result = recvfrom(
			m_hHandle, 
			(char*)data, 
			static_cast<int>(size), 
			0, 
			(sockaddr*)&from, 
			&fromLength
		);

		if (result == 0)
			return std::make_tuple(Status::DISCONNECTED, 0);

#if PLATFORM == PLATFORM_WINDOWS
		if (result == SOCKET_ERROR)
		{
			result = WSAGetLastError();
			switch(result)
			{
				case WSAEWOULDBLOCK:
					return std::make_tuple(Status::WOULD_BLOCK, 0);

				case WSAEMSGSIZE:
				default:
					throw std::runtime_error("receive overflow");					
			}
		}
#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
		if (result < 0)
		{
			switch (errno)
			{
				//case EAGAIN:
				case EWOULDBLOCK:
					return std::make_tuple(Status::WOULD_BLOCK, 0);

				default:
					throw std::runtime_error("receive overflow");
			}
		}
#endif

		unsigned int from_address = ntohl(from.sin_addr.s_addr);

		unsigned int from_port = ntohs(from.sin_port);

		sender = NetworkAddress(from_address, from_port);

		return std::make_tuple(Status::OK, result);
	}

	std::tuple<Socket::Status, size_t> Socket::Receive(void *data, size_t size)
	{
		assert(m_hHandle != NULL_SOCKET);

		auto result = recv(
			m_hHandle, 
			(char*)data, 
			static_cast<int>(size), 
			0
		);

		if (result == 0)
			return std::make_tuple(Status::DISCONNECTED, 0);

#if PLATFORM == PLATFORM_WINDOWS
		if (result == SOCKET_ERROR)
		{
			result = WSAGetLastError();
			switch (result)
			{
				case WSAECONNRESET:
					return std::make_pair(Status::DISCONNECTED, 0);

				case WSAEWOULDBLOCK:
					return std::make_pair(Status::WOULD_BLOCK, 0);

				case WSAEMSGSIZE:
					throw std::runtime_error("receive overflow");

				default:
					throw std::runtime_error(fmt::format("unknown socket error: {}", result));
			}
		}
#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
		if (result < 0)
		{
			switch (errno)
			{
				case ENOTCONN:
					return std::make_pair(Status::DISCONNECTED, 0);

				case EWOULDBLOCK:
					return std::make_pair(Status::WOULD_BLOCK, 0);

				default:
					throw std::runtime_error(fmt::format("unknown socket error: {}", result));
			}
		}
#endif

		return std::make_tuple(Status::OK, result);
	}
}
