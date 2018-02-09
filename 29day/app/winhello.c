#include "../api/apislib.h"

/* draw line */
void HariMain(void)
{
	char buf[160 * 100];	//note:_alloca
	int win, i;
	api_initmalloc();
	//buf = api_malloc(160 * 100);
	win = api_openwin(buf, 160, 100, -1, "star");
	for (i = 0; i < 8; i++) { //�������30����
		api_linewin(win + 1, 8, 26, 77, i * 9 + 26, i);
		api_linewin(win + 1, 88, 26, i * 9 + 88, 89, i);
	}
	api_refreshwin(win, 6, 26, 154, 90);
	while (api_getkey(1) != 0x0a) {

	}
	api_closewin(win);
	api_end();
}

/* star */
//void HariMain(void) {
//	char *buf;
//	int win, i, x, y;
//	api_initmalloc();
//	buf = api_malloc(150 * 50);
//	win = api_openwin(buf, 150, 100, -1, "star");
//	api_boxfillwin(win + 1, 6, 26, 143, 93, 0);
//	for (i = 0; i < 30; i++) { //�������30����
//		x = rand() % 137 + 6;
//		y = rand() % 67 + 26;
//		api_point(win + 1, x, y, 3);
//	}
//	api_refreshwin(win, 6, 26, 144, 94);
//	api_end();
//}
//char buf[150 * 50];
//void HariMain(void) {
//	char *buf;
//	int win;
//	api_initmalloc();
//	buf = api_malloc(150 * 50);
//	win = api_openwin(buf, 150, 50, -1, "hello win");
//	api_boxfillwin(win, 8, 36, 141, 43, 3);
//	api_putstrwin(win, 28, 28, 0, 13, "hello world!");
//	api_end();
//}
