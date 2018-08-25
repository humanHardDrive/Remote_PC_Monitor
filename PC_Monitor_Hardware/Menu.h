#ifndef __MENU_H__
#define __MENU_H__

#include <Arduino.h>

#define RETURN_KEY    0x0D
#define ESCAPE_KEY    0x1B
#define BACKSPACE_KEY 0x08

#define RETURN_ON_ESCAPE(quit)      \
if (quit)                           \
{                                   \
  Menu_Display(CurrentMenu, true);  \
  return;                           \
}                                   \

#define WAIT_FOR_KEY()      \
while(!Serial.available()); \
Serial.read();              \

typedef struct MENU_OPTION
{
	char key; //The key to execute the option
	const char* DisplayString; //The option name
	void (*function)(void); //The function to call when executing the option
};

enum MENU
{
  MAIN_MENU = 0,
  SERVER_MENU,
  EEPROM_MENU,
  NVRAM_MENU,
  SENSOR_MENU,
  ALL_MENUS
};

void Menu_Update(char c);

void Menu_Clear();
void Menu_Display(byte menu, bool cls);
void ExeMenuOption(byte menu, char option);

unsigned int GetString(char* buf, unsigned int maxlen);
long GetNum(bool* escape);

#endif
