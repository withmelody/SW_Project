#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include "tinyfs.h"

int		IncUseInode();	// Inode 1개 사용
int		DecUseInode();	// Inode 1개 해제
int		IncUseBlock();	// Block 1개 사용
int		DecUseBlock();	// Block 1개 해제

int		GetFreeEntry(char* Bitmap, int BitmapBlockSize);
int		GetFreeInode();
int		GetFreeBlock();
void	GetEntryPath(char* abspath, char* filename);	// 절대경로에서 파일 혹은 디렉토리가 위치한 절대경로를 구함
int		GetEntryName(char* dest, char* abspath); 		// 절대경로에서 파일 혹은 디렉토리의 이름을 무함
int		GetFreeDir(tiny_dirblk* dirBlock);
int		SetFreeToAlloc(char* Bitmap, int BitmapBlockSize);
int		SetInodeFreeToAlloc();
int		SetBlockFreeToAlloc();
int		SetInodeAllocToFree(int inodeno);
int		SetBlockAllocToFree(int blockno);
void	ReadInode(tiny_inode* inodeInfo, int inodeNo);
void	ReadDirBlock(tiny_dirblk* dirBlock, int blockNo);
void	WriteInode(tiny_inode* inodeInfo, int inodeNo);
void	WriteDirBlock(tiny_dirblk* dirBlock, int blockNo);
int		MakeDirentry(tiny_inode* inodInfo, char* dirname);
int		RemoveDirentry(tiny_inode* inodInfo, char* dirname);
int		DirIsEmpty(tiny_inode* inodeInfo);

#endif /* FILESYSTEM_H_ */
