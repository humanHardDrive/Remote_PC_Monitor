#ifndef __FILE_HELPER__
#define __FILE_HELPER__

#include <Arduino.h>

void File_write(uint32_t index, uint8_t* buf, uint8_t len);
void File_read(uint32_t index, uint8_t* buf, uint8_t len);

void File_flush();

#endif
