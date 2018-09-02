#ifndef __FILE_HELPER__
#define __FILE_HELPER__

#include <Arduino.h>

#define FILE_INFO_PAGE  0
#define FILE_DATA_PAGE  1

struct FILE_INFO
{
  uint32_t length;
  uint16_t checksum;
};

void File_init();

uint8_t File_write(uint32_t index, uint8_t* buf, uint8_t len);
uint8_t File_read(uint32_t index, uint8_t* buf, uint8_t len);

void File_reset();
void File_finalize();
uint8_t File_flush();

uint32_t File_GetFileLength();
uint16_t File_GetFileChecksum();

#endif
