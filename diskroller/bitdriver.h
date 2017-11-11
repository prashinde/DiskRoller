#ifndef BITMAP_DATA
#define BITMAP_DATA

#include "ioctlhandler.h"
#include <linux/types.h>

#define MAX_NR_SECTOR 2048

typedef struct sec_mdata {
	mdata_t *msec[MAX_NR_SECTOR];
	int index;
} sec_mdata_t;

void bitdriver_init_bitmap(sector_t num_sector);

void bitdriver_update_bitmap(sector_t sector_num, void *page, long len, long offset);
#endif
