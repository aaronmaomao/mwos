#include "bootpack.h"
#include "stdio.h"

static char keytable[0x54] = { 0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0, 0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@',
		'[', 0, 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0, 0, ']', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.' };

extern void putfonts8_asc_sht(SHEET *sht, int lx, int ly, int color, int bcolor, char *str, int length);
void task_b_main(SHEET *sht_back);

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	int fifobuf[128];
	FIFO32 fifo;
	int mx, my, dat, course_x = 8, course_c = COL8_FFFFFF;
	char temp[40];
	MOUSE_DESCODE mdecode;
	uint memtotal;
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;	//初始化内存空闲表的地址（注：表大小为32K）
	SHEETCTL *shtctl;
	SHEET *sht_back, *sht_mouse, *sht_win, *sht_win_b[3];
	uchar *buf_back, buf_mouse[16 * 16], *buf_win, *buf_win_b;
	TIMER *timer;
	TASK *task_a, *task_b[3];

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
	fifo32_init(&fifo, 128, fifobuf, 0);
	init_pit();
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdecode);
	io_out8(PIC0_IMR, 0xf8); /* PIC1とキーボードを許可(11111000) */
	io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */

	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);

	memtotal = memtest(0x00400000, 0xffffffff);	//获取最大内存地址
	memman_init(memman);
	memman_free(memman, 0x004000000, memtotal - 0x00400000);	//将最大空闲内存放入管理表中

	init_palette();
	shtctl = sheetctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	task_a = task_init(memman);
	fifo.task = task_a;	//设置任务的fifo
	task_run(task_a, 1, 0);

	sht_back = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	sht_win = sheet_alloc(shtctl);
	buf_back = (uchar *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	buf_win = (uchar *) memman_alloc_4k(memman, 144 * 52);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	sheet_setbuf(sht_win, buf_win, 144, 52, -1);
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);
	init_mouse_cursor8(buf_mouse, 99);

	for (dat = 0; dat < 3; dat++) {
		sht_win_b[dat] = sheet_alloc(shtctl);
		buf_win_b = (uchar *) memman_alloc_4k(memman, 144 * 52);
		sheet_setbuf(sht_win_b[dat], buf_win_b, 144, 52, -1);
		sprintf(temp, "task_b%d", dat);
		make_window8(buf_win_b, 144, 52, temp, 0);
		task_b[dat] = task_alloc();
		task_b[dat]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
		task_b[dat]->tss.eip = (int) &task_b_main;
		task_b[dat]->tss.es = 1 * 8;
		task_b[dat]->tss.cs = 2 * 8;
		task_b[dat]->tss.ss = 1 * 8;
		task_b[dat]->tss.ds = 1 * 8;
		task_b[dat]->tss.fs = 1 * 8;
		task_b[dat]->tss.gs = 1 * 8;
		*((int *) (task_b[dat]->tss.esp + 4)) = (int) sht_win_b[dat];
		task_run(task_b[dat], 2, dat + 1);
	}

	make_window8(buf_win, 144, 52, "window", 1);
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	mx = (binfo->scrnx - 16) / 2; /* 画面中央になるように座標計算 */
	my = (binfo->scrny - 28 - 16) / 2;
	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_win_b[0], 168, 56);
	sheet_slide(sht_win_b[1], 8, 116);
	sheet_slide(sht_win_b[2], 168, 116);
	sheet_slide(sht_win, 8, 56);
	sheet_slide(sht_mouse, mx, my);
	sheet_updown(sht_back, 0);
	sheet_updown(sht_win_b[0], 1);
	sheet_updown(sht_win_b[1], 2);
	sheet_updown(sht_win_b[2], 3);
	sheet_updown(sht_win, 4);
	sheet_updown(sht_mouse, 5);

	sprintf(temp, "(%3d, %3d)", mx, my);
	putfonts8_asc(buf_back, sht_back->xsize, 0, 0, COL8_FFFFFF, temp);
	sprintf(temp, "memory = %dMB ,   free = %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, temp, 40);
	for (;;) {
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			//io_stihlt();
			task_sleep(task_a);
			io_sti();
		} else {
			dat = fifo32_get(&fifo);
			io_sti();
			if (256 <= dat && dat <= 511) {		//键盘数据
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
			} else if (dat == 1) {
				timer_init(timer, &fifo, 0);
				course_c = COL8_000000;
				timer_settime(timer, 50);
				boxfill8(sht_win->buf, sht_win->xsize, course_c, course_x, 28, course_x + 7, 28 + 15);
				sheet_refresh(sht_win, course_x, 28, course_x + 8, 28 + 16);
			} else if (dat == 0) {
				timer_init(timer, &fifo, 1);
				course_c = COL8_FFFFFF;
				timer_settime(timer, 50);
				boxfill8(sht_win->buf, sht_win->xsize, course_c, course_x, 28, course_x + 7, 28 + 15);
				sheet_refresh(sht_win, course_x, 28, course_x + 8, 28 + 16);
			}
		}
	}
}

void task_b_main(SHEET *sht_win_b)
{
	FIFO32 fifo;
	TIMER *timer_put;
	int dat, fifobuf[128];
	long count = 0;
	char temp[30];

	fifo32_init(&fifo, 128, fifobuf, 0);
	timer_put = timer_alloc();
	timer_init(timer_put, &fifo, 1);
	timer_settime(timer_put, 10);
	for (;;) {
		count++;
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_sti();
		} else {
			dat = fifo32_get(&fifo);
			io_sti();
			if (dat == 1) {
				sprintf(temp, "%16d", count);
				putfonts8_asc_sht(sht_win_b, 4, 28, COL8_000000, COL8_C6C6C6, temp, 16);
				timer_settime(timer_put, 10);
			}
		}
	}
}
