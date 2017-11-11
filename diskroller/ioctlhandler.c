/* bitdriver.c */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include<linux/slab.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include "ioctlhandler.h"
#include "logger.h"
#include "bitdriver.h"
#define SUCCESS 0

#define DEVICE_NAME "bit_driver"

/* No two opens at the any time */
static int num_device_open = 0;

extern sec_mdata_t global;
extern void *page_list;
extern int nr_pages;
#ifndef VM_RESERVED
# define  VM_RESERVED   (VM_DONTEXPAND | VM_DONTDUMP)
#endif

struct mmap_info {
    char *data;            
    int reference;      
};

/* Open the device */
static int device_open(struct inode *inode, struct file *file)
{

	struct mmap_info *info = vmalloc(sizeof(struct mmap_info));    
	LOG_MSG(INFO, "device_open(%p)\n", file);
	if (num_device_open)
		return -EBUSY;
	info->data = (char *)get_zeroed_page(GFP_KERNEL);
	memcpy(info->data, "hello from kernel this is file: ", 32);
	/* assign this info struct to the file */
	file->private_data = info;
	num_device_open++;

	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
	struct mmap_info *info = file->private_data;
	LOG_MSG(INFO, "device_release(%p,%p)\n", inode, file);

	free_page((unsigned long)info->data);
	vfree(info);
	file->private_data = NULL;

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
	mdata_t **ucs;
	int i;
	long ret_i = 0;
	switch (ioctl_num) {
		case IOCTL_GET_CHANGED_SECTOR:
		ucs = (mdata_t **)ioctl_param;
		
		for (i = 0; i < global.index; i++) {
			copy_to_user(ucs[i], global.msec[i], sizeof(mdata_t));
			kfree(global.msec[i]);
		}

		LOG_MSG(INFO, "Changed Sectors: %d", global.index);
		ret_i = global.index;
		global.index = 0;
		nr_pages = 0;
		break; 
	}
	//return global_bitmap.index;
	return ret_i;
}

 
void mmap_open(struct vm_area_struct *vma)
{
	struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
	info->reference++;
}
 
void mmap_close(struct vm_area_struct *vma)
{
	struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
	info->reference--;
}
 
static int mmap_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	struct page *page;
	struct mmap_info *info;    

	info = (struct mmap_info *)vma->vm_private_data;
	if (!info->data) {
		printk("No data\n");
		return 0;
	}

	LOG_MSG(INFO, "Faulting address:%p\n", (void*)vmf->address);
	LOG_MSG(INFO, "Faulting Page offset:%ld\n", vmf->pgoff);
	page = vmalloc_to_page(page_list+((vmf->pgoff)*PAGE_SIZE));
	LOG_MSG(INFO, "Logical Address:%p\n", (void*)info->data);
	LOG_MSG(INFO, "Corresponding physical Address:%p\n", (void*)__pa(info->data));
	LOG_MSG(INFO, "Page address:%p\n", (void*)page);
	get_page(page);
	vmf->page = page;
	return 0;
}

struct vm_operations_struct mmap_vm_ops = {
	.open = mmap_open,
	.close = mmap_close,
	.fault = mmap_fault,    
};
 
int device_mmap(struct file *file, struct vm_area_struct *vma)
{
	vma->vm_ops = &mmap_vm_ops;
	vma->vm_flags |= VM_RESERVED;
	vma->vm_private_data = file->private_data;
	mmap_open(vma);
	return 0;
}

struct file_operations Fops = {
	.read = device_read,
	.write = device_write,
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_release,	/* a.k.a. close */
	.mmap = device_mmap,
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

	LOG_MSG(INFO, "mknod %s c %d 0\n", DEVICE_FILE_NAME, MAJOR_NUM);
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
