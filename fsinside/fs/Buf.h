#ifndef __BUF_H__
#define __BUF_H__

#include <stdio.h>
#include <sys/queue.h>
#include <math.h>		// ceil()
#include "Disk.h"

#define HASH_TBL_SIZE	(8)
#define MAX_BUFLIST_NUM	(3)
#define BLKNO_INVALID	(-1)
#define FALSE			(0)
#define TRUE			(!FALSE)
#define WRONG_VALUE		(-1)

typedef int BOOL;
typedef struct Buf Buf;

typedef enum __BufState
{
	BUF_STATE_CLEAN,
	BUF_STATE_DIRTY
} BufState;
typedef enum __BufList{
    BUF_LIST_DIRTY,
    BUF_LIST_CLEAN,
    BUF_LIST_FREE
} BufList;

struct Buf
{
    int					blkno;
    BufState			state;
    void*				pMem;
    TAILQ_ENTRY(Buf)	hash;
    TAILQ_ENTRY(Buf)	link;
};

TAILQ_HEAD(hashTable, Buf) ppHashTable[HASH_TBL_SIZE];
TAILQ_HEAD(bufList, Buf) ppBufListHead[MAX_BUFLIST_NUM];

// assignment1
void	BufInsert(Buf* pBuf, BufList listNum);
Buf*	BufFind(int blkno);
BOOL	BufDelete(Buf* pBuf);
Buf*	BufGetNewBuffer(void);
void	InsertBufIntoFreelist(Buf* pObj);
void	GetBufInfoByListNum(BufList listNum, Buf** ppObjInfo, int* pNumBuf);
void	GetBufInfoByHashIndex(int index, Buf** ppObjInfo, int* pNumBuf);
void	Init(void);
// custom global function
void	InitObjectLink(Buf* pBuf);

// function appended from assignment2
Buf*	BufRead(int blkno);
BOOL	BufWrite(Buf* pBuf, void* pData, int size);
void	BufSync(void);

#endif		// __BUF_H__
