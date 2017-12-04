#include "bootpack.h"
#include "stdio.h"

static char keytable[0x54] = { 0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0, 0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@',
		'[', 0, 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0, 0, ']', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.' };

extern void putfonts8_asc_sht(SHEET *sht, int lx, int ly, int color, int bcolor, char *str, int length);
void task_b_main(SHEET *sht_back);

/*
 *  task status segment
 * 	note：TSS也是内存段的一种，在切换任务时会保存当前任务的状态，读取要切换的任务的状态
 */
typedef struct TSS32 {
	int backline, esp0, ss0, esp1, ss1, esp2, ss2, cr3;	//与任务设置相关的信息
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;	//记录该任务的32位寄存器值
	int es, cs, ss, ds, fs, gs;	//记录该任务的段寄存器值
	int ldtr, iomap;
} TSS32;

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	int fifobuf[128];
	FIFO32 fifo;
	int mx, my, dat, course_x = 8, course_c = COL8_FFFFFF, task_b_esp;
	char temp[40];
	MOUSE_DESCODE mdecode;
	uint memtotal;
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;	//初始化内存空闲表的地址（注：表大小为32K）
	SHEETCTL *shtctl;
	SHEET *sht_back, *sht_mouse, *sht_win;
	uchar *buf_back, buf_mouse[16 * 16], *buf_win;
	TIMER *timer, *timer2, *timer3, *timer_ts;
	TSS32 tss_a, tss_b;

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
	fifo32_init(&fifo, 128, fifobuf);
	init_pit();
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdecode);
	io_out8(PIC0_IMR, 0xf8); /* PIC1とキーボードを許可(11111000) */
	io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */

	timer = timer_alloc();
	timer_init(timer, &fifo, 10);
	timer_settime(timer, 1000);
	timer2 = timer_alloc();
	timer_init(timer2, &fifo, 3);
	timer_settime(timer2, 300);
	timer3 = timer_alloc();
	timer_init(timer3, &fifo, 1);
	timer_settime(timer3, 50);
	timer_ts = timer_alloc();
	timer_init(timer_ts, &fifo, 2);
	timer_settime(timer_ts, 2);

	memtotal = memtest(0x00400000, 0xffffffff);	//获取最大内存地址
	memman_init(memman);
	memman_free(memman, 0x004000000, memtotal - 0x00400000);	//将最大空闲内存放入管理表中

	init_palette();
	shtctl = sheetctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	sht_back = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	sht_win = sheet_alloc(shtctl);
	buf_back = (uchar *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	buf_win = (uchar *) memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	sheet_setbuf(sht_win, buf_win, 160, 52, -1);

	init_screen8(buf_back, binfo->scrnx, binfo->scrny);
	init_mouse_cursor8(buf_mouse, 99);
	make_window8(buf_win, 160, 52, "window");
	make_textbox8(sht_win, 8, 28, 144, 16, COL8_FFFFFF);
	sheet_slide(sht_back, 0, 0);
	mx = (binfo->scrnx - 16) / 2; /* 画面中央になるように座標計算 */
	my = (binfo->scrny - 28 - 16) / 2;
	sheet_slide(sht_mouse, mx, my);
	sheet_slide(sht_win, 80, 72);
	sheet_updown(sht_back, 0);
	sheet_updown(sht_win, 1);
	sheet_updown(sht_mouse, 2);
	sprintf(temp, "(%3d, %3d)", mx, my);
	putfonts8_asc(buf_back, sht_back->xsize, 0, 0, COL8_FFFFFF, temp);
	sprintf(temp, "memory = %dMB ,   free = %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, temp, 40);

	tss_a.ldtr = 0;
	tss_a.iomap = 0x40000000;
	tss_b.ldtr = 0;
	tss_b.iomap = 0x40000000;
	SEGMENT_DESC *gdt = (SEGMENT_DESC *) ADR_GDT;
	set_segmdesc(gdt + 3, 103, (int) &tss_a, AR_TSS32);
	set_segmdesc(gdt + 4, 103, (int) &tss_b, AR_TSS32);

	load_tr(3 * 8);	//把任务a的gdt号放到tr中，在下次切换任务时就会自动把当前任务（即主程序）的状态存在tss_a中，类似于tts_a与主程序绑定
	task_b_esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;	//给任务b申请64k的堆栈段内存
	tss_b.eip = (int) &task_b_main;		//在任务切换的时候会读tss的配置
	tss_b.eflags = 0x00000202;	//IF = 1
	tss_b.eax = 0;
	tss_b.ecx = 0;
	tss_b.edx = 0;
	tss_b.ebx = 0;
	tss_b.esp = task_b_esp;
	tss_b.ebp = 0;
	tss_b.esi = 0;
	tss_b.edi = 0;
	tss_b.es = 1 * 8;
	tss_b.cs = 2 * 8;
	tss_b.ss = 1 * 8;
	tss_b.ds = 1 * 8;
	tss_b.fs = 1 * 8;
	tss_b.gs = 1 * 8;

	*((int *) (task_b_esp + 4)) = (int) sht_back;	//因为：函数的第一个参数在esp+4处，即task_b_esp+4 ~ task_b_esp+7
	for (;;) {
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_stihlt();
		} else {
			dat = fifo32_get(&fifo);
			io_sti();
			if (dat == 2) {
				//taskswitch4();
				farjmp(0, 4 * 8);
				timer_settime(timer_ts, 2);
			} else if (256 <= dat && dat <= 511) {		//键盘数据
				sprintf(temp, "%02X", dat - 256);
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, temp, 2);
				if (dat < 256 + 0x54) {
					if (keytable[dat - 256] != 0 && course_x < 144) {
						temp[0] = keytable[dat - 256];
						temp[1] = 0;
						putfonts8_asc_sht(sht_win, course_x, 28, COL8_000000, COL8_FFFFFF, temp, 1);
						course_x += 8;
					}
				}
				if (dat == 256 + 0x0e && course_x > 8) {	//退格键
					putfonts8_asc_sht(sht_win, course_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
					course_x -= 8;
				}
				boxfill8(sht_win->buf, sht_win->xsize, course_c, course_x, 28, course_x + 7, 28 + 15);
				sheet_refresh(sht_win, course_x, 28, course_x + 8, 28 + 16);
			} else if (512 <= dat && dat <= 767) {	//鼠标数据
				if (mouse_decode(&mdecode, dat - 512) != 0) {
					sprintf(temp, "[lcr %4d %4d]", mdecode.x, mdecode.y);
					if ((mdecode.btn & 0x01) != 0) {
						temp[1] = 'L';
					}
					if ((mdecode.btn & 0x02) != 0) {
						temp[3] = 'R';
					}
					if ((mdecode.btn & 0x04) != 0) {
						temp[2] = 'C';
					}
					putfonts8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, temp, 15);
					mx += mdecode.x;
					my += mdecode.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 1) {
						mx = binfo->scrnx - 1;
					}
					if (my > binfo->scrny - 1) {
						my = binfo->scrny - 1;
					}
					sprintf(temp, "(%3d, %3d)", mx, my);
					putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, temp, 10);
					sheet_slide(sht_mouse, mx, my);
					if ((mdecode.btn & 0x01) != 0) {	//左键按下
						sheet_slide(sht_win, mx - 80, my - 8);
					}
				}
			} else if (dat == 10) {
				putfonts8_asc_sht(sht_back, 0, 64, COL8_FFFFFF, COL8_008484, "10[sec]", 7);
			} else if (dat == 3) {
				putfonts8_asc_sht(sht_back, 0, 80, COL8_FFFFFF, COL8_008484, "3[sec]", 6);
			} else if (dat == 1) {
				timer_init(timer3, &fifo, 0);
				course_c = COL8_000000;
				timer_settime(timer3, 50);
				boxfill8(sht_win->buf, sht_win->xsize, course_c, course_x, 28, course_x + 7, 28 + 15);
				sheet_refresh(sht_win, course_x, 28, course_x + 8, 28 + 16);
			} else if (dat == 0) {
				timer_init(timer3, &fifo, 1);
				course_c = COL8_FFFFFF;
				timer_settime(timer3, 50);
				boxfill8(sht_win->buf, sht_win->xsize, course_c, course_x, 28, course_x + 7, 28 + 15);
				sheet_refresh(sht_win, course_x, 28, course_x + 8, 28 + 16);
			}
		}
	}
}

void task_b_main(SHEET *sht_back)
{
	FIFO32 fifo;
	TIMER *timer_ts, *timer_put;
	int dat, fifobuf[128], count = 0;
	char temp[30];
	//SHEET *sht_back = (SHEET*) *((int *) 0x0fec);

	fifo32_init(&fifo, 128, fifobuf);
	timer_ts = timer_alloc();
	timer_init(timer_ts, &fifo, 2);
	timer_settime(timer_ts, 2);
	timer_put = timer_alloc();
	timer_init(timer_put, &fifo, 1);
	timer_settime(timer_put, 1);
	for (;;) {
		count++;
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_sti();
		} else {
			dat = fifo32_get(&fifo);
			io_sti();
			if (dat == 2) {	//超时5秒
				farjmp(0, 3 * 8);
				timer_settime(timer_ts, 2);
			} else if (dat == 1) {
				sprintf(temp, "%10d", count);
				putfonts8_asc_sht(sht_back, 0, 144, COL8_FFFFFF, COL8_008484, temp, 10);
				timer_settime(timer_put, 1);
			}
		}
	}
}
