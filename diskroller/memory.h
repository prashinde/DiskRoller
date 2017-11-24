#ifndef __INTERCEPT_BUFF_
#define __INTERCEPT_BUFF_
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include "logger.h"
struct op_md {
	uint64_t          oms_pg_id;
	void             *oms_page;
	uint64_t          oms_seqno;
	sector_t          oms_sector;
	unsigned int      oms_off;
	unsigned int      oms_len;
	/* To spin or to sleep?, that is the question! */
	spinlock_t        oms_lock;	
	struct list_head  oms_list;
};
int dr_alloc_buffer(uint64_t nr_pages);
int dr_free_buffer(void);
#endif
