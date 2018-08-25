#ifndef __SENSOR_MANAGER_H__
#define __SENSOR_MANAGER_H__

#include <stdint.h>

#define TABLE_END	-1
#define DEFAULT_UPDATE_PERIOD 1000

typedef uint16_t (*SensorUpdateFn)(void);

enum SENSOR_LIST
{
	CURRENT_SENSOR = 0,
	VOLTAGE_SENSOR,
	TEMP_SENSOR,
	RH_SENSOR,
	ALL_SENSORS
};

struct SENSOR_ENTRY
{
	char* name;
	uint16_t lastknownval;
	char scalar;
	SensorUpdateFn fn;
};

SENSOR_ENTRY* SensorManager_GetEntry(SENSOR_LIST index);
void SensorManager_Update();
void SensorManager_Poll();

void SensorManager_SetUpdatePeriod(uint16_t ms);

uint16_t SensorManager_GetUpdatePeriod();

#endif
