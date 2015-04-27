#include "project.h"
#include <pthread.h>

#define LOGO_WIDTH (terminal_screen_col-2 - 116)

static Position bottom_menu_pos = {0, 0};

char* menu_str[] = {
	// Must contain blank
	" FSinside(f) ",
	" Edit(e)     ",
	" Options(o)  ",
	" About(a)    ",
	" Exit(x)     "
};


void printLogo();
void displayMenu();
void displayDiskUsage();
void input_select_menu();
void* updateTime(void*);

int load_screen() {

	pthread_t timer_thread, controller_thread;
	int ret;

	displayDisk();
	displayMenu();
	displayDiskUsage();
	printLogo();

	ret = pthread_create(&timer_thread, 0, updateTime, 0);	

	// Infinity loop
	input_select_menu();

	return 0;
}

void* updateTime(void* nouse) {
	time_t      tloc;
	struct tm   *tp;
	char        str_date[20];
	char        str_time[20];
	int x, y;

	getmaxyx(top_clock, y, x);
	while(1) {
		time (&tloc);
		tp = localtime(&tloc);
		strftime(str_date, 20, "%Y/%m/%d", tp);
		strftime(str_time, 20, "%H:%M:%S", tp);

		mvwprintw(top_clock, 2 , x - 15, "%s", str_date);
		mvwprintw(top_clock, 3 , x - 13, "%s", str_time);
		wrefresh(top_clock);
		sleep(1);
	}
}

void refreshAll() {
	wrefresh(top_clock);
	wrefresh(disk_screen);
	wrefresh(displayDisk_frame);
	wrefresh(bottom_menu);
	wrefresh(display_diskUsage);
}

void exit_fsinside() {
	endwin();
	exit(0);
}

void change_select_menu_color(int selected) {
	int i;

	//        Status           Font         Background
	init_pair(MN_COLOR_SELECT, COLOR_RED, COLOR_GREEN);

	for(i=0; i<MENU_SELECT_MAX; i++) {
		if (i == selected) {
			wbkgd(select_menu[i], A_BOLD | COLOR_PAIR(MN_COLOR_SELECT));
		}
		else {
			wbkgd(select_menu[i], 0);
		}
		wrefresh(select_menu[i]);
	}
	mvwprintw(top_clock, 1 , 1, "######### %d", selected);
	wrefresh(top_clock);
}


void do_selected_menu(int selected) {
	switch( selected ) {
		case 3:
			about();
			break;
		case 4:
			exit_fsinside();
			break;
	}

}

void input_select_menu() {
	int key;
	int selected = 0;
	while(1) {
		halfdelay(100);
		switch ( key = getch() ) {
			case KEY_LEFT:
				selected--;
				if (selected == -1) selected = MENU_SELECT_MAX - 1;
				break;
			case KEY_RIGHT:
				selected++;
				if (selected == MENU_SELECT_MAX) selected = 0;
				break;
			case KEY_ENTER:
			case '\n':
				do_selected_menu(selected);
				break;
			case 'a':
			case 'A':
				selected = 3;
				about();
				break;
			case 'x':
			case 'X':
				selected = 4;
				exit_fsinside();
				break;
			default:
				mvwprintw(top_clock, 3 , 3, "######### %d", key);
				continue;
		}
		change_select_menu_color(selected);

	}
}

void fs_color_init() {
	int i;

	//        Status         Font         Background
	init_pair(FS_COLOR_UNUSED,  COLOR_BLACK, COLOR_BLACK);
	init_pair(FS_COLOR_READING, COLOR_GREEN, COLOR_GREEN);
	init_pair(FS_COLOR_WRITING, COLOR_BLUE,  COLOR_BLUE);
	init_pair(FS_COLOR_LOCKED,  COLOR_RED,   COLOR_RED);
}

void printLogo() {
	int left_black = (LOGO_WIDTH) / 2;
	top_clock = subwin(stdscr,11, terminal_screen_col, 0, 0);
	box(top_clock, 0, 0);
	wborder(top_clock, '|', '|', '-', '-', '+', '+', '+', '+');
	mvwprintw(top_clock,	1	, left_black,     "	      :+++++++++++++/   -+ossssssssso.`+++++/                                   `:::::.          `:::::-                	");
	mvwprintw(top_clock,	2	, left_black,     "	     `mMMMMMMMMMMMMN+ .dMMMMMMMMMMMMd`sMMMMM/                                   hMMMMN.          /MMMMMo                	");
	mvwprintw(top_clock,	3	, left_black,     "	     yMMMMMsooooo++/ `dMMMMdooooooo+./ooooo- -::::-.::::::-`    `-::::::::::- `:ooooo.  .:::::::.NMMMMd   .::::::::::-` 	");
	mvwprintw(top_clock,	4	, left_black,     "	    /NMMMMs          yMMMMN/.....`  -NMMMMd``mMMMMNddmNMMMMm` :hNMMNddddddmNs /NMMMMs -dMMMMNddmNMMMMm.`omMMMNddmMMMMMd`	");
	mvwprintw(top_clock,	5	, left_black,     "	   -NMMMMMdhhhhhhy` +MMMMMMMMMMMMN+`dMMMMN- yMMMMN-  .mMMMMh`:NMMMMNhhhhhhs-`.mMMMMd`-NMMMMy`  sMMMMN/ hMMMMN-``-NMMMMs 	");
	mvwprintw(top_clock,	6	, left_black,     "	  `dMMMMMMMMMMMMM+  .syyyyyyyNMMMN.sMMMMN/ /NMMMM+  `hMMMMm.`dMMMMMMMMMMMMM+`hMMMMm.`dMMMMm`  :NMMMMs oMMMMMNNNNNMMMMd` 	");
	mvwprintw(top_clock,	7	, left_black,     "	  sMMMMN+```````` :s/:::::::hMMMN//MMMMMy -NMMMMy   oMMMMN/ `ohhhhhhdMMMMMh oMMMMN/ yMMMMN-  .mMMMMh`:NMMMMs.........`  	");
	mvwprintw(top_clock,	8	, left_black,     "	 +MMMMMy         .mMMMMMMMMMMMMNo-NMMMMm``dMMMMm`  :NMMMMs smhyyyyyydMMMNy`:NMMMMs .NMMMMNhyhNMMMMN. sMMMMMdyyyyyyhy.   	");
	mvwprintw(top_clock,	9	, left_black,     "	 .ooooo.         `/++++++++++/- `+ooooo: :+++++:  `/+++++. /+++++++++++:` `/+++++.  -+++++++:/++++/  `:++++++++++++. 	");


	wrefresh(top_clock);
}

int displayDisk() {
	int i, j;
	int windowW = terminal_screen_col/2;
	int windowH = terminal_screen_row/2;


	displayDisk_width  = windowW - 2;
	displayDisk_height = windowH - 2;
	displayDisk_frame = subwin(stdscr, windowH, windowW, 11, 0);
	wborder(displayDisk_frame, '#', '#', '#', '#', '#', '#', '#', '#');
	disk_screen = subwin(stdscr,windowH-2, windowW-2, 12, 1);
	start_color();
	fs_color_init();

	box(top_clock, 0, 0);

	chtype ch = ' ';
	addch(ch | COLOR_PAIR(FS_COLOR_READING)); 
	for(i=0; i<windowW; i++) {
		for(j=0;j<windowH; j++) {
			if (!blocks[i+j].isUse) {
				wattron(disk_screen, COLOR_PAIR(FS_COLOR_READING));
				mvwaddch(disk_screen, j, i, ch);
				wattroff(disk_screen, COLOR_PAIR(FS_COLOR_READING));
			}
			else {
				wattron(disk_screen, COLOR_PAIR(FS_COLOR_READING));
				mvwprintw(disk_screen, j, i, ".");
				wattroff(disk_screen, COLOR_PAIR(FS_COLOR_READING));
			}
		}
	}

	wrefresh(displayDisk_frame);
	wrefresh(disk_screen);

	// Save bottom window position
	bottom_menu_pos.y = displayDisk_height + 13;
}	

void menu_color_init() {
	//        Status         Font         Background
	//	init_pair(COLOR_UNUSED,  COLOR_BLACK, COLOR_BLACK);
}


// BOTTOM MENU CREATE
void menu_create() {
	box(bottom_menu, 0,0);
	wbkgd(bottom_menu, A_REVERSE);

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

	//	usage_percent = 10;

	wattron(display_diskUsage, COLOR_PAIR(DU_COLOR_FREE));
	for (i=0; i<usage_percent; i++) {
		mvwaddch(display_diskUsage, 0, i, ch_usage);
		mvwaddch(display_diskUsage, 1, i, ch_usage);
	}
	//	mvwprintw(display_diskUsage, 1, bar_width / 2, "feawfeawfeawfaew%d %d %f", usage_percent, bar_width, usage_percent/(float)bar_width);
	wattroff(display_diskUsage, COLOR_PAIR(DU_COLOR_FREE));

	wattron(display_diskUsage, COLOR_PAIR(DU_COLOR_USAGE));
	for(; i<bar_width; i++) {
		mvwaddch(display_diskUsage, 0, i, ch_free);
		mvwaddch(display_diskUsage, 1, i, ch_free);
	}
	wattroff(display_diskUsage, COLOR_PAIR(DU_COLOR_USAGE));

}

void displayDiskUsage() {
	int usage, free;		// Usage / free disk space (percent)
	int i;
	int bar_width = terminal_screen_col - 2;
	display_diskUsage = subwin(stdscr, 3, bar_width, bottom_menu_pos.y+1 , 1);
	diskUsage_color_init();

	// for test
	displayBar(68, bar_width);

	wrefresh(display_diskUsage);
}
