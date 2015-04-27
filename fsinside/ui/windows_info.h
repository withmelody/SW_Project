#ifndef __WINDOWS_INFO_H__
#define __WINDOWS_INFO_H__

#define MENU_SELECT_MAX 5


typedef struct __position {
	int x;
	int y;
} Position;

int terminal_screen_row, terminal_screen_col;

int displayDisk_width;
int displayDisk_height;

// Top side - Logo, System time
WINDOW* top_clock;

// Left side - File system display
WINDOW* disk_screen;
WINDOW* displayDisk_frame;

// Right side - File Information

// Bottom side - Menus
WINDOW* bottom_menu;
WINDOW* display_diskUsage;
WINDOW* select_menu[MENU_SELECT_MAX];

#endif // __WINDOWS_INFO_H__
