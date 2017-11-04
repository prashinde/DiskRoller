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
#include <sys/mman.h>

#define PAGE_SIZE 4096
unsigned long ioctl_get_changed_sector(int file_desc, unsigned long *sectors_to_read)
{
	int i = 0;
	int ret_val = ioctl(file_desc, IOCTL_GET_CHANGED_SECTOR, sectors_to_read);
	if (ret_val < 0) {
		printf("IOCTL failed:%d: %s", errno, strerror(errno));
		exit(-1);
	}
	printf("Return value = %d", ret_val);
	printf("\nChanged sectors are:");
	for (i=0; i < ret_val; i++)
		printf(" %ld ", sectors_to_read[i]);
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
int main()
{
	unsigned long sectors_to_read[255];
	int num_bytes = 0;
	void *address;
	int file_desc = open(DEVICE_FILE_NAME, O_RDWR);
	if (file_desc < 0) {
		printf("Cant open device:%s", DEVICE_FILE_NAME);
		exit(-1);
	}

	num_bytes = ioctl_get_changed_sector(file_desc, sectors_to_read);

	read_from_sectors(sectors_to_read, num_bytes);

	address = mmap(NULL, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, file_desc, 0);
	if (address == MAP_FAILED) {
		perror("mmap operation failed");
		return -1;
	}
	printf("\nAccessing a kernel memory:%s", (char *)address);
	printf("\n"); 
	close(file_desc);
	return 0;
}
