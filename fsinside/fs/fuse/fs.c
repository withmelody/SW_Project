#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "tinyfs.h"
#include "fs.h"

extern FileSysInfo tiny_superblk;

int GetFreeEntry(char* Bitmap, int BitmapBlockSize)
{
/*
 * precondition		: usage ) GetFreeEntry(tiny_superblk.s_ibitmap_ptr, tiny_superblk.s_ibitmap_size);
 * 					  BitmapBlockSize는 Mount()에서 이미 구해진 블럭사이즈 변수이다.
 * postcondition	: 해당 비트의 위치를 리턴함
 * 					  실패시 -1 리턴
 */
	int blockno = 0;
	int bitno = 0;
	int i = 0;
	int BitmapSize = BitmapBlockSize * BLOCK_SIZE;

	for ( blockno = 0 ; blockno < BitmapSize ; blockno++ )
	{
		for ( bitno = 0 ; bitno < 8 ; bitno++ )
		{
			if ( *(Bitmap + blockno) & (0x1 << bitno) )
			{
				return (blockno * 8) + bitno;
			}
		}
	}
	return WRONG_VALUE;
}
int SetFreeToAlloc(char* Bitmap, int BitmapBlockSize)
{
/*
 * precondition		: usage ) SetFreeToAlloc(tiny_superblk.s_dbitmap_ptr, tiny_superblk.s_dbitmap_size, dest);
 * postcondition	: 해당 dest의 bit를 사용중(0)으로 바꾼다.
 * 					  성공시 0, 해당 bitmap이 모두 사용중이면 -1 리턴
 */
	int blockno = 0;
	int bitno = 0;
	int bitLocation = 0;
	if ( (bitLocation = GetFreeEntry(Bitmap, BitmapBlockSize * BLOCK_SIZE)) == WRONG_VALUE )
	{
		fprintf(stderr, "* GetFreeEntry() error!\n");
		return WRONG_VALUE;
	}

	bitno = bitLocation % 8/*bit size of char*/;
	blockno = bitLocation / 8/*bit size of char*/;
	*(Bitmap + blockno) ^= (0x1 << bitno);
	return 0;
}
int GetFreeInode()
{
/*
 * precondition		:
 * postcondition	: first-fit 조건임. free inode number를 반환한다.
 * 					  성공시 0, inode가 모두 사용중이면 -1 리턴
 */
	if ( tiny_superblk.s_ninode_free == 0 )	return WRONG_VALUE;
	return GetFreeEntry(tiny_superblk.s_ibitmap_ptr, tiny_superblk.s_ibitmap_size);
}
int GetFreeBlock()
{
/*
 * precondition		:
 * postcondition	: first-fit 조건임. free block number를 반환한다.
 * 					  성공시 0, block이 모두 사용중이면 -1 리턴
 */
	if ( tiny_superblk.s_nblk_free == 0 )	return WRONG_VALUE;
	return tiny_superblk.s_datablk_start + GetFreeEntry(tiny_superblk.s_dbitmap_ptr, tiny_superblk.s_dbitmap_size);
}
int SetInodeFreeToAlloc()
{
/*
 * precondition		:
 * postcondition	: first-fit 조건임. inodebitmap에서 제일 앞쪽 미사용중인 inode를
 * 					  사용중(bit:0)으로 바꾼다. 성공시 0, 모두 사용중이라면 -1 리턴
 */
	if ( tiny_superblk.s_ninode_free == 0 )	return WRONG_VALUE;
	IncUseInode();
	return SetFreeToAlloc(tiny_superblk.s_ibitmap_ptr, tiny_superblk.s_ibitmap_size);
}
int SetBlockFreeToAlloc()
{
/*
 * precondition		:
 * postcondition	: first-fit 조건임. blockbitmap에서 제일 앞쪽 미사용중인 block을
 * 					  사용중(bit:0)으로 바꾼다. 성공시 0, 모두 사용중이라면 -1 리턴
 */
	if ( tiny_superblk.s_nblk_free == 0 )	return WRONG_VALUE;
	IncUseBlock();
	return SetFreeToAlloc(tiny_superblk.s_dbitmap_ptr, tiny_superblk.s_dbitmap_size);
}
int SetInodeAllocToFree(int inodeno)
{
/*
 * precondition		:
 * postcondition	: 인자 inodeno는 inode number이다.
 * 					  해당 inode number를 inodebitmap에서 free 상태로 만든다.
 * 					  성공시 0, 해당 inode number가 이미 free 상태이면 -1 리턴
 */
	int block = inodeno / 8/*size of char*/;
	int bitno = inodeno % 8/*size of char*/;

	if ( tiny_superblk.s_ninode_use == 0 )	return WRONG_VALUE;
	if ( *(tiny_superblk.s_ibitmap_ptr + block) & 0x1 << bitno )
	{
		fprintf(stderr, "* SetInodeAllocToFree()\n");
		fprintf(stderr, "* Already Free\n");
		return WRONG_VALUE;
	}
	*(tiny_superblk.s_ibitmap_ptr + block) ^= 0x1 << bitno;
	DecUseInode();
	return 0;
}
int SetBlockAllocToFree(int blockno)
{
/*
 * precondition		:
 * postcondition	: 인자 blockno는 block number이다.
 * 					  해당 block number를 blockbitmap에서 free 상태로 만든다.
 * 					  성공시 0, 해당 block number가 이미 free 상태이면 -1 리턴
 */
	int block = (blockno - tiny_superblk.s_datablk_start) / 8/*size of char*/;
	int bitno = (blockno - tiny_superblk.s_datablk_start) % 8/*size of char*/;

	if ( tiny_superblk.s_nblk_use == 0 )	return WRONG_VALUE;
	if(blockno < tiny_superblk.s_datablk_start)
	{
		fprintf(stderr, "  invalid blockno!\n");
		fprintf(stderr, "* SetBlockAllocToFree()\n");
		return WRONG_VALUE;
	}
	if ( *(tiny_superblk.s_dbitmap_ptr + block) & 0x1 << bitno )
	{
		fprintf(stderr, "  Already Free\n");
		fprintf(stderr, "* SetBlockAllocToFree()\n");
		return WRONG_VALUE;
	}
	*(tiny_superblk.s_dbitmap_ptr + block) ^= 0x1 << bitno;
	DecUseBlock();
	return 0;
}
int IncUseInode()	// Inode 1개 사용
{
/*
 * precondition		:
 * postcondition	: 사용 inode를 1 증가시키고
 * 					  미사용 inode를 1 감소시킨다
 * 					  성공시 0, 이미 미사용 inode가 0이면 -1 리턴
 */
	if ( 0 < tiny_superblk.s_ninode_free )
	{
		tiny_superblk.s_ninode_use++;
		tiny_superblk.s_ninode_free--;
		return 0;
	}
	return WRONG_VALUE;
}
int DecUseInode()	// Inode 1개 해제
{
/*
 * precondition		:
 * postcondition	: 사용 inode를 1 감소시키고
 * 					  미사용 inode를 1 증가시킨다
 * 					  성공시 0, 이미 사용 inode가 0이면 -1 리턴
 */
	if ( 0 < tiny_superblk.s_ninode_use )
	{
		tiny_superblk.s_ninode_use--;
		tiny_superblk.s_ninode_free++;
		return 0;
	}
	return WRONG_VALUE;
}
int IncUseBlock()	// Block 1개 사용
{
/*
 * precondition		:
 * postcondition	: 사용 block를 1 증가시키고
 * 					  미사용 block를 1 감시킨다
 * 					  성공시 0, 이미 사용 block이 0이면 -1 리턴
 */
	if ( 0 < tiny_superblk.s_nblk_free)
	{
		tiny_superblk.s_nblk_use++;
		tiny_superblk.s_nblk_free--;
		return 0;
	}
	return WRONG_VALUE;
}
int DecUseBlock()	// Block 1개 해제
{
/*
 * precondition		:
 * postcondition	: 사용 block를 1 감소시키고
 * 					  미사용 block를 1 증가시킨다
 * 					  성공시 0, 이미 사용 block 0이면 -1 리턴
 */
	if ( 0 < tiny_superblk.s_nblk_use )
	{
		tiny_superblk.s_nblk_use--;
		tiny_superblk.s_nblk_free++;
		return 0;
	}
	return WRONG_VALUE;
}
void ReadInode(tiny_inode* inodeInfo, int inodeNo)
{
/*
 * precondition		:
 * postcondition	: 인자 inodeNo는 inode number이다.
 * 					  해당 inode number를 가진 inodeInfo를 디스크에서 읽어온다.
 */
	Buf* pBuf = NULL;
	tiny_inode* pMem = NULL;
	int block = tiny_superblk.s_inodeblk_start + inodeNo / NUM_OF_INODE_IN_1BLK;	// inodeNo가 위치한 블럭
	int inode = inodeNo % NUM_OF_INODE_IN_1BLK;	// 해당 block에서 inode가 위치한 순서

	pBuf = BufRead(block);
	pMem = pBuf->pMem;
	pMem = pMem + inode;
	memcpy(inodeInfo, pMem/*(tiny_inode*)pBuf->pMem + inode*/, sizeof(tiny_inode));
}
void WriteInode(tiny_inode* inodeInfo, int inodeNo)
{
/*
 * precondition		:
 * postcondition	: 인자 inodeNo는 inodeInfo의 inode number이다.
 * 					  inodeInfo를 디스크에 저장한다.
 */
	Buf* pBuf = NULL;
	void* pMem = malloc(BLOCK_SIZE);
	tiny_inode* pCur = pMem;
	int block = tiny_superblk.s_inodeblk_start + inodeNo / NUM_OF_INODE_IN_1BLK;
	int inode = inodeNo % NUM_OF_INODE_IN_1BLK;

	pBuf = BufRead(block);
	memcpy(pMem, pBuf->pMem, BLOCK_SIZE);
	pCur = pCur + inode;
	memcpy(pCur, inodeInfo, sizeof(tiny_inode));
	BufWrite(pBuf, pMem, BLOCK_SIZE);
	free(pMem);
}
void ReadDirBlock(tiny_dirblk* dirBlock, int blockNo)
{
/*
 * precondition		:
 * postcondition	: 인자 blockNo는 block number이다.
 * 					  해당 block number의 block을 디스크에서 읽어 온다.
 */
	Buf* pBuf = NULL;

	pBuf = BufRead(blockNo);
	memcpy(dirBlock, (tiny_dirblk*)pBuf->pMem, sizeof(tiny_dirblk));
}
void WriteDirBlock(tiny_dirblk* dirBlock, int blockNo)
{
/*
 * precondition		:
 * postcondition	: 인자 blockNo는 dirBlock의 block number이다.
 * 					  dirBlock을 디스크에 저장한다.
 */
	Buf* pBuf = NULL;

	pBuf = BufRead(blockNo);
	BufWrite(pBuf, dirBlock, sizeof(tiny_dirblk));
}
void GetEntryPath(char* abspath, char* filename)
{
/*
 * precondition		: GetEntryName()을 통해 filename을 미리 추출해놀것
 * 					  abspath != NULL, filename != NULL
 * postcondition	: 절대경로 abspath를 해당 파일 또는 디렉토리의 상위디렉토리 경로까지만 나타냄
 */
	char* ptr = strstr(abspath, filename);
	memset(ptr-1, 0, strlen(ptr) + 1);
}
int GetEntryName(char* dest, char* abspath)
{
/*
 * precondition		: dest != NULL, abspath != NULL
 * postcondition	: 절대경로 abspath에서 최하위 엔트리네임만 dest에 복사
 * 					  성공시 0, 최하위 엔트리 네임이 NAME_LEN_MAX를 넘는다면 -1 리턴
*/
	//
	char* ptr = malloc(strlen(abspath)+1);
	char* del = ptr;
	memcpy(ptr, abspath, strlen(abspath)+1);
	ptr = strtok(ptr, "/");
	while(ptr)
	{
		if(!(strlen(ptr) < NAME_LEN_MAX))
		{
			return WRONG_VALUE;
		}
		strncpy(dest, ptr, NAME_LEN_MAX);
		ptr = strtok(NULL, "/");
	}
	free(del);
	return 0;
}
int GetFreeDir(tiny_dirblk* dirBlock)
{
/*
 * precondition		: dirBlock != NULL
 * postcondition	: SetFreeDir()에서 밖엔 쓰이지 않는다.
 * 					  제일 앞쪽부터 찾아 비어있는 dirent index를 리턴함
 * 					  성공시 미사용 중인 entry index, 모두 사용중이면 -1 리턴
*/
	int i = 0;

	for ( i = 0 ; i < NUM_OF_DIRENT_IN_1BLK ; i++ )
	{
		if ( strcmp( dirBlock->dirEntries[i].name, "") == 0 )
		{
			return i;
		}
	}
	return WRONG_VALUE;
}
int MakeDirentry(tiny_inode* inodeInfo, char* dirname)
{
/*
 * precondition		: inodeInfo != NULL, dirname != NULL
 * postcondition	: 해당 inodeInfo의 indirect block에 directory entry가 생성됨
 * 					  제일 앞쪽부터 찾아 비어있는 directory entry index에 name과 inodeno를 할당
 * 					  free directory entry가 없으면 -1 리턴
*/
	int block = 0;
	int index = 0;
	int	parent_inodeno = 0;
	int	current_inodeno = 0;
	tiny_inode newtiny_inode;
	tiny_dirblk dirBlock;
	ReadDirBlock(&dirBlock, inodeInfo->i_block[0]);
	parent_inodeno = dirBlock.dirEntries[0].inodeNum;
	for ( block = 0 ; block < inodeInfo->i_nblk ; block++ )
	{
		ReadDirBlock(&dirBlock, inodeInfo->i_block[block]);
		if (( index = GetFreeDir(&dirBlock)) != WRONG_VALUE)
		{// 비어있는곳을 찾으면
			strcpy(dirBlock.dirEntries[index].name, dirname);
			dirBlock.dirEntries[index].inodeNum = GetFreeInode();
			if ( SetInodeFreeToAlloc() == WRONG_VALUE )		return WRONG_VALUE;
			dirBlock.dirEntries[index].type = FILE_TYPE_DIR;
			current_inodeno = dirBlock.dirEntries[index].inodeNum;
			WriteDirBlock(&dirBlock, inodeInfo->i_block[block]);
			newtiny_inode.i_nblk = 1;
			newtiny_inode.i_size = BLOCK_SIZE;
			newtiny_inode.i_type = FILE_TYPE_DIR;
			newtiny_inode.i_block[0] = GetFreeBlock();
			SetBlockFreeToAlloc();
		////////////////////////////////////////////
		//	생성한 directory block 초기화
			ReadDirBlock(&dirBlock, newtiny_inode.i_block[0]);
			strcpy(dirBlock.dirEntries[0].name, ".");
			dirBlock.dirEntries[0].inodeNum = current_inodeno;
			dirBlock.dirEntries[0].type = FILE_TYPE_DIR;
			strcpy(dirBlock.dirEntries[1].name, "..");
			dirBlock.dirEntries[1].inodeNum = parent_inodeno;
			dirBlock.dirEntries[1].type = FILE_TYPE_DIR;
			WriteDirBlock(&dirBlock, newtiny_inode.i_block[0]);
		//
		////////////////////////////////////////////
			WriteInode(&newtiny_inode, current_inodeno);
			return 0;
		}
	}
	if( inodeInfo->i_nblk < TINY_N_DIRECT_BLOCKS)
	{// indirect block을 더 생성할 수 있을때
		inodeInfo->i_nblk++;
		inodeInfo->i_block[inodeInfo->i_nblk - 1] = GetFreeBlock();
		SetBlockFreeToAlloc();
		WriteInode(inodeInfo, parent_inodeno);
		ReadDirBlock(&dirBlock, inodeInfo->i_block[inodeInfo->i_nblk - 1]);
		strcpy(dirBlock.dirEntries[0].name, dirname);
		dirBlock.dirEntries[0].inodeNum = GetFreeInode();
		if ( SetInodeFreeToAlloc() == WRONG_VALUE )		return WRONG_VALUE;
		dirBlock.dirEntries[0].type = FILE_TYPE_DIR;
		current_inodeno = dirBlock.dirEntries[0].inodeNum;
		WriteDirBlock(&dirBlock, inodeInfo->i_block[block]);
		newtiny_inode.i_nblk = 1;
		newtiny_inode.i_size += BLOCK_SIZE;
		newtiny_inode.i_type = FILE_TYPE_DIR;
		newtiny_inode.i_block[0] = GetFreeBlock();
		SetBlockFreeToAlloc();
	////////////////////////////////////////////
	//	생성한 directory block 초기화
		ReadDirBlock(&dirBlock, newtiny_inode.i_block[0]);
		memset(&dirBlock, 0, sizeof(tiny_dirblk));
		strcpy(dirBlock.dirEntries[0].name, ".");
		dirBlock.dirEntries[0].inodeNum = current_inodeno;
		dirBlock.dirEntries[0].type = FILE_TYPE_DIR;
		strcpy(dirBlock.dirEntries[1].name, "..");
		dirBlock.dirEntries[1].inodeNum = parent_inodeno;
		dirBlock.dirEntries[1].type = FILE_TYPE_DIR;
		WriteDirBlock(&dirBlock, newtiny_inode.i_block[0]);
	//
	////////////////////////////////////////////
		WriteInode(&newtiny_inode, current_inodeno);
		return 0;
	}
	// 모든 indirect block을 사용중이며 또한 모든 directory block에 빈 공간이 없다.
	return -EDQUOT;
}
int RemoveDirentry(tiny_inode* inodeInfo, char* dirname)
{
/*
 * precondition		: inodeInfo != NULL, dirname != NULL
 * postcondition	: dirname은 삭제를 원하는 디렉토리명이며,
 * 					  inodeInfo는 dirname을 가지고 있을 것으로 추정되는 inode이다.
 * 					  성공시 0, 실패시 -1 리턴
*/
	int block = 0, index = 0;
	int	parent_inodeno = 0;
	int	current_inodeno = 0;
	int ret_value = 0;
	int i = 0;
	tiny_inode delinode;
	tiny_inode nullinode;
	tiny_dirblk dirBlock;
	tiny_dirblk nullblock;
	memset(&nullinode, 0, sizeof(tiny_inode));
	memset(&nullblock, 0, sizeof(tiny_dirblk));

	ReadDirBlock(&dirBlock, inodeInfo->i_block[0]);
	current_inodeno = dirBlock.dirEntries[0].inodeNum;
	parent_inodeno = dirBlock.dirEntries[1].inodeNum;
	for ( block = 0 ; block < inodeInfo->i_nblk ; block++ )
	{
		ReadDirBlock(&dirBlock, inodeInfo->i_block[block]);
		for ( index = 0 ; index < MAX_INDEX_OF_DIRBLK ; index++ )
		{
			if(strcmp(dirBlock.dirEntries[index].name, dirname) == 0
					&& dirBlock.dirEntries[index].type == FILE_TYPE_DIR)
			{// 지울 디렉토리명을 찾으면
				ReadInode(&delinode, dirBlock.dirEntries[index].inodeNum);
				if ( (ret_value = DirIsEmpty(&delinode)) == WRONG_VALUE )
				{
					fprintf(stderr, "dirname is not a DIRECTORY!\n");
					fprintf(stderr, "DirIsEmpty() Error!\n");
				}
				if ( ret_value == TRUE )
				{// 디렉토리가 비어있다. ==> 삭제
					for ( i = 0 ; i < delinode.i_nblk ; i++ )
					{
						WriteDirBlock(&nullblock, delinode.i_block[i]);
						SetBlockAllocToFree(delinode.i_block[i]);
					}
					WriteInode(&nullinode, dirBlock.dirEntries[index].inodeNum);
					SetInodeAllocToFree(dirBlock.dirEntries[index].inodeNum);
					dirBlock.dirEntries[index].inodeNum = 0;
					strcpy(dirBlock.dirEntries[index].name, "");
					// tiny_dirblk과 tiny_inode 갱신하기
					WriteDirBlock(&dirBlock, inodeInfo->i_block[block]);
					return 0;
				}
				else if ( ret_value == FALSE ) {
					// 디렉토리 비어있지 않음
					return -ENOTEMPTY;
				}
			}
		}
	}
	return -ENOENT;
}
int DirIsEmpty(tiny_inode* inodeInfo)
{
/*
 * precondition		: inodeInfo != NULL
 * postcondition	: inodeInfo가 비어있는 디렉토리이면 1 리턴
 * 					  비어있지 않으면 0 리턴, 디렉토리가 아니면 -1 리턴
*/
	int block = 0, index = 0;
	tiny_dirblk dirBlock;

	if(inodeInfo->i_type != FILE_TYPE_DIR)		return WRONG_VALUE;

	ReadDirBlock(&dirBlock, inodeInfo->i_block[0]);
	for( index = 0 ; index < MAX_INDEX_OF_DIRBLK ; index++ )
	{	// .과 ..이 있는 블럭일때
		if( strcmp(dirBlock.dirEntries[index].name, ".") == 0
			|| strcmp(dirBlock.dirEntries[index].name, "..") == 0)
			continue;
		if( strcmp(dirBlock.dirEntries[index].name, "") != 0)
			return FALSE;
	}
	for ( block = 1 ; block < inodeInfo->i_nblk ; block++)
	{
		ReadDirBlock(&dirBlock, inodeInfo->i_block[block]);
		for( index = 0 ; index < MAX_INDEX_OF_DIRBLK ; index++ )
		{
			if( strcmp(dirBlock.dirEntries[index].name, "") != 0)
				return FALSE;
		}
	}
	return TRUE;
}
