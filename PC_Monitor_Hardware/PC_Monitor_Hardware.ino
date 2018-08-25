#include "Server.h"
#include "SensorManager.h"
#include "Menu.h"

void setup()
{
  Serial.begin(115200);

  Server_Begin();
}

void loop()
{
  if(Serial.available())
    Menu_Update((uint8_t)Serial.read());
  
  Server_Update();
  SensorManager_Update();
}
