#include "header/project.h"
#include <pthread.h>

Position bottom_menu_pos = {0, 0};

// flushinp

void  input_select_menu();
void* updateTime(void*);
void* display_disk_info(void*);
void* middle_right_screen_init(void*);
void* messageReceiver(void*);

int load_screen() {
// for teset
	srand(time(NULL));
	pthread_t timer_thread, disk_screen_thread, info_thread, message_thread;
	int ret;

	ret = pthread_create(&timer_thread, 0, updateTime, 0);	
	ret = pthread_create(&disk_screen_thread, 0, display_disk_info, 0);
	ret = pthread_create(&info_thread, 0, middle_right_screen_init, 0);
	ret = pthread_create(&message_thread, 0, messageReceiver, 0);

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
/*
void refreshAll() {
	wrefresh(top_clock);
	wrefresh(disk_screen);
	wrefresh(displayDisk_frame);
	wrefresh(bottom_menu);
	wrefresh(display_diskUsage);
}
*/
