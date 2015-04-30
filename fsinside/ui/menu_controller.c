#include "header/project.h"


#define EXIT_POPUP_WIDTH (terminal_screen_col/4-8)

extern int isDiskChanged;
static int exit_flag;

void exit_fsinside() {
	int key;
	int isExitSelected = true;
	exit_flag = true;

	WINDOW* exit_popup;
	exit_popup = newwin(11, terminal_screen_col / 4, terminal_screen_row - 15, terminal_screen_col / 4 + terminal_screen_col / 8);
	werase(exit_popup);
	box(exit_popup, 0, 0);
	wborder(exit_popup, '|', '|', '-', '-', '+', '+', '+', '+');

	init_pair(EXIT_COLOR_BKGD, COLOR_BLACK, COLOR_YELLOW);
	wbkgd(exit_popup, COLOR_PAIR(EXIT_COLOR_BKGD));
	mvwprintw(exit_popup, 3, EXIT_POPUP_WIDTH/2 - 3, "Exit program?");

	wattron(exit_popup, COLOR_PAIR( MN_COLOR_SELECT) );
	mvwprintw(exit_popup, 5, EXIT_POPUP_WIDTH/2 - 7, "         ");
	mvwprintw(exit_popup, 6, EXIT_POPUP_WIDTH/2 - 7, "   Yes   ");
	mvwprintw(exit_popup, 7, EXIT_POPUP_WIDTH/2 - 7, "         ");

	wattroff(exit_popup, COLOR_PAIR( MN_COLOR_SELECT) );
	mvwprintw(exit_popup, 5, EXIT_POPUP_WIDTH/2 + 7, "        ");
	mvwprintw(exit_popup, 6, EXIT_POPUP_WIDTH/2 + 7, "   No   ");
	mvwprintw(exit_popup, 7, EXIT_POPUP_WIDTH/2 + 7, "        ");

	touchwin(exit_popup);
	wrefresh(exit_popup);

	while(1) {
		switch ( key = getch() ) {
			case KEY_LEFT:
			case KEY_RIGHT:
				if (isExitSelected) isExitSelected = false;
				else                isExitSelected = true;
				break;
			case 'y':
			case 'Y':
				goto _EXIT_YES_SELECTED_;
				break;
			case 'n':
			case 'N':
				goto _EXIT_NO_SELECTED_;
				break;
			case KEY_ENTER:
			case '\n':
				if (isExitSelected) {
					goto _EXIT_YES_SELECTED_;
				}
				else {
					goto _EXIT_NO_SELECTED_;
				}
			default:
				break;
		}
		// Paint selected item
		if (isExitSelected) {
			wattron(exit_popup, COLOR_PAIR( MN_COLOR_SELECT) );
			mvwprintw(exit_popup, 5, EXIT_POPUP_WIDTH/2 - 7, "         ");
			mvwprintw(exit_popup, 6, EXIT_POPUP_WIDTH/2 - 7, "   Yes   ");
			mvwprintw(exit_popup, 7, EXIT_POPUP_WIDTH/2 - 7, "         ");

			wattroff(exit_popup, COLOR_PAIR( MN_COLOR_SELECT) );
			mvwprintw(exit_popup, 5, EXIT_POPUP_WIDTH/2 + 7, "        ");
			mvwprintw(exit_popup, 6, EXIT_POPUP_WIDTH/2 + 7, "   No   ");
			mvwprintw(exit_popup, 7, EXIT_POPUP_WIDTH/2 + 7, "        ");
		}
		else {
			mvwprintw(exit_popup, 5, EXIT_POPUP_WIDTH/2 - 7, "         ");
			mvwprintw(exit_popup, 6, EXIT_POPUP_WIDTH/2 - 7, "   Yes   ");
			mvwprintw(exit_popup, 7, EXIT_POPUP_WIDTH/2 - 7, "         ");
			wattron(exit_popup, COLOR_PAIR( MN_COLOR_SELECT) );

			mvwprintw(exit_popup, 5, EXIT_POPUP_WIDTH/2 + 7, "        ");
			mvwprintw(exit_popup, 6, EXIT_POPUP_WIDTH/2 + 7, "   No   ");
			mvwprintw(exit_popup, 7, EXIT_POPUP_WIDTH/2 + 7, "        ");
			wattroff(exit_popup, COLOR_PAIR( MN_COLOR_SELECT) );

		}
		wrefresh(exit_popup);
	}
_EXIT_YES_SELECTED_:
	PROGRAM_EXIT_FLAG = true;
	endwin();
	exit(0);

_EXIT_NO_SELECTED_:
	wclear(exit_popup);
	delwin(exit_popup);
	redrawwin(stdscr);//top_clock);
	exit_flag = false;
	return;

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
	/*
	for(i=0; i<SUBMENU_2_SELECT_MAX; i++) {
		wbkgd(submenu2[i], 0);
		wrefresh(submenu2[i]);
	}
*/
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
			/*
		case 1:	
			for(i=0; i<SUBMENU_2_SELECT_MAX; i++) {
				if (sub_selected == i)
					wbkgd(submenu2[sub_selected], COLOR_PAIR(MN_COLOR_SELECT));
				else
					wbkgd(submenu2[i], 0);
				wrefresh(submenu2[i]);
			}
			break;
			*/
	}

	//	mvwprintw(stdscr, 3, 3, "%d %d", selected, sub_selected);

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void set_show_bitmap(int sub_selected) {
	if (sub_selected == 0) 
		CURRENT_MODE = DISPLAY_INODE_BITMAP;
	else if (sub_selected == 1)
		CURRENT_MODE = DISPLAY_BLOCK_BITMAP;
	isDiskChanged = true;
}

void do_selected_menu(int selected, int sub_selected) {
	switch( selected ) {
		case 0:
			set_show_bitmap(sub_selected);
			break;
		case 2:
			about();
			break;
		case 3:
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
				run_flag = true;
				break;
			case 'b':
			case 'B':
				sub_selected = 0;
				run_flag = true;
				selected = 0;
				break;
			case 'C':
			case 'c':
				sub_selected = 0;
				run_flag = true;
				selected = 1;
				break;
			case 'a':
			case 'A':
				sub_selected = 0;
				run_flag = true;
				selected = 2;
				break;
			case 'x':
			case 'X':
				sub_selected = 0;
				run_flag = true;;
				selected = 3;
				break;
			default:
				break;
		}


		switch(selected) {
			case 0:
				if (sub_selected < 0) sub_selected = SUBMENU_1_SELECT_MAX - 1;
				else if (sub_selected == SUBMENU_1_SELECT_MAX) sub_selected = 0;
				break;
	/*
			case 1:
				if (sub_selected < 0) sub_selected = SUBMENU_2_SELECT_MAX - 1;
				else if (sub_selected == SUBMENU_2_SELECT_MAX) sub_selected = 0;
				break;
				*/
		}
		change_select_menu_color(selected, sub_selected);
		if (run_flag)
			do_selected_menu(selected, sub_selected);

	}
}
