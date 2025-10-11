#define TOMTOM_VENDOR_ID (0x1390)
#define TOMTOM_MULTISPORT_PRODUCT_ID (0x7474)
#define TOMTOM_SPARK_MUSIC_PRODUCT_ID (0x7475)
#define TOMTOM_SPARK_CARDIO_PRODUCT_ID (0x7477)
#define TOMTOM_TOUCH_PRODUCT_ID (0x7480)

#define IS_SPARK(id)                             \
    (((id) == TOMTOM_SPARK_MUSIC_PRODUCT_ID) ||  \
     ((id) == TOMTOM_SPARK_CARDIO_PRODUCT_ID) || \
     ((id) == TOMTOM_TOUCH_PRODUCT_ID))

// Add blank line here at end of file