int api_openwin(char *buf, int xsize, int ysize, int col_inv, char *title);
void api_putstrwin(int win, int lx, int ly, int col, int len, char *str);
void api_boxfillwin(int win, int lx0, int ly0, int lx1, int ly1, int col);

void api_end(void);

char buf[150 * 50];

void HariMain(void) {
	int win;
	win = api_openwin(buf, 150, 50, -1, "hello win");
	api_boxfillwin(win, 8, 36, 141, 43, 3);
	api_putstrwin(win, 28, 28, 0, 13, "hello world!");
	api_end();
}