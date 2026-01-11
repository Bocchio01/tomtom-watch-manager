# TomTom-Watch-Manager
CLI and lightweight GUI tool to download, visualize, and manage GPS tracks from TomTom sport watches. Supports USB connection, GPX export, and activity organization.

### Resources
[Details on USB- no luck so far](https://stackoverflow.com/questions/17679480/details-on-usb-no-luck-so-far/17679571#17679571)


0x0073xxnn | History data. For each activity type (xx) for the last 10 activities (0xnn) such a file is generated: these are the files that alow to show the summary when waiting for the watch to get satelite. They cannot be deleted when the ttbin is downloaded beacuse they are referenced by the activity summaries file.

0x008300xx | Activity summary. Each file contains the last 10 activities. This file is used by the watch to show the last 10 activities for each activity type xx. These are like the thumbnails of the activities. They cannot be deleted when the ttbin is downloaded because they are referenced by the activity summaries file.

Both files are deleted only when all the ttbin files are downloaded and removed from the watch.


```C
#define TTWATCH_FILE_BLE_FIRMWARE           (0x00000012)
#define TTWATCH_FILE_SYSTEM_FIRMWARE        (0x000000f0)
#define TTWATCH_FILE_GPSQUICKFIX_DATA       (0x00010100)
#define TTWATCH_FILE_GPS_FIRMWARE           (0x00010200)
#define TTWATCH_FILE_BLE_UPDATE_LOG         (0x00013001)
#define TTWATCH_FILE_BLE_PEER_NAME          (0x00020002) // Missing from PDF
#define TTWATCH_FILE_BLE_DEVICE_DESCRIPTION (0x00020005) // Missing from PDF
#define TTWATCH_FILE_BLE_PAIRING_CODES      (0x0002000f) // Missing from PDF
#define TTWATCH_FILE_MANIFEST1              (0x00850000)
#define TTWATCH_FILE_MANIFEST2              (0x00850001)
#define TTWATCH_FILE_PREFERENCES_XML        (0x00f20000)
#define TTWATCH_FILE_RACE_DATA              (0x00710000)
#define TTWATCH_FILE_RACE_HISTORY_DATA      (0x00720000)
#define TTWATCH_FILE_HISTORY_DATA           (0x00730000)
#define TTWATCH_FILE_HISTORY_SUMMARY        (0x00830000)
#define TTWATCH_FILE_TTBIN_DATA             (0x00910000)
#define TTWATCH_FILE_ACTIVITY_SUMMARY       (0x00b30000)
```