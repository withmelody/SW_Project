#ifndef __DISKINFO__H_
#define __DISKINFO__H_

//#define FS_DISK_CAPACITY    (8388608) /* 8M */

#define NUM_OF_BLOCKS 16384 // 8MB / 512 Byte

#define FILE_SYSTEM_MAX_SIZE 8*1024*1024


#define FS_INODE_COUNT          (128)
#define BLOCK_SIZE              (512)
//#define NUM_OF_INODE_IN_1BLK    (BLOCK_SIZE / sizeof(tiny_inode))
//#define NUM_OF_DIRENT_IN_1BLK   (BLOCK_SIZE / sizeof(tiny_dentry))
//#define MAX_INDEX_OF_DIRBLK     (NUM_OF_DIRENT_IN_1BLK)
//#define NAME_LEN_MAX            (60)
//#define TINY_N_DIRECT_BLOCKS    (12)

#endif  // __DISKINFO__H_
