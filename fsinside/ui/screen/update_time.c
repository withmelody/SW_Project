#include "../header/project.h"

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

