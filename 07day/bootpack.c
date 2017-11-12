#include <stdio.h>
#include "bootpack.h"

extern FIFO8 keyfifo;
extern FIFO8 mousefifo;

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	char temp[40], dat;
	char keybuf[32];	//键盘缓冲区
	char mousebuf[128];	//鼠标缓冲区
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
	putfonts(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "Hello MaoZhengjun!");
	putfonts(binfo->vram, binfo->scrnx, 0, 16, COL8_00FFFF, "Welcome to MWOS!");
	sprintf(temp, "    scrnx = %d", binfo->scrnx);	//将输出结果放入到temp中
	putfonts(binfo->vram, binfo->scrnx, 0, 32, COL8_00FFFF, temp);
	init_mouse_cursor8(mcursor, COL8_008484);	//制作鼠标指针图像
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, scrnmx, scrnmy, mcursor, 16);	//显示鼠标指针图像

	enable_mouse();

	for (;;) {
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) != 0) {
			if (fifo8_status(&keyfifo) != 0) {
				dat = fifo8_get(&keyfifo);
				io_sti();
				sprintf(temp, "%02X", dat);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 10, 100, 125, 115);
				putfonts(binfo->vram, binfo->scrnx, 10, 100, COL8_000000, temp);
			} else if (fifo8_status(&mousefifo) != 0) {
				dat = fifo8_get(&mousefifo);
				io_sti();
				sprintf(temp, "%02X", dat);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 10, 130, 125, 145);
				putfonts(binfo->vram, binfo->scrnx, 10, 130, COL8_000000, temp);
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
	while ((io_in8(PORT_KEYSTA)) & KEYSTA_SEND_NOTREADY != 0)
		;
	/*for (;;) {
	 if ((io_in8(PORT_KEYSTA)) & KEYSTA_SEND_NOTREADY == 0) {	//如果准备完毕了，从设备号为0x0064处读取的数据的倒数第二位处应该为0
	 break;
	 }
	 }*/
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
