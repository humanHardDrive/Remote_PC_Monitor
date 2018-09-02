#pragma once

#include <stdint.h>

#define WRAPPER_STX		0x55

#define MAX_WRAPPER_SIZE		128
#define MAX_PAYLOAD_SiZE		124
#define MAX_BAGGAGE_SIZE		123

#define EEPROM_PAGE_SIZE		128

enum COMMAND_TYPE
{
	NO_COMMAND = 0,
	SEND_FILE,
	SEND_FILE_INFO,
	VALIDATE_FILE,
	LOAD_FILE,
	LIST_SENSORS,
	UPDATE_SENSOR,
	LIST_PARAMETERS,
	READ_PARAMETER,
	MODIFY_PARAMETER,
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

struct FILE_INFO
{
	uint32_t MemAdd; //Where the data segment gets stored in flash
	uint32_t StorageAdd; //Where the data segment would be stored in EEPROM
};

#define APPLICATION_DATA_START		0x4000
#define SEND_FILE_RESET_INDEX		0x10
#define SEND_FILE_FINALIZE_INDEX	0x20
#define SEND_FILE_CLOSE_INDEX		0x00

struct SEND_FILE_MSG
{
	uint32_t index;
	uint8_t len;
	uint8_t data[EEPROM_PAGE_SIZE/2];
};

#define ACK_OK		0
struct SEND_FILE_RSP
{
	uint32_t index;
	uint8_t ack;
};

struct VALIDATE_FILE_MSG
{
	uint32_t len;
	uint16_t checksum;
};

#define FILE_VALID			0x00
#define FILE_LEN_INVALID	0x01
#define FILE_CHKSUM_INVALID	0x02
struct VALIDATE_FILE_RSP
{
	//This is a bit field of the above
	uint8_t valid;
};

//I'm using the load file to do a reset of the board
#define REBOOT_ONLY			0
#define REBOOT_LOAD_FILE	1
//useful when changing server parameters
struct LOAD_FILE_MSG
{
	uint8_t code;
};

#define ACK_OK			0
#define ACK_NO_FILE		1
struct LOAD_FILE_RSP
{
	uint8_t ack;
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

struct LIST_PARAMETERS_MSG
{
	uint8_t index;
};

struct LIST_PARAMETERS_RSP
{
	uint8_t index;
	uint8_t len;
	char name[114];
};

struct READ_PARAMETER_MSG
{
	uint8_t index;
};

struct READ_PARAMETER_RSP
{
	uint8_t index;
	uint16_t val;
};

struct MODIFY_PARAMETER_MSG
{
	uint8_t index;
	uint16_t val;
};

struct MODIFY_PARAMETER_RSP
{
	uint8_t index;
};
