/* bitdriver.c */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/types.h>

#include "ioctlhandler.h"
#include "logger.h"

#define SUCCESS 0

#define DEVICE_NAME "bit_driver"

/* No two opens at the any time */
static int num_device_open = 0;

/* Open the device */
static int device_open(struct inode *inode, struct file *file)
{

	LOG_MSG(INFO, "device_open(%p)\n", file);

	if (num_device_open)
		return -EBUSY;

	num_device_open++;

	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
	LOG_MSG(INFO, "device_release(%p,%p)\n", inode, file);

	/* 
	 * We're now ready for our next caller 
	 */
	num_device_open--;

	return SUCCESS;
}

/* 
 * This function is called whenever a process which has already opened the
 * device file attempts to read from it.
 */
static ssize_t device_read(struct file *file,	/* see include/linux/fs.h   */
			   char __user * buffer,	/* buffer to be
							 * filled with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{

	return 0;
}

/* 
 * This function is called when somebody tries to
 * write into our device file. 
 */
static ssize_t
device_write(struct file *file,
	     const char __user * buffer, size_t length, loff_t * offset)
{
	return 0;
}

/* 
 * This function is called whenever a process tries to do an ioctl on our
 * device file.
 * This function return the map of the changed sectors to user space.
 */
long device_ioctl(struct file *file,	/* ditto */
		 unsigned int ioctl_num,	/* number and param for ioctl */
		 unsigned long ioctl_param)
{

	/*switch (ioctl_num) {
		case IOCTL_GET_CHANGED_SECTOR:

		user_changed_sectors = (sector_t *)ioctl_param;
		copy_to_user(user_changed_sectors, global_bitmap.changed_sectors, sizeof(sector_t)*(global_bitmap.index));
		LOG_MSG(INFO, "Changed Sectors:::");
		for (i=0; i < global_bitmap.index; i++) 
			LOG_MSG(INFO, " %ld ", global_bitmap.changed_sectors[i]);
		break; 
	}*/
	//return global_bitmap.index;
	return 0;
}

/* Module Declarations */

/* 
 * This structure will hold the functions to be called
 * when a process does something to the device we
 * created. Since a pointer to this structure is kept in
 * the devices table, it can't be local to
 * init_module. NULL is for unimplemented functions. 
 */
struct file_operations Fops = {
	.read = device_read,
	.write = device_write,
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_release,	/* a.k.a. close */
};

/* 
 * Initialize the module - Register the character device 
 */
int init_module()
{
	int ret_val;
	/* 
	 * Register the character device (atleast try) 
	 */
	ret_val = register_chrdev(MAJOR_NUM, DEVICE_NAME, &Fops);

	/* 
	 * Negative values signify an error 
	 */
	if (ret_val < 0) {
		LOG_MSG(ALERT, "%s failed with %d\n",
		       "Sorry, registering the character device ", ret_val);
		return ret_val;
	}

	LOG_MSG(INFO, "%s The major device number is %d.\n",
	       "Registeration is a success", MAJOR_NUM);
	LOG_MSG(INFO, "If you want to talk to the device driver,\n");
	LOG_MSG(INFO, "you'll have to create a device file. \n");
	LOG_MSG(INFO, "We suggest you use:\n");
	LOG_MSG(INFO, "mknod %s c %d 0\n", DEVICE_FILE_NAME, MAJOR_NUM);
	LOG_MSG(INFO, "The device file name is important, because\n");
	LOG_MSG(INFO, "the ioctl program assumes that's the\n");
	LOG_MSG(INFO, "file you'll use.\n");

	return 0;
}

/* 
 * Cleanup - unregister the appropriate file from /proc 
 */
void cleanup_module()
{

	unregister_chrdev(MAJOR_NUM, DEVICE_NAME);

	LOG_MSG(ALERT, "unregister_chrdev:");
}
