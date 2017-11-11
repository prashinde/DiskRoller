/* bitdriver.c */

#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#include <linux/slab.h>
#include "bitdriver.h"
#include "logger.h"

#define SUCCESS 0

sec_mdata_t global;

int nr_pages = 0;
spinlock_t page_lock;
void *page_list;
   
static void init_bitmap(sector_t num_sector);

void bitdriver_update_bitmap(sector_t sector_num, void *page, long len, long offset)
{
	mdata_t *m;
	LOG_MSG(INFO, "Calling from hooked function..");
	LOG_MSG(INFO, "Sector number=%lu", sector_num);

	m = kmalloc(sizeof(mdata_t), GFP_KERNEL);
	if(m == NULL)
		return ;

	spin_lock(&page_lock);
	/* Crappy code. memcpy() with spinlock :D */
	if(nr_pages < 500)
		memcpy(page_list+(nr_pages*PAGE_SIZE), page, PAGE_SIZE);
	
	nr_pages++;
	m->s = sector_num;
	m->len = len;
	m->offset = offset;
	global.msec[global.index] = m;
	global.index++;
	spin_unlock(&page_lock);	
	LOG_MSG(INFO, "Number of Pages:%d ", nr_pages);
	//dump_bitmap();
}
EXPORT_SYMBOL(bitdriver_update_bitmap);

void bitdriver_init_bitmap(sector_t num_sector)
{
	LOG_MSG(INFO, "number of sectors = %lu", num_sector);
	spin_lock_init(&page_lock);

	page_list = vmalloc(500*PAGE_SIZE);
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
		global.msec[i] = NULL;	
}
#if 0
static void dump_bitmap(void)
{
	int i = 0;

	LOG_MSG(INFO, "Changed sectors are:");

	for (i = 0; i < global_bitmap.index; i++) {
		LOG_MSG(INFO, " %lu ", global_bitmap.changed_sectors[i]);
	}
}

#endif
