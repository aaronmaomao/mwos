/*
 * keyboard.c
 *
 *  Created on: 2017年11月18日
 *      Author: aaron
 */

#include "bootpack.h"

FIFO32 *keyfifo;
int keydata_base;

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

void init_keyboard(FIFO32 *fifo, int keybase)
{
	keyfifo = fifo;
	keydata_base = keybase;
	/* キーボードコントローラの初期化 */
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

/** 来自ps2键盘的中断处理 */
void inthandler21(int *esp)
{
	int data;
	io_out8(PIC0_OCW2, 0x61);	//通知PIC ，IRQ1已受理完毕
	data = io_in8(PORT_KEYDAT);
	fifo32_put(keyfifo, data + keydata_base);
	return;
}
