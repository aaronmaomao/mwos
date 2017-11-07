#include <stdio.h>

#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15

typedef struct BOOTINFO	 {	//�����������Ϣ
	char cyls, leds, vmode, reserver;
	short scrnx, scrny;
	char *vram;
} BOOTINFO;

typedef struct SEGMENT_DESC {	//��������
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
} SEGMENT_DESC;

typedef struct GATE_DESC {	//�ж�������
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
} GATE_DESC;

void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

void init_palette(void);
void set_palette(int start, int end, unsigned char * rgb);
void boxfill8(unsigned char * vram, int xsize, unsigned char color, int x0, int y0, int x1, int y1);
void initScreen(char *vram, int xsize, int ysize);
void putfont(char *vram, int xsize, int x, int y, char color, char *font);
void putfonts(char *vram, int xsize, int x, int y, char color, unsigned char *str);
void init_mouse_cursor8(char *mouseVram, char bg);
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int pxl, int pyl, char *buf, int bxsize);
void init_gdtidt(void);
void set_segmdesc(SEGMENT_DESC *sd, unsigned int limit, int base, int ar);
void set_gatedesc(GATE_DESC *gd, int offset, int selector, int ar);

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *) 0x0ff0;
	char temp[40];
	char mcursor[256];	//�������ָ��ͼ���ÿһ�����ص���Ϣ
	int scrnmx = (binfo->scrnx - 16) / 2; 
	int scrnmy = (binfo->scrny - 16) / 2;
	init_palette();
	initScreen(binfo->vram, binfo->scrnx, binfo->scrny);
	putfonts(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "Hello MaoZhengjun!");
	putfonts(binfo->vram, binfo->scrnx, 0, 16, COL8_00FFFF, "Welcome to MWOS!");
	sprintf(temp, "    scrnx = %d", binfo->scrnx);	//�����������뵽temp��
	putfonts(binfo->vram, binfo->scrnx, 0, 32, COL8_00FFFF, temp);
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, scrnmx, scrnmy, mcursor, 16);
	
	for(;;)
	{
		io_hlt();
	}
}

//��ʼ����ɫ��
void init_palette(void)
{
	static unsigned char table_rgb[16 * 3] = 
	{
		0x00, 0x00, 0x00,	/*  0:�\ */
		0xff, 0x00, 0x00,	/*  1:���뤤�� */
		0x00, 0xff, 0x00,	/*  2:���뤤�v */
		0xff, 0xff, 0x00,	/*  3:���뤤��ɫ */
		0x00, 0x00, 0xff,	/*  4:���뤤�� */
		0xff, 0x00, 0xff,	/*  5:���뤤�� */
		0x00, 0xff, 0xff,	/*  6:���뤤ˮɫ */
		0xff, 0xff, 0xff,	/*  7:�� */
		0xc6, 0xc6, 0xc6,	/*  8:���뤤��ɫ */
		0x84, 0x00, 0x00,	/*  9:������ */
		0x00, 0x84, 0x00,	/* 10:�����v */
		0x84, 0x84, 0x00,	/* 11:������ɫ */
		0x00, 0x00, 0x84,	/* 12:������ */
		0x84, 0x00, 0x84,	/* 13:������ */
		0x00, 0x84, 0x84,	/* 14:����ˮɫ */
		0x84, 0x84, 0x84	/* 15:������ɫ */
	};
	set_palette(0, 15, table_rgb);
	return;
}

//���Կ����õ�ɫ��
void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	eflags = io_load_eflags();	//��¼�ж���ɱ��λ
	io_cli();	//����жϱ��λ����ֹ�ж�
	io_out8(0x03c8, start);	//���õ�ɫ��ָ�startΪɫ��,��0��ʼ
	for(i = start; i<= end; i++)
	{
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags);	//��ԭ�жϱ��λ
	return;
}

//������
void boxfill8(unsigned char * vram, int xsize, unsigned char color, int x0, int y0, int x1, int y1)
{
	int x, y;
	for(x = x0; x<=x1; x++)
	{
		for(y = y0; y<=y1; y++)
		{
			vram[x + y * xsize] = color;
		}
	}
	return;
}

//��ʼ������
void initScreen(char *vram, int xsize, int ysize)
{
	boxfill8(vram, xsize, COL8_008484,  0,         0,          xsize -  1, ysize - 29);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 28, xsize -  1, ysize - 28);
	boxfill8(vram, xsize, COL8_FFFFFF,  0,         ysize - 27, xsize -  1, ysize - 27);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 26, xsize -  1, ysize -  1);

	boxfill8(vram, xsize, COL8_FFFFFF,  3,         ysize - 24, 59,         ysize - 24);
	boxfill8(vram, xsize, COL8_FFFFFF,  2,         ysize - 24,  2,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484,  3,         ysize -  4, 59,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484, 59,         ysize - 23, 59,         ysize -  5);
	boxfill8(vram, xsize, COL8_000000,  2,         ysize -  3, 59,         ysize -  3);
	boxfill8(vram, xsize, COL8_000000, 60,         ysize - 24, 60,         ysize -  3);

	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize -  4, ysize - 24);
	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize -  4);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize -  3, xsize -  4, ysize -  3);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize -  3, ysize - 24, xsize -  3, ysize -  3);
}

//��ʾһ��8*16�����ַ�
void putfont(char *vram, int xsize, int x, int y, char color, char *font)
{
	int i;
	char *p, d;
	for(i = 0; i<16; i++)
	{
		d = font[i];
		p = vram + x + (y+i) * xsize;
		
		if((d&0x80)) {p[0] = color;}	//����һ�����ص�
		if((d&0x40)) {p[1] = color;}
		if((d&0x20)) {p[2] = color;}
		if((d&0x10)) {p[3] = color;}
		if((d&0x08)) {p[4] = color;}
		if((d&0x04)) {p[5] = color;}
		if((d&0x02)) {p[6] = color;}
		if((d&0x01)) {p[7] = color;}
	}
}

//��ʾ�ַ���
void putfonts(char *vram, int xsize, int x, int y, char color, unsigned char *str)
{
	extern char font[2048];
	while(*str != 0x00)
	{
		putfont(vram, xsize, x, y, color, font + *str * 16);
		x += 8;
		str++;
	}
}

//��ʼ�����ָ��ͼ�Σ�mouseVram�Ǳ����ͼƬ��Ϣ���ڴ��ַ
void init_mouse_cursor8(char *mouseVram, char bc)
{
	static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*O*O*O*O*OO*....",
		"*OOOOOOOOO*.....",
		"*O*O*O*OO*......",
		"*OOOOOOO*.......",
		"*O*O*O*O*.......",
		"*OOOOOOOO*......",
		"*O*OO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	int x, y;
	for(y=0; y<16; y++) {
		for(x=0; x<16; x++) {
			if(cursor[x][y] == '*')
				mouseVram[y*16+x] = COL8_000000;
			if(cursor[x][y] == 'O')
				mouseVram[y*16+x] = COL8_FFFFFF;
			if(cursor[x][y] == '.')
				mouseVram[y*16+x] = bc;
		}
	}
}

void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int pxl, int pyl, char *buf, int bxsize)
{
	int x, y;
	for(y=0; y<pysize; y++) {
		for(x=0; x<pxsize; x++) {
			vram[(pyl+y)*vxsize + (pxl+x)] = buf[y*bxsize+x];	//��mcursor�б����������ɫ��Ϣ �ŵ��Դ�Ķ�Ӧλ�ô�
		}
	}
}

void init_gdtidt(void)
{
	struct SEGMENT_DESC *gdt = (SEGMENT_DESC *) 0x00270000;
	struct GATE_DESC *idt = (GATE_DESC *) 0x0026f800;
	int i;

	/* GDT�γ��ڻ� */
	for (i = 0; i < 8192; i++) {
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
	set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);
	load_gdtr(0xffff, 0x00270000);

	/* IDT�γ��ڻ� */
	for (i = 0; i < 256; i++) {
		set_gatedesc(idt + i, 0, 0, 0);
	}
	load_idtr(0x7ff, 0x0026f800);

	return;
}

void set_segmdesc(SEGMENT_DESC *sd, unsigned int limit, int base, int ar)
{
	if (limit > 0xfffff) {
		ar |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;
	}
	sd->limit_low    = limit & 0xffff;
	sd->base_low     = base & 0xffff;
	sd->base_mid     = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high    = (base >> 24) & 0xff;
	return;
}

void set_gatedesc(GATE_DESC *gd, int offset, int selector, int ar)
{
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	return;
}
