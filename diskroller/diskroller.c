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


	struct op_md *f;

	LOG_MSG(INFO, "Updating the entry");	
	f = dr_get_free_md();
	if(f == NULL) {
		LOG_MSG(CRIT, "IB Overload! Abort");
		return -ENOMEM;
	}

	LOG_MSG(INFO, "Adding slot in Ready List");	
	f->oms_seqno = 0;
	f->oms_sector = sector_num;
	f->oms_len = len;
	f->oms_off = offset;
	memcpy(f->oms_page, page, PAGE_SIZE);
	/* Put it in a ready list */
	dr_put_ready_list(f);
	return 0;
}
EXPORT_SYMBOL(dr_update_entry);

int dr_init_ib(void)
{
	int rc;
	/* Buffer size should be configurable. */
	rc = dr_alloc_buffer(17000);
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
