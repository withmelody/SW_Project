#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include "msglib.h"

#define RECV_MQ(name, type)											\
	long Recv##name(int qid, type *t) {					\
		int st = msgrcv(qid, t, sizeof(type), 1L, IPC_NOWAIT);	\
		if(st < 0) return -1L;										\
		return st; }												

#define SEND_MQ(name, type)											\
long Send##name(int qid, type t) {					\
		int st;	t.to_mtype = 1L;									\
		st = msgsnd(qid, &t, sizeof(type), IPC_NOWAIT);				\
		if(st < 0) return -1L;										\
		return st; }

int CreateMQ(key_t key)
{
	return (msgget(key, IPC_CREAT | 0777));
}

int OpenMQ(key_t key)
{
	return (msgget(key, 0));
}

int RemoveMQ(int qid)
{
	return (msgctl(qid, IPC_RMID, 0));
}

RECV_MQ(Ctrl, Msg_t)
RECV_MQ(SuperBlk, SuperBlk_t)
RECV_MQ(InodeBM, InodeBitmap_t)
RECV_MQ(BlockBM, BlockBitmap_t)

SEND_MQ(Ctrl, Msg_t)
SEND_MQ(SuperBlk, SuperBlk_t)
SEND_MQ(InodeBM, InodeBitmap_t)
SEND_MQ(BlockBM, BlockBitmap_t)

long SendMQ(int qid, MSG_TYPE type, void *data)
{
	Msg_t ctrl_msg;

	ctrl_msg.type = type;
	if(SendCtrl(qid, ctrl_msg) < 0)
	{
		printf("ctrl q send fail\n");
		return -1L;
	}

	switch(type)
	{
		case MSG_SUPER_BLOCK:
			if(SendSuperBlk(qid, *(SuperBlk_t*)data) < 0) {
				printf("q send fail\n");
				return -1L;
			}
			break;
		case MSG_INODE_BITMAP:
			if(SendInodeBM(qid, *(InodeBitmap_t*)data) < 0) {
				printf("q send fail\n");
				return -1L;
			}
			break;
		case MSG_BLOCK_BITMAP:
			printf("size = %lu\n", (*(BlockBitmap_t*)data).size);
			if(SendBlockBM(qid, *(BlockBitmap_t*)data) < 0) {
				printf("bbm send fail %d\n", errno);
				return -1L;
			}
			break;
		default:
			return 1L;
	}
}

