#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/bio.h>
#include <linux/device-mapper.h>

#include "diskroller.h"
#include "logger.h"

/* This is a structure which will store  information about the underlying device 
 * Param:
 */
struct dr_dm_target {
		struct dm_dev *dev;
		sector_t start;
};

#define IS_WRITE(x) op_is_write(bio_op(x))
static int intercept_map(struct dm_target *ti, struct bio *bio)
{
		struct dr_dm_target *mdt = (struct dr_dm_target *) ti->private;
	struct bio_vec	 bvl;
	struct bvec_iter bvi;
	void *page;

		bio->bi_bdev = mdt->dev->bdev;

		if(IS_WRITE(bio)) {
		LOG_MSG(INFO, "Calling hook function");
		LOG_MSG(INFO, "IO size:%d", bio->bi_vcnt);

		bvi = bio->bi_iter;
		LOG_MSG(INFO, "*****************\n");
		for_each_bvec(bvl, bio->bi_io_vec, bvi, bvi) {
			LOG_MSG(INFO, "Device sector number:%lu,size:%u, index:%u", bvi.bi_sector, bvi.bi_size, bvi.bi_idx);
			page = kmap_atomic(bvl.bv_page);
			LOG_MSG(INFO, "LEm:%u off=%u\n", bvl.bv_len, bvl.bv_offset);
			dr_update_entry(bio->bi_iter.bi_sector, page, bvl.bv_len, bvl.bv_offset);
			kunmap_atomic(page);
		}
		LOG_MSG(INFO, "*****************\n");
	}
		/*else {
		LOG_MSG(INFO, "IO size:%d\n", bio->bi_size);
		LOG_MSG(INFO, "Reading from sector :%lu\n", bio->bi_sector);
		bio_for_each_segment(bvl, bio, i) {
			void *page = kmap_atomic(bvl->bv_page);
			LOG_MSG(INFO, "Read Paged Data is:%s", (char *)page);
			kunmap_atomic(page);
		}
	}*/

		submit_bio(bio);
		return DM_MAPIO_SUBMITTED;
}

static int 
intercept_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
		struct dr_dm_target *mdt;
		unsigned long long start;

		if (argc != 2) {
				LOG_MSG(CRIT, "\n Invalid no.of arguments.\n");
				ti->error = "Invalid argument count";
				return -EINVAL;
		}

		mdt = kmalloc(sizeof(struct dr_dm_target), GFP_KERNEL);
		if(mdt==NULL)
		{
				LOG_MSG(CRIT, "Mdt is null");
				ti->error = "dm-intercept: Cannot allocate linear context";
				return -ENOMEM;
		}		

		if(sscanf(argv[1], "%llu", &start)!=1)
		{
				ti->error = "dm-intercept: Invalid device sector";
				goto bad;
		}

		mdt->start=(sector_t)start;	
		if (dm_get_device(ti, argv[0], dm_table_get_mode(ti->table), &mdt->dev)) {
				ti->error = "dm-intercept: Device lookup failed";
				goto bad;
		}

		ti->private = mdt;
		return 0;
bad:
		kfree(mdt);
		return -EINVAL;
}

static void intercept_dtr(struct dm_target *ti)
{
	struct dr_dm_target *mdt = (struct dr_dm_target *) ti->private;
	dm_put_device(ti, mdt->dev);
	kfree(mdt);
}

static struct target_type intercept = {
		
		.name = "intercept",
		.version = {1,0,0},
		.module = THIS_MODULE,
		.ctr = intercept_ctr,
		.dtr = intercept_dtr,
		.map = intercept_map,
};
		

static int init_intercept(void)
{
		int result;
		result = dm_register_target(&intercept);
		if(result < 0)
				LOG_MSG(CRIT, "\n Error in registering target \n");
	dr_init_ib();
		return 0;
}


static void cleanup_intercept(void)
{
	dr_release_ib();
		dm_unregister_target(&intercept);
}

module_init(init_intercept);
module_exit(cleanup_intercept);
MODULE_LICENSE("GPL");
