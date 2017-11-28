#include "memory.h"
struct op_md free_list; 
struct op_md ready_list; 
struct op_md mapped_list;

/*
 * Returns free page slot from free list
 * Return NULL if no free slot is available
 **/
struct op_md *dr_get_free_md(void)
{
	struct op_md *free_s;

	spin_lock(&free_list.oms_lock);
	free_s = list_first_entry_or_null(&free_list.oms_list, 
					   struct op_md, oms_list);
	if(free_s == NULL) {
		spin_unlock(&free_list.oms_lock);
		LOG_MSG(CRIT, "No free slot available. Abort");
		return NULL;
	}
	/* Remove this entry from free list */
	list_del_init(&free_s->oms_list);
	spin_unlock(&free_list.oms_lock);
	LOG_MSG(INFO, "Returning a free page..");
	return free_s;
}

/*
 * Put page slot in ready list to be picked up by
 * the shadofax
 **/
int dr_put_ready_list(struct op_md *t)
{
	struct op_md *tmp;
	struct list_head *pos;
	spin_lock(&ready_list.oms_lock);
	list_add_tail(&(t->oms_list), &(ready_list.oms_list));
	spin_unlock(&ready_list.oms_lock);
	LOG_MSG(INFO, "Put page in a ready list page..");
	
	LOG_MSG(CRIT, "t->prev:%p t->next:%p\n", t->oms_list.prev, t->oms_list.next);
	list_for_each(pos, &(ready_list.oms_list)) {
		tmp = list_entry(pos, struct op_md, oms_list);
		LOG_MSG(INFO, "Free List:Sequence number:%llu, %p", tmp->oms_pg_id, tmp->oms_page);
	}
	return 0;
}

/*
 * Move N Page slots from ready to mapped list 
 * Need exclusive lock on both lists.
 * Whole movement should be atomic
 * Once pages are moved to mapped list they can be accessed
 * by ioctlhandler. 
 * Main focus is to release free_list lock asap.
 */
int dr_move_ready_mapped(uint32_t n)
{
	int           i;
	struct op_md *r;

	LOG_MSG(INFO, "Moving pages from ready to mapped list");
	//spin_lock(&ready_list.oms_lock);
	for(i = 0; i < n; i++) {
		r = list_first_entry_or_null(&ready_list.oms_list, 
					      struct op_md, oms_list);
		if(r == NULL) {
			LOG_MSG(CRIT, "R is fucking NULL");
			/*spin_unlock(&ready_list.oms_lock);*/
			return i;
		}
		/* Remove this entry from ready list */
		list_del_init(&r->oms_list);

		/* Add to mapped list */
		list_add_tail(&(r->oms_list), &(mapped_list.oms_list));
	}
	//spin_unlock(&ready_list.oms_lock);
	LOG_MSG(INFO, "ready->mapped success");
	return i;
}

/*
 * Move all mapped slots to free slots.
 **/
int dr_move_mapped_free(void)
{
	struct op_md *tmp;
	struct list_head *pos, *q;

		
	LOG_MSG(INFO, "Mapped->free");
	spin_lock(&free_list.oms_lock);
	list_for_each_safe(pos, q, &(mapped_list.oms_list)) {
		tmp = list_entry(pos, struct op_md, oms_list);
		list_del_init(pos);

		/* Add to mapped list */
		list_add_tail(&(tmp->oms_list), &(free_list.oms_list));
	}
	spin_unlock(&free_list.oms_lock);
	LOG_MSG(INFO, "Mapped->free succedd");
	return 0;
}

int dr_alloc_buffer(uint64_t nr_pages)
{
	int i;
	struct op_md *tmp;
	struct list_head *pos;

	INIT_LIST_HEAD(&free_list.oms_list);
	INIT_LIST_HEAD(&ready_list.oms_list);
	INIT_LIST_HEAD(&mapped_list.oms_list);

	spin_lock_init(&free_list.oms_lock);
	spin_lock_init(&ready_list.oms_lock);
	spin_lock_init(&mapped_list.oms_lock);

	for(i = 0; i < nr_pages; i++) {
		tmp = kmalloc(sizeof(*tmp), GFP_KERNEL);
		if(tmp == NULL) {
			LOG_MSG(ERROR, "IB metadata allocation failed");
			dr_free_buffer();
			return -ENOMEM;
		}

		tmp->oms_page = vmalloc(PAGE_SIZE);
		if(tmp->oms_page == NULL) {
			kfree(tmp);
			dr_free_buffer();
			LOG_MSG(ERROR, "IB page allocation failed");
			return -ENOMEM;
		}

		tmp->oms_pg_id = i;
		list_add_tail(&(tmp->oms_list), &(free_list.oms_list));
	}

	list_for_each(pos, &(free_list.oms_list)) {
		tmp = list_entry(pos, struct op_md, oms_list);
		LOG_MSG(INFO, "Sequence number:%llu, %p", tmp->oms_pg_id, tmp->oms_page);
	}

	return 0;
}

int dr_free_buffer(void)
{
	struct op_md *tmp;
	struct list_head *pos, *q;

	list_for_each_safe(pos, q, &(free_list.oms_list)) {
		tmp = list_entry(pos, struct op_md, oms_list);
		list_del(pos);
		LOG_MSG(INFO, "Sequence number:%llu, %p", tmp->oms_pg_id, tmp->oms_page);
		vfree(tmp->oms_page);
		kfree(tmp);
	}
	return 0;
}

void *dr_read_n_entries(int num)
{
	struct list_head *pos;
	void             *mem;
	struct op_md     *tmp;
	struct entry     *e;

	mem = kmalloc(sizeof(struct entry)*num, GFP_KERNEL);
	if(mem == NULL) {
		LOG_MSG(CRIT, "IB Overflow! Abort!");
		return mem;
	}

	e = (struct entry *)mem;
	list_for_each(pos, &(mapped_list.oms_list)) {
		tmp = list_entry(pos, struct op_md, oms_list);
		e->e_pg_id = tmp->oms_pg_id;
		e->e_seqno = tmp->oms_seqno;
		e->e_sector = tmp->oms_sector;
		e->e_off = tmp->oms_off;
		e->e_len = tmp->oms_len;
		e = e + sizeof(struct entry);
	}

	return mem;
}

void *dr_get_page(pgoff_t pgoff)
{
	void             *page;
	struct op_md     *tmp;
	struct list_head *pos;

	page = NULL;

	/* Page has to be found in the mapped list */
	list_for_each(pos, &(mapped_list.oms_list)) {
		tmp = list_entry(pos, struct op_md, oms_list);
		if(tmp->oms_pg_id == pgoff) {
			page = tmp->oms_page;
			break;
		}
	}

	return page;
}

long dr_get_nr_mapped(void)
{
	struct list_head *pos;
	long              count = 0;

	/* How many pages in ready list? */	
	list_for_each(pos, &(ready_list.oms_list)) {
		count++;
	}

	return count;
}
