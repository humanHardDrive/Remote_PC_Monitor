#include "FileHelper.h"
#include "Debug.h"
#include "Msgs.h"

#include <extEEPROM.h>

uint8_t FileBuffer[EEPROM_PAGE_SIZE];
uint16_t CurrentFilePage = FILE_DATA_PAGE;
bool BufferDirty = false;

uint32_t FileLength;
uint16_t FileChecksum;

extEEPROM filemem(kbits_128, 1, 128);

void File_init()
{
  CurrentFilePage = FILE_DATA_PAGE;
#ifndef USE_HARDWARE
  filemem.read(FILE_DATA_PAGE * EEPROM_PAGE_SIZE, FileBuffer, EEPROM_PAGE_SIZE);
#else
  memset(FileBuffer, 0, EEPROM_PAGE_SIZE);
#endif
}


uint8_t File_write(uint32_t index, uint8_t* buf, uint8_t len)
{
  uint16_t desiredpage, pageoffset;
  uint8_t retval = 0;

  desiredpage = (index / EEPROM_PAGE_SIZE) + FILE_DATA_PAGE;
  pageoffset = index % EEPROM_PAGE_SIZE;

  for (uint8_t i = 0; i < len; i++)
  {
    if (pageoffset >= EEPROM_PAGE_SIZE)
    {
      pageoffset -= EEPROM_PAGE_SIZE;
      desiredpage++;
    }

    if (desiredpage != CurrentFilePage)
    {
#ifdef DEBUG_3
      Serial.print(F("Page mismatch: "));
      Serial.print(desiredpage, HEX);
      Serial.print(F(" vs "));
      Serial.println(CurrentFilePage, HEX);
#endif

      retval = File_flush();
      if(retval)
        return retval;

#ifndef USE_HARDWARE
      retval = filemem.read(desiredpage * EEPROM_PAGE_SIZE, FileBuffer, EEPROM_PAGE_SIZE);
      if(retval)
        return retval;
#else
      memset(FileBuffer, 0, EEPROM_PAGE_SIZE);
#endif

      CurrentFilePage = desiredpage;
      BufferDirty = false;
    }

    if (!BufferDirty && FileBuffer[pageoffset] != buf[i])
    {
      BufferDirty = true;

#ifdef DEBUG_3
      Serial.println("Dirty");
#endif
    }

    FileBuffer[pageoffset] = buf[i];
    pageoffset++;
  }

  if (desiredpage * EEPROM_PAGE_SIZE + pageoffset > FileLength)
  {
    FileLength = (uint32_t)(desiredpage * EEPROM_PAGE_SIZE + pageoffset);

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

  desiredpage = (index / EEPROM_PAGE_SIZE) + FILE_DATA_PAGE;
  pageoffset = index % EEPROM_PAGE_SIZE;

  for (uint8_t i = 0; i < len; i++)
  {
    if (pageoffset + i >= EEPROM_PAGE_SIZE)
    {
      pageoffset = 0;
      desiredpage++;
    }

    if (desiredpage != CurrentFilePage)
    {
#ifdef DEBUG_3
      Serial.print(F("Page mismatch: "));
      Serial.print(desiredpage, HEX);
      Serial.print(F(" vs "));
      Serial.println(CurrentFilePage, HEX);
#endif

      retval = File_flush();
      if(retval)
        return retval;

#ifndef USE_HARDWARE
      retval = filemem.read(desiredpage * EEPROM_PAGE_SIZE, FileBuffer, EEPROM_PAGE_SIZE);
      if(retval)
        return retval;
#else
      memset(FileBuffer, 0, EEPROM_PAGE_SIZE);
#endif

      CurrentFilePage = desiredpage;
      BufferDirty = false;
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
      filemem.read(page * EEPROM_PAGE_SIZE, buffer, EEPROM_PAGE_SIZE);
#else
      memset(buffer, 0, EEPROM_PAGE_SIZE);
#endif
    }

    FileChecksum += buffer[pageoffset];
    pageoffset++;
  }
}

uint8_t File_flush()
{
  uint8_t retval = 0;
  
  if (BufferDirty)
  {
#ifdef USE_HARDWARE
    retval = filemem.write(CurrentFilePage * EEPROM_PAGE_SIZE, FileBuffer, EEPROM_PAGE_SIZE);
#endif

#ifdef DEBUG_3
    Serial.print("Page ");
    Serial.print(CurrentFilePage);
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
    BufferDirty = false;
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

