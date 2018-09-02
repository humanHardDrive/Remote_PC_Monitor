#ifndef __CONTROL_PARAM_H__
#define __CONTROL_PARAM_H__

#include <Arduino.h>

#define MAGIC_NUMBER	0x35  

enum
{
	SERVER_PORT = 0,
	SENSOR_UPDATE_RATE,
	ALL_PARAMETERS
};

struct CTRL_PARAM
{
	char* name;
	uint16_t val;
};

struct SavedControlParameters
{
	CTRL_PARAM params[ALL_PARAMETERS];
	uint8_t magicnum;
	uint8_t checksum;
};

#endif