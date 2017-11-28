#ifndef BITDRIVER
#define BITDRIVER

#include <linux/types.h>
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
/* 
 * The name of the device file 
 */
#define DEVICE_FILE_NAME "/dev/bit_driver"

#endif
