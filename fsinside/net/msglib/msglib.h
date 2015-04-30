#ifndef __MSG_LIB_H__
#define __MSG_LIB_H__

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "../tinyfs.h"

#define INODE_BITMAP_SIZE 100		// byte
#define BLOCK_BITMAP_SIZE 2048		// byte

typedef enum {
	MSG_SUPER_BLOCK = 0,
	MSG_INODE_BITMAP,
	MSG_BLOCK_BITMAP,
	MSG_FILEIO,
} MSG_TYPE;

typedef struct {
	long to_mtype;
	MSG_TYPE type;
} Msg_t;

typedef struct {
	long to_mtype;
	FileSysInfo fsi;
} SuperBlk_t;

typedef struct {
	long to_mtype;	
	char s_ibitmap_ptr[INODE_BITMAP_SIZE];
	int size;
} InodeBitmap_t;

typedef struct {
	long to_mtype;
	char s_dbitmap_ptr[BLOCK_BITMAP_SIZE]; 
	int size;
} BlockBitmap_t;
///
typedef struct {
	long to_mtype;
	tiny_inode inode;
	tiny_dentry dentry;
	char flag;
	unsigned int size;
} FileIO_t;

///
int CreateMQ(key_t);
int OpenMQ(key_t);
int RemoveMQ(int);

long SendMQ(int, MSG_TYPE, void*);

long RecvCtrl(int, Msg_t*);
long RecvSuperBlk(int, SuperBlk_t*);
long RecvInodeBM(int, InodeBitmap_t*);
long RecvBlockBM(int, BlockBitmap_t*);
long RecvFileIO(int, FileIO_t *);

#endif


