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

	void StartListener();

	bool SendFile(uint32_t index, uint8_t* buf, uint8_t len, uint8_t* ack);
	bool ValidateFile(uint32_t len, uint16_t checksum, uint8_t& valid);
	bool RebootSystem(bool loadfile, uint8_t& ack);

	bool ListSensors(uint8_t index, CString& name);
	bool UpdateSensor(uint8_t index, float &val);

	bool ListControlParameters(uint8_t index, CString& name);
	bool ReadControlParameter(uint8_t index, uint16_t& val);
	bool ModifyControlParameter(uint8_t index, uint16_t val);

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

