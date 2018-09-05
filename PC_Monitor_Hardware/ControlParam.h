#ifndef __CONTROL_PARAM_H__
#define __CONTROL_PARAM_H__

#include <Arduino.h>
//#include <Adafruit_SleepyDog.h>

#define MAGIC_NUMBER	0x35

enum
{
  SERVER_PORT = 0,
  SENSOR_UPDATE_RATE,
  LOAD_ON_RESET,
  ALL_PARAMETERS
};

struct CTRL_PARAM
{
  char* name;
  uint16_t val;
};

struct SavedControlParameters
{
  CTRL_PARAM* params;
  uint8_t magicnum;
  uint8_t checksum;
};

CTRL_PARAM* ControlParam_GetParam(uint8_t index);

void ControlParam_Load();
void ControlParam_Save();

#endif
