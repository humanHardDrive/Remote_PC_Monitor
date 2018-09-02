#include "SensorManager.h"

#include <Arduino.h>

//Sensor update routines
uint16_t l_UpdateCurrentSensor();
uint16_t l_UpdateVoltageSensor();
uint16_t l_UpdateTempSensor();
uint16_t l_UpdateRHSensor();

uint32_t l_LastUpdateTime = 0;
uint16_t l_UpdatePeriod = DEFAULT_UPDATE_PERIOD;

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
  if(millis() - l_LastUpdateTime > l_UpdatePeriod)
  {
    SensorManager_Poll();
    l_LastUpdateTime = millis();
  }
}

void SensorManager_Poll()
{
  SENSOR_ENTRY* sensor;

  for (uint8_t i = 0; i < ALL_SENSORS; i++)
  {
    sensor = SensorManager_GetEntry((SENSOR_LIST)i);

    if (sensor)
      sensor->lastknownval = sensor->fn();
  }
}


void SensorManager_SetUpdatePeriod(uint16_t ms)
{
  l_UpdatePeriod = ms;
}


uint16_t SensorManager_GetUpdatePeriod()
{
  return l_UpdatePeriod;
}


uint16_t l_UpdateCurrentSensor()
{
  return 0;
}

//10mV per degree C
uint16_t l_UpdateVoltageSensor()
{
  return (analogRead(A1) * 330) / 4096;
}

uint16_t l_UpdateTempSensor()
{
  return (analogRead(A0) * 330) / 4096;
}

uint16_t l_UpdateRHSensor()
{
  return 0;
}

