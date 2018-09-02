#ifndef __FRAM_H__
#define __FRAM_H__

#include <Arduino.h>

class FRAM
{
  public:
    FRAM(uint8_t cs, uint32_t size);

    uint32_t Size();
    uint8_t CS();

    void Write(uint32_t address, uint8_t val);
    void Write(uint32_t address, uint8_t* buf, uint32_t len);

    uint8_t Read(uint32_t address);
    void Read(uint32_t address, uint8_t* buf, uint32_t len);

  public:
    const uint8_t WRITE_ENABLE = 0x06;
    const uint8_t WRITE_DISABLE = 0x04;
    const uint8_t READ_STATUS = 0x05;
    const uint8_t WRITE_STATUS = 0x01;
    const uint8_t READ = 0x03;
    const uint8_t WRITE = 0x02;
    const uint8_t SLEEP = 0xB9;
    const uint8_t WAKE = 0xAB;

  private:
    uint32_t m_DevSize;
    uint8_t m_CS;
};

#endif
