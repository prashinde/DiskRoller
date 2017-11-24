/* dr.c */

#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/types.h>

#include "memory.h"
#include "diskroller.h"
#include "logger.h"

int dr_update_entry(sector_t sector_num, void *page, long len, long offset)
{
#if 0
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
	//dump_entry();
#endif
	return 0;
}
EXPORT_SYMBOL(dr_update_entry);

int dr_init_ib(void)
{
	int rc;
	/* Buffer size should be configurable. */
	rc = dr_alloc_buffer(10);
	if(rc != 0) {
		LOG_MSG(ERROR, "Interceptor buffer allocation failed");
		return rc;
	}

	return 0;
}
EXPORT_SYMBOL(dr_init_ib);

void dr_release_ib(void)
{
	dr_free_buffer();
}
EXPORT_SYMBOL(dr_release_ib);
