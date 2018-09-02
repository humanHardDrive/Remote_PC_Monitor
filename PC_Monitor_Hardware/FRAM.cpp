#include "FRAM.h"

#include <SPI.h>

FRAM::FRAM(uint8_t cs, uint32_t size)
{
  this->m_CS = cs;
  this->m_DevSize = size;
}


uint32_t FRAM::Size()
{
  return m_DevSize;
}

uint8_t FRAM::CS()
{
  return m_CS;
}


void FRAM::Write(uint32_t address, uint8_t val)
{
  uint8_t buffer[5];
  buffer[0] = FRAM::WRITE;
  buffer[1] = ((uint8_t*)&address)[2];
  buffer[2] = ((uint8_t*)&address)[1];
  buffer[3] = ((uint8_t*)&address)[0];
  buffer[4] = val;

  pinMode(m_CS, LOW);
  SPI.transfer(buffer, 5);
  pinMode(m_CS, HIGH);
}

void FRAM::Write(uint32_t address, uint8_t* buf, uint32_t len)
{
  uint8_t buffer[4];
  buffer[0] = FRAM::WRITE;
  buffer[1] = ((uint8_t*)&address)[2];
  buffer[2] = ((uint8_t*)&address)[1];
  buffer[3] = ((uint8_t*)&address)[0];

  pinMode(m_CS, LOW);
  SPI.transfer(buffer, 4);
  for (uint32_t i = 0; i < len; i++)
    SPI.transfer(buf[i]);
  pinMode(m_CS, HIGH);
}


uint8_t FRAM::Read(uint32_t address)
{
  uint8_t buffer[5];
  buffer[0] = FRAM::READ;
  buffer[1] = ((uint8_t*)&address)[2];
  buffer[2] = ((uint8_t*)&address)[1];
  buffer[3] = ((uint8_t*)&address)[0];
  buffer[4] = 0x00;

  pinMode(m_CS, LOW);
  SPI.transfer(buffer, 5);
  pinMode(m_CS, HIGH);

  return buffer[4];
}

void FRAM::Read(uint32_t address, uint8_t* buf, uint32_t len)
{
  uint8_t buffer[4];
  buffer[0] = FRAM::READ;
  buffer[1] = ((uint8_t*)&address)[2];
  buffer[2] = ((uint8_t*)&address)[1];
  buffer[3] = ((uint8_t*)&address)[0];

  pinMode(m_CS, LOW);
  SPI.transfer(buffer, 4);
  for (uint32_t i = 0; i < len; i++)
    buf[i] = SPI.transfer(0x00);
  pinMode(m_CS, HIGH);
}
