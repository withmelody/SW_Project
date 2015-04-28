#include "header/project.h"


void exit_fsinside() {
	endwin();
	exit(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Change menu color
void sub_change_select_menu_color(int selected, int sub_selected);
void change_select_menu_color(int selected, int sub_selected) {
	int i;

	//        Status           Font         Background
	init_pair(MN_COLOR_SELECT, COLOR_RED, COLOR_GREEN);

	for(i=0; i<MENU_SELECT_MAX; i++) {
		if (i == selected) {
			wbkgd(select_menu[i], COLOR_PAIR(MN_COLOR_SELECT));
		}
		else {
			wbkgd(select_menu[i], 0);
		}
		wrefresh(select_menu[i]);
	}

	sub_change_select_menu_color(selected, sub_selected);

	wrefresh(bottom_menu);
}

void sub_change_select_menu_color(int selected, int sub_selected) {
	// Initialize all submenus
	int i;
	for(i=0; i<SUBMENU_1_SELECT_MAX; i++) {
		wbkgd(submenu1[i], 0);
		wrefresh(submenu1[i]);
	}
	for(i=0; i<SUBMENU_2_SELECT_MAX; i++) {
		wbkgd(submenu2[i], 0);
		wrefresh(submenu2[i]);
	}
	for(i=0; i<SUBMENU_3_SELECT_MAX; i++) {
		wbkgd(submenu3[i], 0);
		wrefresh(submenu3[i]);
	}

	switch(selected) {
		case 0:	
			for(i=0; i<SUBMENU_1_SELECT_MAX; i++) {
				if (sub_selected == i)
					wbkgd(submenu1[sub_selected], COLOR_PAIR(MN_COLOR_SELECT));
				else
					wbkgd(submenu1[i], 0);
				wrefresh(submenu1[i]);
			}
			break;
		case 1:	
			for(i=0; i<SUBMENU_2_SELECT_MAX; i++) {
				if (sub_selected == i)
					wbkgd(submenu2[sub_selected], COLOR_PAIR(MN_COLOR_SELECT));
				else
					wbkgd(submenu2[i], 0);
				wrefresh(submenu2[i]);
			}
			break;
		case 2:	
			for(i=0; i<SUBMENU_3_SELECT_MAX; i++) {
				if (sub_selected == i)
					wbkgd(submenu3[sub_selected], COLOR_PAIR(MN_COLOR_SELECT));
				else
					wbkgd(submenu3[i], 0);
				wrefresh(submenu3[i]);
			}
			break;

	}

//	mvwprintw(stdscr, 3, 3, "%d %d", selected, sub_selected);

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////


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
	int sub_selected = 0;
	int run_flag = false;
	while(1) {
		run_flag = false;
		switch ( key = getch() ) {
			case KEY_LEFT:
				sub_selected = 0;
				selected--;
				if (selected == -1) selected = MENU_SELECT_MAX - 1;
				break;
			case KEY_RIGHT:
				sub_selected = 0;
				selected++;
				if (selected == MENU_SELECT_MAX) selected = 0;
				break;
			case KEY_UP:
				sub_selected--;
				break;
			case KEY_DOWN:
				sub_selected++;
				break;
			case KEY_ENTER:
			case '\n':
				run_flag = true;;
				break;
			case 'f':
			case 'F':
				sub_selected = 0;
				run_flag = true;
				selected = 0;
				break;
			case 'e':
			case 'E':
				sub_selected = 0;
				run_flag = true;
				selected = 1;
				break;
			case 'o':
			case 'O':
				sub_selected = 0;
				run_flag = true;
				selected = 2;
				break;
			case 'a':
			case 'A':
				sub_selected = 0;
				run_flag = true;
				selected = 3;
				break;
			case 'x':
			case 'X':
				sub_selected = 0;
				run_flag = true;;
				selected = 4;
				break;
			default:
				break;
		}


		switch(selected) {
			case 0:
				if (sub_selected < 0) sub_selected = SUBMENU_1_SELECT_MAX - 1;
				else if (sub_selected == SUBMENU_1_SELECT_MAX) sub_selected = 0;
				break;
			case 1:
				if (sub_selected < 0) sub_selected = SUBMENU_2_SELECT_MAX - 1;
				else if (sub_selected == SUBMENU_2_SELECT_MAX) sub_selected = 0;
				break;
			case 2:
				if (sub_selected < 0) sub_selected = SUBMENU_3_SELECT_MAX - 1;
				else if (sub_selected == SUBMENU_3_SELECT_MAX) sub_selected = 0;
				break;
		}
		change_select_menu_color(selected, sub_selected);
		if (run_flag)
			do_selected_menu(selected);

	}
}
