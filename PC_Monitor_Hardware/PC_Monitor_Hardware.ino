#include "Server.h"
#include "SensorManager.h"
#include "FileHelper.h"
#include "Menu.h"

void setup()
{
  Serial.begin(115200);

  File_init();
  Server_Begin();
}

void loop()
{
  if(Serial.available())
    Menu_Update((uint8_t)Serial.read());
  
  Server_Update();
  SensorManager_Update();
}
