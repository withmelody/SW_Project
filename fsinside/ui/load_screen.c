#include "header/project.h"
#include <pthread.h>

Position bottom_menu_pos = {0, 0};

void  input_select_menu();
void* updateTime(void*);
void* display_disk_info(void*);

int load_screen() {
// for teset
	srand(time(NULL));
	pthread_t timer_thread, screen_thread, controller_thread;
	int ret;

	printLogo();
	ret = pthread_create(&timer_thread, 0, updateTime, 0);	
	ret = pthread_create(&screen_thread, 0, display_disk_info, 0);

	// Infinity loop
	input_select_menu();

	return 0;
}

void* display_disk_info(void* nouse) {

	displayDisk();
	displayMenu();
	displayDiskUsage();
	printLogo();
}

void refreshAll() {
	wrefresh(top_clock);
	wrefresh(disk_screen);
	wrefresh(displayDisk_frame);
	wrefresh(bottom_menu);
	wrefresh(display_diskUsage);
}
