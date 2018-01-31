#include "Socket.h"

#include <cassert>
#include <stdexcept>

#include <plog/Log.h>

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

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#endif

#if PLATFORM == PLATFORM_WINDOWS
#pragma comment( lib, "wsock32.lib" )
#pragma comment( lib, "Ws2_32.lib" )
#endif

namespace dcclite
{
	static sockaddr_in MakeAddr(const Address &address)
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

	Socket::Socket(Socket &&other):
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

	Socket &Socket::operator=(Socket &&other)
	{
		if (this != &other)
		{
			m_hHandle = other.m_hHandle;

			other.m_hHandle = NULL_SOCKET;
		}

		return *this;
	}

	bool Socket::Open(Port_t port, Type type)
	{
		assert(g_iCount > 0);

		if (m_hHandle != NULL_SOCKET)
			this->Close();

		auto intType = type == Type::DATAGRAM ? SOCK_DGRAM : SOCK_STREAM;
		auto intProto = type == Type::DATAGRAM ? IPPROTO_UDP : IPPROTO_TCP;

		m_hHandle = socket(AF_INET, intType, intProto);

		if (m_hHandle == INVALID_SOCKET)
		{
			LOG_ERROR << "Failed to create socket.";
			return false;
		}

		#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

		int nonBlocking = 1;
		if (fcntl(m_hHandle, F_SETFL, O_NONBLOCK, nonBlocking) == -1)
		{
			BOOST_LOG_TRIVIAL(error) << "Failed to set socket to non-blocking mode.";
			return false;
		}

		#elif PLATFORM == PLATFORM_WINDOWS

		DWORD nonBlocking = 1;
		if (ioctlsocket(m_hHandle, FIONBIO, &nonBlocking) != 0)
		{
			this->Close();

			LOG_ERROR << "Failed to set socket to non-blocking mode.";
			return false;
		}

		int noDelay = 1;
		if ((type == Type::STREAM) && (setsockopt(m_hHandle, IPPROTO_TCP, TCP_NODELAY, (const char *)&noDelay, sizeof(int)) != 0))
		{
			this->Close();

			LOG_ERROR << "Failed enable NO_DELAY.";
			return false;
		}

		#endif

		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons((unsigned short)port);

		if (bind(m_hHandle, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0)
		{
			this->Close();

			LOG_ERROR << "Failed to bind socket.";
			return false;
		}

		return true;
	}

	void Socket::Close()
	{		
		if (m_hHandle == NULL_SOCKET)
			return;

#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
		close(m_iHandle);
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

	bool Socket::StartConnection(const Address &serverAddress)
	{
		auto addr = MakeAddr(serverAddress);

		auto rc = connect(m_hHandle, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr));

		if ((rc == SOCKET_ERROR) && (WSAGetLastError() != WSAEWOULDBLOCK))
		{			
			LOG_ERROR << "Unknown connect error";
		}		

		//connection in progress, check it
		return true;
	}

	Socket::Status Socket::GetConnectionProgress()
	{		
		fd_set set;

		set.fd_array[0] = this->m_hHandle;
		set.fd_count = 1;

		timeval tval = { 0 };

		int rc = select(0, nullptr, &set, nullptr, &tval);
		if (rc < 0)
			throw std::runtime_error("select (write test) failed for GetConnectionProgress");

		if (rc > 0)
			return Status::OK;

		//rc == 0, this means that socket is not ready, but it can have failed, so try again and check error state
		set.fd_array[0] = this->m_hHandle;
		set.fd_count = 1;

		rc = select(0, nullptr, nullptr, &set, &tval);
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

	std::tuple<Socket::Status, Socket, Address> Socket::TryAccept()
	{
		sockaddr_in addr;
		int addrSize = sizeof(addr);

		auto s = accept(m_hHandle, (sockaddr*)&addr, &addrSize);

		if (s == NULL_SOCKET)
		{
			return std::make_tuple(Status::WOULD_BLOCK, Socket(), Address());
		}

		unsigned int from_address = ntohl(addr.sin_addr.s_addr);

		unsigned int from_port = ntohs(addr.sin_port);		

		return std::make_tuple(Status::OK, Socket(s), Address(from_address, from_port));
	}

	bool Socket::IsOpen() const
	{
		return m_hHandle != NULL_SOCKET;
	}

	bool Socket::Send(const Address &destination, const void *data, size_t size)
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
			LOG_ERROR << "Failed to send packet.";
			return false;
		}

		return true;
	}

	std::tuple<Socket::Status, size_t> Socket::Receive(Address &sender, void *data, size_t size)
	{	
#if PLATFORM == PLATFORM_WINDOWS
		typedef int socklen_t;
#endif
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

		if (result < SOCKET_ERROR)
		{
			result = WSAGetLastError();
			switch(result)
			{
				case WSAEWOULDBLOCK:
					return std::make_tuple(Status::WOULD_BLOCK, 0);

				case WSAEMSGSIZE:
					throw std::runtime_error("receive overflow");					
			}
		}

		unsigned int from_address = ntohl(from.sin_addr.s_addr);

		unsigned int from_port = ntohs(from.sin_port);

		sender = Address(from_address, from_port);

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

		if (result == SOCKET_ERROR)
		{
			result = WSAGetLastError();
			switch (result)
			{
				case WSAEWOULDBLOCK:
					return std::make_pair(Status::WOULD_BLOCK, 0);

				case WSAEMSGSIZE:
					throw std::runtime_error("receive overflow");
			}
		}

		return std::make_tuple(Status::OK, result);
	}
}
