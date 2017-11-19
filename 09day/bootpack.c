#include "bootpack.h"
#include <stdio.h>

extern FIFO8 keyfifo, mousefifo;

uint memtest(uint start, uint end);

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	char temp[40], mcursor[256], keybuf[32], mousebuf[128];
	uint mx, my, dat;
	MOUSE_DESCODE mdecode;

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	io_out8(PIC0_IMR, 0xf9); /* PIC1とキーボードを許可(11111001) */
	io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */

	init_keyboard();

	init_palette();
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);
	mx = (binfo->scrnx - 16) / 2; /* 画面中央になるように座標計算 */
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	sprintf(temp, "(%3d, %3d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, temp);

	enable_mouse(&mdecode);

	dat = memtest(0x00400000, 0xbfffffff) / (1024 * 1024);
	sprintf(temp, "memory size = %dMB", dat);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 30, COL8_FFFFFF, temp);

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

#define EFLAGS_AC_BIT		0x00040000	//如果是486以上cpu，eflags寄存器的18位是有效的，否则此位恒为0
#define CR0_CACHE_DISABLE	0x60000000	//禁止CPU缓存（386及其以下的cpu没有缓存）

/*
 * 获取内存大小
 */
uint memtest(uint start, uint end)
{
	char flg486 = 0;
	uint eflag, cr0, i;
	eflag = io_load_eflags();	//开始检查额flag的18位是否有效
	eflag |= EFLAGS_AC_BIT;
	io_store_eflags(eflag);
	eflag = io_load_eflags();
	if ((eflag & EFLAGS_AC_BIT) != 0) {
		flg486 = 1;
	}
	eflag &= ~EFLAGS_AC_BIT;	//将eflag还原
	io_store_eflags(eflag);

	if (flg486 != 0) {	//如果是486
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE;	//禁止缓存
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flg486 != 0) {	//如果是486
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE;	//允许缓存
		store_cr0(cr0);
	}
	return i;
}

#define MEMMAN_FREES 4096

typedef struct FREEINFO {
	uint addr, size;
} FREEINFO;

typedef struct MEMMAN {
	int frees, maxfress, lostsize, losts;
	FREEINFO free[MEMMAN_FREES];
} MEMMAN;

void memman_init(MEMMAN *man)
{
	man->frees = 0;
	man->maxfress = 0;
	man->lostsize = 0;
	man->losts = 0;
	return;
}

/**
 * 返回空闲内存大小
 */
uint memman_total(MEMMAN *man)
{
	uint i, t = 0;
	for (i = 0; i < man->frees; i++) {
		t += man->free[i].size;
	}
	return t;
}

uint memman_alloc(MEMMAN *man, uint size)
{
	uint i, addr;
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].size >= size) {
			addr = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if (man->free[i].size == 0) {
				man->frees--;
				for (; i < man->frees; i++) {
					man->free[i] = man->free[i + 1];	//将后面的结构体前移
				}
			}
			return addr;
		}
	}
	return 0;
}
