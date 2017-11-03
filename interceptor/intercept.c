#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/bio.h>
#include <linux/device-mapper.h>

#include "bitdriver.h"

#include "logger.h"

/* This is a structure which will store  information about the underlying device 
 * Param:
 * dev : underlying device
 * start:  Starting sector number of the device
 */
struct dr_dm_target {
        struct dm_dev *dev;
        sector_t start;
};

/* This is map function of basic target. This function gets called whenever you get a new bio
 * request.The working of map function is to map a particular bio request to the underlying device. 
 * The request that we receive is submitted to out device so  bio->bi_bdev points to our device.
 * We should point to the bio-> bi_dev field to bdev of underlying device. Here in this function,
 * we can have other processing like changing sector number of bio request, splitting bio etc. 
 *
 *  Param : 
 *  ti : It is the dm_target structure representing our basic target
 *  bio : The block I/O request from upper layer
 *
 *: Return values from target map function:
 *  DM_MAPIO_SUBMITTED :  Your target has submitted the bio request to underlying request
 *  DM_MAPIO_REMAPPED  :  Bio request is remapped, Device mapper should submit bio.  
 *  DM_MAPIO_REQUEUE   :  Some problem has happened with the mapping of bio, So 
 *                                                re queue the bio request. So the bio will be submitted 
 *                                                to the map function  
 */

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


/* This is Constructor Function of basic target 
 *  Constructor gets called when we create some device of type 'intercept'.
 *  So it will get called when we execute command 'dmsetup create'
 *  This  function gets called for each device over which you want to create basic 
 *  target. Here it is just a basic target so it will take only one device so it  
 *  will get called once. 
 */
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
        

        /* dm_get_table_mode 
         * Gives out you the Permissions of device mapper table. 
         * This table is nothing but the table which gets created
         * when we execute dmsetup create. This is one of the
         * Data structure used by device mapper for keeping track of its devices.
         *
         * dm_get_device 
         * The function sets the mdt->dev field to underlying device dev structure.
         */

    
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



/*
 * This is destruction function
 * This gets called when we remove a device of type basic target. The function gets 
 * called per device. 
 */
static void intercept_dtr(struct dm_target *ti)
{
        struct dr_dm_target *mdt = (struct dr_dm_target *) ti->private;
        LOG_MSG(CRIT, "\n<<in function intercept_dtr \n");        
        dm_put_device(ti, mdt->dev);
        kfree(mdt);
        LOG_MSG(CRIT, "\n>>out function intercept_dtr \n");               
}

/*

 * This structure is fops for basic target.

 */
static struct target_type intercept = {
        
        .name = "intercept",
        .version = {1,0,0},
        .module = THIS_MODULE,
        .ctr = intercept_ctr,
        .dtr = intercept_dtr,
        .map = intercept_map,
};
        
/*---------Module Functions -----------------*/

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
