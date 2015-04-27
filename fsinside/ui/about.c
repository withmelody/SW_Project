#include "project.h"

void about() {
//	return;


	WINDOW* about_window = newwin(0, 0, 0, 0);
	box(about_window, 0, 0);
	wborder(about_window, '#', '#', '#', '#', '#', '#', '#', '#');
	touchwin(about_window);
	wrefresh(about_window);

	getch();
//	wclear(about_window);
	delwin(about_window);
	refreshAll();
//	wrefresh(stdscr);
}
