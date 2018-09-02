#include "Server.h"
#include "SensorManager.h"
#include "FileHelper.h"
#include "Menu.h"
#include "Debug.h"

void setup()
{
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  delay(5000);

#ifdef DEBUG_2
  Serial.println("Serial init");
#endif

  File_init();

#ifdef DEBUG_2
  Serial.println(F("File init"));
#endif

  Server_Begin();

#ifdef DEBUG_2
  Serial.println(F("Server init"));
#endif
}

void loop()
{
  if (Serial.available())
    Menu_Update(Serial.read());

  Server_Update();
  SensorManager_Update();
}
