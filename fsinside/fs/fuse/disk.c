#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>
#include "tinyfs.h"
#include "disk.h"

extern FileSysInfo tiny_superblk;

void DevInit(void)
{
	char* buf = malloc(FS_DISK_CAPACITY);
	memset(buf, 0x0, FS_DISK_CAPACITY);

	tiny_superblk.s_disk_fd = open("MY_DISK", O_RDWR | O_CREAT | O_TRUNC, 0644);

	write(tiny_superblk.s_disk_fd, buf, FS_DISK_CAPACITY);
	lseek(tiny_superblk.s_disk_fd, 0, SEEK_SET);
	free(buf);
}

void DevLoad(void)
{
	tiny_superblk.s_disk_fd = open("MY_DISK", O_RDWR);
}

void DevRelease(void)
{
	close(tiny_superblk.s_disk_fd);
}

void DevMoveBlock(int blkno){
    lseek(tiny_superblk.s_disk_fd, (off_t)+(BLOCK_SIZE*blkno),SEEK_SET);
}

void DevReadBlock(int blkno, char* pBuf)
{
   DevMoveBlock(blkno);
   read(tiny_superblk.s_disk_fd, pBuf, BLOCK_SIZE);
}

void DevWriteBlock(int blkno, char* pBuf)
{
   DevMoveBlock(blkno);
   write(tiny_superblk.s_disk_fd, pBuf, BLOCK_SIZE);
}
