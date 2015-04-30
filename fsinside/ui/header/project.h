#ifndef __PROJECT_H__
#define __PROJECT_H__

#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "color.h"
#include "define_mode.h"
#include "windows_info.h"
#include "diskInfo.h"
#include "../messageQ/msglib.h"
#include "../tinyfs.h"

#define true 1
#define false 0

#define THREAD_LOCK   {pthread_mutex_lock(&curses_mutex);}
#define THREAD_UNLOCK {pthread_mutex_unlock(&curses_mutex);}

int PROGRAM_EXIT_FLAG;
int CURRENT_MODE;

typedef struct __FB{
	int isUse;
	int isReading;
	int isWriting;
	int isLocked;
	int isRemoved;
	Position pos;
} FileBlock;

typedef struct __FS{
	FileBlock* blocks;
} FileStructure;

FileBlock blocks[NUM_OF_BLOCKS];

typedef enum __BlockInfo{
	UNUSED,
	READING,
	WRITING,
	LOCKED
} BlockInfo;

InodeBitmap_t ibm;
BlockBitmap_t bbm;

int displayDisk_width;
int displayDisk_height;

pthread_mutex_t curses_mutex;

#endif // __PROJECT_H__
