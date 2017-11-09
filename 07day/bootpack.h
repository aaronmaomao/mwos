/** common */
#define uchar unsigned char

/** asmhead.asm */
typedef struct BOOTINFO	 {	//启动所需的信息，共16Byte
	char cyls;	//启动区读的硬盘数量
	char leds;	//启动时键盘的led的状态
	char vmode;	//显卡模式
	char reserver;	//
	short scrnx, scrny;	//画面分辨率
	char *vram;	//显存起始地址
} BOOTINFO;

#define ADR_BOOTINFO	0x00000ff0	//显存信息的地址

/** asmfun.asm */
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
void io_out8(int port, int data);
int io_in8(int port);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);

/** graphic.c */
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
#define COL8_008400		10	//desktop color
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

/** dsctbl.c */
#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092		//段的访问权限
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e		//用于中断处理的有效设定

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

void init_gdtidt(void);
void set_segmdesc(SEGMENT_DESC *sd, unsigned int limit, int base, int ar);
void set_gatedesc(GATE_DESC *gd, int offset, int selector, int ar);

/** int.c */
#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

#define FLAGS_OVERRUN	0x0001

typedef struct FIFO8 {
	unsigned char *buf;
	int p, q, size, free, flags;
} FIFO8;

/* 初始化pic可编程中断控制器 */
void init_pic(void);
/* 来自ps2键盘的中断处理(IRQ1->INT21) */
void inthandler21(int *esp);
/* 来自ps2鼠标的中断处理(IRQ12->INT2c) */
void inthandler2c(int *esp);
void inthandler27(int *esp);

void init_fifo8(FIFO8 *fifo, int size, uchar *buf);
/* 向fifo缓冲区添加数据 */
int fifo8_put(FIFO8 *fifo, uchar data);
/* 从缓冲区取数据 */
int fifo8_get(FIFO8 *fifo); 
/* 缓冲区的状态 */
int fifo8_status(FIFO8 *fifo);
