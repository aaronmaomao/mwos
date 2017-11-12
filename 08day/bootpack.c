#include <stdio.h>
#include "bootpack.h"

extern FIFO8 keyfifo, mousefifo;

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	char temp[40], dat;
	char keybuf[32], mousebuf[128];	//键盘缓冲区；鼠标缓冲区
	uchar mouse_dbuf[3], mouse_phase = 0;	//存放每次鼠标发送的3字节数据；记录接收到第几个数据了

	char mcursor[256];	//保存鼠标指针图像的每一个像素的信息

	int scrnmx = (binfo->scrnx - 16) / 2;
	int scrnmy = (binfo->scrny - 16) / 2;

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */

	init_fifo8(&keyfifo, 32, keybuf);
	init_fifo8(&mousefifo, 128, mousebuf);
	io_out8(PIC0_IMR, 0xf9); /* PIC1とキーボードを許可(11111001) */
	io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */

	init_keyboard();

	init_palette();
	initScreen(binfo->vram, binfo->scrnx, binfo->scrny);
	init_mouse_cursor8(mcursor, COL8_008484);	//制作鼠标指针图像
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, scrnmx, scrnmy, mcursor, 16);	//显示鼠标指针图像
	sprintf(temp, "(%d, %d)", scrnmx, scrnmy);	//将输出结果放入到temp中
	putfonts(binfo->vram, binfo->scrnx, 0, 0, COL8_00FFFF, temp);

	enable_mouse();

	for (;;) {
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) != 0) {
			if (fifo8_status(&keyfifo) != 0) {	//查询是否收到键盘数据
				dat = fifo8_get(&keyfifo);
				io_sti();
				sprintf(temp, "%02X", dat);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
				putfonts(binfo->vram, binfo->scrnx, 0, 16, COL8_000000, temp);
			} else if (fifo8_status(&mousefifo) != 0) {	//查询是否收到鼠标数据
				dat = fifo8_get(&mousefifo);
				io_sti();
				if (mouse_phase == 0) {
					if (dat == 0xfa) {		//第一个字节0xfa是鼠标的标识数据，舍弃不用
						mouse_phase = 1;
					}
				} else if (mouse_phase == 1) {
					mouse_dbuf[0] = dat;
					mouse_phase = 2;
				} else if (mouse_phase == 2) {
					mouse_dbuf[1] = dat;
					mouse_phase = 3;
				} else if (mouse_phase == 3) {
					mouse_dbuf[2] = dat;
					mouse_phase = 1;
					sprintf(temp, "%02X %02X %02X", mouse_dbuf[0], mouse_dbuf[1], mouse_dbuf[2]);
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 8 * 8 - 1, 31);
					putfonts(binfo->vram, binfo->scrnx, 32, 16, COL8_000000, temp);
				}
			}
		} else {
			io_stihlt();
		}
	}
}

/**
 * 等待键盘控制电路准备完毕（键盘控制与鼠标控制为同一块电路）
 */
void wait_KBC_sendready(void)
{
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

/**
 * 初始化键盘/鼠标控制电路
 */
void init_keyboard(void)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

/**
 * 激活鼠标
 */
void enable_mouse(void)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	return;
}
