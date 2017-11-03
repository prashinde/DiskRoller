#ifndef BITMAP_DATA
#define BITMAP_DATA

#include <linux/types.h>
typedef struct bit_map {
#define MAX_NR_SECTOR 255
	sector_t changed_sectors[MAX_NR_SECTOR];
	int index;
} bit_map_t;

void bitdriver_init_bitmap(sector_t num_sector);

void bitdriver_update_bitmap(sector_t sector_num);
#endif
