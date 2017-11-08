#include <stdio.h>
#include "bootpack.h"

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	char temp[40];
	char mcursor[256];	//�������ָ��ͼ���ÿһ�����ص���Ϣ
	int scrnmx = (binfo->scrnx - 16) / 2; 
	int scrnmy = (binfo->scrny - 16) / 2;
	
	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PIC�γ��ڻ����K��ä��Τ�CPU�θ���z�߽�ֹ���� */
	
	init_palette();
	initScreen(binfo->vram, binfo->scrnx, binfo->scrny);
	putfonts(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "Hello MaoZhengjun!");
	putfonts(binfo->vram, binfo->scrnx, 0, 16, COL8_00FFFF, "Welcome to MWOS!");
	sprintf(temp, "    scrnx = %d", binfo->scrnx);	//�����������뵽temp��
	putfonts(binfo->vram, binfo->scrnx, 0, 32, COL8_00FFFF, temp);
	init_mouse_cursor8(mcursor, COL8_008484);	//�������ָ��ͼ��
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, scrnmx, scrnmy, mcursor, 16);	//��ʾ���ָ��ͼ��
	
	io_out8(PIC0_IMR, 0xf9); /* PIC1�ȥ��`�ܩ`�ɤ��S��(11111001) */
	io_out8(PIC1_IMR, 0xef); /* �ޥ������S��(11101111) */
	
	for(;;)
	{
		io_hlt();
	}
}
