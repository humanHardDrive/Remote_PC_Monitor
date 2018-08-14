#include "Server.h"
#include "Msgs.h"

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>

#include <stdint.h>

uint8_t mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

EthernetServer server(4040);
EthernetClient client;

PARSING_STATE l_CurrentParseState = WAITING_FOR_STX;

uint8_t l_MessageCount = 0;
PACKET_WRAPPER l_SharePacket;

void l_ParsePacket(uint8_t c);
void l_HandleCommand(COMMAND_PAYLOAD* p);
void l_BuildAndSendPacket(COMMAND_TYPE cmd, uint8_t* buf, uint8_t len);

uint8_t l_CalculateChecksum(PACKET_WRAPPER *p);

void l_ProcessSensorList(COMMAND_PAYLOAD* p);

void Server_Begin()
{
  Ethernet.begin(mac);
  server.begin();

  client = NULL;

  Serial.print("Server is open at ");
  Serial.println(Ethernet.localIP());
}


void Server_Update()
{
  if (!client)
    client = server.available();
  else
  {
    if (client.connected())
    {
      if (client.available())
        l_ParsePacket(client.read());
    }
    else
    {
      //client.stop();
      client = NULL;
    }
  }
}


void l_ParsePacket(uint8_t c)
{
  static uint8_t k = 0;

  Serial.print("0x");
  if ((uint8_t)c < 0x10)
    Serial.print("0");
  Serial.print((uint8_t)c, HEX);
  Serial.print(" ");

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
      Serial.println();
      
      Serial.print(l_SharePacket.checksum, HEX);
      Serial.print(" vs. ");
      Serial.println(c, HEX);
      
      if (c == l_SharePacket.checksum)
        l_HandleCommand((COMMAND_PAYLOAD*)l_SharePacket.payload);
      l_CurrentParseState = WAITING_FOR_STX;
      break;
  }
}

void l_HandleCommand(COMMAND_PAYLOAD* p)
{
  switch (p->cmd)
  {
    case SEND_FILE:
      Serial.println("SEND_FILE");
      break;

    case VALIDATE_FILE:
      Serial.println("VALIDATE_FILE");
      break;

    case LOAD_FILE:
      Serial.println("LOAD_FILE");
      break;

    case LIST_SENSORS:
      Serial.println("LIST_SENSORS");
      l_ProcessSensorList(p);
      break;

    case UPDATE_SENSOR:
      Serial.println("UPDATE_SENSOR");
      break;

    default:
      Serial.print("UNKNOWN ");
      Serial.println(p->cmd);
      break;
  }
}

uint8_t l_CalculateChecksum(PACKET_WRAPPER *p)
{
  uint8_t i;
  uint8_t checksum = 0;

  for(i = 0; i < sizeof(PACKET_WRAPPER) - 1; i++)
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
  LIST_SENSORS_MSG msg;
  LIST_SENSORS_RSP rsp;
  
  memcpy(&msg, p->baggage, 1);

  rsp.index = msg.index;
  rsp.len = 10;
  strcpy(rsp.name, "ABCDEFGHIJ");

  l_BuildAndSendPacket(LIST_SENSORS, (uint8_t*)&rsp, 116);
}

