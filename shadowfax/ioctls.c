/* 
 * device specifics, such as ioctl numbers and the
 * major device file. 
 */
#include "ioctlhandler.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>		/* open */
#include <unistd.h>
#include <sys/ioctl.h>		/* ioctl */
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>

#define PAGE_SIZE 4096
unsigned long ioctl_test_ib(int file_desc, int n)
{
	/* Mapping Interceptor buffer */
	int ret_val;
	struct entry *mem;
	struct mdata md;
	long nr;
	int i;

	mem = (struct entry *)malloc(n*sizeof(struct entry));
	if(mem == NULL) {
		printf("OVERLOAD!!! Abort");
		return 0;
	}

	ret_val = ioctl(file_desc, IOCTL_TEST_GET_NR_B, &nr);
	if (ret_val < 0) {
		printf("IOCTL failed:%d: %s", errno, strerror(errno));
		free(mem);
		return -1;
	}

	printf("Number of mapped pages:%ld\n", nr);
	md.md_num = nr;
	md.md_ent = mem;

	printf("MApping the IB\n");

	ret_val = ioctl(file_desc, IOCTL_TEST_MAP_IB, &md);
	if (ret_val < 0) {
		printf("IOCTL failed:%d: %s", errno, strerror(errno));
		free(mem);
		return -1;
	}

	for(i = 0; i < nr; i++) {
		printf("Page metadata:");
		printf("Page id:%llu\n", md.md_ent[i].e_pg_id);
	}
	/* Unmapping interceptor buffer */
	ret_val = ioctl(file_desc, IOCTL_TEST_UNMAP_IB, n);
	if (ret_val < 0) {
		printf("IOCTL failed:%d: %s", errno, strerror(errno));
		free(mem);
		return -1;
	}
	free(mem);
	
	ret_val = ioctl(file_desc, IOCTL_TEST_GET_NR_B, &nr);
	if (ret_val < 0) {
		printf("IOCTL failed:%d: %s", errno, strerror(errno));
		free(mem);
		return -1;
	}

	printf("Number of mapped pages:%ld\n", nr);

	return 0;
}
#if 0
unsigned long ioctl_get_changed_sector(int file_desc, mdata_t **str)
{
	int i = 0;
	int dev_fd;
	int ret;
	void *address;
	mdata_t *m;
	int ret_val = ioctl(file_desc, IOCTL_GET_CHANGED_SECTOR, str);
	if (ret_val < 0) {
		printf("IOCTL failed:%d: %s", errno, strerror(errno));
		exit(-1);
	}
	dev_fd = open("/dev/loop1", O_RDWR);
	printf("Return value = %d", ret_val);
	printf("\nChanged sectors are:");
	for (i = 0; i < ret_val; i++) {
		m = str[i];
		ret = lseek(dev_fd, (m->s)*512, SEEK_SET);
		printf("Writing on sector:%ld\n", m->s);
		address = mmap(NULL, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, file_desc, 4096*i);
		if (address == MAP_FAILED) {
			perror("mmap operation failed");
			return -1;
		}
		write(dev_fd, (address+m->offset), m->len);
	}
	close(dev_fd);
	return ret_val;
}

void read_from_sectors(unsigned long *sectors_to_read, int num_bytes)
{
	int i = 0;
	int dev_fd = open("/dev/loop0", O_RDONLY);
	int ret = 0;

	char buf[512];

	memset(buf, 0, 512);
	if (dev_fd == -1) {
		printf("unable to open device");
		return;
	}

	for (i=0; i < num_bytes; i++) {
		int j = 0;
		for (j=0; j < sectors_to_read[i]; j++)
			ret = lseek(dev_fd, 512, SEEK_CUR);

		ret = read(dev_fd, buf, 512);
		printf("\nBuffer changed at sector %ld is %s\n", sectors_to_read[i], buf);
		lseek(dev_fd, 0, SEEK_SET);
	}

	close(dev_fd);
}
#endif
int main()
{
	int num_bytes = 0;
	int i;
	void *address;
	int file_desc = open(DEVICE_FILE_NAME, O_RDWR);
	if (file_desc < 0) {
		printf("Cant open device:%s:%d\n", DEVICE_FILE_NAME, errno);
		exit(-1);
	}

	//num_bytes = ioctl_get_changed_sector(file_desc, str);

	num_bytes = ioctl_test_ib(file_desc, 10);
	//read_from_sectors(sectors_to_read, num_bytes);

	printf("\n"); 
	close(file_desc);
	return 0;
}
