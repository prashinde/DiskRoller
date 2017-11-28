#ifndef __DR_H_
#define __DR_H_

#include "ioctlhandler.h"
#include <linux/types.h>

int dr_init_ib(void);
void dr_release_ib(void);
int dr_update_entry(sector_t sector_num, void *page, long len, long offset);
#endif
