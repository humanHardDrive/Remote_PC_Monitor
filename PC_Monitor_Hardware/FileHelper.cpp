#include "FileHelper.h"
#include "Debug.h"
#include "Msgs.h"

#include <Wire.h>
#include <string.h>
#include <extEEPROM.h>

uint8_t FileBuffer[EEPROM_PAGE_SIZE];
uint16_t CurrentFileBufferPage = FILE_DATA_PAGE;
bool FileBufferDirty = false;

uint32_t FileLength;
uint16_t FileChecksum;

FILE_INFO fInfo;

#ifdef USE_HARDWARE
extEEPROM exteeprom(kbits_1024, 1, 128);
#endif

void File_init()
{
  byte i2cstat;

  CurrentFileBufferPage = FILE_DATA_PAGE;
#ifdef USE_HARDWARE
  i2cstat = exteeprom.begin(exteeprom.twiClock100kHz);

#ifdef DEBUG_2
  Serial.println(__FUNCTION__);
  Serial.print(F("I2C Status: "));
  Serial.print(i2cstat, HEX);
  Serial.println();
#endif
  exteeprom.read(CurrentFileBufferPage * EEPROM_PAGE_SIZE, FileBuffer, EEPROM_PAGE_SIZE);
  Serial.print(F("Length: "));
  Serial.println(exteeprom.length());
  exteeprom.read(FILE_INFO_PAGE * EEPROM_PAGE_SIZE, (uint8_t*)&fInfo, sizeof(FILE_INFO));
  Serial.print(F("Length: "));
  Serial.println(exteeprom.length());
#else
  memset(FileBuffer, 0, EEPROM_PAGE_SIZE);
#endif
}


uint8_t File_write(uint32_t index, uint8_t* buf, uint8_t len)
{
  uint16_t desiredpage, pageoffset;
  uint8_t retval = 0;

  desiredpage = (index / EEPROM_PAGE_SIZE);
  pageoffset = index % EEPROM_PAGE_SIZE;

  for (uint8_t i = 0; i < len; i++)
  {
    if (pageoffset >= EEPROM_PAGE_SIZE)
    {
      pageoffset -= EEPROM_PAGE_SIZE;
      desiredpage++;
    }

    if (desiredpage != CurrentFileBufferPage)
    {
#ifdef DEBUG_3
      Serial.print(F("Page mismatch: "));
      Serial.print(desiredpage, HEX);
      Serial.print(F(" vs "));
      Serial.println(CurrentFileBufferPage, HEX);
#endif

      retval = File_flush();
      if (retval)
        return retval;

#ifdef USE_HARDWARE
      retval = exteeprom.read(desiredpage * EEPROM_PAGE_SIZE, FileBuffer, EEPROM_PAGE_SIZE);
      if (retval)
        return retval;
#else
      memset(FileBuffer, 0, EEPROM_PAGE_SIZE);
#endif

      CurrentFileBufferPage = desiredpage;
      FileBufferDirty = false;
    }

    if (!FileBufferDirty && FileBuffer[pageoffset] != buf[i])
    {
      FileBufferDirty = true;

#ifdef DEBUG_3
      Serial.print(F("Page: "));
      Serial.print(desiredpage, HEX);
      Serial.println(F(" dirty"));
#endif
    }

    FileBuffer[pageoffset] = buf[i];
    pageoffset++;
  }

  if (desiredpage * EEPROM_PAGE_SIZE + pageoffset > FileLength)
  {
    FileLength = (uint32_t)((desiredpage - FILE_DATA_PAGE) * EEPROM_PAGE_SIZE + pageoffset);

#ifdef DEBUG_3
    Serial.print(F("File length updated: "));
    Serial.println(FileLength, HEX);
#endif
  }

  return retval;
}

uint8_t File_read(uint32_t index, uint8_t* buf, uint8_t len)
{
  uint16_t desiredpage, pageoffset;
  uint8_t retval = 0;

  desiredpage = (index / EEPROM_PAGE_SIZE);
  pageoffset = index % EEPROM_PAGE_SIZE;

  for (uint8_t i = 0; i < len; i++)
  {
    if (pageoffset >= EEPROM_PAGE_SIZE)
    {
      pageoffset = 0;
      desiredpage++;
    }

    if (desiredpage != CurrentFileBufferPage)
    {
#ifdef DEBUG_3
      Serial.print(F("Page mismatch: "));
      Serial.print(desiredpage, HEX);
      Serial.print(F(" vs "));
      Serial.println(CurrentFileBufferPage, HEX);
#endif

      retval = File_flush();
      if (retval)
        return retval;

#ifdef USE_HARDWARE
      retval = exteeprom.read(desiredpage * EEPROM_PAGE_SIZE, FileBuffer, EEPROM_PAGE_SIZE);
      if (retval)
        return retval;
#else
      memset(FileBuffer, 0, EEPROM_PAGE_SIZE);
#endif

      CurrentFileBufferPage = desiredpage;
      FileBufferDirty = false;
    }

    buf[i] = FileBuffer[pageoffset];
    pageoffset++;
  }

  return 0;
}

void File_reset()
{
  FileChecksum = FileLength = 0;
  File_init();
}

void File_finalize()
{
  uint32_t i;
  uint16_t page = 0, pageoffset = EEPROM_PAGE_SIZE;
  uint8_t buffer[EEPROM_PAGE_SIZE];

  FileChecksum = 0;
  for (i = 0; i < FileLength; i++)
  {
    if (pageoffset >= EEPROM_PAGE_SIZE)
    {
      pageoffset -= EEPROM_PAGE_SIZE;
      page++;

#ifdef USE_HARDWARE
      exteeprom.read(page * EEPROM_PAGE_SIZE, buffer, EEPROM_PAGE_SIZE);
#else
      memset(buffer, 0, EEPROM_PAGE_SIZE);
#endif
    }

    FileChecksum += buffer[pageoffset];
    pageoffset++;
  }

#ifdef DEBUG_3
  Serial.println(__FUNCTION__);
  Serial.print(F("Len: "));
  Serial.print(FileLength, HEX);
  Serial.print(F("\tChecksum: "));
  Serial.println(FileChecksum, HEX);
#endif

#ifdef USE_HARDWARE
  exteeprom.read(FILE_INFO_PAGE * EEPROM_PAGE_SIZE, (uint8_t*)&fInfo, sizeof(FILE_INFO));

  if (fInfo.length != FileLength || fInfo.checksum != FileChecksum)
  {
    fInfo.length = FileLength;
    fInfo.checksum = FileChecksum;

    exteeprom.write(FILE_INFO_PAGE * EEPROM_PAGE_SIZE, (uint8_t*)&fInfo, sizeof(FILE_INFO));
  }
#endif
}

uint8_t File_flush()
{
  uint8_t retval = 0;

  if (FileBufferDirty)
  {
#ifdef USE_HARDWARE
    retval = exteeprom.write(CurrentFileBufferPage * EEPROM_PAGE_SIZE, FileBuffer, EEPROM_PAGE_SIZE);
#endif

#ifdef DEBUG_3
    Serial.println(__FUNCTION__);
    Serial.print("Page ");
    Serial.print(CurrentFileBufferPage);
    for (uint8_t i = 0; i < EEPROM_PAGE_SIZE; i++)
    {
      if (!(i & 0x0F))
        Serial.println();

      if (FileBuffer[i] < 0x10)
        Serial.print('0');
      Serial.print(FileBuffer[i], HEX);
      Serial.print(' ');
    }
    Serial.println();
#endif
    FileBufferDirty = false;
  }

  return retval;
}


uint32_t File_GetFileLength()
{
  return FileLength;
}

uint16_t File_GetFileChecksum()
{
  return FileChecksum;
}

