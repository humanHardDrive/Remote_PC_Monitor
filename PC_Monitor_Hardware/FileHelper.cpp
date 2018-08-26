#include "FileHelper.h"
#include "Msgs.h"

#include <extEEPROM.h>

uint8_t FileBuffer[EEPROM_PAGE_SIZE];
uint16_t CurrentFilePage = FILE_DATA_PAGE;
bool BufferDirty = false;

uint16_t FileChecksum, FileLength;

extEEPROM filemem(kbits_128, 1, 128);

void File_init()
{
#ifndef __DEBUG__
      filemem.read(FILE_DATA_PAGE * EEPROM_PAGE_SIZE, FileBuffer, EEPROM_PAGE_SIZE);
#else
      memset(FileBuffer, 0, EEPROM_PAGE_SIZE);
#endif
}


void File_write(uint32_t index, uint8_t* buf, uint8_t len)
{
  uint16_t desiredpage, pageoffset;

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
      File_flush();

#ifndef __DEBUG__
      filemem.read(desiredpage * EEPROM_PAGE_SIZE, FileBuffer, EEPROM_PAGE_SIZE);
#else
      memset(FileBuffer, 0, EEPROM_PAGE_SIZE);
#endif

      CurrentFilePage = desiredpage;
      BufferDirty = false;
    }

    if (!BufferDirty && FileBuffer[pageoffset] != buf[i])
    {
      BufferDirty = true;

#ifdef __DEBUG__
      Serial.println("Dirty");
#endif
    }

    FileBuffer[pageoffset] = buf[i];
    pageoffset++;
  }
}

void File_read(uint32_t index, uint8_t* buf, uint8_t len)
{
  uint16_t desiredpage, pageoffset;

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
      File_flush();

#ifndef __DEBUG__
      filemem.read(desiredpage * EEPROM_PAGE_SIZE, FileBuffer, EEPROM_PAGE_SIZE);
#else
      memset(FileBuffer, 0, EEPROM_PAGE_SIZE);
#endif

      CurrentFilePage = desiredpage;
      BufferDirty = false;
    }

    buf[i] = FileBuffer[pageoffset];
    pageoffset++;
  }
}

void File_reset()
{
  FileChecksum = FileLength = 0;
}

void File_finalize()
{
  
}

void File_flush()
{
  if (BufferDirty)
  {
#ifndef __DEBUG__
    filemem.write(CurrentFilePage * EEPROM_PAGE_SIZE, FileBuffer, EEPROM_PAGE_SIZE);
#else
    Serial.print("Page ");
    Serial.print(CurrentFilePage);
    for (uint8_t i = 0; i < EEPROM_PAGE_SIZE; i++)
    {
      if (!(i & 0x0F))
        Serial.println();

      if (SendFileBuffer[i] < 0x10)
        Serial.print('0');
      Serial.print(SendFileBuffer[i], HEX);
      Serial.print(' ');
    }
    Serial.println();
#endif
    BufferDirty = false;
  }
}


uint16_t File_GetFileLength()
{
  return FileLength;
}

uint16_t File_GetFileChecksum()
{
  return FileChecksum;
}

