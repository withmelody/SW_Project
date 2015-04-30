#include "../header/project.h"

#define LOGO_WIDTH (terminal_screen_col-2 - 116)

void printLogo() {
	int left_black = (LOGO_WIDTH) / 2;

	THREAD_LOCK;

	top_clock = subwin(stdscr,11, terminal_screen_col, 0, 0);
	box(top_clock, 0, 0);
	wborder(top_clock, '|', '|', '-', '-', '+', '+', '+', '+');
	mvwprintw(top_clock,    1   , left_black,     "       :+++++++++++++/   -+ossssssssso.`+++++/                                   `:::::.          `:::::-                    ");
	mvwprintw(top_clock,    2   , left_black,     "      `mMMMMMMMMMMMMN+ .dMMMMMMMMMMMMd`sMMMMM/                                   hMMMMN.          /MMMMMo                    ");
	mvwprintw(top_clock,    3   , left_black,     "      yMMMMMsooooo++/ `dMMMMdooooooo+./ooooo- -::::-.::::::-`    `-::::::::::- `:ooooo.  .:::::::.NMMMMd   .::::::::::-`     ");
	mvwprintw(top_clock,    4   , left_black,     "     /NMMMMs          yMMMMN/.....`  -NMMMMd``mMMMMNddmNMMMMm` :hNMMNddddddmNs /NMMMMs -dMMMMNddmNMMMMm.`omMMMNddmMMMMMd`    ");
	mvwprintw(top_clock,    5   , left_black,     "    -NMMMMMdhhhhhhy` +MMMMMMMMMMMMN+`dMMMMN- yMMMMN-  .mMMMMh`:NMMMMNhhhhhhs-`.mMMMMd`-NMMMMy`  sMMMMN/ hMMMMN-``-NMMMMs     ");
	mvwprintw(top_clock,    6   , left_black,     "   `dMMMMMMMMMMMMM+  .syyyyyyyNMMMN.sMMMMN/ /NMMMM+  `hMMMMm.`dMMMMMMMMMMMMM+`hMMMMm.`dMMMMm`  :NMMMMs oMMMMMNNNNNMMMMd`     ");
	mvwprintw(top_clock,    7   , left_black,     "   sMMMMN+```````` :s/:::::::hMMMN//MMMMMy -NMMMMy   oMMMMN/ `ohhhhhhdMMMMMh oMMMMN/ yMMMMN-  .mMMMMh`:NMMMMs.........`      ");
	mvwprintw(top_clock,    8   , left_black,     "  +MMMMMy         .mMMMMMMMMMMMMNo-NMMMMm``dMMMMm`  :NMMMMs smhyyyyyydMMMNy`:NMMMMs .NMMMMNhyhNMMMMN. sMMMMMdyyyyyyhy.       ");
	mvwprintw(top_clock,    9   , left_black,     "  .ooooo.         `/++++++++++/- `+ooooo: :+++++:  `/+++++. /+++++++++++:` `/+++++.  -+++++++:/++++/  `:++++++++++++.    ");
	
	wrefresh(top_clock);

	THREAD_UNLOCK;
}


