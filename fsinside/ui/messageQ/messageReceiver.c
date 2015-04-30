#include <stdio.h>
#include <string.h>

#include "msglib.h"
#include "../header/project.h"

void printSuperBlock(SuperBlk_t *);
void print_superblock(SuperBlk_t *);
void updateBlocks(BlockBitmap_t *);
void updateInodes(InodeBitmap_t *);
void updateBlocks_for_IO(FileIO_t *);

SuperBlk_t sb;
void LoadSuperBlk(int qid)
{
//	THREAD_LOCK;
	if(RecvSuperBlk(qid, &sb) < 0)
	{
		printf("super block recv fail\n");
		return ;
	}
	printSuperBlock(&sb);
//	THREAD_UNLOCK;
}

void LoadInodeBM(int qid)
{
	InodeBitmap_t ibm;
//	THREAD_LOCK;

	if(RecvInodeBM(qid, &ibm) < 0)
	{
		printf("inode bitmap recv fail\n");
		return ;
	}
	updateInodes(&ibm);
//	THREAD_UNLOCK;
}

void LoadBlockBM(int qid)
{
	BlockBitmap_t bbm;
//	THREAD_LOCK;

	if(RecvBlockBM(qid, &bbm) < 0)
	{
		printf("block bitmap recv fail\n");
		return ;
	}
	updateBlocks(&bbm);
//	THREAD_UNLOCK;
}

void LoadFileIO(int qid) {
	FileIO_t fio;
//	THREAD_LOCK;
	if(RecvFileIO(qid, &fio) < 0)
	{
		printf("FILE IO recv fail\n");
		return ;
	}
	int i;
	for(i=0; i<12; i++)
		fio.inode.i_block[i] -= sb.fsi.s_datablk_start;
	updateBlocks_for_IO(&fio);
//	THREAD_UNLOCK;
}

void* messageReceiver(void* nouse)
{
	int qid;
	Msg_t ctrl_msg;
	long mtype;

	qid = CreateMQ(5000);
	if(qid < 0)
	{
		printf("q open fail\n");
		return NULL;
	}

	while(!PROGRAM_EXIT_FLAG)
	{
		mtype = RecvCtrl(qid, &ctrl_msg);

		if(mtype > 0)
		{
			switch(ctrl_msg.type)
			{
				case MSG_SUPER_BLOCK:
					LoadSuperBlk(qid);
					break;
				case MSG_INODE_BITMAP:
					LoadInodeBM(qid);
					break;
				case MSG_BLOCK_BITMAP:
					LoadBlockBM(qid);
					break;
				case MSG_FILEIO:
					LoadFileIO(qid);
					break;
				default:
					printf("default\n");
			}
		}
	}
	RemoveMQ(qid);

	return NULL;
}
