/*
 * mouse.c
 *
 *  Created on: 2017年11月18日
 *      Author: aaron
 */

#include "bootpack.h"

FIFO8 mousefifo;

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

/** 来自ps2鼠标的中断处理 */
void inthandler2c(int *esp)
{
	uchar data;
	io_out8(PIC1_OCW2, 0x64);	//通知PIC ，IRQ12已受理完毕
	io_out8(PIC0_OCW2, 0x62);	//通知PIC ，IRQ02已受理完毕,因为pic1连在IRQ2上
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&mousefifo, data);
	return;
}
