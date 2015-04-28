#include "header/project.h"

int main(int argc, char** argv) {

	// Get terminal size
	initscr();
	getmaxyx(stdscr, terminal_screen_row, terminal_screen_col);

	// Check terminal width 
	if (terminal_screen_col < 197) {
		fprintf(stderr, " - FSinside initialize error : \r\n");
		fprintf(stderr, "       This Terminal width is too short! Must be longer than 196.\r\n");
		fprintf(stderr, "       Current width : %d\n", terminal_screen_col);
		endwin();
		return 0;
	}

	// Screen Initialize
	if (has_colors() == FALSE) {
		fprintf(stderr, " - FSinside initialize error : \r\n");
		fprintf(stderr, "       This Terminal cannot support color display.\n");
		endwin();
		return 0;
	}

	// Do not print keyboard input
	noecho();

	// Cursor visibility off
	curs_set(0);

	// Enable keypad
    keypad(stdscr, TRUE);

	// Print screens
	load_screen();

	// Free
	endwin();
	return 0;
}
