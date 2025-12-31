#define TOMTOM_VENDOR_ID (0x1390)
#define TOMTOM_MULTISPORT_PRODUCT_ID (0x7474)
#define TOMTOM_SPARK_MUSIC_PRODUCT_ID (0x7475)
#define TOMTOM_SPARK_CARDIO_PRODUCT_ID (0x7477)
#define TOMTOM_TOUCH_PRODUCT_ID (0x7480)

#define IS_SPARK(id)                             \
    (((id) == TOMTOM_SPARK_MUSIC_PRODUCT_ID) ||  \
     ((id) == TOMTOM_SPARK_CARDIO_PRODUCT_ID) || \
     ((id) == TOMTOM_TOUCH_PRODUCT_ID))

#ifdef TT_BIG_ENDIAN
#define TT_BIGENDIAN(x) (x)
#else
#define TT_BIGENDIAN(x) (((((x) >> 24) & 0xff) << 0) | \
                         ((((x) >> 16) & 0xff) << 8) | \
                         ((((x) >> 8) & 0xff) << 16) | \
                         ((((x) >> 0) & 0xff) << 24))
#endif
