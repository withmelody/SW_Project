#include <stdio.h>
#include <string.h>

#include "msglib.h"
			
void LoadSuperBlk(int qid)
{
	printf("super blk\n");
	SuperBlk_t sb;

	if(RecvSuperBlk(qid, &sb) < 0)
	{
		printf("q recv fail\n");
		return ;
	}
	printf("sb.fsi.blocks = %d\n", sb.fsi.blocks);
}

void LoadInodeBM(int qid)
{
	printf("inode bitmap\n");
	InodeBitmap_t ibm;
	int i=0;

	if(RecvInodeBM(qid, &ibm) < 0)
	{
		printf("q recv fail\n");
		return ;
	}
	for(i=0; i<ibm.size; i++)
		printf("%x", ibm.InodeBitmap[i]);
	printf("\n");
}

void LoadBlockBM(int qid)
{
	printf("block bitmap\n");
	BlockBitmap_t bbm;
	int i=0;

	if(RecvBlockBM(qid, &bbm) < 0)
	{
		printf("q recv fail\n");
		return ;
	}
	for(i=0; i<bbm.size; i++)
		printf("%x", bbm.BlockBitmap[i]);
	printf("\n");
}

int main()
{
	int qid;
	Msg_t ctrl_msg;
	long mtype;

	qid = CreateMQ(5000);
	if(qid < 0)
	{
		printf("q open fail\n");
		return -1;
	}
	
	while(1)
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
				default:
					printf("default\n");
			}
		}
	}
	RemoveMQ(qid);
	
	return 0;
}
