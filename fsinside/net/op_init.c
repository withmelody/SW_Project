#include <math.h>
#include "tinyfs.h"
#include "msglib/msglib.h"

extern FileSysInfo tiny_superblk;

void *tiny_init(struct fuse_conn_info *conn)
{
/*
 * precondition		: usage ) Mount(MT_TYPE_FORMAT);
 * postcondition	: 가상 디스크의 파일 시스템을 초기화 한다.
 * 			  (1) MT_TYPE_FORMAT : 가상 디스크, 즉, 디스크를 에뮬레이션할 파일을 생성하고,
 * 				   그 파일을 파일 시스템이 인식할 수 있도록 organize 함.
 * 				   즉, 디스크 포맷처럼 가상 디스크 역할을 담당하는 파일 내에
 * 				   superblock, inode bitmap, block bitmap, inode list
 * 				   등을 초기화한다. 이때, 생성될 가상디스크의 크기는 8MB로 한다.
 * 			  (2) MT_TYPE_READWRITE : MT_TYPE_FORMAT과 달리 가상 디스크를 포맷하지 않으며,
 * 				  파일 시스템 unmount 되기 이전의 디스크 사용 상태를 유지시키면서
 * 				  파일 시스템을 초기화한다. 이때, 내부적으로 가상 디스크용 파일을
 * 				  리눅스의 “open” system call을 사용하여 파일 열기를 수행하며,
 * 				  file system info, inode bitmap, block bitmap을
 * 				  정의된 in-memory data structure에 load한다.
 */
	int i = 0;
	Buf* pBuf = NULL;
	tiny_inode	inodeInfo;
	tiny_dirblk	dirBlock;
	MountType type;
	FileSysInfo temp_superblk = {0,};

	memset(&tiny_superblk, 0x00, sizeof(tiny_superblk));

//////////////////////////////////////////////////////////////////////
// 모두 block 단위임
//	tiny_superblk.s_ibitmap_size = (FS_INODE_COUNT / 8/*1byte*/ / BLOCK_SIZE == 0) ? 1 : (int)(ceil((double)FS_INODE_COUNT / (double)8 / (double)BLOCK_SIZE));// FS_INODE_COUNT / 8/*1byte*/ / BLOCK_SIZE + 1;	// block 단위
//	tiny_superblk.s_inodeblk_size = (int)(ceil((double)FS_INODE_COUNT / (double)NUM_OF_INODE_IN_1BLK)); // block 단위
//	tiny_superblk.s_datablk_size = (FS_DISK_CAPACITY / BLOCK_SIZE - 1/*FileSysInfo block*/ /* - 1*//*BlockBitmap block*/
//						- (double)tiny_superblk.s_ibitmap_size - (double)tiny_superblk.s_inodeblk_size);
//	tiny_superblk.s_dbitmap_size = ceil(tiny_superblk.s_datablk_size / 8/*1byte*/ / BLOCK_SIZE);	// block 단위
//	tiny_superblk.s_datablk_size = tiny_superblk.s_datablk_size - tiny_superblk.s_dbitmap_size;
//
//////////////////////////////////////////////////////////////////////
	Init();			// 버퍼캐시 생성 및 초기화

	DevLoad();		// 디스크 로드
	pBuf = BufRead(0);	/* FileSysInfo */
	memcpy(&tiny_superblk, pBuf->pMem, sizeof(temp_superblk) - sizeof(char*) * 2 /*포인터변수 2개 제외*/);

	if(tiny_superblk.s_nblk == 0) { // 0이 초기값인 임의의 데이터 확인
		type = MT_TYPE_FORMAT;
//////////////////////////////////////////////////////////////////////
// 모두 block 단위임
		tiny_superblk.s_ibitmap_size = (FS_INODE_COUNT / 8/*1byte*/ / BLOCK_SIZE == 0) ? 1 : (int)(ceil((double)FS_INODE_COUNT / (double)8 / (double)BLOCK_SIZE));// FS_INODE_COUNT / 8/*1byte*/ / BLOCK_SIZE + 1;	// block 단위
		tiny_superblk.s_inodeblk_size = (int)(ceil((double)FS_INODE_COUNT / (double)NUM_OF_INODE_IN_1BLK)); // block 단위
		tiny_superblk.s_datablk_size = (FS_DISK_CAPACITY / BLOCK_SIZE - 1/*FileSysInfo block*/ /* - 1*//*BlockBitmap block*/
				- (double)tiny_superblk.s_ibitmap_size - (double)tiny_superblk.s_inodeblk_size);
		tiny_superblk.s_dbitmap_size = ceil(tiny_superblk.s_datablk_size / 8/*1byte*/ / BLOCK_SIZE);	// block 단위
		tiny_superblk.s_datablk_size = tiny_superblk.s_datablk_size - tiny_superblk.s_dbitmap_size;
//
//////////////////////////////////////////////////////////////////////
	} else if ( tiny_superblk.s_nblk > 0 ) {
		type = MT_TYPE_READWRITE;
	} else {
		type == MT_TYPE_UNKNOWN;
	}
	DevRelease();

	switch(type)
	{
	case MT_TYPE_FORMAT:
		DevInit();		// 디스크 초기화
		tiny_superblk.s_nblk = BLOCK_SIZE;
		tiny_superblk.s_rdirino = 0; // 수정해야함
		tiny_superblk.s_ndatablk = tiny_superblk.s_datablk_size;	// 수퍼블락을 제외한 순수 data rigion의 블록사이즈
		tiny_superblk.s_nblk_use = 0;
		tiny_superblk.s_nblk_free = tiny_superblk.s_ndatablk;
		tiny_superblk.s_ninode = FS_INODE_COUNT;
		tiny_superblk.s_ninode_use = 0;
		tiny_superblk.s_ninode_free = FS_INODE_COUNT;
		tiny_superblk.s_ibitmap_start = 1; // inode bitmap이 저장된 블록번호
		tiny_superblk.s_dbitmap_start = tiny_superblk.s_ibitmap_start + tiny_superblk.s_ibitmap_size; // block bitmap이 저장된 블록 번호
		tiny_superblk.s_inodeblk_start = tiny_superblk.s_dbitmap_start + tiny_superblk.s_dbitmap_size; // inode list를 저장하는 영역의 시작 블록 번호
		tiny_superblk.s_datablk_start = tiny_superblk.s_inodeblk_start + tiny_superblk.s_inodeblk_size; // data region의 시작 블록 번호
		tiny_superblk.s_ibitmap_ptr = malloc(tiny_superblk.s_ibitmap_size * BLOCK_SIZE);
		tiny_superblk.s_dbitmap_ptr = malloc((int)tiny_superblk.s_dbitmap_size * BLOCK_SIZE);

		memset(tiny_superblk.s_ibitmap_ptr, 0xFF, tiny_superblk.s_ibitmap_size * BLOCK_SIZE);
		memset(tiny_superblk.s_dbitmap_ptr, 0xFF, (int)tiny_superblk.s_dbitmap_size * BLOCK_SIZE);
		memset(&inodeInfo, 0x0, sizeof(tiny_inode));
		memset(&dirBlock, 0x0, sizeof(tiny_dirblk));

	////////////////////////////////////////////////////
	// root 생성
	//
		// inode 데이터 대입
		inodeInfo.i_size = 0;
		inodeInfo.i_type = FILE_TYPE_DIR;
		inodeInfo.i_nblk = 1;
		inodeInfo.i_block[0] = tiny_superblk.s_datablk_start;

		// inode 저장
		pBuf = BufRead(tiny_superblk.s_inodeblk_start);
		BufWrite(pBuf, &inodeInfo, sizeof(tiny_inode));
		// inode 사용현황 변경
		if ( SetInodeFreeToAlloc() == WRONG_VALUE )
			fprintf(stderr, "* SetInodeFreeToAlloc() error!\n");
		// Directory Block 생성
		strncpy(dirBlock.dirEntries[0].name, ".", NAME_LEN_MAX);
		dirBlock.dirEntries[0].inodeNum = 0;
		dirBlock.dirEntries[0].type = FILE_TYPE_DIR;
		// dir block 저장
		pBuf = BufRead(tiny_superblk.s_datablk_start);
		BufWrite(pBuf, &dirBlock, sizeof(tiny_dirblk));
		// block 사용현황 변경
		if ( SetBlockFreeToAlloc() == WRONG_VALUE )
			fprintf(stderr, "* SetBlockFreeToAlloc() error!\n");
	//
	////////////////////////////////////////////////////
		break;

	case MT_TYPE_READWRITE:			// 버퍼캐시 생성 및 초기
		DevLoad();		// 디스크 로
		pBuf = BufRead(0);	/* FileSysInfo */
		memcpy(&tiny_superblk, pBuf->pMem, sizeof(tiny_superblk) - sizeof(char*) * 2 /*포인터변수 2개*/);

		tiny_superblk.s_ibitmap_ptr = malloc(tiny_superblk.s_ibitmap_size * BLOCK_SIZE);
		tiny_superblk.s_dbitmap_ptr = malloc((int)tiny_superblk.s_dbitmap_size * BLOCK_SIZE);
		memset(tiny_superblk.s_ibitmap_ptr, 0, tiny_superblk.s_ibitmap_size * BLOCK_SIZE);
		memset(tiny_superblk.s_dbitmap_ptr, 0, (int)tiny_superblk.s_dbitmap_size * BLOCK_SIZE);

		// inodeBitmap 로드
		for ( i = tiny_superblk.s_ibitmap_start ; i < tiny_superblk.s_ibitmap_start + tiny_superblk.s_ibitmap_size ; i++ )
		{
			pBuf = BufRead(i);
			memcpy(tiny_superblk.s_ibitmap_ptr + (i - tiny_superblk.s_ibitmap_start) * BLOCK_SIZE, pBuf->pMem, BLOCK_SIZE);
		}
		// blockBitmap 로드
		for ( i = tiny_superblk.s_dbitmap_start ; i < tiny_superblk.s_dbitmap_start + tiny_superblk.s_dbitmap_size ; i++ )
		{
			pBuf = BufRead(i);
			memcpy(tiny_superblk.s_dbitmap_ptr + (i - tiny_superblk.s_dbitmap_start) * BLOCK_SIZE, pBuf->pMem, BLOCK_SIZE);
		}
		break;
	}

	int qid = OpenMQ(5000);
	if(qid < 0)
	{
		printf("q open fail\n");
		return ;
	}

	SuperBlk_t sb;
	sb.fsi = tiny_superblk;
	if(SendMQ(qid, MSG_SUPER_BLOCK, &sb) < 0)
	{
		printf("superblk send fail\n");
		return ;
	}

	InodeBitmap_t ibm;
	ibm.size = tiny_superblk.s_ninode / 8; /*byte*/
	memcpy(ibm.s_ibitmap_ptr, tiny_superblk.s_ibitmap_ptr, ibm.size);
	if(SendMQ(qid, MSG_INODE_BITMAP, &ibm) < 0)
	{
		printf("ibm send fail\n");
		return ;
	}
	
	BlockBitmap_t bbm;
	bbm.size = tiny_superblk.s_datablk_size / 8;  /*byte*/
	memcpy(bbm.s_dbitmap_ptr, tiny_superblk.s_dbitmap_ptr, bbm.size);
	if(SendMQ(qid, MSG_BLOCK_BITMAP, &bbm) < 0)
	{
		printf("bbm send fail\n");
		return ;
	}
}
