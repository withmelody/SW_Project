#ifndef __WINDOWS_INFO_H__
#define __WINDOWS_INFO_H__

#define MENU_SELECT_MAX 5

#define SUBMENU_1_SELECT_MAX 3
#define SUBMENU_2_SELECT_MAX 3
#define SUBMENU_3_SELECT_MAX 5

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

// Submenus
WINDOW* submenu1[SUBMENU_1_SELECT_MAX];
WINDOW* submenu2[SUBMENU_2_SELECT_MAX];
WINDOW* submenu3[SUBMENU_3_SELECT_MAX];



#endif // __WINDOWS_INFO_H__
