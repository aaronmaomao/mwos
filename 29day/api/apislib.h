/*
 * apislib.h
 *
 *  Created on: 2018年2月7日
 *      Author: mao-zhengjun
 */

#ifndef API_APISLIB_H_
#define API_APISLIB_H_

void api_putchar(int c);
void api_putstr0(char *str);
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
void api_initmalloc(void);
char *api_malloc(int size);
void api_free(char *addr, int size);
void api_beep(int tone);
void api_end(void);

int api_fopen(char *fname);
void api_fclose(int fhandle);
void api_fseek(int fhandle, int offset, int mode);
int api_fsize(int fhandle, int mode);
int api_fread(char *buf, int size, int fhandle);

int api_cmdline(char *buf, int maxsize);

#endif /* API_APISLIB_H_ */
