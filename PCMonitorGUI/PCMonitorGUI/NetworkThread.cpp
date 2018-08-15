#include "stdafx.h"
#include "NetworkThread.h"


NetworkThread::NetworkThread()
{
	m_NetworkSocket = INVALID_SOCKET;
	m_IP = _T("127.0.0.1");
	m_Port = _T("4041");
	m_Disconnected = true;
}


NetworkThread::~NetworkThread()
{
	Disconnect();
}

void NetworkThread::SetIP(CString IP)
{
	this->m_ConfigMutex.lock();
	this->m_IP = IP;
	this->m_ConfigMutex.unlock();
}

void NetworkThread::SetPort(CString port)
{
	this->m_ConfigMutex.lock();
	this->m_Port = port;
	this->m_ConfigMutex.unlock();
}

bool NetworkThread::Connect()
{
	if (m_Disconnected)
	{
		// create WSADATA object
		WSADATA wsaData;
		this->m_NetworkSocket = INVALID_SOCKET;
		int iResult;
		u_long iMode = 1;

		// holds address info for socket to connect to
		struct addrinfo *result = NULL,
			*ptr = NULL,
			hints;

		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

		if (iResult != 0)
		{
			printf("WSAStartup failed with error: %d\n", iResult);
			return false;
		}

		// set address info
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;  //TCP connection!!!

		//resolve server address and port
		iResult = getaddrinfo("192.168.1.177", "4040", &hints, &result);

		if (iResult != 0)
		{
			WSACleanup();
			return false;
		}

		// Attempt to connect to an address until one succeeds
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
		{

			// Create a SOCKET for connecting to server
			this->m_NetworkSocket = socket(ptr->ai_family, ptr->ai_socktype,
				ptr->ai_protocol);

			if (this->m_NetworkSocket == INVALID_SOCKET)
			{
				WSACleanup();
				return false;
			}

			// Connect to server.
			iResult = connect(this->m_NetworkSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

			if (iResult == SOCKET_ERROR)
			{
				closesocket(this->m_NetworkSocket);
				this->m_NetworkSocket = INVALID_SOCKET;
				return false;
			}
		}

		// no longer need address info for server
		freeaddrinfo(result);

		// if connection failed
		if (this->m_NetworkSocket == INVALID_SOCKET)
		{
			WSACleanup();
			return false;
		}

		iResult = ioctlsocket(this->m_NetworkSocket, FIONBIO, &iMode);
		if (iResult == SOCKET_ERROR)
		{
			closesocket(this->m_NetworkSocket);
			WSACleanup();
			return false;
		}

		m_StopConnection = false;
		m_Disconnected = false;

		m_RXThread = std::thread(&NetworkThread::RXHandler, this);
		m_TXThread = std::thread(&NetworkThread::TXHandler, this);
		m_UpdateThread = std::thread(&NetworkThread::UpdateHandler, this);

		return true;
	}

	return false;
}

void NetworkThread::Disconnect()
{
	if(m_NetworkSocket != INVALID_SOCKET)
	{
		m_StopConnection = true;
		shutdown(m_NetworkSocket, SD_BOTH);

		if(m_TXThread.joinable())
			m_TXThread.join();

		if(m_RXThread.joinable())
			m_RXThread.join();

		closesocket(m_NetworkSocket);
		m_NetworkSocket = INVALID_SOCKET;

		m_Disconnected = true;
		m_StopConnection = false;

		if (m_UpdateThread.joinable())
			m_UpdateThread.join();
	}

	m_TXQ.empty();
	m_RXQ.empty();
}

bool NetworkThread::Connected()
{
	return !m_Disconnected;
}


void NetworkThread::write(uint8_t* buf, uint8_t len)
{
	m_TXMutex.lock();

	for(uint8_t i = 0; i < len; i++)
		m_TXQ.push(buf[i]);

	m_TXMutex.unlock();
}

uint8_t NetworkThread::read(uint8_t* buf, uint8_t len)
{
	uint8_t i;

	m_RXMutex.lock();

	for(i = 0; i < len && m_RXQ.size() > 0; i++)
	{
		buf[i] = m_RXQ.front();
		m_RXQ.pop();
	}

	m_RXMutex.unlock();

	return i;
}


void NetworkThread::TXHandler()
{
	unsigned char buffer[128];
	unsigned char i;

	while(!this->m_StopConnection)
	{
		this->m_TXMutex.lock();

		memset(buffer, 0, sizeof(buffer));
		for(i = 0; i < 128 && this->m_TXQ.size() > 0; i++)
		{
			buffer[i] = this->m_TXQ.front();
			this->m_TXQ.pop();
		}

		if(i > 0)
			send(this->m_NetworkSocket, (const char*)buffer, i, 0);

		this->m_TXMutex.unlock();

		std::this_thread::yield();
	}
}

void NetworkThread::RXHandler()
{
	unsigned char buffer[128];
	int bytesread = 0;

	while(!this->m_StopConnection)
	{
		bytesread = recv(this->m_NetworkSocket, (char*)buffer, 128, 0);

		if(bytesread > 0)
		{
			this->m_RXMutex.lock();

			for(unsigned char i = 0; i < bytesread; i++)
				this->m_RXQ.push(buffer[i]);

			this->m_RXMutex.unlock();
		}
		else if(WSAGetLastError() != WSAEWOULDBLOCK)
			this->m_Disconnected = true;

		std::this_thread::yield();
	}
}

void NetworkThread::UpdateHandler()
{
	while(!this->m_Disconnected);

	Disconnect();
}
