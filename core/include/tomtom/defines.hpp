#define TOMTOM_VENDOR_ID (0x1390)
#define TOMTOM_MULTISPORT_PRODUCT_ID (0x7474)
#define TOMTOM_SPARK_MUSIC_PRODUCT_ID (0x7475)
#define TOMTOM_SPARK_CARDIO_PRODUCT_ID (0x7477)
#define TOMTOM_TOUCH_PRODUCT_ID (0x7480)

#define IS_SPARK(id)                             \
    (((id) == TOMTOM_SPARK_MUSIC_PRODUCT_ID) ||  \
     ((id) == TOMTOM_SPARK_CARDIO_PRODUCT_ID) || \
     ((id) == TOMTOM_TOUCH_PRODUCT_ID))

#define MSG_OPEN_FILE_WRITE (0x02)
#define MSG_DELETE_FILE (0x03)
#define MSG_WRITE_FILE_DATA (0x04)
#define MSG_GET_FILE_SIZE (0x05)
#define MSG_OPEN_FILE_READ (0x06)
#define MSG_READ_FILE_DATA_REQUEST (0x07)
#define MSG_READ_FILE_DATA_RESPONSE (0x09)
#define MSG_FIND_CLOSE (0x0a)
#define MSG_CLOSE_FILE (0x0c)
#define MSG_UNKNOWN_0D (0x0d)
#define MSG_FORMAT_WATCH (0x0e)
#define MSG_RESET_DEVICE (0x10)
#define MSG_FIND_FIRST_FILE (0x11)
#define MSG_FIND_NEXT_FILE (0x12)
#define MSG_GET_CURRENT_TIME (0x14)
#define MSG_UNKNOWN_1A (0x1a)
#define MSG_RESET_GPS_PROCESSOR (0x1d)
#define MSG_UNKNOWN_1F (0x1f)
#define MSG_GET_PRODUCT_ID (0x20)
#define MSG_GET_FIRMWARE_VERSION (0x21)
#define MSG_UNKNOWN_22 (0x22)
#define MSG_UNKNOWN_23 (0x23)
#define MSG_GET_BLE_VERSION (0x28)

#ifdef TT_BIG_ENDIAN
#define TT_BIGENDIAN(x) (x)
#else
#define TT_BIGENDIAN(x) (((((x) >> 24) & 0xff) << 0) | \
                         ((((x) >> 16) & 0xff) << 8) | \
                         ((((x) >> 8) & 0xff) << 16) | \
                         ((((x) >> 0) & 0xff) << 24))
#endif
