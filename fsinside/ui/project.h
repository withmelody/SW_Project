#ifndef __PROJECT_H__
#define __PROJECT_H__

#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "color.h"
#include "windows_info.h"

#define true 1
#define false 0

#define FILE_SYSTEM_MAX_SIZE 8*1024*1024

typedef struct __FB{
	int isUse;
	Position pos;
} FileBlock;

typedef struct __FS{
	FileBlock* blocks;
} FileStructure;

FileBlock blocks[128];

typedef enum __BlockInfo{
	UNUSED,
	READING,
	WRITING,
	LOCKED
} BlockInfo;

int displayDisk_width;
int displayDisk_height;

#endif // __PROJECT_H__
