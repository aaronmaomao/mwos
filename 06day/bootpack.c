#include <stdio.h>
#include "bootpack.h"

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *) 0x0ff0;
	char temp[40];
	char mcursor[256];	//保存鼠标指针图像的每一个像素的信息
	int scrnmx = (binfo->scrnx - 16) / 2; 
	int scrnmy = (binfo->scrny - 16) / 2;
	init_gdtidt();
	init_palette();
	initScreen(binfo->vram, binfo->scrnx, binfo->scrny);
	putfonts(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "Hello MaoZhengjun!");
	putfonts(binfo->vram, binfo->scrnx, 0, 16, COL8_00FFFF, "Welcome to MWOS!");
	sprintf(temp, "    scrnx = %d", binfo->scrnx);	//将输出结果放入到temp中
	putfonts(binfo->vram, binfo->scrnx, 0, 32, COL8_00FFFF, temp);
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, scrnmx, scrnmy, mcursor, 16);
	
	for(;;)
	{
		io_hlt();
	}
}
