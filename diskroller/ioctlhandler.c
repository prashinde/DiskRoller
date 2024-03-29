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
#include "diskroller.h"
#include "memory.h"

#define DEVICE_NAME "bit_driver"

#ifndef VM_RESERVED
# define  VM_RESERVED   (VM_DONTEXPAND | VM_DONTDUMP)
#endif

/* Open the device */
static int device_open(struct inode *inode, struct file *file)
{
#if 0
	struct mmap_info *info = vmalloc(sizeof(struct mmap_info));    
	LOG_MSG(INFO, "device_open(%p)\n", file);
	if (num_device_open)
		return -EBUSY;
	info->data = (char *)get_zeroed_page(GFP_KERNEL);
	memcpy(info->data, "hello from kernel this is file: ", 32);
	/* assign this info struct to the file */
	file->private_data = info;
	num_device_open++;
#endif
	return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
#if 0
	struct mmap_info *info = file->private_data;
	LOG_MSG(INFO, "device_release(%p,%p)\n", inode, file);

	free_page((unsigned long)info->data);
	vfree(info);
	file->private_data = NULL;

	num_device_open--;
#endif
	return 0;
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
	long          ret_i = 0;
	void         *to_user;
	long          nr;
	long         *ul;
	struct mdata *ucs;
	switch (ioctl_num) {
		case IOCTL_GET_CHANGED_SECTOR:
#if 0
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
#endif
		case IOCTL_TEST_MAP_IB:
		ucs = (struct mdata *)ioctl_param;
		dr_move_ready_mapped(ucs->md_num);
		to_user = dr_read_n_entries(ucs->md_num);
		copy_to_user(ucs->md_ent, to_user, sizeof(*ucs)*ucs->md_num);
		kfree(to_user);
		break;

		case IOCTL_TEST_UNMAP_IB:
		dr_move_mapped_free();
		break;

		case IOCTL_TEST_GET_NR_B:
		ul = (long *)ioctl_param;
		nr = dr_get_nr_mapped();
		copy_to_user(ul, &nr, sizeof(long));
		break;
	}
	return ret_i;
}

 
void mmap_open(struct vm_area_struct *vma)
{
#if 0
	struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
	info->reference++;
#endif
}
 
void mmap_close(struct vm_area_struct *vma)
{
#if 0
	struct mmap_info *info = (struct mmap_info *)vma->vm_private_data;
	info->reference--;
#endif
}
 
static int mmap_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	struct page *page;
	void *mpage;

	LOG_MSG(INFO, "Faulting address:%p\n", (void*)vmf->address);
	LOG_MSG(INFO, "Faulting Page offset:%ld\n", vmf->pgoff);
	mpage = dr_get_page(vmf->pgoff);
	if(mpage == NULL) {
		LOG_MSG(ERROR, "Offset is not found in the mapped range");
		return -EINVAL;
	}
	page = vmalloc_to_page(mpage);
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
