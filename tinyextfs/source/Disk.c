#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>
#include "Disk.h"
#include "FilesysConfig.h"

int fd; 

void DevInit(void)
{
	char* buf = malloc(FS_DISK_CAPACITY);
	memset(buf, 0x0, FS_DISK_CAPACITY);

	fd = open("MY_DISK", O_RDWR | O_CREAT | O_TRUNC, 0644);

	write(fd, buf, FS_DISK_CAPACITY);
	lseek(fd, 0, SEEK_SET);
	free(buf);
}

void DevLoad(void)
{
	fd = open("MY_DISK", O_RDWR);
}

void DevRelease(void)
{
	close(fd);
}
void DevMoveBlock(int blkno){
    lseek(fd, (off_t)+(BLOCK_SIZE*blkno),SEEK_SET);
}

void DevReadBlock(int blkno, char* pBuf)
{
   DevMoveBlock(blkno);
   read(fd, pBuf, BLOCK_SIZE);
}

void DevWriteBlock(int blkno, char* pBuf)
{
   DevMoveBlock(blkno);
   write(fd, pBuf, BLOCK_SIZE);
}
