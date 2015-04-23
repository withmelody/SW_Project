#include "Buf.h"

void Init(void)
{
/*
 * precondition		: usage ) Init();
 * postcondition	: TAILQ_HEAD 변수 모두 초기화
 * 					  가상디스크 초기화
 * 					  버퍼캐쉬노드 10개 생성
 */
	int i;
	Buf* pNew = NULL;

	for ( i = 0 ; i < HASH_TBL_SIZE ; i++ )
		TAILQ_INIT(&ppHashTable[i]);
	for ( i = 0 ; i < MAX_BUFLIST_NUM ; i++ )
		TAILQ_INIT(&ppBufListHead[i]);

	for ( i = 0 ; i < 10 ; i++ )
	{
		pNew = (Buf*)malloc(sizeof(Buf));
		memset(pNew, NULL, sizeof(Buf));
		InsertBufIntoFreelist(pNew);
	}

//	DevInit();
}
void InsertBufIntoFreelist(Buf* pBuf)
{
/*
 * precondition		: usage ) InsertBufIntoFreelist(pBuf);
 * 					  pObj != NULL
 * postcondition	: the Buf is inserted into the tail of the Buf list Free.
 * 					  If needed, the number of the Buf is set to BLKNO_INVALID.
 * 					  Buf List Free의 맨 뒷 부분에 Buf 추가 및 blkno에 BLKNO_INVALID 대입
 */
	if ( pBuf == NULL)
	{
		fprintf(stderr, "* InsertBufIntoFreelist() error\n");
		fprintf(stderr, "\tpObj is NULL\n");
		return;
	}
	pBuf->blkno = BLKNO_INVALID;
	pBuf->pMem = (void*)malloc(BLOCK_SIZE);
	pBuf->state = WRONG_VALUE;
	memset(pBuf->pMem, 0, BLOCK_SIZE);
	TAILQ_INSERT_TAIL(&ppBufListHead[BUF_LIST_FREE], pBuf, link);
}
Buf* BufGetNewBuffer(void)
{
/*
 * precondition 	: usage ) pNew = GetNewObjectFromContainter(void);
 * postcondition 	: If successful, this returns the pointer of the new Buf.
 *					  Otherwise, it returns NULL. This is because there is not any Buf in the container.
 *					  the returned Buf has the Block number of BLKNO_INVALID
 *					  free list가 고갈 되었을시 clean 또는 dirty 리스트를 사용한다.
 *					  (LRU(Least Recently Used) 구현)
 */
	Buf* pBuf = NULL;

	if ( (pBuf = TAILQ_FIRST(&ppBufListHead[BUF_LIST_FREE])) != NULL )
	{
		TAILQ_REMOVE(&ppBufListHead[BUF_LIST_FREE], ppBufListHead[BUF_LIST_FREE].tqh_first, link);
		return pBuf;
	}
	// BUF_LIST_FREE에 남은 버퍼가 없다?
	else
	{
		// priority Buffer Replacement
		// 1. Clean list
		// 2. Dirty list
		if( (pBuf = TAILQ_FIRST(&ppBufListHead[BUF_LIST_CLEAN])) != NULL )
		// Clean list에 버퍼가 있다!
		{
			if( !BufDelete(pBuf))
			{
				fprintf(stderr, "* BufRead()\n");
				fprintf(stderr, "\tBufDelete() Error!\n");
				return NULL;
			}
			return pBuf;
		}
		else if ( (pBuf = TAILQ_FIRST(&ppBufListHead[BUF_LIST_DIRTY])) != NULL )
		// Clean list는 비어있고 Dirty list에 버퍼가 있다!
		{
			DevWriteBlock(pBuf->blkno, pBuf->pMem);
			if( !BufDelete(pBuf))
			{
				fprintf(stderr, "* BufRead()\n");
				fprintf(stderr, "\tBufDelete() Error!\n");
				return NULL;
			}
			return pBuf;
		}
		// Clean list와 Dirty list 모두 비어 있는 경우는 없다.
		// 위 2개의 리스트에 아무것도 없으면 Free list에 반드시 있음
	}
	return BLKNO_INVALID;
}
void BufInsert(Buf* pBuf, BufList listNum)
{
/*
 * precondition		: usage ) BufInsert(pBuf, BUF_LIST_DIRTY);
 * 					  pBuf != NULL, 0 <= listNum < MAX_BUFLIST_NUM - 1
 * 					  listNum의 대입값은 {BUF_LIST_DIRTY, BUF_LIST_CLEAN}로 제한된다.
 * postcondition	: Insert an Buf with blkno into a list indicated by list number.
 */
	int index;

	if ( pBuf->blkno < 0 )		return;

	index = pBuf->blkno % HASH_TBL_SIZE;
	if ( pBuf == NULL )
	{
		fprintf(stderr, "* BufInsert() error\n");
		fprintf(stderr, "\tpBuf is NULL\n");
		return;
	}
	if ( !( 0 <= listNum && listNum < MAX_BUFLIST_NUM - 1 ))
	{
		fprintf(stderr, "* BufInsert() error\n");
		fprintf(stderr, "\tWrong list number\n");
		return;
	}
	switch(listNum)
	{
	case BUF_LIST_CLEAN:
		pBuf->state = BUF_STATE_CLEAN;
		break;
	case BUF_LIST_DIRTY:
		pBuf->state = BUF_STATE_DIRTY;
		break;
	}
	TAILQ_INSERT_TAIL(&ppHashTable[index], pBuf, hash);
	TAILQ_INSERT_TAIL(&ppBufListHead[listNum], pBuf, link);
}
Buf* BufFind(int blkno)
{
/*
 * precondition		: usage ) pNew = GetObject(9);
 * postcondition	: If successful, this returns the pointer of the found Buf.
 *					  Otherwise, it returns NULL.
 * 					  Not remove the Buf from the hash table or two lists.
 */
	Buf* pWalker = ppHashTable[blkno % HASH_TBL_SIZE].tqh_first;

	while(pWalker)
	{
		if ( pWalker->blkno == blkno )	return pWalker;
		pWalker = pWalker->hash.tqe_next;
	}
	return NULL;
}
BOOL BufDelete(Buf* pBuf)
{
/*
 * precondition		: usage ) BufDelete(pBuf);
 * postcondition	: If successful, this returns TRUE.
 *					  Otherwise, it returns FALSE.
 *					  The failure can be incurred by the absence of the object
 *					  in the hash table.
 */
	Buf* 	pWalker = NULL;
	BOOL 	ret_value = FALSE;
	int		list_type = 0;

	if ( pBuf == NULL )		return FALSE;

	switch(pBuf->state)
	{
	case BUF_STATE_CLEAN:
		list_type = BUF_LIST_CLEAN;
		break;
	case BUF_STATE_DIRTY:
		list_type = BUF_LIST_DIRTY;
		break;
	}

	TAILQ_FOREACH(pWalker, &ppHashTable[pBuf->blkno % HASH_TBL_SIZE], hash)
	{
		if ( pWalker == pBuf )	// FOUND IT!
		{
			// hash table link 단절 및 전후 노드 연결
			TAILQ_REMOVE(&ppHashTable[pBuf->blkno % HASH_TBL_SIZE], pWalker, hash);
			break;
		}
	}
	TAILQ_FOREACH(pWalker, &ppBufListHead[list_type], link)
	{
		if ( pWalker == pBuf )	// FOUND IT!
		{
			// link 단절 및 전후 노드 연결
			TAILQ_REMOVE(&ppBufListHead[list_type], pWalker, link);
			return TRUE;
		}
	}
	return FALSE;
}
void GetBufInfoByListNum(BufList listNum, Buf** ppObjInfo, int* pNumBuf)
{
/*
 * precondition		: usage ) GetBufInfoByListNum(BUF_LIST_DIRTY, ppObjInfo, &numObj);
 * postcondition	: ppBufListHead[listNum]에 연결되어 있는 오브젝트 하나하나를
 * 					  ppObjectInfo[]에 대입, pNumBuf는 변수는 초기화되며 
 *					  대입된 오브젝트의 개수를 저장한다.
 */
	Buf* pWalker = NULL;

	pWalker = ppBufListHead[listNum].tqh_first;
	*pNumBuf = 0;

	if ( pWalker == NULL )	return;

	TAILQ_FOREACH(pWalker, &ppBufListHead[listNum], link)
	{
		ppObjInfo[(*pNumBuf)++] = pWalker;
	}
}
void GetBufInfoByHashIndex(int index, Buf** ppObjInfo, int* pNumBuf)
{
/*
 * precondition		: usage ) GetBufInfoByHashIndex(BUF_LIST_DIRTY, ppObjInfo, &numObj);
 * postcondition	: ppHashTable[index]에 연결되어 있는 오브젝트 하나하나를
 * 					  ppObjectInfo[]에 대입, pNumBuf는 변수는 초기화되며 
 *					  대입된 오브젝트의 개수를 저장한다.
 */
	Buf* pWalker = NULL;

	pWalker = ppHashTable[index].tqh_first;
	*pNumBuf = 0;

	if ( pWalker == NULL )	return;

	TAILQ_FOREACH(pWalker, &ppHashTable[index], hash)
	{
		ppObjInfo[(*pNumBuf)++] = pWalker;
	}
}
void InitObjectLink(Buf* pBuf)
{
/*
 * precondition		: usage ) InitObjectLink(pBuf);
 * postcondition	: pBuf의 모든 링크를 초기화
 */
	if ( pBuf == NULL)
	{
		fprintf(stderr, "* InitObjectLink() error\n");
		fprintf(stderr, "  pBuf is NULL\n");
		return;
	}
	pBuf->blkno = BLKNO_INVALID;
	pBuf->hash.tqe_next = NULL;
	pBuf->hash.tqe_prev = NULL;
	pBuf->link.tqe_next = NULL;
	pBuf->link.tqe_prev = NULL;
}
// function appended from assignment2
Buf* BufRead(int blkno)
{
/*
 * precondition		: usage) pNew = BufRead(blkno);
 * postcondition	: Read a block with block no.
 * 					  from disk and returns the buffer including the block.
 * 					  If successful, this returns the pointer to a buffer including a block.
 * 					  Otherwise, NULL.
 * 					  If a block to be read is in the buffer cache, the buffer is inserted into
 * 					  the tail of the list including the buffer and returned.
 * 					  Otherwise, the block is read from disk into a buffer, which is returned.
 */
	Buf* pBuf;
	int list_type = 0;

	// 캐쉬에서 검색
	if( pBuf = BufFind(blkno) ) // found blkno
	{
		////////////////////////////////////////////////////////////
		// 가장 최근에 접근했던것을 LRU(Least Recently Used)에 따라
		// 리스트의 맨 끝(가장 최근에 접근한 것을 의미)으로 이동
		//
		switch(pBuf->state)
		{
		case BUF_STATE_CLEAN:
			list_type = BUF_LIST_CLEAN;
			break;
		case BUF_STATE_DIRTY:
			list_type = BUF_LIST_DIRTY;
			break;
		}
		if ( !BufDelete(pBuf) )				// 지우고
		{
			fprintf(stderr, "* BufRead\n");
			fprintf(stderr, "\tBufDelete() Error!\n");
			return NULL;
		}
		BufInsert(pBuf, list_type);			// 다시 삽입
		//
		////////////////////////////////////////////////////////////
		return pBuf;
	}
	// blkno가 현재 캐쉬에 없다? => 디스크에서 읽어온다.
	else
	{
		// 새로운 버퍼캐쉬 할당
		if ( pBuf = BufGetNewBuffer() )
		{
			pBuf->blkno = blkno;
			DevReadBlock(blkno, (char*)pBuf->pMem);
			BufInsert(pBuf, BUF_LIST_CLEAN);
			return pBuf;
		}
	}

	return NULL;

}
BOOL BufWrite(Buf* pBuf, void* pData, int size)
{
/*
 * precondition		: usage) BufWrite(buffer, src, size);
 * postcondition	: Write a block into a buffer in the buffer cache.
 * 					  if parameter is invalid, return NULL.
 */
	if ( BLOCK_SIZE < size )
	{
		fprintf(stderr, "* BufWrite() Error!\n");
		fprintf(stderr, "\tsize is too large\n");
		return FALSE;
	}
	if ( pData == NULL )
	{
		fprintf(stderr, "* BufWrite() Error!\n");
		fprintf(stderr, "\tpData is NULL\n");
		return FALSE;
	}
	if ( pBuf->pMem == NULL )
	{
		fprintf(stderr, "* BufWrite()\n");
		fprintf(stderr, "\tpBuf->pMem is NULL\n");
		return FALSE;
	}
	if ( !BufDelete(pBuf) )		// 여기서 BufDelete()
	{
		fprintf(stderr, "*BufWrite() Error!\n");
		fprintf(stderr, "\tBufDelete() Error!\n");
		return FALSE;
	}
	memcpy(pBuf->pMem, pData, size);
	BufInsert(pBuf, BUF_LIST_DIRTY);

	return TRUE;
}
void BufSync(void)
{
/*
 * precondition		: usage) BufSync();
 * postcondition	: Dirty list에 있는
 * 					  모든 버퍼의 데이터를 디스크에 저장하며
 * 					  Clean list로 이동한다.
 */
	Buf* pWalker = ppBufListHead[BUF_LIST_DIRTY].tqh_first;
	Buf* pCur = NULL;

	while ( pWalker )
	{
		pCur = pWalker;
		pWalker = pWalker->link.tqe_next;

		DevWriteBlock(pCur->blkno, pCur->pMem);	// 데이터 갱신

		if ( !BufDelete(pCur))					// dirty 에서 지우고
		{
			fprintf(stderr, "* BufSync()\n");
			fprintf(stderr, "\tBufDelete() Error!\n");
			return;
		}
		BufInsert(pCur, BUF_LIST_CLEAN);		// clean 삽입
	}
}
