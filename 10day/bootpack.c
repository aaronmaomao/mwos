#include "bootpack.h"
#include <stdio.h>

typedef struct SHEET {	//图层
	uchar *buf;
	int xsize, ysize, lx, ly, col_inv, zindex, flags;
} SHEET;

#define MAX_SHEETS 256		//最大图层数
typedef struct SHEETCTL {	//所有图层管理(共9232byte)
	uchar *vram;	//显存地址（只为方便）
	int xsize, ysize, top;
	SHEET *sheets_addr[MAX_SHEETS];
	SHEET sheets[MAX_SHEETS];
} SHEETCTL;

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	char temp[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my, dat;
	MOUSE_DESCODE mdecode;
	uint memtotal;
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;	//初始化内存空闲表的地址（注：表大小为32K）

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	io_out8(PIC0_IMR, 0xf9); /* PIC1とキーボードを許可(11111001) */
	io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */

	init_keyboard();

	memtotal = memtest(0x00400000, 0xffffffff);	//获取最大内存地址
	memman_init(memman);
	memman_free(memman, 0x004000000, memtotal - 0x00400000);	//将最大空闲内存放入管理表中

	init_palette();
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
	mx = (binfo->scrnx - 16) / 2; /* 画面中央になるように座標計算 */
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	sprintf(temp, "(%3d, %3d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, temp);

	sprintf(temp, "memory = %dMB , free = %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 30, COL8_FFFFFF, temp);

	enable_mouse(&mdecode);

	for (;;) {
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		} else {
			if (fifo8_status(&keyfifo) != 0) {
				dat = fifo8_get(&keyfifo);
				io_sti();
				sprintf(temp, "%02X", dat);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, temp);
			} else if (fifo8_status(&mousefifo) != 0) {
				dat = fifo8_get(&mousefifo);
				io_sti();
				if (mouse_decode(&mdecode, dat) != 0) {
					sprintf(temp, "[lcr %4d %4d]", mdecode.x, mdecode.y);
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, temp);
					if ((mdecode.btn & 0x01) != 0) {
						temp[1] = 'L';
					}
					if ((mdecode.btn & 0x02) != 0) {
						temp[3] = 'R';
					}
					if ((mdecode.btn & 0x04) != 0) {
						temp[2] = 'C';
					}
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15);
					putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, temp);
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15); /* マウス消す */
					mx += mdecode.x;
					my += mdecode.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 16) {
						mx = binfo->scrnx - 16;
					}
					if (my > binfo->scrny - 16) {
						my = binfo->scrny - 16;
					}

					sprintf(temp, "(%3d, %3d)", mx, my);
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15); /* 座標消す */
					putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, temp); /* 座標書く */
					putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
				}
			}
		}
	}
}

/**
 * 生成一个图层管理表
 */
SHEETCTL *sheetctl_init(MEMMAN *memman, uchar *vram, int xsize, int ysize)
{
	SHEETCTL *ctl;
	int i;
	ctl = (SHEETCTL *) memman_alloc_4k(memman, sizeof(SHEETCTL));
	if (ctl == 0) {
		goto err;
	}
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1;	//一个图层都没有
	for (i = 0; i < MAX_SHEETS; i++) {
		ctl->sheets[i].flags = 0;	//标记为未使用
	}
	err: return ctl;
}

#define SHEET_USE 1

/**
 * 申请一个空闲的图层
 */
SHEET *sheet_alloc(SHEETCTL *ctl)
{
	SHEET *sheet;
	int i;
	for (i = 0; i < MAX_SHEETS; i++) {
		if (ctl->sheets[i].flags == 0) {
			sheet = &(ctl->sheets[i]);	//取得空闲图层的地址
			sheet->flags = SHEET_USE;	//标记为该图层正在使用
			sheet->zindex = -1;	//影藏
			return sheet;
		}
	}
	return 0;	//没图层了
}

