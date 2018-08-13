#pragma once

#include "NetworkThread.h"
#include "Msgs.h"

#include <vector>
#include <utility>
#include <thread>
#include <mutex>

class Commander
{
	public:
	Commander();
	~Commander();

	void SetNetworkThread(NetworkThread *nt);

	void SendFile(CString path);
	void ValidateFile(CString path);
	void LoadFile();

	void ListSensors(uint8_t index, CString name);
	void UpdateSensor(uint8_t index);

	private:
	void BuildAndSendPacket(COMMAND_TYPE cmd, uint8_t* buf, uint8_t len);
	uint8_t CalculateChecksum(PACKET_WRAPPER* wrapper);

	COMMAND_PAYLOAD* WaitOnReturn(COMMAND_TYPE cmd);

	void PacketParsingThread();

	private:
	NetworkThread* m_NetworkThread;

	uint8_t m_MessageCount;

	std::thread m_PacketParseThread;
	std::mutex m_PayloadMutex;
	PARSING_STATE m_ParsingState;

	std::pair<bool, COMMAND_PAYLOAD> m_CommandWaiting[ALL_COMMANDS];
};

