#ifndef BITDRIVER
#define BITDRIVER

#include <linux/ioctl.h>


#define MAJOR_NUM 100

/* 
 * Get the Bitmap 
 */
#define IOCTL_GET_CHANGED_SECTOR _IOWR(MAJOR_NUM, 2, unsigned long)

/* 
 * The name of the device file 
 */
#define DEVICE_FILE_NAME "/dev/bit_driver"

#endif
