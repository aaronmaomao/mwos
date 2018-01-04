int api_openwin(char *buf, int xsize, int ysize, int col_inv, char *title);
void api_putstrwin(int win, int lx, int ly, int col, int len, char *str);
void api_boxfillwin(int win, int lx0, int ly0, int lx1, int ly1, int col);
void api_point(int win, int x, int y, int col);
void api_refreshwin(int win, int x0, int y0, int x1, int y1);

void api_initmalloc(void);
char *api_malloc(int size);
void api_free(char *addr, int size);

void api_end(void);

/* star */
void HariMain(void) {
	char *buf;
	int win, i, x, y;
	api_initmalloc();
	buf = api_malloc(150 * 50);
	win = api_openwin(buf, 150, 100, -1, "star");
	api_boxfillwin(win + 1, 6, 26, 143, 93, 0);
	for (i = 0; i < 30; i++) { //随机产生30个点
		x = rand() % 137 + 6;
		y = rand() % 67 + 26;
		api_point(win + 1, x, y, 3);
	}
	api_refreshwin(win, 6, 26, 144, 94);
	api_end();
}

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
