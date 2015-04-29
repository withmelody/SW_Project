#include "../header/project.h"
#include <pthread.h>
extern isAboutOpen;
extern Position bottom_menu_pos;
void* refresh_bitmap_screen(void*);

void fs_color_init() {
	//        Status         Font         Background
	init_pair(FS_COLOR_UNUSED,  COLOR_WHITE, COLOR_BLACK);
	init_pair(FS_COLOR_READING, COLOR_GREEN, COLOR_GREEN);
	init_pair(FS_COLOR_WRITING, COLOR_BLUE,  COLOR_BLUE);
	init_pair(FS_COLOR_LOCKED,  COLOR_RED,   COLOR_RED);
	init_pair(FS_COLOR_IDLE  ,  COLOR_WHITE, COLOR_WHITE);
}


int displayDisk() {
	int i, j, ret;
	int windowW = terminal_screen_col/2;
	int windowH = terminal_screen_row/2;
	pthread_t disk_screen_thread;

	displayDisk_width  = windowW - 2;
	displayDisk_height = windowH - 2;
	displayDisk_frame = subwin(stdscr, windowH, windowW, 11, 0);
	wborder(displayDisk_frame, '#', '#', '#', '#', '#', '#', '#', '#');
	disk_screen = subwin(stdscr,windowH-2, windowW-2, 12, 1);
	start_color();
	fs_color_init();

	box(top_clock, 0, 0);

	ret = pthread_create(&disk_screen_thread, 0, refresh_bitmap_screen, 0);

	// Save bottom window position
	bottom_menu_pos.y = displayDisk_height + 13;
}

//////////////////////////////////////////////////
// Display colored blocks
void* refresh_bitmap_screen(void* nouse) {
	int i, j, k, index, tmp;
	int windowW, windowH;
	struct timespec ts = {0, 5000000};
	getmaxyx(disk_screen, windowH, windowW);

	for(index = 0; index < NUM_OF_BLOCKS; index++)
		blocks[index].isUse = blocks[index].isReading = blocks[index].isWriting = blocks[index].isLocked = false;

	/////////////////////////////
	// test code
	while(1) {

		if (isAboutOpen)
			continue;

		werase(disk_screen);

		////////////////////////////////////////////////////////////////////////////////////
		//
		// Display Inode bitmap
		//
		// Use global value : ibm
		if (CURRENT_MODE == DISPLAY_INODE_BITMAP) {

			//mvwprintw(disk_screen, 3, 3, " INODE : [[ %d ]]    ", ibm.size);
			//	wrefresh(disk_screen);
			//	continue;

			for(i=0; i< windowH; i++) {
				for(j=0;j< windowW; j++) {
					index = j + (i * windowW);
					if (index >= ibm.size)
						break;

					for (k=0x01; k < 0x100; k = k << 1) {
						if ( (k & ibm.s_ibitmap_ptr[index]) != 0 ) {
							wattron(disk_screen, COLOR_PAIR(FS_INODE_COLOR_UNUSED));
							//mvwprintw(disk_screen, i, j, " ");
							wprintw(disk_screen, "1");
							wattroff(disk_screen, COLOR_PAIR(FS_INODE_COLOR_UNUSED));
						}
						else {
							wattron(disk_screen, COLOR_PAIR(FS_INODE_COLOR_USED));
							//mvwprintw(disk_screen, i, j, " ");
							wprintw(disk_screen, "0");
							wattroff(disk_screen, COLOR_PAIR(FS_INODE_COLOR_USED));
						}
					}

					//	wprintw(disk_screen, "[%d] %2X ", index, (char) ibm.s_ibitmap_ptr[index]);
				}
			}

			wrefresh(disk_screen);
		}

		////////////////////////////////////////////////////////////////////////////////////
		//
		// Display Block Bitmap
		//
		// Use global value : bbm

		else if (CURRENT_MODE == DISPLAY_BLOCK_BITMAP) {
			// Block Initialize (Used / Free)
			/*
			   mvwprintw(disk_screen, 3, 3, " BLOCK : [[ %d ]] ", bbm.size);
			   wrefresh(disk_screen);
			   continue;
			 */
			for(i=0; i<bbm.size; i++) {
				tmp = 0;
				for (k=0x01; k < 0x100; k = k << 1) {
					index = (i*8) + tmp;
					tmp++;

					// Initialize All temp blocks
			//		blocks[index].isUse = blocks[index].isReading = blocks[index].isWriting = blocks[index].isLocked = false;

					if  ( (k & bbm.s_dbitmap_ptr[i]) == 0) {
						blocks[index].isUse = true;
					}
					else
						blocks[index].isUse = false;
				}
			}

			//////////////////////////////
			// n block = 1024
			// screen = 2400
			for(i=0; i<windowH; i++) {
				for(j=0;j<windowW; j++) {
					index = j + (i * windowW);
					if (index >= bbm.size)
						break;
					// Free block
					if (blocks[index].isUse == false) {
						wattron(disk_screen, COLOR_PAIR(FS_COLOR_UNUSED));
						mvwprintw(disk_screen, i, j, ".");
						wattroff(disk_screen, COLOR_PAIR(FS_COLOR_UNUSED));
					}
					// Used block
					else {
						wattron(disk_screen, COLOR_PAIR(FS_COLOR_IDLE));
						mvwprintw(disk_screen, i, j, " ");
						wattroff(disk_screen, COLOR_PAIR(FS_COLOR_IDLE));
					}
					if (blocks[index].isLocked) {
						wattron(disk_screen, COLOR_PAIR(FS_COLOR_LOCKED));
						mvwprintw(disk_screen, i, j, " ");
						wattroff(disk_screen, COLOR_PAIR(FS_COLOR_LOCKED));
					}
					if (blocks[index].isReading) {
						wattron(disk_screen, COLOR_PAIR(FS_COLOR_READING));
						mvwprintw(disk_screen, i, j, " ");
						wattroff(disk_screen, COLOR_PAIR(FS_COLOR_READING));
					}
					if (blocks[index].isWriting) {
						wattron(disk_screen, COLOR_PAIR(FS_COLOR_WRITING));
						mvwprintw(disk_screen, i, j, " ");
						wattroff(disk_screen, COLOR_PAIR(FS_COLOR_WRITING));
					}
				}
			}
			wrefresh(displayDisk_frame);
			wrefresh(disk_screen);
		}
		nanosleep(&ts, NULL);
	}
}
