#include "header/project.h"

int main(int argc, char** argv) {

	// Screen Initialize
	initscr();
	if (has_colors() == FALSE) {
		printf("This Terminal cannot support colored display\n");
		return 0;
	}

	// Do not print keyboard input
	noecho();
	// Cursor visibility off
	curs_set(0);

	// Enable keypad
    keypad(stdscr, TRUE);

	// Get terminal size
	getmaxyx(stdscr, terminal_screen_row, terminal_screen_col);

	// Print screens
	load_screen();

	// Free
	endwin();
	return 0;
}
