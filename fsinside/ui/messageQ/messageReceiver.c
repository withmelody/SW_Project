#include <stdio.h>
#include <string.h>

#include "msglib.h"
#include "../header/project.h"

void print_superblock(SuperBlk_t *);
void updateBlocks(BlockBitmap_t *);
void updateInodes(InodeBitmap_t *);
void updateBlocks_for_IO(FileIO_t *);

void LoadSuperBlk(int qid)
{
	SuperBlk_t sb;

	if(RecvSuperBlk(qid, &sb) < 0)
	{
		printf("super block recv fail\n");
		return ;
	}
	update_DisplayBar(&sb);
}

void LoadInodeBM(int qid)
{
	InodeBitmap_t ibm;

	if(RecvInodeBM(qid, &ibm) < 0)
	{
		printf("inode bitmap recv fail\n");
		return ;
	}
	updateInodes(&ibm);
}

void LoadBlockBM(int qid)
{
	BlockBitmap_t bbm;

	if(RecvBlockBM(qid, &bbm) < 0)
	{
		printf("block bitmap recv fail\n");
		return ;
	}
	updateBlocks(&bbm);
}

void LoadFileIO(int qid) {
	FileIO_t fio;
	if(RecvFileIO(qid, &fio) < 0)
	{
		printf("FILE IO recv fail\n");
		return ;
	}
	updateBlocks_for_IO(&fio);
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
