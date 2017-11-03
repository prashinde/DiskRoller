#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/bio.h>
#include <linux/device-mapper.h>

#include "bitdriver.h"
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
	struct bio_vec   bvl;
	struct bvec_iter bvi;

        LOG_MSG(CRIT, "\n<<in function intercept_map \n");

        bio->bi_bdev = mdt->dev->bdev;

        if(IS_WRITE(bio)) {
		LOG_MSG(INFO, "Calling hook function");
		LOG_MSG(INFO, "IO size:%d", bio->bi_vcnt);
		bitdriver_update_bitmap(bio->bi_iter.bi_sector);

		bvi = bio->bi_iter;
		for_each_bvec(bvl, bio->bi_io_vec, bvi, bvi) {
			void *page = kmap_atomic(bvl.bv_page);
			LOG_MSG(INFO, "Write Paged Data is:%s", (char *)page);
			kunmap_atomic(page);
		}
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

        LOG_MSG(CRIT, "\n>>out function intercept_map \n");       
        return DM_MAPIO_SUBMITTED;
}


static int 
intercept_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
        struct dr_dm_target *mdt;
        unsigned long long start;

        LOG_MSG(CRIT, "\n >>in function intercept_ctr \n");

        if (argc != 2) {
                LOG_MSG(CRIT, "\n Invalid no.of arguments.\n");
                ti->error = "Invalid argument count";
                return -EINVAL;
        }

        mdt = kmalloc(sizeof(struct dr_dm_target), GFP_KERNEL);

        if(mdt==NULL)
        {
                LOG_MSG(CRIT, "\n Mdt is null\n");
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

	LOG_MSG(INFO, "DEvice mapper nmber of sectors = %lu", mdt->dev->bdev->bd_disk->part0.nr_sects);

	/* Init Bitmap function */
	bitdriver_init_bitmap(mdt->dev->bdev->bd_disk->part0.nr_sects);

        LOG_MSG(CRIT, "\n>>out function intercept_ctr \n");                       
        return 0;

bad:
        kfree(mdt);
        LOG_MSG(CRIT, "\n>>out function intercept_ctr with errorrrrrrrrrr \n");           
        return -EINVAL;
}



static void intercept_dtr(struct dm_target *ti)
{
        struct dr_dm_target *mdt = (struct dr_dm_target *) ti->private;
        LOG_MSG(CRIT, "\n<<in function intercept_dtr \n");        
        dm_put_device(ti, mdt->dev);
        kfree(mdt);
        LOG_MSG(CRIT, "\n>>out function intercept_dtr \n");               
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
        return 0;
}


static void cleanup_intercept(void)
{
        dm_unregister_target(&intercept);
}

module_init(init_intercept);
module_exit(cleanup_intercept);
MODULE_LICENSE("GPL");
