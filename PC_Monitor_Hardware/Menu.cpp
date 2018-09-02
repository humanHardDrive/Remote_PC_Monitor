#include "Menu.h"
#include "Msgs.h"
#include "Server.h"
#include "SensorManager.h"
#include "FileHelper.h"

MENU CurrentMenu = MAIN_MENU;

//Menu switch functions
void l_GoToServerMenu();
void l_GoToEEPROMMenu();
void l_GoToNVRAMMenu();
void l_GoToSensorMenu();

//Server menu functions
void l_DisplayServerInfo();
void l_ResetServerStatistics();
void l_ToggleAllowConnections();

//EEPROM menu functions
void l_ReadEEPROMPage();
void l_WriteEEPROMPage();
void l_ViewFileInfo();

//Sensor menu functions
void l_ListSensors();
void l_ForceSensorUpdate();
void l_ChangeUpdateTime();

const char* MenuName[] = 
{
  "MAIN MENU",
  "SERVER MENU",
  "EEPROM MENU",
  "NVRAM MENU",
  "SENSOR MENU"
};

MENU_OPTION MainMenu[] = 
{
  {'1', "Server", l_GoToServerMenu},
  {'2', "EEPROM", l_GoToEEPROMMenu},
  {'3', "NVRAM", l_GoToNVRAMMenu},
  {'4', "Sensor", l_GoToSensorMenu},
  {'\0', "", NULL}
};

MENU_OPTION ServerMenu[] =
{
  {'1', "Info", l_DisplayServerInfo},
  {'2', "Toggle Allow Connections", l_ToggleAllowConnections},
  {'3', "Reset Statistics", l_ResetServerStatistics},
  {'\0', "", NULL}
};

MENU_OPTION EEPROMMenu[] =
{
  {'1', "Read Page", l_ReadEEPROMPage},
  {'2', "Write Page", l_WriteEEPROMPage},
  {'3', "File Info", l_ViewFileInfo},
  {'\0', "", NULL}
};

MENU_OPTION NVRAMMenu[] =
{
  {'\0', "", NULL}
};

MENU_OPTION SensorMenu[] =
{
  {'1', "List Sensors", l_ListSensors},
  {'2', "Force Sensor Update", l_ForceSensorUpdate},
  {'3', "Change Update Time", l_ChangeUpdateTime},
  {'\0', "", NULL}
};

MENU_OPTION* MenuTable[] =
{
  MainMenu,
  EEPROMMenu,
  NVRAMMenu,
  SensorMenu
};

void Menu_Update(char c)
{
  byte i = 0;

  if (c == RETURN_KEY) //Hitting the return key in TeraTerm
    Menu_Display(CurrentMenu, true);
  else if (c == ESCAPE_KEY) //Hitting the escape key in TeraTerm
  { //Go back to the main menu
    CurrentMenu = MAIN_MENU;
    Menu_Display(CurrentMenu, true);
  }
  else
    ExeMenuOption(CurrentMenu, c);
}

void Menu_Clear()
{
  //Magic sequence
  Serial.write(ESCAPE_KEY);
  Serial.write(0x5B);
  Serial.write('2');
  Serial.write('J');

  //Moves the cursor back to home
  //Sometimes when clearing the screen things would not be aligned
  Serial.write('\r');
}

void Menu_Display(byte menu, bool cls)
{
  byte i = 0;
  char LineBuffer[32];

  if (cls)
    Menu_Clear();

  Serial.println(MenuName[menu]);

  while (MenuTable[menu][i].key)
  {
    Serial.print(MenuTable[menu][i].key);
    Serial.print(F(". "));
    Serial.println(MenuTable[menu][i].DisplayString);
    i++;
  }
}

void ExeMenuOption(byte menu, char option)
{
  byte i = 0;
  char LineBuffer[32];

  //Search through all the menu options
  while (MenuTable[menu][i].key)
  {
    if (option == MenuTable[menu][i].key)
    {
      Menu_Clear();
      Serial.println(MenuTable[menu][i].DisplayString);
      Serial.println();
      MenuTable[menu][i].function();
      return;
    }

    i++;
  }
}


unsigned int GetString(char* buf, unsigned int maxlen)
{
  byte i = 0;

  while (1)
  {
    if (Serial.available())
    {
      char c = Serial.read();

      if (c == RETURN_KEY)
      {
        buf[i] = '\0'; //Null terminator
        return i;
      }
      else if (c == ESCAPE_KEY)
        return 0;
      else if (c == BACKSPACE_KEY) //At least one implementation that works
      {
        if (i > 0)
        {
          Serial.write(c);
          Serial.write((uint8_t)'\0');

          i--;
          buf[i] = '\0';
        }
      }
      else
      {
        Serial.write(c);

        buf[i] = c;
        i++;
      }
    }

    if (i >= maxlen)
    {
      return maxlen;
    }
  }
}

long GetNum(bool* escape)
{
  char buf[16];
  long retval;

  if (escape)
    *escape = true;

  if (GetString(buf, 15))
  {
    retval = atol(buf);

    if (escape)
      *escape = false;
  }

  return retval;
}

/*--------------Menu Function Definitions----------------*/
void l_GoToServerMenu()
{
  CurrentMenu = SERVER_MENU;
  Menu_Display(CurrentMenu, true);
}

void l_GoToEEPROMMenu()
{
  CurrentMenu = EEPROM_MENU;
  Menu_Display(CurrentMenu, true);
}

void l_GoToNVRAMMenu()
{
  CurrentMenu = NVRAM_MENU;
  Menu_Display(CurrentMenu, true);
}

void l_GoToSensorMenu()
{
  CurrentMenu = SENSOR_MENU;
  Menu_Display(CurrentMenu, true);
}


/*---------------Server Function Definitions--------------------*/
void l_DisplayServerInfo()
{
  Serial.print(F("Running: "));
  Serial.println(Server_Running() ? F("Y") : F("N"));
  Serial.print(F("Local IP: "));
  Serial.println(Server_GetLocalIP());
  Serial.print(F("Active Client: "));
  Serial.println(Server_ClientConnected() ? F("Y") : F("N"));
  Serial.print(F("Connections Allowed: "));
  Serial.println(Server_ConnectionsAllowed() ? F("Y") : F("N"));

  Serial.print(F("Clients Connected: "));
  Serial.println(Server_GetClientsConnected());
  Serial.print(F("Bad Checksums: "));
  Serial.println(Server_GetBadChecksumCount());
  Serial.print(F("Invalid Commands: "));
  Serial.println(Server_GetBadMessageCount());
}

void l_ResetServerStatistics()
{
  Server_ResetStatistics();
  RETURN_ON_ESCAPE(true);
}

void l_ToggleAllowConnections()
{
  Server_ToggleConnectionsAllowed();
  Serial.print(F("Connections Allowed: "));
  Serial.println(Server_ConnectionsAllowed() ? F("Y") : F("N"));
  
  WAIT_FOR_KEY();
}


/*---------------EEPROM Function Definitions----------------*/
void l_ReadEEPROMPage()
{
  bool quit = false;
  uint8_t buffer[EEPROM_PAGE_SIZE];
  uint16_t page;
  uint8_t retval = 0;

  page = GetNum(&quit);
  RETURN_ON_ESCAPE(quit);

  retval = File_read(page*EEPROM_PAGE_SIZE, buffer, EEPROM_PAGE_SIZE);

  for(uint8_t i = 0; i < EEPROM_PAGE_SIZE; i++)
  {
    if(!(i & 0x0F))
      Serial.println();

    if(buffer[i] < 0x10)
      Serial.print('0');
    Serial.print(buffer[i], HEX);
    Serial.print(' ');
  }
  
  Serial.println();
  Serial.print(F("Returns "));
  Serial.println(retval, HEX);
}

uint8_t HexToDec(char h)
{
  switch(h)
  {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    return (uint8_t)(h - '0');
    break;

    case 'a':
    case 'A':
    return 10;
    break;

    case 'b':
    case 'B':
    return 11;
    break;

    case 'c':
    case 'C':
    return 12;
    break;

    case 'd':
    case 'D':
    return 13;
    break;

    case 'e':
    case 'E':
    return 14;
    break;

    case 'f':
    case 'F':
    return 15;
    break;

    default:
    return 0;
    break;
  }
}

void l_WriteEEPROMPage()
{
  bool quit = false;
  uint8_t buffer[EEPROM_PAGE_SIZE];
  char strbuffer[2*EEPROM_PAGE_SIZE];
  uint8_t strsize, retval;
  uint16_t page;

  Serial.print(F("NOTE: Excessive writting to EEPROM is discouraged. The number of write"));
  Serial.println(F(" cycles is extrememly litmited"));
  Serial.print(F("Page: "));

  page = GetNum(&quit);
  RETURN_ON_ESCAPE(quit);
  
  Serial.println();
  Serial.print(F("Data: "));

  strsize = GetString(strbuffer, 2*EEPROM_PAGE_SIZE);
  RETURN_ON_ESCAPE(!strsize);
  
  for(uint8_t i = 0; i < strsize; i++)
  {
    buffer[i/2] *= 16;
    buffer[i/2] += HexToDec(strbuffer[i]);
  }

  retval = File_write(page*EEPROM_PAGE_SIZE, buffer, strsize/2);

  Serial.println();
  Serial.print(F("Returns "));
  Serial.println(retval, HEX);
}

void l_ViewFileInfo()
{
  Serial.print(F("File Length: "));
  Serial.println(File_GetFileLength());
  Serial.print(F("Checksum: "));
  Serial.println(File_GetFileChecksum(), HEX);
}

/*-----------------------Sensor menu functions------------------------------*/
void l_ListSensors()
{
  SENSOR_ENTRY* sen;
  
  for(uint8_t i = 0; i < ALL_SENSORS; i++)
  {
    sen = SensorManager_GetEntry((SENSOR_LIST)i);

    Serial.print(F("Name: "));
    Serial.println(sen->name);

    Serial.print(F("Last Value: "));
    Serial.println(sen->lastknownval);

    Serial.print(F("Scalar: 10^"));
    Serial.println((int)sen->scalar);

    Serial.println();
  }
}

void l_ForceSensorUpdate()
{
  SensorManager_Poll();
  RETURN_ON_ESCAPE(true);
}

void l_ChangeUpdateTime()
{
  uint16_t time;
  bool quit = false;

  Serial.print(F("Update Time: "));
  Serial.print(SensorManager_GetUpdatePeriod());
  Serial.println(F("ms"));

  Serial.println();
  Serial.print(F("New time: "));
  
  time = GetNum(&quit);
  RETURN_ON_ESCAPE(quit);

  SensorManager_SetUpdatePeriod(time);

  l_ChangeUpdateTime();
}

