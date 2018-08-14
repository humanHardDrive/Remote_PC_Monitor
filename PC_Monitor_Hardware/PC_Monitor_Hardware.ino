#include <SPI.h>
#include <Ethernet.h>

#include "Server.h"

void setup()
{
  Serial.begin(115200);

  Server_Begin();
}

void loop()
{
  // put your main code here, to run repeatedly:
  Server_Update();
}
