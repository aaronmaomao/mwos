#include <stdio.h>
#include "bootpack.h"

extern FIFO8 keyfifo;

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	char temp[40], dat;
	char keybuf[32];	//键盘缓冲区
	char mcursor[256];	//保存鼠标指针图像的每一个像素的信息
	
	int scrnmx = (binfo->scrnx - 16) / 2; 
	int scrnmy = (binfo->scrny - 16) / 2;
	
	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
	
	init_fifo8(&keyfifo, 32, keybuf);
	io_out8(PIC0_IMR, 0xf9); /* PIC1とキーボードを許可(11111001) */
	io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */
	
	init_palette();
	initScreen(binfo->vram, binfo->scrnx, binfo->scrny);
	putfonts(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "Hello MaoZhengjun!");
	putfonts(binfo->vram, binfo->scrnx, 0, 16, COL8_00FFFF, "Welcome to MWOS!");
	sprintf(temp, "    scrnx = %d", binfo->scrnx);	//将输出结果放入到temp中
	putfonts(binfo->vram, binfo->scrnx, 0, 32, COL8_00FFFF, temp);
	init_mouse_cursor8(mcursor, COL8_008484);	//制作鼠标指针图像
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, scrnmx, scrnmy, mcursor, 16);	//显示鼠标指针图像
	
	for(;;)
	{
		io_cli();
		if(fifo8_status(&keyfifo) != 0){
			dat = fifo8_get(&keyfifo);
			io_sti();
			sprintf(temp, "%02X", dat);
			boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 10, 100, 125, 115);
			putfonts(binfo->vram, binfo->scrnx, 10, 100, COL8_000000, temp);
		} else {
			io_stihlt();
		}
	}
}
