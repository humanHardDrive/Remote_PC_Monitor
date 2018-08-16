#include "stdafx.h"
#include "Commander.h"

#include <string.h>
#include <cmath>
#include <chrono>

Commander::Commander()
{}

Commander::~Commander()
{}

void Commander::SetNetworkThread(NetworkThread * nt)
{
	this->m_NetworkThread = nt;
}

void Commander::StartListener()
{
	if (m_PacketParseThread.joinable())
		m_PacketParseThread.join();

	m_PacketParseThread = std::thread(&Commander::PacketParsingThread, this);
}

bool Commander::SendFile(uint32_t index, uint8_t * buf, uint8_t len)
{
	SEND_FILE_MSG msg;
	SEND_FILE_RSP rsp;
	COMMAND_PAYLOAD *payload;

	if (len > EEPROM_PAGE_SIZE / 2)
		return false;

	msg.index = index;
	msg.len = len;
	if(len > 0)
		memcpy(msg.data, buf, len);
	
	BuildAndSendPacket(SEND_FILE, (uint8_t*)&msg, sizeof(msg));

	payload = WaitOnReturn(SEND_FILE);
	if (payload)
	{
		memcpy(&rsp, payload->baggage, sizeof(rsp));

		return true;
	}

	return false;
}


bool Commander::ListSensors(uint8_t index, CString& name)
{
	LIST_SENSORS_MSG msg;
	LIST_SENSORS_RSP rsp;
	COMMAND_PAYLOAD* payload;

	msg.index = index;
	BuildAndSendPacket(LIST_SENSORS, (uint8_t*)&msg, sizeof(LIST_SENSORS_MSG));

	payload = WaitOnReturn(LIST_SENSORS);
	if (payload)
	{
		memcpy(&rsp, payload->baggage, sizeof(LIST_SENSORS_RSP));
		name = CString(rsp.name, rsp.len);

		return true;
	}

	return false;
}

bool Commander::UpdateSensor(uint8_t index, float &val)
{
	UPDATE_SENSOR_MSG msg;
	UPDATE_SENSOR_RSP rsp;
	COMMAND_PAYLOAD* payload;

	msg.index = index;
	BuildAndSendPacket(UPDATE_SENSOR, (uint8_t*)&msg, sizeof(msg));

	payload = WaitOnReturn(UPDATE_SENSOR);
	if (payload)
	{
		memcpy(&rsp, payload->baggage, sizeof(UPDATE_SENSOR_RSP));
		val = (float)rsp.val * (float)pow(10, rsp.scalar);

		return true;
	}

	return false;
}

void Commander::BuildAndSendPacket(COMMAND_TYPE cmd, uint8_t* buf, uint8_t len)
{
	PACKET_WRAPPER wrapper;

	m_MessageCount++;

	wrapper.STX = WRAPPER_STX;
	wrapper.len = len;
	wrapper.rng = m_MessageCount;
	*wrapper.payload = cmd;
	memcpy(wrapper.payload + 1, buf, len);
	wrapper.checksum = CalculateChecksum(&wrapper);

	m_NetworkThread->write((uint8_t*)&wrapper, sizeof(PACKET_WRAPPER));
}

uint8_t Commander::CalculateChecksum(PACKET_WRAPPER * wrapper)
{
	uint8_t i;
	uint8_t checksum = 0;

	for(i = 0; i < sizeof(PACKET_WRAPPER) - 1; i++)
		checksum += *(((uint8_t*)wrapper) + i);

	return checksum;
}

COMMAND_PAYLOAD * Commander::WaitOnReturn(COMMAND_TYPE cmd)
{
	std::chrono::steady_clock::time_point end;
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

	do 
	{
		end = std::chrono::steady_clock::now();
	}while (!m_CommandWaiting[cmd].first && std::chrono::duration_cast<std::chrono::seconds>(end - start).count() < 1);

	if (std::chrono::duration_cast<std::chrono::seconds>(end - start).count() >= 1)
		return NULL;

	m_PayloadMutex.lock();
	//Clear the waiting flag
	m_CommandWaiting[cmd].first = false;
	m_PayloadMutex.unlock();

	return &m_CommandWaiting[cmd].second;}

void Commander::PacketParsingThread()
{
	uint8_t bytesread = 0, length, rng, checksum, k;
	uint8_t buffer[MAX_WRAPPER_SIZE];
	uint8_t payload[MAX_PAYLOAD_SiZE];

	while(m_NetworkThread->Connected())
	{
		bytesread = m_NetworkThread->read(buffer, 128);

		for(uint8_t i = 0; i < bytesread; i++)
		{
			switch(m_ParsingState)
			{
				case WAITING_FOR_STX:
				if (buffer[i] == WRAPPER_STX)
				{
					m_ParsingState = WAITING_FOR_RNG;

					k = 0;
					checksum = WRAPPER_STX;
					memset(payload, 0, sizeof(payload));
				}
				break;

				case WAITING_FOR_RNG:
				rng = buffer[i];
				checksum += rng;
				m_ParsingState = WAITING_FOR_LEN;
				break;

				case WAITING_FOR_LEN:
				length = buffer[i];
				checksum += length;
				m_ParsingState = READING_PAYLOAD;
				break;

				case READING_PAYLOAD:
				payload[k] = buffer[i];
				checksum += payload[k];

				k++;

				if (k >= MAX_PAYLOAD_SiZE)
					m_ParsingState = WAITING_FOR_CHECKSUM;
				break;

				case WAITING_FOR_CHECKSUM:
				if (checksum == buffer[i])
				{
					m_PayloadMutex.lock();

					COMMAND_PAYLOAD* command = (COMMAND_PAYLOAD*)payload;
					memcpy(&m_CommandWaiting[command->cmd].second, command, sizeof(COMMAND_PAYLOAD));
					m_CommandWaiting[command->cmd].first = true;

					m_PayloadMutex.unlock();
				}
				m_ParsingState = WAITING_FOR_STX;
				break;
			}
		}
	}
}
