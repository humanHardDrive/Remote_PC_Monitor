#pragma once

#include <stdint.h>

#define WRAPPER_STX		0x55

#define MAX_WRAPPER_SIZE		128
#define MAX_PAYLOAD_SiZE		124
#define MAX_BAGGAGE_SIZE		123

enum COMMAND_TYPE
{
	NO_COMMAND = 0,
	SEND_FILE,
	VALIDATE_FILE,
	LOAD_FILE,
	LIST_SENSORS,
	UPDATE_SENSOR,
	ALL_COMMANDS
};

enum PARSING_STATE
{
	WAITING_FOR_STX = 0,
	WAITING_FOR_RNG,
	WAITING_FOR_LEN,
	READING_PAYLOAD,
	WAITING_FOR_CHECKSUM
};

#pragma pack(1)
struct PACKET_WRAPPER
{
	uint8_t STX;
	uint8_t rng;
	uint8_t len;

	uint8_t payload[MAX_PAYLOAD_SiZE];

	uint8_t checksum;
};

struct COMMAND_PAYLOAD
{
	uint8_t cmd;
	uint8_t baggage[MAX_BAGGAGE_SIZE];
};

struct SEND_FILE_MSG
{
	uint32_t index;
	uint8_t len;
	uint8_t data[64];
};

struct SEND_FILE_RSP
{
	uint32_t index;
	uint8_t ack;
};

struct VALIDATE_FILE_MSG
{
};

struct VALIDATE_FILE_RSP
{
	uint8_t valid;
};

struct LIST_SENSORS_MSG
{
	uint8_t index;
};

struct LIST_SENSORS_RSP
{
	uint8_t index;
	uint8_t len;
	char name[114];
};

struct UPDATE_SENSOR_MSG
{
	uint8_t index;
};

struct UPDATE_SENSOR_RSP
{
	uint8_t index;
	uint16_t val;
	char scalar;
};
