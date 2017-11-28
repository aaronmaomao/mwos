/*
 * window.c
 *
 *  Created on: 2017年11月28日
 *      Author: mao-zhengjun
 */

#include "bootpack.h"

//绘制窗口
void make_window8(uchar *buf, int xsize, int ysize, char *title)
{
	static char closebtn[14][16] = {
			"OOOOOOOOOOOOOOO@",
			"OQQQQQQQQQQQQQ$@",
			"OQQQQQQQQQQQQQ$@",
			"OQQQ@@QQQQ@@QQ$@",
			"OQQQQ@@QQ@@QQQ$@",
			"OQQQQQ@@@@QQQQ$@",
			"OQQQQQQ@@QQQQQ$@",
			"OQQQQQ@@@@QQQQ$@",
			"OQQQQ@@QQ@@QQQ$@",
			"OQQQ@@QQQQ@@QQ$@",
			"OQQQQQQQQQQQQQ$@",
			"OQQQQQQQQQQQQQ$@",
			"O$$$$$$$$$$$$$$@",
			"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	char color;
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, xsize - 1, 0);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, xsize - 2, 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, 0, ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, 1, ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2, 2, xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_000084, 3, 3, xsize - 4, 20);
	boxfill8(buf, xsize, COL8_848484, 1, ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize - 1);
	putfonts8_asc(buf, xsize, 24, 4, COL8_FFFFFF, title);		//放置title字符串
	for (y = 0; y < 14; y++) {	//放置btn
		for (x = 0; x < 16; x++) {

			switch (closebtn[y][x]) {
			case '@':
				color = COL8_000000;
				break;
			case 'Q':
				color = COL8_C6C6C6;
				break;
			case '$':
				color = COL8_848484;
				break;
			default:
				color = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = color;
		}
	}
	return;
}
