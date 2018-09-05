#include "ControlParam.h"

CTRL_PARAM parameters[] =
{
  {"Server Port",	4040},
  {"Sensor Update Rate", 1000},
  {"Load on Reset", 0}
};

SavedControlParameters savedparameters =
{
  parameters,
  MAGIC_NUMBER,
  0
};

CTRL_PARAM* ControlParam_GetParam(uint8_t index)
{
  if (index < ALL_PARAMETERS)
    return &parameters[index];

  return NULL;
}

void ControlParam_Load()
{
#ifdef USE_HARDWARE
#endif
}

void ControlParam_Save()
{
#ifdef USE_HARDWARE
#endif
}

