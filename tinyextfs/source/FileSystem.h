#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include "FilesysConfig.h"
#include "Buf.h"

#define NUM_OF_INODE_IN_1BLK	(BLOCK_SIZE / sizeof(InodeInfo))
#define NUM_OF_DIRENT_IN_1BLK	(BLOCK_SIZE / sizeof(DirEntry))
#define MAX_INDEX_OF_DIRBLK		(NUM_OF_DIRENT_IN_1BLK)
#define NAME_LEN_MAX			(60)
#define NUM_OF_INDIRECT_BLOCK	(12)

typedef enum __openFlag {
	OPEN_FLAG_READWRITE,
	/* 파일을 read/write permission으로 open함.
	본 과제에서는 이 flag가 입력되었을 때,
	리눅스와 달리 open file table 또는
	file descriptor table에 셋팅을 하지 않음.
	단순히 파일 open을 수행함*/
	OPEN_FLAG_CREATE	// 파일이 존재하지 않으면 생성 후 파일을 open함
} OpenFlag;

typedef enum __fileType {
    FILE_TYPE_FILE,
    FILE_TYPE_DIR,
    FILE_TYPE_DEV
} FileType;

typedef enum __fileMode {
	FILE_MODE_READONLY,
	FILE_MODE_READWRITE,
	FILE_MODE_EXEC
}FileMode;

typedef struct __dirEntry {
    char name[NAME_LEN_MAX/*60byte 혹은 가변적*/];
    int inodeNum;
    FileType type;
} DirEntry;

typedef enum __mountType {
    MT_TYPE_FORMAT,		// 마운트가 되는 해당 파티션은 포맷된다.
    MT_TYPE_READWRITE,	// 마운트가 되는 해당 파티션은 그대로 유지된다.
} MountType;

typedef struct _fileSysInfo {
	int blocks;				// 파일 시스템의 설정된 블록 크기
	int rootInodeNum; 		// root inode number 저장
	int diskCapacity;		// 디스크의 용량. 블록 개수로 저장됨
	int numAllocBlocks;		// 할당된 블록 개수
	int numFreeBlocks;		// 할당되지 않은 블록 개수
	int numInodes;			// 전체 inode의 개수
	int numAllocInodes;		// 할당된 inode의 개수
	int numFreeInodes;		// 할당되지 않은 inode의 개수
	int inodeBitmapStart;	// inode bitmap이 저장된 블록의 번호
	int blockBitmapStart;	// block bitmap이 저장된 블록 번호
	int inodeListStart;		// inode list를 저장하는 영역의 시작 블록 번호
	int dataStart;			// data 영역의 시작 블록 번호
	char* pInodeBitmap; 	// inode bitmap 정보가 저장된 메모리 주소
	char* pBlockBitmap; 	// block bitmap 정보가 저장된 메모리 주소
} FileSysInfo;

typedef struct __inodeInfo {
	int			size;
	FileType	type;
	FileMode	mode;
	int			blocks;
	int			i_block[NUM_OF_INDIRECT_BLOCK];
}InodeInfo;

typedef struct __dirBlock {
	DirEntry	dirEntries[NUM_OF_DIRENT_IN_1BLK];
}DirBlock;

typedef struct __fileDesc {
	int	valid_bit;
	int	offset;
	int	inodeNo;
}FileDesc;

typedef struct __fileDescTable {
	FileDesc	file[FS_INODE_COUNT];
}FileDescTable;

int		OpenFile(const char* pFileName, OpenFlag flag);
int		WriteFile(int fileDesc, char* pBuffer, int length);
int		ReadFile(int fileDesc, char* pBuffer, int length);
int		CloseFile(int fileDesc);
int		RemoveFile(const char* pFileName);
int		MakeDir(const char* pDirName);
int		RemoveDir(const char* pDirName);
int		EnumerateDirStatus(const char* pDirName, DirEntry* pDirEntry, int dirEntries);
void	Mount(MountType type);
void	Unmount(void);

int		IncUseInode();	// Inode 1개 사용
int		DecUseInode();	// Inode 1개 해제
int		IncUseBlock();	// Block 1개 사용
int		DecUseBlock();	// Block 1개 해제
int		GetFreeEntry(char* Bitmap, int BitmapBlockSize);
int		GetFreeInode();
int		GetFreeBlock();
void	GetEntryPath(char* abspath, char* filename);	// 절대경로에서 파일 혹은 디렉토리가 위치한 절대경로를 구함
int		GetEntryName(char* dest, char* abspath); 		// 절대경로에서 파일 혹은 디렉토리의 이름을 무함
int		GetFreeDir(DirBlock* dirBlock);
int		SetFreeToAlloc(char* Bitmap, int BitmapBlockSize);
int		SetInodeFreeToAlloc();
int		SetBlockFreeToAlloc();
int		SetInodeAllocToFree(int inodeno);
int		SetBlockAllocToFree(int blockno);
void	ReadInode(InodeInfo* inodeInfo, int inodeNo);
void	ReadDirBlock(DirBlock* dirBlock, int blockNo);
void	WriteInode(InodeInfo* inodeInfo, int inodeNo);
void	WriteDirBlock(DirBlock* dirBlock, int blockNo);
int		MakeDirentry(InodeInfo* inodInfo, char* dirname);
int		RemoveDirentry(InodeInfo* inodInfo, char* dirname);
int		DirIsEmpty(InodeInfo* inodeInfo);
#endif /* FILESYSTEM_H_ */
