void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char * rgb);
void boxfill8(unsigned char * vram, int xsize, unsigned char color, int x0, int y0, int x1, int y1);
void initScreen(char *vram, int xsize, int ysize);
void putfont(char *vram, int xsize, int x, int y, char color, char *font);

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

typedef struct BOOTINFO
{
	char cyls, leds, vmode, reserver;
	short scrnx, scrny;
	char *vram;
} BOOTINFO;

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *) 0x0ff0;
	extern char cons16[2064];
	init_palette();
	initScreen(binfo->vram, binfo->scrnx, binfo->scrny);
	putfont(binfo->vram, binfo->scrnx, 0, 0, 7, cons16 + 'M' * 16);
	putfont(binfo->vram, binfo->scrnx, 8, 0, 7, cons16 + 'a' * 16);
	putfont(binfo->vram, binfo->scrnx, 16, 0, 7, cons16 + 'o' * 16);
	putfont(binfo->vram, binfo->scrnx, 24, 0, 7, cons16 + 'Z' * 16);
	putfont(binfo->vram, binfo->scrnx, 32, 0, 7, cons16 + 'J' * 16);
	
	for(;;)
	{
		io_hlt();
	}
}

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