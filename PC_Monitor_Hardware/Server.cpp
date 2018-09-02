#include "Server.h"
#include "Msgs.h"
#include "SensorManager.h"
#include "FileHelper.h"
#include "Debug.h"

#include <SPI.h>

#include <stdint.h>

uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

EthernetServer server(4040);
EthernetClient client;

PARSING_STATE l_CurrentParseState = WAITING_FOR_STX;

uint8_t l_MessageCount = 0;
PACKET_WRAPPER l_SharePacket;

uint16_t l_BadChecksumCount, l_BadCommandCount, l_ClientCount;
bool l_ServerRunning = false, l_ConnectionsAllowed = false;

void l_ParsePacket(uint8_t c);
void l_HandleCommand(COMMAND_PAYLOAD* p);
void l_BuildAndSendPacket(COMMAND_TYPE cmd, uint8_t* buf, uint8_t len);

uint8_t l_CalculateChecksum(PACKET_WRAPPER *p);

void l_ProcessSendFile(COMMAND_PAYLOAD *p);
void l_ProcessLoadFile(COMMAND_PAYLOAD *p);
void l_ProcessValidateFile(COMMAND_PAYLOAD *p);

void l_ProcessSensorList(COMMAND_PAYLOAD* p);
void l_ProcessSensorUpdate(COMMAND_PAYLOAD *p);

void l_ProcessListParameters(COMMAND_PAYLOAD* p);
void l_ProcessReadParameter(COMMAND_PAYLOAD* p);
void l_ProcessModifyParameter(COMMAND_PAYLOAD* p);

void Server_Begin()
{
  Ethernet.begin(mac);
  server.begin();

#ifdef DEBUG_1
  Serial.print("Server is open at ");
  Serial.println(Ethernet.localIP());
#endif

  Server_ResetStatistics();

  l_ServerRunning = true;
  l_ConnectionsAllowed = true;
  client.stop();
}


void Server_Update()
{
  if (!client && l_ConnectionsAllowed)
  {
    client = server.available();

    if (client)
    {
      l_ClientCount++;
#ifdef DEBUG_2
      Serial.println("New client");
#endif
    }
  }
  else if (l_ConnectionsAllowed)
  {
    if (client.connected())
    {
      if (client.available())
        l_ParsePacket(client.read());
    }
    else
    {
#ifdef DEBUG_2
      Serial.println("Client disconnected");
#endif
      client.stop();
    }
  }
}


bool Server_Running()
{
  return l_ServerRunning;
}

bool Server_ClientConnected()
{
  return (client);
}

IPAddress Server_GetLocalIP()
{
  return Ethernet.localIP();
}

bool Server_ConnectionsAllowed()
{
  return l_ConnectionsAllowed;
}

uint16_t Server_GetClientsConnected()
{
  return l_ClientCount;
}

uint16_t Server_GetBadChecksumCount()
{
  return l_BadChecksumCount;
}

uint16_t Server_GetBadMessageCount()
{
  return l_BadCommandCount;
}


void Server_ToggleConnectionsAllowed()
{
  if (!l_ConnectionsAllowed)
    l_ConnectionsAllowed = true;
  else
  {
    l_ConnectionsAllowed = false;
    client.stop();
  }
}

void Server_ResetStatistics()
{
  l_BadChecksumCount = l_BadCommandCount = l_ClientCount = 0;
}



void l_ParsePacket(uint8_t c)
{
  static uint8_t k = 0;

  switch (l_CurrentParseState)
  {
    case WAITING_FOR_STX:
      if (c == WRAPPER_STX)
      {
        l_SharePacket.STX = WRAPPER_STX;
        k = 0;
        l_SharePacket.checksum = WRAPPER_STX;

        l_CurrentParseState = WAITING_FOR_RNG;
      }
      break;

    case WAITING_FOR_RNG:
      l_SharePacket.rng = c;
      l_SharePacket.checksum += l_SharePacket.rng;

      l_CurrentParseState = WAITING_FOR_LEN;
      break;

    case WAITING_FOR_LEN:
      l_SharePacket.len = c;
      l_SharePacket.checksum += l_SharePacket.len;

      if (l_SharePacket.len > 0)
        l_CurrentParseState = READING_PAYLOAD;
      else
        l_CurrentParseState = WAITING_FOR_CHECKSUM;
      break;

    case READING_PAYLOAD:
      l_SharePacket.payload[k] = c;
      l_SharePacket.checksum += l_SharePacket.payload[k];

      k++;
      if (k >= MAX_PAYLOAD_SiZE)
        l_CurrentParseState = WAITING_FOR_CHECKSUM;
      break;

    case WAITING_FOR_CHECKSUM:
      if (c == l_SharePacket.checksum)
        l_HandleCommand((COMMAND_PAYLOAD*)l_SharePacket.payload);
      else
      {
        l_BadChecksumCount++;
#ifdef DEBUG_3
        Serial.print("Invalid checksum ");
        Serial.print(c, HEX);
        Serial.print(" vs. ");
        Serial.println(l_SharePacket.checksum);
#endif
      }
      l_CurrentParseState = WAITING_FOR_STX;
      break;
  }
}

void l_HandleCommand(COMMAND_PAYLOAD* p)
{
  switch (p->cmd)
  {
    case SEND_FILE:
      l_ProcessSendFile(p);
      break;

    case VALIDATE_FILE:
      break;

    case LOAD_FILE:
      break;

    case LIST_SENSORS:
      l_ProcessSensorList(p);
      break;

    case UPDATE_SENSOR:
      l_ProcessSensorUpdate(p);
      break;

    case LIST_PARAMETERS:
      l_ProcessListParameters(p);
      break;

    case READ_PARAMETER:
      l_ProcessReadParameter(p);
      break;

    case MODIFY_PARAMETER:
      l_ProcessModifyParameter(p);
      break;

    default:
      l_BadCommandCount++;
#ifdef DEBUG_3
      Serial.print("UNKNOWN ");
      Serial.println(p->cmd);
#endif
      break;
  }
}

uint8_t l_CalculateChecksum(PACKET_WRAPPER *p)
{
  uint8_t i;
  uint8_t checksum = 0;

  for (i = 0; i < sizeof(PACKET_WRAPPER) - 1; i++)
    checksum += *(((uint8_t*)p) + i);

  return checksum;
}

void l_BuildAndSendPacket(COMMAND_TYPE cmd, uint8_t* buf, uint8_t len)
{
  l_MessageCount++;
  l_SharePacket.STX = WRAPPER_STX;
  l_SharePacket.rng = l_MessageCount;
  l_SharePacket.len = len;
  l_SharePacket.payload[0] = cmd;
  memcpy(l_SharePacket.payload + 1, buf, len);
  l_SharePacket.checksum = l_CalculateChecksum(&l_SharePacket);

  client.write((uint8_t*)&l_SharePacket, MAX_WRAPPER_SIZE);
}

void l_ProcessSensorList(COMMAND_PAYLOAD* p)
{
  LIST_SENSORS_MSG* msg;
  LIST_SENSORS_RSP rsp;
  SENSOR_ENTRY* sensor = NULL;

#ifdef DEBUG_2
  Serial.println(__FUNCTION__);
#endif

  msg = (LIST_SENSORS_MSG*)p->baggage;

  rsp.index = msg->index;
  sensor = SensorManager_GetEntry((SENSOR_LIST)rsp.index);
  if (sensor)
  {
    rsp.len = strlen(sensor->name);
    strcpy(rsp.name, sensor->name);
  }
  else
  {
    rsp.len = 0;
  }

  l_BuildAndSendPacket(LIST_SENSORS, (uint8_t*)&rsp, sizeof(rsp));
}

void l_ProcessSensorUpdate(COMMAND_PAYLOAD* p)
{
  UPDATE_SENSOR_MSG* msg;
  UPDATE_SENSOR_RSP rsp;
  SENSOR_ENTRY *sensor = NULL;

#ifdef DEBUG_2
  Serial.println(__FUNCTION__);
#endif

  msg = (UPDATE_SENSOR_MSG*)p->baggage;

  rsp.index = msg->index;
  sensor = SensorManager_GetEntry((SENSOR_LIST)rsp.index);

  if (sensor)
  {
    rsp.val = sensor->lastknownval;
    rsp.scalar = sensor->scalar;
  }
  else
  {
    rsp.val = 0xFFFF;
    rsp.scalar = 0;
  }

  l_BuildAndSendPacket(UPDATE_SENSOR, (uint8_t*)&rsp, sizeof(rsp));
}

void l_ProcessSendFile(COMMAND_PAYLOAD *p)
{
  SEND_FILE_MSG* msg;
  SEND_FILE_RSP rsp;

#ifdef DEBUG_2
  Serial.println(__FUNCTION__);
#endif

  msg = (SEND_FILE_MSG*)p->baggage;

#ifdef DEBUG_3
  Serial.print(F("Index: "));
  Serial.print(msg->index, HEX);
  Serial.print(F("\tLen: "));
  Serial.println(msg->len, HEX);
#endif

  if (msg->len > 0)
  {
    rsp.ack = File_write(msg->index, msg->data, msg->len);
    rsp.index = msg->index;
  }
  else //A 0 length message forces a flush of whatever is in the page buffer
  {
    rsp.ack = 0;
    if (msg->index == SEND_FILE_RESET_INDEX)
      File_reset();
    else if (msg->index == SEND_FILE_FINALIZE_INDEX)
      File_finalize();
    else
      rsp.ack = File_flush();

    rsp.index = msg->index;
  }

#ifdef DEBUG_3
  Serial.print(F("ACK: "));
  Serial.println(rsp.ack, HEX);
#endif

  l_BuildAndSendPacket(SEND_FILE, (uint8_t*)&rsp, sizeof(rsp));
}

void l_ProcessLoadFile(COMMAND_PAYLOAD *p)
{
  LOAD_FILE_MSG* msg;
  LOAD_FILE_RSP rsp;

#ifdef DEBUG_2
  Serial.println(__FUNCTION__);
#endif

  msg = (LOAD_FILE_MSG*)p->baggage;

#ifdef DEBUG_3
  Serial.print(F("Code: "));
  Serial.println(msg->code, HEX);
#endif

  rsp.ack = NACK;

#ifdef DEBUG_3
  Serial.print(F("ACK: "));
  Serial.println(rsp.ack, HEX);
#endif

  l_BuildAndSendPacket(LOAD_FILE, (uint8_t*)&rsp, sizeof(rsp));
}

void l_ProcessValidateFile(COMMAND_PAYLOAD *p)
{
  VALIDATE_FILE_MSG* msg;
  VALIDATE_FILE_RSP rsp;
  
#ifdef DEBUG_2
  Serial.println(__FUNCTION__);
#endif

  msg = (VALIDATE_FILE_MSG*)p->baggage;

#ifdef DEBUG_3
  Serial.print(F("Len: "));
  Serial.print(msg->len, HEX);
  Serial.print(F("\tChecksum: "));
  Serial.println(msg->checksum, HEX);
#endif

  rsp.valid = FILE_VALID;
  if(msg->len != File_GetFileLength())
    rsp.valid |= FILE_LEN_INVALID;

  if(msg->checksum != File_GetFileChecksum())
    rsp.valid |= FILE_CHKSUM_INVALID;

#ifdef DEBUG_3
  Serial.print(F("Valid: "));
  Serial.println(rsp.valid, HEX);
#endif

  l_BuildAndSendPacket(VALIDATE_FILE, (uint8_t*)&rsp, sizeof(rsp));
}

void l_ProcessListParameters(COMMAND_PAYLOAD* p)
{
#ifdef DEBUG_2
  Serial.println(__FUNCTION__);
#endif
}

void l_ProcessReadParameter(COMMAND_PAYLOAD* p)
{
#ifdef DEBUG_2
  Serial.println(__FUNCTION__);
#endif
}

void l_ProcessModifyParameter(COMMAND_PAYLOAD* p)
{
#ifdef DEBUG_2
  Serial.println(__FUNCTION__);
#endif
}

