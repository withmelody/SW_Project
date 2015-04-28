#include "../header/project.h"
#include <pthread.h>
extern isAboutOpen;
extern Position bottom_menu_pos;
void* refresh_disk_block_screen(void*);

void fs_color_init() {
	//        Status         Font         Background
	init_pair(FS_COLOR_UNUSED,  COLOR_BLACK, COLOR_BLACK);
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

	ret = pthread_create(&disk_screen_thread, 0, refresh_disk_block_screen, 0);

	// Save bottom window position
	bottom_menu_pos.y = displayDisk_height + 13;
}

void* refresh_disk_block_screen(void* nouse) {
	int i, j;
	int windowW, windowH;
	getmaxyx(disk_screen, windowH, windowW);

	/////////////////////////////
	// test code

	for(i=0; i<NUM_OF_BLOCKS; i++) {
		blocks[i].isUse = blocks[i].isReading = blocks[i].isWriting = blocks[i].isLocked = false;

		if (rand()%100 < 80) {
			blocks[i].isUse = true;
			if (rand() % 1000 < 10)
				blocks[i].isLocked = true;
		}
		else
			blocks[i].isUse = false;
	}
	while(1) {
		if (!isAboutOpen) {
			srand(time(NULL));

			for(i=0; i<NUM_OF_BLOCKS; i++) {
				blocks[i].isReading = blocks[i].isWriting = false;
				if (blocks[i].isUse && !blocks[i].isLocked) {
					if (rand() % 1000 < 10)
						blocks[i].isReading = true;
					else if (rand() % 1000 < 20)
						blocks[i].isWriting = true;
				}
			}
			//////////////////////////////
			// n block = 1024
			// screen = 2400

			for(i=0; i<windowW; i++) {
				for(j=0;j<windowH; j++) {
					// Free block
					if (!blocks[j+(i*windowH)].isUse) {
						wattron(disk_screen, COLOR_PAIR(FS_COLOR_UNUSED));
						mvwprintw(disk_screen, j, i, " ");
						wattroff(disk_screen, COLOR_PAIR(FS_COLOR_UNUSED));
					}
					// Used block
					else {
						wattron(disk_screen, COLOR_PAIR(FS_COLOR_IDLE));
						mvwprintw(disk_screen, j, i, " ");
						wattroff(disk_screen, COLOR_PAIR(FS_COLOR_IDLE));
					}
					if (blocks[j+(j*i)].isLocked) {
						wattron(disk_screen, COLOR_PAIR(FS_COLOR_LOCKED));
						mvwprintw(disk_screen, j, i, " ");
						wattroff(disk_screen, COLOR_PAIR(FS_COLOR_LOCKED));
					}
					if (blocks[j+(j*i)].isReading) {
						wattron(disk_screen, COLOR_PAIR(FS_COLOR_READING));
						mvwprintw(disk_screen, j, i, " ");
						wattroff(disk_screen, COLOR_PAIR(FS_COLOR_READING));
					}
					if (blocks[j+(j*i)].isWriting) {
						wattron(disk_screen, COLOR_PAIR(FS_COLOR_WRITING));
						mvwprintw(disk_screen, j, i, " ");
						wattroff(disk_screen, COLOR_PAIR(FS_COLOR_WRITING));
					}
				}
			}
		}

		wrefresh(displayDisk_frame);
		wrefresh(disk_screen);
		usleep(50 * 1000);
	}

}
