#include "bootpack.h"

//初始化调色板
void init_palette(void)
{
	static unsigned char table_rgb[16 * 3] = 
	{
		0x00, 0x00, 0x00,	/*  0:黒 */
		0xff, 0x00, 0x00,	/*  1:明るい赤 */
		0x00, 0xff, 0x00,	/*  2:明るい緑 */
		0xff, 0xff, 0x00,	/*  3:明るい黄色 */
		0x00, 0x00, 0xff,	/*  4:明るい青 */
		0xff, 0x00, 0xff,	/*  5:明るい紫 */
		0x00, 0xff, 0xff,	/*  6:明るい水色 */
		0xff, 0xff, 0xff,	/*  7:白 */
		0xc6, 0xc6, 0xc6,	/*  8:明るい灰色 */
		0x84, 0x00, 0x00,	/*  9:暗い赤 */
		0x00, 0x84, 0x00,	/* 10:暗い緑 */
		0x84, 0x84, 0x00,	/* 11:暗い黄色 */
		0x00, 0x00, 0x84,	/* 12:暗い青 */
		0x84, 0x00, 0x84,	/* 13:暗い紫 */
		0x00, 0x84, 0x84,	/* 14:暗い水色 */
		0x84, 0x84, 0x84	/* 15:暗い灰色 */
	};
	set_palette(0, 15, table_rgb);
	return;
}

//给显卡设置调色板
void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	eflags = io_load_eflags();	//记录中断许可标记位
	io_cli();	//清除中断标记位，禁止中断
	io_out8(0x03c8, start);	//设置调色板指令，start为色号,从0开始
	for(i = start; i<= end; i++)
	{
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags);	//复原中断标记位
	return;
}

//画矩形
void boxfill8(unsigned char * vram, int xsize, unsigned char color, int x0, int y0, int x1, int y1)
{
	int x, y;
	for(x = x0; x<=x1; x++)
	{
		for(y = y0; y<=y1; y++)
		{
			vram[x + y * xsize] = color;
		}
	}
	return;
}

//初始化桌面
void initScreen(char *vram, int xsize, int ysize)
{
	boxfill8(vram, xsize, COL8_008484,  0,         0,          xsize -  1, ysize - 29);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 28, xsize -  1, ysize - 28);
	boxfill8(vram, xsize, COL8_FFFFFF,  0,         ysize - 27, xsize -  1, ysize - 27);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 26, xsize -  1, ysize -  1);

	boxfill8(vram, xsize, COL8_FFFFFF,  3,         ysize - 24, 59,         ysize - 24);
	boxfill8(vram, xsize, COL8_FFFFFF,  2,         ysize - 24,  2,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484,  3,         ysize -  4, 59,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484, 59,         ysize - 23, 59,         ysize -  5);
	boxfill8(vram, xsize, COL8_000000,  2,         ysize -  3, 59,         ysize -  3);
	boxfill8(vram, xsize, COL8_000000, 60,         ysize - 24, 60,         ysize -  3);

	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize -  4, ysize - 24);
	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize -  4);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize -  3, xsize -  4, ysize -  3);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize -  3, ysize - 24, xsize -  3, ysize -  3);
}

//显示一个8*16点阵字符
void putfont(char *vram, int xsize, int x, int y, char color, char *font)
{
	int i;
	char *p, d;
	for(i = 0; i<16; i++)
	{
		d = font[i];
		p = vram + x + (y+i) * xsize;
		
		if((d&0x80)) {p[0] = color;}	//点亮一个像素点
		if((d&0x40)) {p[1] = color;}
		if((d&0x20)) {p[2] = color;}
		if((d&0x10)) {p[3] = color;}
		if((d&0x08)) {p[4] = color;}
		if((d&0x04)) {p[5] = color;}
		if((d&0x02)) {p[6] = color;}
		if((d&0x01)) {p[7] = color;}
	}
}

//显示字符串
void putfonts(char *vram, int xsize, int x, int y, char color, unsigned char *str)
{
	extern char font[2048];
	while(*str != 0x00)
	{
		putfont(vram, xsize, x, y, color, font + *str * 16);
		x += 8;
		str++;
	}
}

//初始化鼠标指针图形，mouseVram是保存该图片信息的内存地址
void init_mouse_cursor8(char *mouseVram, char bc)
{
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*O*O*O*O*OO*....",
		"*OOOOOOOOO*.....",
		"*O*O*O*OO*......",
		"*OOOOOOO*.......",
		"*O*O*O*O*.......",
		"*OOOOOOOO*......",
		"*O*OO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	int x, y;
	for(y=0; y<16; y++) {
		for(x=0; x<16; x++) {
			if(cursor[x][y] == '*')
				mouseVram[y*16+x] = COL8_000000;
			if(cursor[x][y] == 'O')
				mouseVram[y*16+x] = COL8_FFFFFF;
			if(cursor[x][y] == '.')
				mouseVram[y*16+x] = bc;
		}
	}
}

void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int pxl, int pyl, char *buf, int bxsize)
{
	int x, y;
	for(y=0; y<pysize; y++) {
		for(x=0; x<pxsize; x++) {
			vram[(pyl+y)*vxsize + (pxl+x)] = buf[y*bxsize+x];	//将mcursor中保存的像素颜色信息 放到显存的对应位置处
		}
	}
}
