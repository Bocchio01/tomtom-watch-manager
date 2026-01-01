#ifdef TT_BIG_ENDIAN
#define TT_BIGENDIAN(x) (x)
#else
#define TT_BIGENDIAN(x) (((((x) >> 24) & 0xff) << 0) | \
                         ((((x) >> 16) & 0xff) << 8) | \
                         ((((x) >> 8) & 0xff) << 16) | \
                         ((((x) >> 0) & 0xff) << 24))
#endif
