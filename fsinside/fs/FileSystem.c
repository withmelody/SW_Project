#include "FileSystem.h"
#include "string.h"		// strtok(), strstr()

FileSysInfo		fileSysInfo;
FileDescTable	fileDescTable;
int		inodeBitmapSize	= 0;
int		inodeListSize 	= 0;
double	blockBitmapSize	= 0;
double	dataRegionSize 	= 0;

int	OpenFile(const char* pFileName, OpenFlag flag)
{
/*
 * precondition		: usage) OpenFile(absfilepath, flag);
 * 					  pFileName은 오픈할 파일의 이름. 단, 파일 이름은 절대경로이다.
 * 					  flag는 OPEN_FLAG_READWRITE, OPEN_FLAG_CREATE가 있다.
 * postcondition	: 파일을 open한다.
 *					  성공하면, file descriptor를 리턴한다.
 * 					  이 file descriptor는 file descriptor table의 entry의 index값으로 정의된다.
 * 					  실패했을때는 -1을 리턴한다.
 */
	InodeInfo	inodeInfo, newInode;
	DirBlock	dirBlock;
	char* 	abspath = malloc(strlen(pFileName) + 1);
	char* 	ptr = malloc(strlen(pFileName) + 1);
	char	filename[NAME_LEN_MAX];
	int 	i = 0, j = 0;
	int		found = 0;
	int		fd = 0;
	int		parent_inodeno = 0;
	int		current_inodeno = 0;

	strcpy(abspath, pFileName);
	strcpy(ptr, pFileName);

	memset(&newInode, 0, sizeof(InodeInfo));

	if (*ptr != '/')		// 절대경로의 시작이 '/'가 아닐때
	{
		free(abspath);
		free(ptr);
		return -1;
	}
	// root inode와 block을 읽음
	// filename에 파일 경로를 얻어냄
	if ( GetEntryName(filename, abspath) == WRONG_VALUE)
	{
		fprintf(stderr, "* GetEntryName error!\n");
		fprintf(stderr, "* filename is too long!\n");
	}
	GetEntryPath(abspath, filename);

	ReadInode(&inodeInfo, fileSysInfo.rootInodeNum);
	abspath = strtok(abspath, "/");
	while(abspath)
	{
		for ( i = 0 ; i < inodeInfo.blocks ; i++ )
		{
			if( inodeInfo.type == FILE_TYPE_DIR ) {
				ReadDirBlock(&dirBlock, inodeInfo.i_block[i]);
				for ( j = 0 ; j < MAX_INDEX_OF_DIRBLK ; j++ )
				{
					if ( strcmp(dirBlock.dirEntries[j].name, abspath) == 0
							&& dirBlock.dirEntries[j].type == FILE_TYPE_DIR)
					{// 마지막 상위 디렉토리(찾는 파일 또는 디렉토리의 ..)를 찾으면
						ReadInode(&inodeInfo, dirBlock.dirEntries[j].inodeNum);
						parent_inodeno = dirBlock.dirEntries[j].inodeNum;
						found++;
						break;
					}
				}
				if ( found == 1 )
				{
					found = 0;
					break;
				}
			}
		}
		abspath = strtok(NULL, "/");
	}
	if ( strcmp(filename, "") == 0)	// 파일명이 없으면 실패리턴
	{
		if( i == inodeInfo.blocks )
		{
			free(abspath);
			free(ptr);
			return WRONG_VALUE;	// 블록 전부를 검색했지만 일치하는 결과가 없으므로 실패
		}
		if( found == 0)
		{
			free(abspath);
			free(ptr);
			return WRONG_VALUE; // dir을 못찾았을
		}
	}
	// 여기까지 왔으면 해당 디렉토리 찾은 것
	switch(flag)
	{
	case OPEN_FLAG_READWRITE:
		// 파일명 찾고 permission readwrite로
		for ( i = 0 ; i < inodeInfo.blocks ; i++ )
		{
			if( inodeInfo.type == FILE_TYPE_DIR )
				ReadDirBlock(&dirBlock, inodeInfo.i_block[i]);
			for ( j = 0 ; j < MAX_INDEX_OF_DIRBLK ; j++ )
			{
				if ( strcmp(dirBlock.dirEntries[j].name, filename) == 0
						&& dirBlock.dirEntries[j].type == FILE_TYPE_FILE)
				{// 파일명 찾으면
					// permission read로
					ReadInode(&inodeInfo, dirBlock.dirEntries[j].inodeNum);
					inodeInfo.mode = FILE_MODE_READWRITE;
					for ( fd = 0 ; fd < FS_INODE_COUNT ; fd++ )
					{// 이미 open 했던 경우
						if ( fileDescTable.file[fd].inodeNo == dirBlock.dirEntries[j].inodeNum )
						{
							fileDescTable.file[fd].offset = 0;
							free(abspath);
							free(ptr);
							return fd;
						}
					}
					for ( fd = 0 ; fd < FS_INODE_COUNT ; fd++ )
					{// 처음 open 하는 경우
						if ( fileDescTable.file[fd].valid_bit == 0 )
						{
							fileDescTable.file[fd].inodeNo = dirBlock.dirEntries[j].inodeNum;
							fileDescTable.file[fd].offset = 0;
							fileDescTable.file[fd].valid_bit = 1;
							free(abspath);
							free(ptr);
							return fd;
						}
					}
				}
			}
		}

		break;
	case OPEN_FLAG_CREATE:
		// 파일명 찾고 있으면 덮어쓰고 없으면 생성
		// 파일명 찾고 permission readonly로
		// 파일이 존재할 경우
		for ( i = 0 ; i < inodeInfo.blocks ; i++ )
		{
			if( inodeInfo.type == FILE_TYPE_DIR )
				ReadDirBlock(&dirBlock, inodeInfo.i_block[i]);
			for ( j = 0 ; j < MAX_INDEX_OF_DIRBLK ; j++ )
			{
				if ( strcmp(dirBlock.dirEntries[j].name, filename) == 0
						&& dirBlock.dirEntries[j].type == FILE_TYPE_FILE)
				{// 파일명 찾으면
					// permission readwrite로
					ReadInode(&inodeInfo, dirBlock.dirEntries[j].inodeNum);
					inodeInfo.mode = FILE_MODE_READWRITE;
					for ( fd = 0 ; fd < FS_INODE_COUNT ; fd++ )
					{// 이미 open 했던 경우
						if ( fileDescTable.file[fd].inodeNo == dirBlock.dirEntries[j].inodeNum )
						{
							fileDescTable.file[fd].offset = 0;
							free(abspath);
							free(ptr);
							return fd;
						}
					}
					for ( fd = 0 ; fd < FS_INODE_COUNT ; fd++ )
					{// 처음 open 하는 경우
						if ( fileDescTable.file[fd].valid_bit == 0 )
						{
							fileDescTable.file[fd].inodeNo = dirBlock.dirEntries[j].inodeNum;
							fileDescTable.file[fd].offset = 0;
							fileDescTable.file[fd].valid_bit = 1;
							free(abspath);
							free(ptr);
							return fd;
						}
					}
				}
			}
		}
		// 파일이 없어서 생성해야 하는 경우
		for ( i = 0 ; i < inodeInfo.blocks ; i++ )
		{
			if( inodeInfo.type == FILE_TYPE_DIR )
				ReadDirBlock(&dirBlock, inodeInfo.i_block[i]);
			for ( j = 0 ; j < MAX_INDEX_OF_DIRBLK ; j++ )
			{
				if ( strcmp(dirBlock.dirEntries[j].name,"") == 0 )
				{// 엔트리가 비어있으면
					strcpy(dirBlock.dirEntries[j].name, filename);
					dirBlock.dirEntries[j].type = FILE_TYPE_FILE;
					dirBlock.dirEntries[j].inodeNum = GetFreeInode();
					WriteDirBlock(&dirBlock, inodeInfo.i_block[i]); // 변경된 상위디렉토리의 dirBlock 갱신

					newInode.size = 0;
					newInode.type = FILE_TYPE_FILE;
					newInode.mode = FILE_MODE_READWRITE;
					newInode.blocks = 1;
					newInode.i_block[0] = GetFreeBlock();

					WriteInode(&newInode, dirBlock.dirEntries[j].inodeNum);
					SetInodeFreeToAlloc();
					SetBlockFreeToAlloc();
					for ( fd = 0 ; fd < FS_INODE_COUNT ; fd++ )
					{// 파일 디스크립터 생성
						if ( fileDescTable.file[fd].valid_bit == 0 )
						{
							fileDescTable.file[fd].inodeNo = dirBlock.dirEntries[j].inodeNum;
							fileDescTable.file[fd].valid_bit = 1;
							free(abspath);
							free(ptr);
							return fd;
						}
					}
				}
			}
		}
		// 사용 가능한 다이렉트블락이 없다 -> 실패리턴
		if ( inodeInfo.blocks != NUM_OF_INDIRECT_BLOCK )
		{
			// 사용 가능한 다이렉트블락이 있다 -> 진행
			inodeInfo.blocks++;
			inodeInfo.i_block[inodeInfo.blocks-1] = GetFreeBlock();
			SetBlockFreeToAlloc();
			WriteInode(&inodeInfo, parent_inodeno);
		//////////////////////////////////////////////////////////////
		//	생성한 directory block 초기화
			ReadDirBlock(&dirBlock, inodeInfo.i_block[inodeInfo.blocks-1]);
			strcpy(dirBlock.dirEntries[0].name, filename);
			dirBlock.dirEntries[0].inodeNum = GetFreeInode();
			SetInodeFreeToAlloc();
			dirBlock.dirEntries[0].type = FILE_TYPE_FILE;
			WriteDirBlock(&dirBlock, inodeInfo.i_block[inodeInfo.blocks-1]);
			return 0;
		//
		//////////////////////////////////////////////////////////////
		}
		break;
	}
	// 여기까지 왔으면 못찾은것임
	free(abspath);
	free(ptr);
	return WRONG_VALUE;
}
int WriteFile(int fileDesc, char* pBuffer, int length)
{
/*
 * precondition		: usage) WriteFile(fileDesc, pBuffer, length);
 *					  fileDesc	: file descriptor
 *					  pBuffer	: 저장할 데이터를 포함하는 메모리의 주소
 *					  length	: 저장될 데이터의 길이
 * postcondition	: open된 파일에 데이터를 저장한다.
 * 					  성공하면 저장된 데이터의 길이 값을 리턴한다. 실패 했을때는 -1을 리턴한다.
 */
	int i = 0, j = 0;
	int	block = fileDescTable.file[fileDesc].offset / BLOCK_SIZE;
	int offset = fileDescTable.file[fileDesc].offset % BLOCK_SIZE;
	int exsist = 0;
	int	remain = length;
	char buf[BLOCK_SIZE] = {NULL,};
	Buf* pBuf = NULL;
	InodeInfo	inodeInfo;
	DirBlock	dirBlock;

	if ( fileDescTable.file[fileDesc].valid_bit != 1 )
		return WRONG_VALUE;
	ReadInode(&inodeInfo, fileDescTable.file[fileDesc].inodeNo);

	// lseek 함수가 없기 때문에 inode에 저장된 indirect block 갯수보다
	// 위에서 계산된 block(fd[].offset / BLOCK_SIZE)이 클수가 없음
	// 따라서 예외처리 안해도 됨
	// 첫번째 블락에 pBuffer 쓰기
	pBuf = BufRead(inodeInfo.i_block[block]);
	memcpy(&buf, pBuf->pMem, BLOCK_SIZE);
	memcpy(&buf + offset, pBuffer, (remain + offset >= BLOCK_SIZE) ? BLOCK_SIZE - offset : remain);
	BufWrite(pBuf, &buf, BLOCK_SIZE);
	if ( inodeInfo.size < (remain - offset) )
		inodeInfo.size = remain - offset;
	fileDescTable.file[fileDesc].offset += (remain + offset >= BLOCK_SIZE) ? BLOCK_SIZE - offset : remain;
	remain -= (BLOCK_SIZE - offset);
	if ( remain <= 0 )		return length;
	// 두번째 블락부터 pBuffer 쓰기
	for ( j = block + 1 ; j < (length / BLOCK_SIZE + 1) + block ; j++ )
	{
		// inodeInfo.blocks가 모자르면 새로 할당해줘야함
		if ( j == inodeInfo.blocks && inodeInfo.blocks < NUM_OF_INDIRECT_BLOCK )
		{
			inodeInfo.i_block[inodeInfo.blocks++] = GetFreeBlock();
			SetBlockFreeToAlloc();
		}
		if ( j == NUM_OF_INDIRECT_BLOCK )
		{	// indirect block 부족으로 쓰기 실패
			WriteInode(&inodeInfo, fileDescTable.file[fileDesc].inodeNo);
			return length - remain;
		}
		pBuf = BufRead(inodeInfo.i_block[j]);
		BufWrite(pBuf, pBuffer + BLOCK_SIZE * ((j - (block + 1) + 1)), remain > BLOCK_SIZE ? BLOCK_SIZE : remain);
		fileDescTable.file[fileDesc].offset += (remain > BLOCK_SIZE ? BLOCK_SIZE : remain);
		remain = remain - BLOCK_SIZE;
		if ( remain <= 0 )
		{
			inodeInfo.size += BLOCK_SIZE + remain;
			WriteInode(&inodeInfo, fileDescTable.file[fileDesc].inodeNo);
			return length;
		}
		else
			inodeInfo.size += BLOCK_SIZE;
	}
	return 0;
}
int ReadFile(int fileDesc, char* pBuffer, int length)
{
/*
 * precondition		: usage) ReadFile(fileDesc, pBuffer, length);
 *					  fileDesc	: file descriptor
 *					  pBuffer	: 저장할 데이터를 포함하는 메모리의 주소
 *					  length	: 저장될 데이터의 길이
 * postcondition	: open된 파일에서 데이터를 읽는다.
 * 					  성공하면 읽은 데이터의 길이 값을 리턴한다. 실패 했을때는 -1을 리턴한다.
 */
	int i = 0, j = 0;
	int	block = fileDescTable.file[fileDesc].offset / BLOCK_SIZE;
	int offset = fileDescTable.file[fileDesc].offset % BLOCK_SIZE;
	int exsist = 0;
	int	wrote = length;
	Buf* pBuf = NULL;
	InodeInfo	inodeInfo;
	DirBlock	dirBlock;

	ReadInode(&inodeInfo, fileDescTable.file[fileDesc].inodeNo);

	// 첫번째 블락에서 data 읽고 pBuffer에 적재
	pBuf = BufRead(inodeInfo.i_block[block]);
	memcpy(pBuffer, (char*)pBuf->pMem + offset, BLOCK_SIZE - offset);
	wrote = length < BLOCK_SIZE - offset ? length : BLOCK_SIZE - offset ;
	fileDescTable.file[fileDesc].offset = wrote;
	if ( length <= 512 )		return wrote;
	// 두번째 블락부터 data 읽고 pBuffer에 적재
	for ( j = block + 1 ; j < (length / BLOCK_SIZE) + block ; j++ )
	{
		if (NUM_OF_INDIRECT_BLOCK == j)
			return  wrote;	// indirect block 12개가 모두 사용중이면 write한 만큼만 리턴
		pBuf = BufRead(inodeInfo.i_block[j]);
		memcpy(pBuffer + BLOCK_SIZE * ((j - (block + 1)) + 1), pBuf->pMem, length - wrote > BLOCK_SIZE ? BLOCK_SIZE : length - wrote);
		wrote = wrote + (length - wrote > BLOCK_SIZE ? BLOCK_SIZE : length - wrote);
		fileDescTable.file[fileDesc].offset = wrote;
		if ( wrote == length )	return wrote;
	}
}
int CloseFile(int fileDesc)
{
/*
 * precondition		: usage ) CloseFile(fileDesc);
 * postcondition	: open된 파일을 닫는다.
 * 					  성공하면 0을 리턴한다. 실패했을 때는 -1을 리턴한다.
 */
	fileDescTable.file[fileDesc].valid_bit = 0;
	BufSync();
}
int RemoveFile(const char* pFileName)
{
/*
 * precondition		: usage ) RemoveFile(pFileName);
 * postcondition	: 파일을 제거한다. 단, open된 파일을 제거 할 수 없다.
 * 					  성공하면 0을 리턴한다. 실패 했을 때는 -1을 리턴한다.
 * 					  실패 원인은 다음과 같다.
 * 					  1) 제거할 파일 이름이 없을 경우
 * 					  2) 제거될 파일이 open 되어 있을 경우
 */
	char filename[NAME_LEN_MAX] = {NULL,};
	char* abspath = malloc(strlen(pFileName) + 1);
	char* ptr = abspath;
	InodeInfo inodeInfo;
	DirBlock dirBlock;
	int i = 0, j = 0;
	int found = 0;
	int parent_blockno = 0;
	memcpy(abspath, pFileName, strlen(pFileName) + 1);

	GetEntryName(filename, abspath);	// dirname 최종 디렉토리명
//	GetEntryPath(abspath, filename);	// abspath는 dirname을 제외한 path
	ReadInode(&inodeInfo, fileSysInfo.rootInodeNum);

	ptr = strtok(abspath, "/");
	while(ptr)
	{ // root 내부로 들어감
		for ( i = 0 ; i < inodeInfo.blocks ; i++ )
		{
			ReadDirBlock(&dirBlock, inodeInfo.i_block[i]);
			for ( j = 0 ; j < MAX_INDEX_OF_DIRBLK ; j++ )
			{
				if( strcmp(ptr, dirBlock.dirEntries[j].name) == 0
						&& dirBlock.dirEntries[j].type == FILE_TYPE_DIR )
				{
					parent_blockno = inodeInfo.i_block[i];
					ReadInode(&inodeInfo, dirBlock.dirEntries[j].inodeNum);
					found++;
					break;
				}
				else if ( strcmp(filename, dirBlock.dirEntries[j].name) == 0
						&& dirBlock.dirEntries[j].type == FILE_TYPE_FILE)
				{
					parent_blockno = inodeInfo.i_block[i];
					ReadInode(&inodeInfo, dirBlock.dirEntries[j].inodeNum);
					found++;
					break;
				}
			}
			if( found == 1 )
			{
				found = 0;
				break;
			}
		}
		ptr = strtok(NULL, "/");
	}
	// 찾는 중간 디렉토리가 없음
	if ( j == MAX_INDEX_OF_DIRBLK )		return WRONG_VALUE;

///////////////////////////////////////////////////
// 여기부터 해당 파일 삭제
	// 사용중인 block 해제
	for ( i = 0 ; i < inodeInfo.blocks ; i++ )
	{
		if(SetBlockAllocToFree(inodeInfo.i_block[i]) == WRONG_VALUE)
		{
			fprintf(stderr, "* RemoveFile() Error!\n");
			return WRONG_VALUE;
		}
	}
	// 사용중인 inode 삭제
	for ( i = 0 ; i < MAX_INDEX_OF_DIRBLK ; i++ )
	{
		if ( strcmp(dirBlock.dirEntries[i].name, filename) == 0 )
		{
			if( SetInodeAllocToFree(dirBlock.dirEntries[i].inodeNum) == WRONG_VALUE)
			{
				fprintf(stderr, "* RemoveFile() Error!\n");
				return WRONG_VALUE;
			}
			strcpy(dirBlock.dirEntries[i].name, "");
			WriteDirBlock(&dirBlock, parent_blockno);
			break;
		}
	}
//
///////////////////////////////////////////////////
}
int MakeDir(const char* pDirName)
{
/*
 * precondition		: usage ) MakeFir(pDirName);
 * postcondition	: 디렉토리를 생성한다.
 * 					  성공하면 0을 리턴한다. 실패 했을 때는 -1을 리턴한다.
 * 					  실패 원인은 생성하고자 하는 디렉토리의 이름과
 * 					  동일한 디렉토리 또는 파일이 존재할 경우이다.
 */
	char dirname[NAME_LEN_MAX] = {NULL,};
	char* abspath = malloc(strlen(pDirName) + 1);
	char* ptr = abspath;
	InodeInfo inodeInfo;
	DirBlock dirBlock;
	int i = 0, j = 0;
	int found = 0;

	memcpy(abspath, pDirName, strlen(pDirName) + 1);

	GetEntryName(dirname, abspath);		// dirname 최종 디렉토리명
	GetEntryPath(abspath, dirname);	// abspath는 dirname을 제외한 path
	ReadInode(&inodeInfo, fileSysInfo.rootInodeNum);

	ptr = strtok(abspath, "/");
	if (!ptr)			// root directory block에 생성할때
	{
		if ( MakeDirentry(&inodeInfo, dirname) == WRONG_VALUE )
		{
			fprintf(stderr, "* MakeDirentry() error!\n");
			return WRONG_VALUE;
		}
		free(abspath);
		return 0;
	}
	while(ptr)
	{ // root directory block에 생성하지 않고 더 내부로 들어감
		for ( i = 0 ; i < inodeInfo.blocks ; i++ )
		{
			ReadDirBlock(&dirBlock, inodeInfo.i_block[i]);
			for ( j = 0 ; j < MAX_INDEX_OF_DIRBLK ; j++ )
			{
				if( strcmp(ptr, dirBlock.dirEntries[j].name) == 0
					&& dirBlock.dirEntries[j].type == FILE_TYPE_DIR)
				{
					ReadInode(&inodeInfo, dirBlock.dirEntries[j].inodeNum);
					found++;
					break;
				}
			}
			if( found == 1 )
			{
				found = 0;
				break;
			}
		}
		ptr = strtok(NULL, "/");
	}
	if ( j == MAX_INDEX_OF_DIRBLK )		return WRONG_VALUE;

	if ( MakeDirentry(&inodeInfo, dirname) == WRONG_VALUE)
	{
		free(abspath);
		return WRONG_VALUE;
	}
	free(abspath);
	return 0;
}
int RemoveDir(const char* pDirName)
{
/*
 * precondition		: usage ) RemoveDir(pDirName);
 * postcondition	: 디렉토리를 제거한다. 단, 리눅스 파일 시스템처럼 빈 디렉토리만 제거가 가능하다.
 * 					  성공하면 0을 리턴한다. 실패 했을 때는 -1을 리턴한다.
 * 					  실패 원인은 다음과 같다.
 * 					  1) 디렉토리에 파일 또는 하위 디렉토리가 존재 할 경우
 * 					  2) 제거하고자 하는 디렉토리가 없을 경우
 */
	char dirname[NAME_LEN_MAX] = {NULL,};
	char* abspath = malloc(strlen(pDirName) + 1);
	char* ptr = abspath;
	InodeInfo inodeInfo;
	DirBlock dirBlock;
	int i = 0, j = 0;
	int found = 0;

	memcpy(abspath, pDirName, strlen(pDirName) + 1);

	GetEntryName(dirname, abspath);	// dirname 최종 디렉토리명
	GetEntryPath(abspath, dirname);	// abspath는 dirname을 제외한 path
	ReadInode(&inodeInfo, fileSysInfo.rootInodeNum);

	ptr = strtok(abspath, "/");
	if (ptr == NULL && strcmp(dirname, "") == 0)			// root directory block에서 지울때
	{
		if ( RemoveDirentry(&inodeInfo, dirname) == WRONG_VALUE )
		{
			fprintf(stderr, "* RemoveDirentry() error!\n");
			return WRONG_VALUE;
		}
		free(abspath);
		return 0;
	}
	while(ptr)
	{ // root 내부로 들어감
		for ( i = 0 ; i < inodeInfo.blocks ; i++ )
		{
			ReadDirBlock(&dirBlock, inodeInfo.i_block[i]);
			for ( j = 0 ; j < MAX_INDEX_OF_DIRBLK ; j++ )
			{
				if( strcmp(ptr, dirBlock.dirEntries[j].name) == 0
						&& dirBlock.dirEntries[j].type == FILE_TYPE_DIR)
				{
					ReadInode(&inodeInfo, dirBlock.dirEntries[j].inodeNum);
					found++;
					break;
				}
			}
			if( found == 1 )
			{
				found = 0;
				break;
			}
		}
		ptr = strtok(NULL, "/");
	}
	// 찾는 디렉토리가 없음
	if ( j == MAX_INDEX_OF_DIRBLK )		return WRONG_VALUE;

	if ( RemoveDirentry(&inodeInfo, dirname) == WRONG_VALUE)
	{
		free(abspath);
		return WRONG_VALUE;
	}
	free(abspath);
	return 0;
}
int EnumerateDirStatus(const char* pDirName, DirEntry* pDirEntry, int dirEntries)
{
/*
 * precondition		: usage ) EnumerateDirStatus(pDirName, pDirEntry, dirEntries);
 * postcondition	: 디렉토리에 포함된 파일 또는 디렉토리 정보를 얻어낸다.
 * 					  이 함수는 해당 디렉토리를 구성하고 있는 디렉토리 엔트리들의
 * 					  묶음을 리턴한다.
 * 					  성공하면 읽어진 디렉토리 엔트리 개수를 리턴한다. 예로, 임의의 디렉토리의 전체
 * 					  디렉토리 엔트리 개수가 40이지만, dirEntries가 60으로 입력됬을때, 리턴되는 값은
 * 					  유효한 디렉토리 엔트리 개수인 40을 리턴해야 한다. 또한, 40개의 디렉토리 엔트리
 * 					  내용을 pDirEntry 배열로 전달해야 한다. 또한, 전체 디렉토리 엔트리 개수가 40이지만
 * 					  dirEntry가 20으로 입력되었을 때, 리턴되는 값은 20이며, 20개의 디렉토리 엔트리의
 * 					  내용이 pDirEntry로 리턴되어야 한다. 에러 발생시 -1을 리턴한다.
 */
	char dirname[NAME_LEN_MAX] = {NULL,};
	char* abspath = malloc(strlen(pDirName) + 1);
	char* ptr = abspath;
	DirEntry* dirEntry = pDirEntry;
	InodeInfo inodeInfo;
	DirBlock dirBlock;
	int i = 0, j = 0;
	int found = 0;
	int	cnt = 0;

	memcpy(abspath, pDirName, strlen(pDirName) + 1);

	GetEntryName(dirname, abspath);	// dirname 최종 디렉토리명
	ReadInode(&inodeInfo, fileSysInfo.rootInodeNum);

	ptr = strtok(abspath, "/");
	if (ptr == NULL && strcmp(dirname,"") == 0)		// root directory 일때
	{
		for ( i = 0 ; i < inodeInfo.blocks ; i++ )
		{
			ReadDirBlock(&dirBlock, inodeInfo.i_block[i]);
			for ( j = 0 ; j < MAX_INDEX_OF_DIRBLK ; j++)
			{
				if ( strcmp(dirBlock.dirEntries[j].name, "") != 0)
				{
					memcpy(dirEntry, &dirBlock.dirEntries[j], sizeof(DirEntry));
					dirEntry++;
					cnt++;
				}
				if ( cnt == dirEntries )
				{
					free(abspath);
					return cnt;
				}
			}
		}
		free(abspath);
		return cnt;
	}
	while(ptr)
	{ // root 내부로 들어감
		for ( i = 0 ; i < inodeInfo.blocks ; i++ )
		{
			ReadDirBlock(&dirBlock, inodeInfo.i_block[i]);
			for ( j = 0 ; j < MAX_INDEX_OF_DIRBLK ; j++ )
			{
				if( strcmp(ptr, dirBlock.dirEntries[j].name) == 0
						&& dirBlock.dirEntries[j].type == FILE_TYPE_DIR)
				{
					ReadInode(&inodeInfo, dirBlock.dirEntries[j].inodeNum);
					found++;
					break;
				}
			}
			if( found == 1 )
			{
				found = 0;
				break;
			}
		}
		ptr = strtok(NULL, "/");
	}
	// 찾는 디렉토리가 없음
	if ( j == MAX_INDEX_OF_DIRBLK )		return WRONG_VALUE;

	for ( i = 0 ; i < inodeInfo.blocks ; i++ )
	{
		ReadDirBlock(&dirBlock, inodeInfo.i_block[i]);
		for ( j = 0 ; j < MAX_INDEX_OF_DIRBLK ; j++)
		{
//			if ( strcmp(dirBlock.dirEntries[j].name, "") != 0)
			if ( strcmp(dirBlock.dirEntries[j].name, "") != 0
					&& strcmp(dirBlock.dirEntries[j].name, ".") != 0
					&& strcmp(dirBlock.dirEntries[j].name, "..") != 0)
			{
				memcpy(dirEntry, &dirBlock.dirEntries[j], sizeof(DirEntry));
				dirEntry++;
				cnt++;
			}
			if ( cnt == dirEntries )
			{
				free(abspath);
				return cnt;
			}
		}
	}
	free(abspath);
	return cnt;
}
void Mount(MountType type)
{
/*
 * precondition		: usage ) Mount(MT_TYPE_FORMAT);
 * postcondition	: 가상 디스크의 파일 시스템을 초기화 한다.
 * 					  (1) MT_TYPE_FORMAT : 가상 디스크, 즉, 디스크를 에뮬레이션할 파일을 생성하고,
 * 										   그 파일을 파일 시스템이 인식할 수 있도록 organize 함.
 * 										   즉, 디스크 포맷처럼 가상 디스크 역할을 담당하는 파일 내에
 * 										   superblock, inode bitmap, block bitmap, inode list
 * 										   등을 초기화한다. 이때, 생성될 가상디스크의 크기는 10MB로 한다.
 * 					  (2) MT_TYPE_READWRITE : MT_TYPE_FORMAT과 달리 가상 디스크를 포맷하지 않으며,
 * 											  파일 시스템 unmount 되기 이전의 디스크 사용 상태를 유지시키면서
 * 											  파일 시스템을 초기화한다. 이때, 내부적으로 가상 디스크용 파일을
 * 											  리눅스의 “open” system call을 사용하여 파일 열기를 수행하며,
 * 											  file system info, inode bitmap, block bitmap을
 * 											  정의된 in-memory data structure에 load한다.
 */
	int i = 0;
	Buf* pBuf = NULL;
	InodeInfo	inodeInfo;
	DirBlock	dirBlock;

//////////////////////////////////////////////////////////////////////
// 모두 block 단위임
	inodeBitmapSize = (FS_INODE_COUNT / 8/*1byte*/ / BLOCK_SIZE == 0) ? 1 : (int)(ceil((double)FS_INODE_COUNT / (double)8 / (double)BLOCK_SIZE));// FS_INODE_COUNT / 8/*1byte*/ / BLOCK_SIZE + 1;	// block 단위
	inodeListSize = (int)(ceil((double)FS_INODE_COUNT / (double)NUM_OF_INODE_IN_1BLK)); // block 단위
	dataRegionSize = (FS_DISK_CAPACITY / BLOCK_SIZE - 1/*FileSysInfo block*/ /* - 1*//*BlockBitmap block*/
						- (double)inodeBitmapSize - (double)inodeListSize);
	blockBitmapSize = ceil(dataRegionSize / 8/*1byte*/ / BLOCK_SIZE);	// block 단위
	dataRegionSize = dataRegionSize - blockBitmapSize;
//
//////////////////////////////////////////////////////////////////////
	memset(&fileDescTable, NULL, sizeof(FileDescTable));
	Init();			// 버퍼캐시 생성 및 초기화

	switch(type)
	{
	case MT_TYPE_FORMAT:
		DevInit();		// 디스크 초기화
		fileSysInfo.blocks = BLOCK_SIZE;
		fileSysInfo.rootInodeNum = 0; // 수정해야함
		fileSysInfo.diskCapacity = dataRegionSize;	// 수퍼블락을 제외한 순수 data rigion의 블록사이즈
		fileSysInfo.numAllocBlocks = 0;
		fileSysInfo.numFreeBlocks = fileSysInfo.diskCapacity;
		fileSysInfo.numInodes = FS_INODE_COUNT;
		fileSysInfo.numAllocInodes = 0;
		fileSysInfo.numFreeInodes = FS_INODE_COUNT;
		fileSysInfo.inodeBitmapStart = 1; // inode bitmap이 저장된 블록번호
		fileSysInfo.blockBitmapStart = fileSysInfo.inodeBitmapStart + inodeBitmapSize; // block bitmap이 저장된 블록 번호
		fileSysInfo.inodeListStart = fileSysInfo.blockBitmapStart + blockBitmapSize; // inode list를 저장하는 영역의 시작 블록 번호
		fileSysInfo.dataStart = fileSysInfo.inodeListStart + inodeListSize; // data region의 시작 블록 번호
		fileSysInfo.pInodeBitmap = malloc(inodeBitmapSize * BLOCK_SIZE);
		fileSysInfo.pBlockBitmap = malloc((int)blockBitmapSize * BLOCK_SIZE);

		memset(fileSysInfo.pInodeBitmap, 0xFF, inodeBitmapSize * BLOCK_SIZE);
		memset(fileSysInfo.pBlockBitmap, 0xFF, (int)blockBitmapSize * BLOCK_SIZE);
		memset(&inodeInfo, 0x0, sizeof(InodeInfo));
		memset(&dirBlock, 0x0, sizeof(DirBlock));

	////////////////////////////////////////////////////
	// root 생성
	//
		// inode 데이터 대입
		inodeInfo.size = 0;
		inodeInfo.type = FILE_TYPE_DIR;
		inodeInfo.mode = FILE_MODE_READONLY;
		inodeInfo.blocks = 1;
		inodeInfo.i_block[0] = fileSysInfo.dataStart;

		// inode 저장
		pBuf = BufRead(fileSysInfo.inodeListStart);
		BufWrite(pBuf, &inodeInfo, sizeof(InodeInfo));
		// inode 사용현황 변경
		if ( SetInodeFreeToAlloc() == WRONG_VALUE )
			fprintf(stderr, "* SetInodeFreeToAlloc() error!\n");
		// Directory Block 생성
		strncpy(dirBlock.dirEntries[0].name, ".", NAME_LEN_MAX);
		dirBlock.dirEntries[0].inodeNum = 0;
		dirBlock.dirEntries[0].type = FILE_TYPE_DIR;
		// dir block 저장
		pBuf = BufRead(fileSysInfo.dataStart);
		BufWrite(pBuf, &dirBlock, sizeof(DirBlock));
		// block 사용현황 변경
		if ( SetBlockFreeToAlloc() == WRONG_VALUE )
			fprintf(stderr, "* SetBlockFreeToAlloc() error!\n");
	//
	////////////////////////////////////////////////////
		break;

	case MT_TYPE_READWRITE:			// 버퍼캐시 생성 및 초기
		DevLoad();		// 디스크 로
		pBuf = BufRead(0);	/* FileSysInfo */
		memcpy(&fileSysInfo, pBuf->pMem, sizeof(fileSysInfo) - sizeof(char*) * 2 /*포인터변수 2개*/);

		fileSysInfo.pInodeBitmap = malloc(inodeBitmapSize * BLOCK_SIZE);
		fileSysInfo.pBlockBitmap = malloc((int)blockBitmapSize * BLOCK_SIZE);
		memset(fileSysInfo.pInodeBitmap, NULL, inodeBitmapSize * BLOCK_SIZE);
		memset(fileSysInfo.pBlockBitmap, NULL, (int)blockBitmapSize * BLOCK_SIZE);

		// inodeBitmap 로드
		for ( i = fileSysInfo.inodeBitmapStart ; i < fileSysInfo.inodeBitmapStart + inodeBitmapSize ; i++ )
		{
			pBuf = BufRead(i);
			memcpy(fileSysInfo.pInodeBitmap + (i - fileSysInfo.inodeBitmapStart) * BLOCK_SIZE, pBuf->pMem, BLOCK_SIZE);
		}
		// blockBitmap 로드
		for ( i = fileSysInfo.blockBitmapStart ; i < fileSysInfo.blockBitmapStart + blockBitmapSize ; i++ )
		{
			pBuf = BufRead(i);
			memcpy(fileSysInfo.pBlockBitmap + (i - fileSysInfo.blockBitmapStart) * BLOCK_SIZE, pBuf->pMem, BLOCK_SIZE);
		}
		break;
	}
}
void Unmount(void)
{
/*
 * precondition		: usage ) Unmount(void);
 * postcondition	: 가상 디스크의 파일 시스템을 종료한다.
 * 					  메모리상에 있는 버퍼에 변경된 데이터와 메타데이터를 디스크와
 * 					  동기화 하고 언마운트 한다.
 */
	int i = 0;
	Buf* pBuf = NULL;

	// fileSysInfo 갱신
	pBuf = BufRead(0);
	BufWrite(pBuf, &fileSysInfo, sizeof(fileSysInfo) - sizeof(char*) * 2);
	// fileSysInfo.pInodeBitmap 갱신
	for ( i = fileSysInfo.inodeBitmapStart ; i < fileSysInfo.inodeBitmapStart + inodeBitmapSize ; i++ )
	{
		pBuf = BufRead(i);
		BufWrite(pBuf, fileSysInfo.pInodeBitmap + (i - fileSysInfo.inodeBitmapStart) * BLOCK_SIZE, 512);
	}
	// fileSysInfo.pBlockBitmap 갱신
	for ( i = fileSysInfo.blockBitmapStart ; i < fileSysInfo.blockBitmapStart + blockBitmapSize ; i++ )
	{
		pBuf = BufRead(i);
		BufWrite(pBuf, fileSysInfo.pBlockBitmap + (i - fileSysInfo.blockBitmapStart) * BLOCK_SIZE, 512);
	}
	BufSync();
}
int GetFreeEntry(char* Bitmap, int BitmapBlockSize)
{
/*
 * precondition		: usage ) GetFreeEntry(fileSysInfo.pInodeBitmap, inodeBitmapSize);
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
 * precondition		: usage ) SetFreeToAlloc(fileSysInfo.pBlockBitmap, blockBitmapSize, dest);
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
	if ( fileSysInfo.numFreeInodes == 0 )	return WRONG_VALUE;
	return GetFreeEntry(fileSysInfo.pInodeBitmap, inodeBitmapSize);
}
int GetFreeBlock()
{
/*
 * precondition		:
 * postcondition	: first-fit 조건임. free block number를 반환한다.
 * 					  성공시 0, block이 모두 사용중이면 -1 리턴
 */
	if ( fileSysInfo.numFreeBlocks == 0 )	return WRONG_VALUE;
	return fileSysInfo.dataStart + GetFreeEntry(fileSysInfo.pBlockBitmap, blockBitmapSize);
}
int SetInodeFreeToAlloc()
{
/*
 * precondition		:
 * postcondition	: first-fit 조건임. inodebitmap에서 제일 앞쪽 미사용중인 inode를
 * 					  사용중(bit:0)으로 바꾼다. 성공시 0, 모두 사용중이라면 -1 리턴
 */
	if ( fileSysInfo.numFreeInodes == 0 )	return WRONG_VALUE;
	IncUseInode();
	return SetFreeToAlloc(fileSysInfo.pInodeBitmap, inodeBitmapSize);
}
int SetBlockFreeToAlloc()
{
/*
 * precondition		:
 * postcondition	: first-fit 조건임. blockbitmap에서 제일 앞쪽 미사용중인 block을
 * 					  사용중(bit:0)으로 바꾼다. 성공시 0, 모두 사용중이라면 -1 리턴
 */
	if ( fileSysInfo.numFreeBlocks == 0 )	return WRONG_VALUE;
	IncUseBlock();
	return SetFreeToAlloc(fileSysInfo.pBlockBitmap, blockBitmapSize);
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

	if ( fileSysInfo.numAllocInodes == 0 )	return WRONG_VALUE;
	if ( *(fileSysInfo.pInodeBitmap + block) & 0x1 << bitno )
	{
		fprintf(stderr, "* SetInodeAllocToFree()\n");
		fprintf(stderr, "* Already Free\n");
		return WRONG_VALUE;
	}
	*(fileSysInfo.pInodeBitmap + block) ^= 0x1 << bitno;
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
	int block = (blockno - fileSysInfo.dataStart) / 8/*size of char*/;
	int bitno = (blockno - fileSysInfo.dataStart) % 8/*size of char*/;

	if ( fileSysInfo.numAllocBlocks == 0 )	return WRONG_VALUE;
	if(blockno < fileSysInfo.dataStart)
	{
		fprintf(stderr, "  invalid blockno!\n");
		fprintf(stderr, "* SetBlockAllocToFree()\n");
		return WRONG_VALUE;
	}
	if ( *(fileSysInfo.pBlockBitmap + block) & 0x1 << bitno )
	{
		fprintf(stderr, "  Already Free\n");
		fprintf(stderr, "* SetBlockAllocToFree()\n");
		return WRONG_VALUE;
	}
	*(fileSysInfo.pBlockBitmap + block) ^= 0x1 << bitno;
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
	if ( 0 < fileSysInfo.numFreeInodes )
	{
		fileSysInfo.numAllocInodes++;
		fileSysInfo.numFreeInodes--;
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
	if ( 0 < fileSysInfo.numAllocInodes )
	{
		fileSysInfo.numAllocInodes--;
		fileSysInfo.numFreeInodes++;
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
	if ( 0 < fileSysInfo.numFreeBlocks)
	{
		fileSysInfo.numAllocBlocks++;
		fileSysInfo.numFreeBlocks--;
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
	if ( 0 < fileSysInfo.numAllocBlocks )
	{
		fileSysInfo.numAllocBlocks--;
		fileSysInfo.numFreeBlocks++;
		return 0;
	}
	return WRONG_VALUE;
}
void ReadInode(InodeInfo* inodeInfo, int inodeNo)
{
/*
 * precondition		:
 * postcondition	: 인자 inodeNo는 inode number이다.
 * 					  해당 inode number를 가진 inodeInfo를 디스크에서 읽어온다.
 */
	Buf* pBuf = NULL;
	InodeInfo* pMem = NULL;
	int block = fileSysInfo.inodeListStart + inodeNo / NUM_OF_INODE_IN_1BLK;	// inodeNo가 위치한 블럭
	int inode = inodeNo % NUM_OF_INODE_IN_1BLK;	// 해당 block에서 inode가 위치한 순서

	pBuf = BufRead(block);
	pMem = pBuf->pMem;
	pMem = pMem + inode;
	memcpy(inodeInfo, pMem/*(InodeInfo*)pBuf->pMem + inode*/, sizeof(InodeInfo));
}
void WriteInode(InodeInfo* inodeInfo, int inodeNo)
{
/*
 * precondition		:
 * postcondition	: 인자 inodeNo는 inodeInfo의 inode number이다.
 * 					  inodeInfo를 디스크에 저장한다.
 */
	Buf* pBuf = NULL;
	void* pMem = malloc(BLOCK_SIZE);
	InodeInfo* pCur = pMem;
	int block = fileSysInfo.inodeListStart + inodeNo / NUM_OF_INODE_IN_1BLK;
	int inode = inodeNo % NUM_OF_INODE_IN_1BLK;

	pBuf = BufRead(block);
	memcpy(pMem, pBuf->pMem, BLOCK_SIZE);
	pCur = pCur + inode;
	memcpy(pCur, inodeInfo, sizeof(InodeInfo));
	BufWrite(pBuf, pMem, BLOCK_SIZE);
	free(pMem);
}
void ReadDirBlock(DirBlock* dirBlock, int blockNo)
{
/*
 * precondition		:
 * postcondition	: 인자 blockNo는 block number이다.
 * 					  해당 block number의 block을 디스크에서 읽어 온다.
 */
	Buf* pBuf = NULL;

	pBuf = BufRead(blockNo);
	memcpy(dirBlock, (DirBlock*)pBuf->pMem, sizeof(DirBlock));
}
void WriteDirBlock(DirBlock* dirBlock, int blockNo)
{
/*
 * precondition		:
 * postcondition	: 인자 blockNo는 dirBlock의 block number이다.
 * 					  dirBlock을 디스크에 저장한다.
 */
	Buf* pBuf = NULL;

	pBuf = BufRead(blockNo);
	BufWrite(pBuf, dirBlock, sizeof(DirBlock));
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
int GetFreeDir(DirBlock* dirBlock)
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
int MakeDirentry(InodeInfo* inodeInfo, char* dirname)
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
	InodeInfo newInodeInfo;
	DirBlock dirBlock;
	ReadDirBlock(&dirBlock, inodeInfo->i_block[0]);
	parent_inodeno = dirBlock.dirEntries[0].inodeNum;
	for ( block = 0 ; block < inodeInfo->blocks ; block++ )
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
			newInodeInfo.blocks = 1;
			newInodeInfo.size = BLOCK_SIZE;
			newInodeInfo.type = FILE_TYPE_DIR;
			newInodeInfo.mode = FILE_MODE_READONLY;
			newInodeInfo.i_block[0] = GetFreeBlock();
			SetBlockFreeToAlloc();
		////////////////////////////////////////////
		//	생성한 directory block 초기화
			ReadDirBlock(&dirBlock, newInodeInfo.i_block[0]);
			strcpy(dirBlock.dirEntries[0].name, ".");
			dirBlock.dirEntries[0].inodeNum = current_inodeno;
			dirBlock.dirEntries[0].type = FILE_TYPE_DIR;
			strcpy(dirBlock.dirEntries[1].name, "..");
			dirBlock.dirEntries[1].inodeNum = parent_inodeno;
			dirBlock.dirEntries[1].type = FILE_TYPE_DIR;
			WriteDirBlock(&dirBlock, newInodeInfo.i_block[0]);
		//
		////////////////////////////////////////////
			WriteInode(&newInodeInfo, current_inodeno);
			return 0;
		}
	}
	if( inodeInfo->blocks < NUM_OF_INDIRECT_BLOCK )
	{// indirect block을 더 생성할 수 있을때
		inodeInfo->blocks++;
		inodeInfo->i_block[inodeInfo->blocks - 1] = GetFreeBlock();
		SetBlockFreeToAlloc();
		WriteInode(inodeInfo, parent_inodeno);
		ReadDirBlock(&dirBlock, inodeInfo->i_block[inodeInfo->blocks - 1]);
		strcpy(dirBlock.dirEntries[0].name, dirname);
		dirBlock.dirEntries[0].inodeNum = GetFreeInode();
		if ( SetInodeFreeToAlloc() == WRONG_VALUE )		return WRONG_VALUE;
		dirBlock.dirEntries[0].type = FILE_TYPE_DIR;
		current_inodeno = dirBlock.dirEntries[0].inodeNum;
		WriteDirBlock(&dirBlock, inodeInfo->i_block[block]);
		newInodeInfo.blocks = 1;
		newInodeInfo.size += BLOCK_SIZE;
		newInodeInfo.type = FILE_TYPE_DIR;
		newInodeInfo.mode = FILE_MODE_READONLY;
		newInodeInfo.i_block[0] = GetFreeBlock();
		SetBlockFreeToAlloc();
	////////////////////////////////////////////
	//	생성한 directory block 초기화
		ReadDirBlock(&dirBlock, newInodeInfo.i_block[0]);
		memset(&dirBlock, 0, sizeof(DirBlock));
		strcpy(dirBlock.dirEntries[0].name, ".");
		dirBlock.dirEntries[0].inodeNum = current_inodeno;
		dirBlock.dirEntries[0].type = FILE_TYPE_DIR;
		strcpy(dirBlock.dirEntries[1].name, "..");
		dirBlock.dirEntries[1].inodeNum = parent_inodeno;
		dirBlock.dirEntries[1].type = FILE_TYPE_DIR;
		WriteDirBlock(&dirBlock, newInodeInfo.i_block[0]);
	//
	////////////////////////////////////////////
		WriteInode(&newInodeInfo, current_inodeno);
		return 0;
	}
	// 모든 indirect block을 사용중이며 또한 모든 directory block에 빈 공간이 없다.
	return WRONG_VALUE;
}
int RemoveDirentry(InodeInfo* inodeInfo, char* dirname)
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
	InodeInfo delInodeInfo;
	InodeInfo nullinode;
	DirBlock dirBlock;
	DirBlock nullblock;
	memset(&nullinode, 0, sizeof(InodeInfo));
	memset(&nullblock, 0, sizeof(DirBlock));

	ReadDirBlock(&dirBlock, inodeInfo->i_block[0]);
	current_inodeno = dirBlock.dirEntries[0].inodeNum;
	parent_inodeno = dirBlock.dirEntries[1].inodeNum;
	for ( block = 0 ; block < inodeInfo->blocks ; block++ )
	{
		ReadDirBlock(&dirBlock, inodeInfo->i_block[block]);
		for ( index = 0 ; index < MAX_INDEX_OF_DIRBLK ; index++ )
		{
			if(strcmp(dirBlock.dirEntries[index].name, dirname) == 0
					&& dirBlock.dirEntries[index].type == FILE_TYPE_DIR)
			{// 지울 디렉토리명을 찾으면
				ReadInode(&delInodeInfo, dirBlock.dirEntries[index].inodeNum);
				if ( (ret_value = DirIsEmpty(&delInodeInfo)) == WRONG_VALUE )
				{
					fprintf(stderr, "dirname is not a DIRECTORY!\n");
					fprintf(stderr, "DirIsEmpty() Error!\n");
				}
				if ( ret_value == TRUE )
				{// 디렉토리가 비어있다. ==> 삭제
					for ( i = 0 ; i < delInodeInfo.blocks ; i++ )
					{
						WriteDirBlock(&nullblock, delInodeInfo.i_block[i]);
						SetBlockAllocToFree(delInodeInfo.i_block[i]);
					}
					WriteInode(&nullinode, dirBlock.dirEntries[index].inodeNum);
					SetInodeAllocToFree(dirBlock.dirEntries[index].inodeNum);
					dirBlock.dirEntries[index].inodeNum = 0;
					strcpy(dirBlock.dirEntries[index].name, "");
					// DirBlock과 InodeInfo 갱신하기
					WriteDirBlock(&dirBlock, inodeInfo->i_block[block]);
					return 0;
				}
			}
		}
	}
	return -1;
}
int DirIsEmpty(InodeInfo* inodeInfo)
{
/*
 * precondition		: inodeInfo != NULL
 * postcondition	: inodeInfo가 비어있는 디렉토리이면 1 리턴
 * 					  비어있지 않으면 0 리턴, 디렉토리가 아니면 -1 리턴
*/
	int block = 0, index = 0;
	DirBlock dirBlock;

	if(inodeInfo->type != FILE_TYPE_DIR)		return WRONG_VALUE;

	ReadDirBlock(&dirBlock, inodeInfo->i_block[0]);
	for( index = 0 ; index < MAX_INDEX_OF_DIRBLK ; index++ )
	{	// .과 ..이 있는 블럭일때
		if( strcmp(dirBlock.dirEntries[index].name, ".") == 0
			|| strcmp(dirBlock.dirEntries[index].name, "..") == 0)
			continue;
		if( strcmp(dirBlock.dirEntries[index].name, "") != 0)
			return FALSE;
	}
	for ( block = 1 ; block < inodeInfo->blocks ; block++)
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
