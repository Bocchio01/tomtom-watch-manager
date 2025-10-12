#include <cstdint>
#include <ctime>

typedef struct
{
    uint8_t direction; // 0x09 = transmit, 0x01 = receive
    uint8_t length;    // of remaining packet (excluding direction and length)
    uint8_t counter;
    uint8_t message_id;
} PacketHeader;

// TX packets
typedef struct
{
    uint32_t _unk[2];
} TXFindFirstFilePacket;

typedef struct
{
    uint32_t id;
} TXFileOperationPacket;

typedef struct
{
    uint32_t id;
    uint32_t length;
} TXReadFileDataPacket;

typedef struct
{
    uint32_t id;
    uint8_t data[246];
} TXWriteFileDataPacket;

// RX packets
typedef struct
{
    uint32_t _unk1;
    uint32_t id;
    uint32_t _unk2;
    uint32_t file_size;
    uint32_t end_of_list;
} RXFindFilePacket;

typedef struct
{
    uint32_t _unk1;
} RXFindClosePacket;

typedef struct
{
    uint32_t _unk1;
    uint32_t id;
    uint32_t _unk2[2];
    uint32_t error;
} RXFileOperationPacket;

typedef struct
{
    uint32_t _unk1;
    uint32_t id;
    uint32_t _unk2;
    uint32_t file_size;
    uint32_t _unk3;
} RXGetFileSizePacket;

typedef struct
{
    uint32_t id;
    uint32_t data_length;
    uint8_t data[242];
} RXReadFileDataPacket;

typedef struct
{
    uint32_t _unk1;
    uint32_t id;
    uint32_t _unk2[3];
} RXWriteFileDataPacket;

typedef struct
{
    uint32_t utc_time;
    uint32_t _unk[4];
} RXGetCurrentTimePacket;

typedef struct
{
    char version[60];
} RXGetFirmwareVersionPacket;

typedef struct
{
    uint32_t product_id;
} RXGetProductIDPacket;

typedef struct
{
    uint32_t ble_version;
} RXGetBLEVersionPacket;

typedef struct
{
    char message[60];
} RXRebootWatchPacket;

typedef struct
{
    uint32_t _unk1[4];
    uint32_t error;
} RXFormatWatchPacket;