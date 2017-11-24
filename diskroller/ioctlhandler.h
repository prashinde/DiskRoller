#ifndef BITDRIVER
#define BITDRIVER

#include <linux/types.h>
#include <linux/ioctl.h>

typedef struct mdata {
	unsigned long s;
	long len;
	long offset;
} mdata_t;

#define MAJOR_NUM 100

/* 
 * Get the Bitmap 
 */
#define IOCTL_GET_CHANGED_SECTOR _IOWR(MAJOR_NUM, 2, unsigned long)

#define IOCTL_TEST_IB _IOWR(MAJOR_NUM, 3, unsigned long)
/* 
 * The name of the device file 
 */
#define DEVICE_FILE_NAME "/dev/bit_driver"

#endif
