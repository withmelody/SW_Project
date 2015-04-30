#include "../header/project.h"
#include "../messageQ/msglib.h"

void* middle_right_screen_init(void* nouse) {
	int i, j, ret;
	int windowW = terminal_screen_col/2;
	int windowH = terminal_screen_row/2;
	init_pair(60, COLOR_RED,     COLOR_RED);

	displayDisk_width  = windowW - 2;
	displayDisk_height = windowH - 2;
	fileinfo_window = subwin(stdscr, windowH, windowW, 11, windowW);
	wborder(fileinfo_window, '#', '#', '#', '#', '#', '#', '#', '#');

	wbkgd(fileinfo_window, 0);

	wrefresh(fileinfo_window);

}

void printCurrentEvent(FileIO_t* fio) {
	int i = 0;
	int block_index;

	werase(fileinfo_window);
	wborder(fileinfo_window, '#', '#', '#', '#', '#', '#', '#', '#');

	mvwprintw(fileinfo_window, 6, 14,  "File I/O Information");
	switch(fio->flag) {
		case 'a':
			mvwprintw(fileinfo_window, 8, 14,  "Operation : Get Attribute");
			break;
		case 'r':
			mvwprintw(fileinfo_window, 8, 14,  "Operation : Read");
			break;
		case 'w':
			mvwprintw(fileinfo_window, 8, 14,  "Operation : Write");
			break;
		case 'd':
			mvwprintw(fileinfo_window, 8, 14,  "Operation : Remove");
			break;

			// Directory Operation

		case 'D':
			mvwprintw(fileinfo_window, 8, 14,  "Operation : Make Directory");
			break;
		case 'R':
			mvwprintw(fileinfo_window, 8, 14,  "Operation : Read Directory");
			break;

	}
	mvwprintw(fileinfo_window, 9, 14,  "Length : %d", fio->size);

	mvwprintw(fileinfo_window, 11, 14,  "Inode Number : %d", fio->dentry.inodeNum);
	mvwprintw(fileinfo_window, 12, 14,  "Inode Size : %d", fio->inode.i_size);
	mvwprintw(fileinfo_window, 13, 14,  "Number of using block(s) : %d", fio->inode.i_nblk);

	mvwprintw(fileinfo_window, 15, 14,  "Block Address :");
	while(i < 8) {
		block_index = fio->inode.i_block[i];
		if (block_index < 0)
			break;
		mvwprintw(fileinfo_window, 16, 14 + (10 * i), "[%06d]", block_index);
		i++;

	}

	wrefresh(fileinfo_window);
}
