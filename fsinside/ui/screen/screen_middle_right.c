#include "../header/project.h"
#include "../messageQ/msglib.h"

void* middle_right_screen_init(void* nouse) {
	int i, j, ret;
	int windowW = terminal_screen_col/2;
	int windowH = terminal_screen_row/2;
	init_pair(60, COLOR_RED,     COLOR_RED);

	displayDisk_width  = windowW - 2;
	displayDisk_height = windowH - 2;
	superblock_win = subwin(stdscr, windowH, windowW, 11, windowW);
	wborder(superblock_win, '#', '#', '#', '#', '#', '#', '#', '#');

	wbkgd(superblock_win, 0);

	wrefresh(superblock_win);

}

void displayBar(int);

void print_superblock(SuperBlk_t* sb) {
	int y ,x;
	int disk_space = sb->fsi.s_ndatablk;
	int disk_usage = sb->fsi.s_nblk_use;
	int disk_free  = sb->fsi.s_nblk_free;
	displayBar( (int) (100 * ( (float) disk_usage / (float) disk_space )) );
/*
	mvwprintw(superblock_win, 3,  3, "%d %d %d", disk_space, disk_usage, disk_free);  
	getmaxyx(display_diskUsage, y, x);
	mvwprintw(superblock_win, 4,  3, "%d %d", x, y);
	getyx(display_diskUsage, y, x);
	mvwprintw(superblock_win, 5,  3, "%d %d BEFORE : ",x, y);
	 wmove(display_diskUsage, 1, 1);
getyx(display_diskUsage, y, x);
	mvwprintw(superblock_win, 6,  3, "%d %d BEFORE : ",x, y);
	 
	displayBar(disk_usage);
*/
	// Update disk space bar
	THREAD_LOCK;
	wrefresh(superblock_win);
	THREAD_UNLOCK;
}


