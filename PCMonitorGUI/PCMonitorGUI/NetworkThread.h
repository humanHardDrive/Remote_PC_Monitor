#pragma once
#include <winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>

#include <thread>
#include <queue>
#include <mutex>

#include <cstdint>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

class NetworkThread
{
	public:
	NetworkThread();
	~NetworkThread();

	

	void SetIP(CString IP);
	void SetPort(CString port);

	bool Connect();
	void Disconnect();

	void write(uint8_t* buf, uint8_t len);
	uint8_t read(uint8_t* buf, uint8_t len);

	private:
	void TXHandler();
	void RXHandler();
	void UpdateHandler();

	private:
	SOCKET m_NetworkSocket;

	std::queue<uint8_t> m_TXQ;
	std::queue<uint8_t> m_RXQ;

	std::thread m_TXThread;
	std::thread m_RXThread;
	std::thread m_UpdateThread;

	std::mutex m_TXMutex;
	std::mutex m_RXMutex;
	std::mutex m_ConfigMutex;

	bool m_StopConnection, m_Disconnected;

	CString m_IP;
	CString m_Port;
};

