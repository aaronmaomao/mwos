#include "stdio.h"

int api_openwin(char *buf, int xsize, int ysize, int col_inv, char *title);
void api_putstrwin(int win, int lx, int ly, int col, int len, char *str);
void api_boxfillwin(int win, int lx0, int ly0, int lx1, int ly1, int col);
void api_point(int win, int x, int y, int col);
void api_refreshwin(int win, int x0, int y0, int x1, int y1);
void api_linewin(int win, int x0, int y0, int x1, int y1, int col);
void api_closewin(int win);
int api_getkey(int mode);
int api_alloctimer(void);
void api_inittimer(int timer, int data);
void api_settimer(int timer, int time);
void api_freetimer(int timer);

void api_beep(int tone);

void api_initmalloc(void);
char *api_malloc(int size);
void api_free(char *addr, int size);

void api_end(void);

void HariMain(void) {
	int i, timer;
	timer = api_alloctimer();
	api_initmalloc(timer, 128);
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