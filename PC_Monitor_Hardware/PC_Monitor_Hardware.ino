#include "Server.h"
#include "SensorManager.h"

void setup()
{
  Serial.begin(115200);

  Server_Begin();
}

void loop()
{
  Server_Update();
  SensorManager_Update();
}
