#ifndef __SERVER_H__
#define __SERVER_H__

#define __DEBUG__

#define ETHERNET2

#include <Arduino.h>

void Server_Begin();

bool Server_Running();
bool Server_ClientConnected();
String Server_GetLocalIP();

void Server_Update();

#endif
