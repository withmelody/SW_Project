#include "../header/project.h"

extern Position bottom_menu_pos;
static WINDOW* superblockWindow;

void createSuperBlockWindow() {
	superblockWindow = subwin(stdscr,0, terminal_screen_col/2, bottom_menu_pos.y + 3 , terminal_screen_col/2);
	wborder(superblockWindow, '#', '#', '#', '#', '#', '#', '#', '#');
	wrefresh(superblockWindow);
}

void printSuperBlock(SuperBlk_t* sb) {

	mvwprintw(superblockWindow, 5, 15,  "Disk FD : %d", sb->fsi.s_disk_fd); 
	mvwprintw(superblockWindow, 6, 15, "Block Size : %d", sb->fsi.s_nblk); 
	mvwprintw(superblockWindow, 8, 15,  "Total Blocks : %d", sb->fsi.s_ndatablk); 
	mvwprintw(superblockWindow, 8, 44, "(Used) / (Free) : %6d / %6d  (%.2f)%%", 
															sb->fsi.s_nblk_use, sb->fsi.s_nblk_free, 100 * ((float)  sb->fsi.s_nblk_use/ (float)sb->fsi.s_nblk_free)); 
	mvwprintw(superblockWindow, 9, 15,  "Total Inodes : %d", sb->fsi.s_ninode);
	mvwprintw(superblockWindow, 9, 44, "(Used) / (Free) : %6d / %6d  (%.2f)%%", 
															sb->fsi.s_ninode_use, sb->fsi.s_ninode_free, 100 * ((float)sb->fsi.s_ninode_use / (float)sb->fsi.s_ninode_free)); 
	
	wrefresh(superblockWindow);
	update_DisplayBar(sb);
}
