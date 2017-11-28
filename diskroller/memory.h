#ifndef __INTERCEPT_BUFF_
#define __INTERCEPT_BUFF_
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

#include "ioctlhandler.h"
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
struct op_md *dr_get_free_md(void);
int dr_put_ready_list(struct op_md *t);
int dr_move_ready_mapped(uint32_t n);
int dr_move_mapped_free(void);
void *dr_read_n_entries(int num);
void *dr_get_page(pgoff_t pgoff);
long dr_get_nr_mapped(void);
#endif
