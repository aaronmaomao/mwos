#include "bootpack.h"
#include <stdio.h>

typedef struct MOUSE_DESCODE {
	uchar buf[3], phase;
	int x, y, btn;
} MOUSE_DESCODE;

extern FIFO8 keyfifo, mousefifo;
void enable_mouse(MOUSE_DESCODE *mdecode);
void init_keyboard(void);
int mouse_decode(MOUSE_DESCODE *mdecode, uchar data);

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	char temp[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my, dat;
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

#define PORT_KEYDAT				0x0060
#define PORT_KEYSTA				0x0064
#define PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

void wait_KBC_sendready(void)
{
	/* キーボードコントローラがデータ送信可能になるのを待つ */
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard(void)
{
	/* キーボードコントローラの初期化 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

void enable_mouse(MOUSE_DESCODE *mdecode)
{
	/* マウス有効 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	mdecode->phase = 0;
	return; /* うまくいくとACK(0xfa)が送信されてくる */
}

int mouse_decode(MOUSE_DESCODE *mdecode, uchar data)
{
	if (mdecode->phase == 0) {
		if (data == 0xfa) {
			mdecode->phase = 1;
		}
		return 0;
	}
	if (mdecode->phase == 1) {
		if ((data & 0xc8) == 0x08) {	//判读数据是否合理
			mdecode->buf[0] = data;
			mdecode->phase = 2;
		}
		return 0;
	}
	if (mdecode->phase == 2) {
		mdecode->buf[1] = data;
		mdecode->phase = 3;
		return 0;
	}
	if (mdecode->phase == 3) {
		/* マウスの3バイト目を待っている段階 */
		mdecode->buf[2] = data;
		mdecode->phase = 1;

		mdecode->btn = mdecode->buf[0] & 0x07;	//取出鼠标的按键数据（按键信息在低3位）
		mdecode->x = mdecode->buf[1];
		mdecode->y = mdecode->buf[2];
		if ((mdecode->buf[0] & 0x10) != 0) {		//切记！！！！加括号再进行比较
			mdecode->x |= 0xffffff00;
		}
		if ((mdecode->buf[0] & 0x20) != 0) {
			mdecode->y |= 0xffffff00;
		}
		mdecode->y = -mdecode->y;	//y方向与画面符号刚好相反
		return 1;		//表示接收完毕
	}
	return -1;
}
