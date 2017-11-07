/* asmhead.asm */
typedef struct BOOTINFO	 {	//启动所需的信息，共16Byte
	char cyls;	//启动区读的硬盘数量
	char leds;	//启动时键盘的led的状态
	char vmode;	//显卡模式
	char reserver;	//
	short scrnx, scrny;	//画面分辨率
	char *vram;	//显存起始地址
} BOOTINFO;

/* asmfun.asm */
void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

/* graphic.c */
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
void init_palette(void);
void set_palette(int start, int end, unsigned char * rgb);
void boxfill8(unsigned char * vram, int xsize, unsigned char color, int x0, int y0, int x1, int y1);
void initScreen(char *vram, int xsize, int ysize);
void putfont(char *vram, int xsize, int x, int y, char color, char *font);
void putfonts(char *vram, int xsize, int x, int y, char color, unsigned char *str);
void init_mouse_cursor8(char *mouseVram, char bg);
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int pxl, int pyl, char *buf, int bxsize);

/* dsctbl.c */
#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
typedef struct SEGMENT_DESC {	//段描述符
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
} SEGMENT_DESC;

typedef struct GATE_DESC {	//中断描述符
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
} GATE_DESC;