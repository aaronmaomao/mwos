/** common */
#define uchar unsigned char
#define uint unsigned int

typedef struct BOOTINFO { /* 0x0ff0-0x0fff */
	char cyls; /* ブートセクタはどこまでディスクを読んだのか */
	char leds; /* ブート時のキーボードのLEDの状態 */
	char vmode; /* ビデオモード  何ビットカラーか */
	char reserve;
	short scrnx, scrny; /* 画面解像度 */
	char *vram;
} BOOTINFO;

#define ADR_BOOTINFO	0x00000ff0

/* naskfunc.nas */
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
int io_in8(int port);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
int load_cr0(void);
void store_cr0(int cr0);
uint memtest_sub(uint start, uint end);	//计算内存大小（通过不断向内存单元存入数据，然后再检查是否正确 来判断内存大小）

/* fifo.c */
typedef struct FIFO8 {
	uchar *buf;
	int p, q, size, free, flags;
} FIFO8;
void fifo8_init(FIFO8 *fifo, int size, uchar *buf);
int fifo8_put(FIFO8 *fifo, uchar data);
int fifo8_get(FIFO8 *fifo);
int fifo8_status(FIFO8 *fifo);

/* graphic.c */
void init_palette(void);
void set_palette(int start, int end, uchar *rgb);
void boxfill8(uchar *vram, int xsize, uchar c, int x0, int y0, int x1, int y1);
void init_screen8(char *vram, int x, int y);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, uchar *s);
void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize);
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

/** dsctbl.c */
#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e
typedef struct SEGMENT_DESC {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
} SEGMENT_DESC;
typedef struct GATE_DESC {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
} GATE_DESC;
void init_gdtidt(void);
void set_segmdesc(SEGMENT_DESC *sd, uint limit, int base, int ar);
void set_gatedesc(GATE_DESC *gd, int offset, int selector, int ar);

/* int.c */
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
void init_pic(void);
void inthandler21(int *esp);
void inthandler27(int *esp);
void inthandler2c(int *esp);

/* keyboard.h */
#define PORT_KEYDAT				0x0060
#define PORT_KEYSTA				0x0064
#define PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47
extern FIFO8 keyfifo;
void wait_KBC_sendready(void);
void init_keyboard(void);

/* mouse.c */
#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

typedef struct MOUSE_DESCODE {	//鼠标数据相关结构体
	uchar buf[3], phase;
	int x, y, btn;
} MOUSE_DESCODE;

extern FIFO8 mousefifo;

int mouse_decode(MOUSE_DESCODE *mdecode, uchar data);
void enable_mouse(MOUSE_DESCODE *mdecode);

/** memory.c */
#define EFLAGS_AC_BIT		0x00040000	//如果是486以上cpu，eflags寄存器的18位是有效的，否则此位恒为0
#define CR0_CACHE_DISABLE	0x60000000	//禁止CPU缓存（386及其以下的cpu没有缓存）
#define MEMMAN_ADDR	 		0x003c0000	//空闲内存管理表的地址
#define MEMMAN_FREES 		4096		//空闲管理表的大小

typedef struct FREEINFO {
	uint addr, size;
} FREEINFO;

typedef struct MEMMAN {
	int frees, maxfress, lostsize, losts;
	FREEINFO free[MEMMAN_FREES];
} MEMMAN;

uint memtest(uint start, uint end);
void memman_init(MEMMAN *man);
uint memman_total(MEMMAN *man);
uint memman_alloc(MEMMAN *man, uint size);
int memman_free(MEMMAN *man, uint addr, uint size);
uint memman_alloc_4k(MEMMAN *man, uint size);	//以4k为单位分配内存
uint memman_free_4k(MEMMAN *man, uint addr, uint size);	//以4k为单位释放内存
