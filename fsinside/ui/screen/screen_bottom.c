#include "../header/project.h" 

extern Position bottom_menu_pos;

char* menu_str[] = {
	// Must contain blank
	" FSinside(f) ",
	" Edit(e)     ",
	" Mode(o)     ",
	" About(a)    ",
	" Exit(x)     "
};

char* submenu1_str[] = {
	" Open        ",
	" Save        ",
	" About       ",
};

char* submenu2_str[] = {
	" Copy        ",
	" Cut         ",
	" Paste       ",
};

char* submenu3_str[] = {
	" All Inodes  ",
	" All Blocks  ",
	" One Inode   ",
	" One Block   ",
	" Cache       ",
};

void create_submenu() {//int setflag, int menu_index) {
	int i;
	const int bottom_menu_y = 5;
	const int bottom_menu_x_distance = 20;

	init_pair(44, COLOR_RED, COLOR_GREEN);

	box(submenu1[0], 0, 0);
	for(i=0; i<SUBMENU_1_SELECT_MAX; i++) {
		submenu1[i]= subwin(stdscr, 1, 15, bottom_menu_pos.y + 10 + i, 5);
		mvwprintw(submenu1[i], 0, 1, "%s", submenu1_str[i]);
		wrefresh(submenu1[i]);
	}
	wbkgd(submenu1[0], COLOR_PAIR(44));

	box(submenu2[0], 0, 0);
	for(i=0; i<SUBMENU_2_SELECT_MAX; i++) {
		submenu2[i]= subwin(stdscr, 1, 15, bottom_menu_pos.y + 10 + i, 5 +  bottom_menu_x_distance);
		mvwprintw(submenu2[i], 0, 1, "%s", submenu2_str[i]);
		wrefresh(submenu2[i]);
	}

	box(submenu3[0], 0, 0);
	for(i=0; i<SUBMENU_3_SELECT_MAX; i++) {
		submenu3[i]= subwin(stdscr, 1, 15, bottom_menu_pos.y + 10 + i, 5 +  bottom_menu_x_distance * 2);
		mvwprintw(submenu3[i], 0, 1, "%s", submenu3_str[i]);
		wrefresh(submenu3[i]);
	}
}

void menu_color_init() {
	//        Status         Font         Background
	//  init_pair(COLOR_UNUSED,  COLOR_BLACK, COLOR_BLACK);
}


// BOTTOM MENU CREATE
void menu_create() {
	box(bottom_menu, 0,0);
//	wbkgd(bottom_menu, A_REVERSE);

	const int bottom_menu_y = 5;
	const int bottom_menu_x_distance = 20;
	int i, j;
	int item_width = 15;

	init_pair(MN_COLOR_SELECT, COLOR_RED, COLOR_GREEN);
	for(i=0; i<MENU_SELECT_MAX; i++) {
		select_menu[i] = subwin(stdscr, 3,  item_width, bottom_menu_pos.y + 6, 5 + bottom_menu_x_distance * i);
		wborder(select_menu[i], ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
		mvwprintw(select_menu[i], 1, 1, menu_str[i]);
	}
	wbkgd(select_menu[0], COLOR_PAIR(MN_COLOR_SELECT));
}

void displayMenu() {
	int i;
	chtype ch_border = '#';
	bottom_menu = subwin(stdscr,0, terminal_screen_col, bottom_menu_pos.y , 0);
	menu_color_init();
	menu_create();
	wborder(bottom_menu, '#', '#', '#', '#', '#', '#', '#', '#');
	for (i=0; i<terminal_screen_col; i++)
		mvwaddch(bottom_menu, 3, i, ch_border);

	create_submenu();
	wrefresh(bottom_menu);
}

void diskUsage_color_init() {
	int i;

	//        Status         Font         Background
	init_pair(DU_COLOR_FREE,  COLOR_WHITE, COLOR_BLUE );
	init_pair(DU_COLOR_USAGE, COLOR_WHITE, COLOR_MAGENTA);
}

void displayBar(int usage, int bar_width) {
	int i;

	chtype ch_usage = ' ';
	chtype ch_free  = ' ';
	addch(ch_usage | COLOR_PAIR(DU_COLOR_USAGE));
	addch(ch_free  | COLOR_PAIR(DU_COLOR_FREE ));

	int usage_percent = bar_width * ((double) usage / 100);

	//  usage_percent = 10;

	wattron(display_diskUsage, COLOR_PAIR(DU_COLOR_FREE));
	for (i=0; i<usage_percent; i++) {
		mvwaddch(display_diskUsage, 0, i, ch_usage);
		mvwaddch(display_diskUsage, 1, i, ch_usage);
	}
	//  mvwprintw(display_diskUsage, 1, bar_width / 2, "feawfeawfeawfaew%d %d %f", usage_percent, bar_width, usage_percent/(float)bar_width);
	wattroff(display_diskUsage, COLOR_PAIR(DU_COLOR_FREE));

	wattron(display_diskUsage, COLOR_PAIR(DU_COLOR_USAGE));
	for(; i<bar_width; i++) {
		mvwaddch(display_diskUsage, 0, i, ch_free);
		mvwaddch(display_diskUsage, 1, i, ch_free);
	}
	wattroff(display_diskUsage, COLOR_PAIR(DU_COLOR_USAGE));

}

void displayDiskUsage() {
	int usage, free;        // Usage / free disk space (percent)
	int i;
	int bar_width = terminal_screen_col - 2;
	display_diskUsage = subwin(stdscr, 3, bar_width, bottom_menu_pos.y+1 , 1);
	diskUsage_color_init();

	// for test
	displayBar(68, bar_width);

	wrefresh(display_diskUsage);
}
