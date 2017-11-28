#ifndef __IOCTL_H_
#define __IOCTL_H_

#ifndef __KERNEL__
#define uint32_t unsigned int
#define uint16_t unsigned short
#define uint8_t unsigned char
#define uint64_t unsigned long long
#define sector_t unsigned long
#else
#include <linux/types.h>
#endif

#include <linux/ioctl.h>

struct entry {
	uint64_t     e_pg_id;
	uint64_t     e_seqno;
	sector_t     e_sector;
	unsigned int e_off;
	unsigned int e_len;
};

struct mdata {
	int            md_num;
	struct entry  *md_ent;
};

#define MAJOR_NUM 100

/* 
 * Get the Bitmap 
 */
#define IOCTL_GET_CHANGED_SECTOR _IOWR(MAJOR_NUM, 2, unsigned long)

#define IOCTL_TEST_MAP_IB _IOWR(MAJOR_NUM, 3, unsigned long)
#define IOCTL_TEST_UNMAP_IB _IOWR(MAJOR_NUM, 4, unsigned long)
#define IOCTL_TEST_GET_NR_B _IOWR(MAJOR_NUM, 5, unsigned long)
/* 
 * The name of the device file 
 */
#define DEVICE_FILE_NAME "/dev/bit_driver"

#endif
