#ifndef __SERVER_H__
#define __SERVER_H__

#define __DEBUG__

#define ETHERNET2

#include <Arduino.h>

#ifdef ETHERNET2
#include <Ethernet2.h>
#else
#include <Ethernet.h>
#endif

void Server_Begin();

bool Server_Running();
bool Server_ClientConnected();
IPAddress Server_GetLocalIP();
bool Server_ConnectionsAllowed();
uint16_t Server_GetClientsConnected();
uint16_t Server_GetBadChecksumCount();
uint16_t Server_GetBadMessageCount();

void Server_ToggleConnectionsAllowed();
void Server_ResetStatistics();

void Server_Update();

#endif
