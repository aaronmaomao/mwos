/*
 * window.c
 *
 *  Created on: 2017年11月28日
 *      Author: mao-zhengjun
 */

#include "bootpack.h"

static char closebtn[14][16] = {	//close 小图标
		"OOOOOOOOOOOOOOO@", "OQQQQQQQQQQQQQ$@", "OQQQQQQQQQQQQQ$@", "OQQQ@@QQQQ@@QQ$@", "OQQQQ@@QQ@@QQQ$@", "OQQQQQ@@@@QQQQ$@", "OQQQQQQ@@QQQQQ$@",
				"OQQQQQ@@@@QQQQ$@", "OQQQQ@@QQ@@QQQ$@", "OQQQ@@QQQQ@@QQ$@", "OQQQQQQQQQQQQQ$@", "OQQQQQQQQQQQQQ$@", "O$$$$$$$$$$$$$$@", "@@@@@@@@@@@@@@@@" };

void make_wtitle8(uchar *buf, int xsize, char *title, char act);

//绘制窗口
void make_window8(uchar *buf, int xsize, int ysize, char *title, char act)
{
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, xsize - 1, 0);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, xsize - 2, 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, 0, ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, 1, ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2, 2, xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_848484, 1, ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize - 1);
	make_wtitle8(buf, xsize, title, act);
	return;
}

/**
 * 画窗口的title部分
 */
void make_wtitle8(uchar *buf, int xsize, char *title, char act)
{
	int x, y;
	char btncol, tcol, tbcol;
	if (act != 0) {
		tcol = COL8_FFFFFF;
		tbcol = COL8_000084;
	} else {
		tcol = COL8_C6C6C6;
		tbcol = COL8_848484;
	}
	boxfill8(buf, xsize, tbcol, 3, 3, xsize - 4, 20);
	putfonts8_asc(buf, xsize, 24, 4, tcol, title);
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			switch (closebtn[y][x]) {
			case '@':
				btncol = COL8_000000;
				break;
			case 'Q':
				btncol = COL8_C6C6C6;
				break;
			case '$':
				btncol = COL8_848484;
				break;
			default:
				btncol = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = btncol;
		}
	}
	return;
}

//绘制textbox
void make_textbox8(SHEET *sht, int lx, int ly, int length, int height, int color)
{
	int x1 = lx + length, y1 = ly + height;
	boxfill8(sht->buf, sht->xsize, COL8_848484, lx - 2, ly - 3, x1 + 1, ly - 3);
	boxfill8(sht->buf, sht->xsize, COL8_848484, lx - 3, ly - 3, lx - 3, y1 + 1);
	boxfill8(sht->buf, sht->xsize, COL8_FFFFFF, lx - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill8(sht->buf, sht->xsize, COL8_FFFFFF, x1 + 2, ly - 3, x1 + 2, y1 + 2);
	boxfill8(sht->buf, sht->xsize, COL8_000000, lx - 1, ly - 2, x1 + 0, ly - 2);
	boxfill8(sht->buf, sht->xsize, COL8_000000, lx - 2, ly - 2, lx - 2, y1 + 0);
	boxfill8(sht->buf, sht->xsize, COL8_C6C6C6, lx - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill8(sht->buf, sht->xsize, COL8_C6C6C6, x1 + 1, ly - 2, x1 + 1, y1 + 1);
	boxfill8(sht->buf, sht->xsize, color, lx - 1, ly - 1, x1 + 0, y1 + 0);
	return;
}
