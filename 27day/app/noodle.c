#include "../api/apislib.h"

void HariMain(void) {
	int i, timer;
	timer = api_alloctimer();
	api_inittimer(timer, 128);
	for (i = 20000000; i >= 20000; i -= i / 100) {
		api_beep(i);
		api_settimer(timer, 1);
		if (api_getkey(1) != 128) {
			break;
		}
	}

	api_beep(0);
	api_end();
}

//void HariMain(void) {
//	char *buf, s[12];
//	int win, timer, hou = 0, min = 0, sec = 0;
//	api_initmalloc();
//	buf = api_malloc(150 * 50);
//	win = api_openwin(buf, 150, 50, -1, "noodle");
//	timer = api_alloctimer();
//	api_inittimer(timer, 128);
//	for (;;) {
//		sprintf(s, "%5d:%2d:%2d", hou, min, sec);
//		api_boxfillwin(win, 28, 27, 115, 41, 7);
//		api_putstrwin(win, 28, 27, 0, 11, s);
//
//		api_settimer(timer, 100);
//		if (api_getkey(1) != 128) {
//			break;
//		}
//		sec++;
//		if (sec == 60) {
//			sec = 0;
//			min++;
//			if (min == 60) {
//				min = 0;
//				hou++;
//			}
//		}
//	}
//	api_end();
//}
