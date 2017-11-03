/* bitdriver.c */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/types.h>

#include "bitdriver.h"
#include "logger.h"

#define SUCCESS 0

bit_map_t global_bitmap;

static void dump_bitmap(void);
static void init_bitmap(sector_t num_sector);
static void dump_bitmap(void);
static void update_bit(sector_t sector_num);

void bitdriver_update_bitmap(sector_t sector_num)
{
	LOG_MSG(INFO, "Calling from hooked function..");
	LOG_MSG(INFO, "Sector number=%lu", sector_num);
	update_bit(sector_num);

	dump_bitmap();
}
EXPORT_SYMBOL(bitdriver_update_bitmap);

void bitdriver_init_bitmap(sector_t num_sector)
{
	LOG_MSG(INFO, "number of sectors = %lu", num_sector);

	/* 
	 * FIX_ME:
	 * Here we need to do calculations which determines how big bitmap we need.
	 * As of now, we will only use one interger (32 bits) to represent bitmap.
	 */
	init_bitmap(num_sector);
}
EXPORT_SYMBOL(bitdriver_init_bitmap);


static void init_bitmap(sector_t num_sector)
{
	int i = 0;

	for (i=0; i < MAX_NR_SECTOR; i++)
		global_bitmap.changed_sectors[i] = 0;	
}

static void dump_bitmap(void)
{
	int i = 0;

	LOG_MSG(INFO, "Changed sectors are:");

	for (i = 0; i < global_bitmap.index; i++) {
		LOG_MSG(INFO, " %lu ", global_bitmap.changed_sectors[i]);
	}
}

static void update_bit(sector_t sector_num)
{
	global_bitmap.changed_sectors[global_bitmap.index] = sector_num;
	global_bitmap.index++;
}
