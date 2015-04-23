#include "FilesysConfig.h"
#include "FileSystem.h"
#include "TestCase.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>


#define DIR_NUM_MAX		100


void ListDirContents(const char* dirName, int dirNum)
{
	int i ;
	DirEntry pDirEntry[DIR_NUM_MAX];
	int count;

	count = EnumerateDirStatus(dirName, pDirEntry, dirNum);
	printf("[%s]Sub-directory:\n", dirName);
	for (i = 0;i < count;i++)
	{
		if (pDirEntry[i].type == FILE_TYPE_FILE)
			printf("\t name:%s, inode no:%d, type:file\n", pDirEntry[i].name, pDirEntry[i].inodeNum);
		else if (pDirEntry[i].type == FILE_TYPE_DIR)
				printf("\t name:%s, inode no:%d, type:directory\n", pDirEntry[i].name, pDirEntry[i].inodeNum);
		else
		{
			assert(0);
		}	
	}
}


void TestCase1(void)
{
	int i;
	char dirName[NAME_LEN_MAX];

	printf(" ---- Test Case 1 ----\n");

	MakeDir("/tmp");
	MakeDir("/usr");
	MakeDir("/etc");
	MakeDir("/home");
	/* make home directory */
	for (i = 0;i < 8;i++)
	{
		memset(dirName, 0, NAME_LEN_MAX);
		sprintf(dirName, "/home/user%d", i);
		MakeDir(dirName);
	}
	/* make etc directory */
	for (i = 0;i < 24;i++)
	{
		memset(dirName, 0, NAME_LEN_MAX);
		sprintf(dirName, "/etc/dev%d", i);
		MakeDir(dirName);
	}
	ListDirContents("/home", 8);
	ListDirContents("/etc", 24);

	/* remove subdirectory of etc directory */
	for (i = 24;i > 0;i--)
	{
		memset(dirName, 0, NAME_LEN_MAX);
		sprintf(dirName, "/etc/dev%d", i);
		RemoveDir(dirName);
	}
	ListDirContents("/etc", 24);
}


void TestCase2(void)
{
	int i, j;
	char fileName[NAME_LEN_MAX];
	char dirName[NAME_LEN_MAX];
	int fd;

	printf(" ---- Test Case 2 ----\n");

	ListDirContents("/home", 8);
	/* make home directory */
	for (i = 0;i < 8;i++)
	{
		for (j = 0;j < 9;j++)
		{
			memset(fileName, 0, NAME_LEN_MAX);
			sprintf(fileName, "/home/user%d/file%d", i,j);
			fd = OpenFile(fileName, OPEN_FLAG_CREATE);
			CloseFile(fd);
		}
	}

	for (i = 0;i < 8;i++)
	{
		memset(dirName, 0, NAME_LEN_MAX);
		sprintf(dirName, "/home/user%d", i);
		ListDirContents(dirName, 9);
	}
}


void TestCase3(void)
{
	int i;
	char fileName[NAME_LEN_MAX];
	int fd;
	char pBuffer1[512];
	char pBuffer2[512];

	printf(" ---- Test Case 3 ----\n");
	for (i = 0;i < 9;i++)
	{
		memset(fileName, 0, NAME_LEN_MAX);
		sprintf(fileName, "/home/user7/file%d", i);
		fd = OpenFile(fileName, OPEN_FLAG_CREATE);
		memset(pBuffer1, 0, 512);
		strcpy(pBuffer1, fileName);
		WriteFile(fd, pBuffer1, 512);
		CloseFile(fd);
	}
	for (i = 0;i < 9;i++)
	{
		memset(fileName, 0, NAME_LEN_MAX);
		sprintf(fileName, "/home/user7/file%d", i);
		fd = OpenFile(fileName, OPEN_FLAG_READWRITE);

		memset(pBuffer1, 0, 512);
		strcpy(pBuffer1, fileName); 

		memset(pBuffer2, 0, 512);
		ReadFile(fd, pBuffer2, 512);
		
		if (strcmp(pBuffer1, pBuffer2))
		{
			printf("TestCase 3: error\n");
			exit(0);
		}
		CloseFile(fd);
	}
	printf(" ---- Test Case 3: files written/read----\n");
	ListDirContents("/home/user7", 9);
}


void TestCase4(void)
{
	int i;
	char fileName[NAME_LEN_MAX];
	char pBuffer[1024];
	int fd;

	printf(" ---- Test Case 4 ----\n");
	for (i = 0;i < 9;i++)
	{
		if (i%2 == 0)
		{
			memset(fileName, 0, NAME_LEN_MAX);
			sprintf(fileName, "/home/user7/file%d", i);
			RemoveFile(fileName);
		}
	}
	printf(" ---- Test Case 4: files of even number removed ----\n");

	for (i = 0;i < 9;i++)
	{
		if (i%2)
		{
			memset(fileName, 0, NAME_LEN_MAX);
			sprintf(fileName, "/home/user7/file%d", i);
			fd = OpenFile(fileName, OPEN_FLAG_READWRITE);
			
			memset(pBuffer, 0, 1024);
			strcpy(pBuffer, fileName); 
			WriteFile(fd, pBuffer, 513);
			CloseFile(fd);
		}
	}

	printf(" ---- Test Case 4: files of odd number overwritten ----\n");
	ListDirContents("/home/user7", 9);

	for (i = 0;i < 9;i++)
	{
		if (i%2 == 0)
		{
			memset(fileName, 0, NAME_LEN_MAX);
			sprintf(fileName, "/home/user7/file%d", i);
			fd = OpenFile(fileName, OPEN_FLAG_CREATE);
			CloseFile(fd);
		}
	}
	printf(" ---- Test Case 4: files of even number re-created ----\n");
	ListDirContents("/home/user7", 9);
}


int main(int argc, char** argv)
{
	int TcNum;

	if (argc < 3)
	{
		printf("Usage: a.out [format | readwrite] TestCaseNumber\n");
		exit(0);
	}
	if (strcmp(argv[1], "format") == 0)
		Mount(MT_TYPE_FORMAT);
	else
		Mount(MT_TYPE_READWRITE);

	TcNum = atoi(argv[2]);

	switch (TcNum)
	{
	case 1:
		TestCase1();
		break;
	case 2:
		TestCase2();
		break;
	case 3:
		TestCase3();
		break;
	case 4:
		TestCase4();
		break;
	default:
		return -1;
	}
	Unmount();
	return 0;	
}
