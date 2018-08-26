#ifndef __FILE_HELPER__
#define __FILE_HELPER__

#include <Arduino.h>

#define FILE_INFO_PAGE  0
#define FILE_DATA_PAGE  1

void File_init();

void File_write(uint32_t index, uint8_t* buf, uint8_t len);
void File_read(uint32_t index, uint8_t* buf, uint8_t len);

void File_reset();
void File_finalize();
void File_flush();

uint32_t File_GetFileLength();
uint16_t File_GetFileChecksum();

#endif
