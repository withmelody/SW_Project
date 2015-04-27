#ifndef __DISK_H__
#define __DISK_H__


#define BLOCK_SIZE 512
//#define BLOCK_SIZE 1024

extern void DevInit(void);

extern void DevLoad(void);

extern void DevMoveBlock(int blkno);
    
extern void DevReadBlock(int blkno, char* pBuf);

extern void DevWriteBlock(int blkno, char* pBuf);

#endif /* __DISK_H__ */
