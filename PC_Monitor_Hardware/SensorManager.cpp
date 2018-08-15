#include "SensorManager.h"

#include <Arduino.h>

//Sensor update routines
uint16_t l_UpdateCurrentSensor();
uint16_t l_UpdateVoltageSensor();
uint16_t l_UpdateTempSensor();
uint16_t l_UpdateRHSensor();

uint32_t l_LastUpdateTime;
uint16_t l_UpdatePeriod;

SENSOR_ENTRY SensorTable[] =
{
  { "Current",
    0,
    0,
    l_UpdateCurrentSensor
  },
  { "Voltage",
    0,
    2,
    l_UpdateVoltageSensor
  },
  { "Temperature",
    0,
    0,
    l_UpdateTempSensor
  },
  { "Relative Humidity",
    0,
    0,
    l_UpdateRHSensor
  }
};


SENSOR_ENTRY* SensorManager_GetEntry(SENSOR_LIST index)
{
  if (index >= ALL_SENSORS)
    return NULL;
  else
    return &SensorTable[index];
}

void SensorManager_Update()
{
  SENSOR_ENTRY* sensor;

  for (uint8_t i = 0; i < ALL_SENSORS; i++)
  {
    sensor = SensorManager_GetEntry(i);

    if (sensor)
      sensor->lastknownval = sensor->fn();
  }
}


uint16_t l_UpdateCurrentSensor()
{
  return 0;
}

uint16_t l_UpdateVoltageSensor()
{
  return (analogRead(A1) * 330) / 4096;
}

uint16_t l_UpdateTempSensor()
{
  return 0;
}

uint16_t l_UpdateRHSensor()
{
  return 0;
}

