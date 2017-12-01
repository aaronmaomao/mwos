/** common */

#ifndef BOOTPACK_H
#define BOOTPACK_H

#include "bootpack.h"

typedef unsigned char uchar;
typedef unsigned int uint;

typedef struct BOOTINFO { /* 0x0ff0-0x0fff */
	char cyls; /* ブートセクタはどこまでディスクを読んだのか */
	char leds; /* ブート時のキーボードのLEDの状態 */
	char vmode; /* ビデオモード  何ビットカラーか */
	char reserve;
	short scrnx, scrny; /* 分辨率大小 */
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
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
int load_cr0(void);
void store_cr0(int cr0);
uint memtest_sub(uint start, uint end);	//计算内存大小（通过不断向内存单元存入数据，然后再检查是否正确 来判断内存大小）

/* fifo.c */
typedef struct FIFO32 {
	int *buf;
	int p, q, size, free, flags;
} FIFO32;

void fifo32_init(FIFO32 *fifo, int size, int *buf);
int fifo32_put(FIFO32 *fifo, int data);
int fifo32_get(FIFO32 *fifo);
int fifo32_status(FIFO32 *fifo);

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

extern FIFO32 *keyfifo;

void wait_KBC_sendready(void);
void init_keyboard(FIFO32 *fifo, int keybase);

/* mouse.c */
#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

typedef struct MOUSE_DESCODE {	//鼠标数据相关结构体
	uchar buf[3], phase;
	int x, y, btn;
} MOUSE_DESCODE;

extern FIFO32 *mousefifo;

int mouse_decode(MOUSE_DESCODE *mdecode, uchar data);
void enable_mouse(FIFO32 *fifo, int mousebase, MOUSE_DESCODE *mdecode);

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
	struct FREEINFO free[MEMMAN_FREES];
} MEMMAN;

uint memtest(uint start, uint end);
void memman_init(MEMMAN *man);
uint memman_total(MEMMAN *man);
uint memman_alloc(MEMMAN *man, uint size);
int memman_free(MEMMAN *man, uint addr, uint size);
uint memman_alloc_4k(MEMMAN *man, uint size);	//以4k为单位分配内存
uint memman_free_4k(MEMMAN *man, uint addr, uint size);	//以4k为单位释放内存

/** sheet.c */
#define MAX_SHEETS 256		//最大图层数

typedef struct SHEET {	//图层
	uchar *buf;
	int xsize, ysize, lx, ly, col_inv, zindex, flags;	//col_inv记录本图层的透明标识
	struct SHEETCTL *ctl;
} SHEET;

typedef struct SHEETCTL {	//所有图层管理(共9232byte)
	uchar *vram;	//显存地址
	uchar *map;	//用来记录每一个像素点是由哪个图层显示出来的
	int xsize, ysize, top;
	struct SHEET *sheetseq[MAX_SHEETS];	//图层的序列，低zindex的在前面，高zindex的在后面
	struct SHEET sheets[MAX_SHEETS];
} SHEETCTL;

/**
 * 生成一个图层管理表
 */
SHEETCTL *sheetctl_init(MEMMAN *memman, uchar *vram, int xsize, int ysize);
/**
 * 申请一个空闲的图层
 */
SHEET *sheet_alloc(SHEETCTL *ctl);
/**
 * 设置图层参数
 */
void sheet_setbuf(SHEET *sht, uchar *buf, int xsize, int ysize, int col_inv);
/**
 * 移动图层位置
 */
void sheet_updown(SHEET *sht, int zindex);
/**
 * 刷新所有图层(从最里面的图层开始，将每个图层的非透明的像素放到对应显存位置，类似于"千手观音"原理)
 */
void sheet_refresh(SHEET *sht, int vx0, int vy0, int vx1, int vy1);
/**
 * 只刷新指定大小位置的区域
 */
void sheet_refreshsub(SHEETCTL *ctl, int vx0, int vy0, int vx1, int vy1, int zindex0, int zindex1);

void sheet_refreshmap(SHEETCTL *ctl, int vx0, int vy0, int vx1, int vy1, int zindex);
/**
 * 处理图层滑动
 */
void sheet_slide(SHEET *sht, int lx, int ly);
/**
 * 释放图层
 */
void sheet_free(SHEET *sht);

/** window.c */
void make_window8(uchar *buf, int xsize, int ysize, char *title);
void make_textbox8(SHEET *sht, int lx, int ly, int length, int height, int color);

/** timer.c */
#define PIT_CTRL	0x0043	//定时器(8254)的port
#define PIT_CNT0	0x0040

#define MAX_TIMER	500			//定时器的数量
#define TIMER_FLAGS_ALLOC	1 	//定时器已配置
#define TIMER_FLAGS_USING	2 	//定时器正在运行中

typedef struct TIMER {
	uint timeout, flags;
	FIFO32 *fifo;
	int data;
	struct TIMER *next;
} TIMER;

/**
 * count:	记录从开机到现在总的定时时间
 * mintime:	记录定时器组中最短的定时时间
 * usingsum:用来记录有几个定时器处于活动状态
 */
typedef struct TIMERCTL {
	uint count, mintimes;//usingsum;
	//TIMER *timerseq[MAX_TIMER];	//定时器序列
	TIMER *mintimer;	//下一个即将超时的定时器（即当前最短的定时器）
	TIMER timers[MAX_TIMER];
} TIMERCTL;

extern TIMERCTL timerctl;

void init_pit();
TIMER *timer_alloc(void);
void timer_free(TIMER *timer);
void timer_init(TIMER *timer, FIFO32 *fifo, int data);
void timer_settime(TIMER *timer, uint timeout);
void inthandler20(int *esp);

#endif
