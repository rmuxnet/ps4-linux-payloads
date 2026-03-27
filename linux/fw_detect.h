/*
 * fw_detect.h — firmware version detection via libc.sprx SCE header
 *
 * Adapted from Al-Azif/ps4-skeleton (MIT).
 * No special SDK required — only libkernel + libc.
 */

#ifndef FW_DETECT_H
#define FW_DETECT_H

#include <sys/types.h>

#ifndef U_TYPES_DEFINED
#define U_TYPES_DEFINED
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
#endif

/*
 * Returns the firmware version as a plain integer.
 * Examples: 505, 672, 700, 702, 900, 903, 960,
 *           1000, 1050, 1100, 1102, 1150, 1200, 1250, 1300, 1302
 *
 * Returns 0 on failure.
 * Result is cached after first call (safe to call multiple times).
 */
u16 get_firmware(void);

#endif /* FW_DETECT_H */
