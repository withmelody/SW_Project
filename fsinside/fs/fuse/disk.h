#ifndef __DISK_H__
#define __DISK_H__

#include "tinyfs.h"

void DevInit(void);

void DevLoad(void);

void DevRelease(void);

void DevMoveBlock(int blkno);
    
void DevReadBlock(int blkno, char* pBuf);

void DevWriteBlock(int blkno, char* pBuf);

#endif /* __DISK_H__ */
