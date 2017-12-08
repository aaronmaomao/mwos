#include "bootpack.h"
#include "stdio.h"

static char keytable0[0x80] = {
		0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 0, 0, 'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 0, '\\', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0x5c, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x5c, 0, 0 };
static char keytable1[0x80] = {
		0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, 0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0, 0, 'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0, '|', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, '_', 0, 0, 0, 0, 0, 0, 0, 0, 0, '|', 0, 0 };

extern void putfonts8_asc_sht(SHEET *sht, int lx, int ly, int color, int bcolor, char *str, int length);
void console_task(SHEET *sht);

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	int fifobuf[128];
	FIFO32 fifo_m;
	int mx, my, dat, cursor_x = 8, course_c = COL8_FFFFFF;
	char temp[40], key_to = 0, key_shift = 0;
	MOUSE_DESCODE mdecode;
	uint memtotal;
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;	//初始化内存空闲表的地址（注：表大小为32K）
	SHEETCTL *shtctl;
	SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons;
	uchar *buf_back, buf_mouse[16 * 16], *buf_win, *buf_cons;
	TIMER *timer;
	TASK *task_m, *task_cons;

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
	fifo32_init(&fifo_m, 128, fifobuf, 0);
	init_pit();
	init_keyboard(&fifo_m, 256);	//鼠标键盘主任务共用一个fifo
	enable_mouse(&fifo_m, 512, &mdecode);
	io_out8(PIC0_IMR, 0xf8); /* PIC1とキーボードを許可(11111000) */
	io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */
	memtotal = memtest(0x00400000, 0xffffffff);	//获取最大内存地址
	memman_init(memman);
	memman_free(memman, 0x004000000, memtotal - 0x00400000);	//将最大空闲内存放入管理表中

	init_palette();
	shtctl = sheetctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	init_mouse_cursor8(buf_mouse, 99);
	task_m = task_init(memman);
	fifo_m.task = task_m;	//设置任务的fifo
	task_run(task_m, 1, 2);
	//init screen
	sht_back = sheet_alloc(shtctl);
	buf_back = (uchar *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);
	/*sht console*/
	sht_cons = sheet_alloc(shtctl);
	buf_cons = (uchar *) memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1);
	make_window8(buf_cons, 256, 165, "console", 0);
	make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
	task_cons = task_alloc();
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
	task_cons->tss.eip = (int) &console_task;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	*((int *) (task_cons->tss.esp + 4)) = (int) sht_cons;
	task_run(task_cons, 2, 2);
	//init win
	sht_win = sheet_alloc(shtctl);
	buf_win = (uchar *) memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_win, buf_win, 144, 52, -1);
	make_window8(buf_win, 144, 52, "window", 1);
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	timer = timer_alloc();
	timer_init(timer, &fifo_m, 1);
	timer_settime(timer, 50);
	//init mouse
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	mx = (binfo->scrnx - 16) / 2; /* 画面中央になるように座標計算 */
	my = (binfo->scrny - 28 - 16) / 2;

	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_cons, 32, 4);
	sheet_slide(sht_win, 64, 56);
	sheet_slide(sht_mouse, mx, my);
	sheet_updown(sht_back, 0);
	sheet_updown(sht_cons, 1);
	sheet_updown(sht_win, 2);
	sheet_updown(sht_mouse, 3);

	sprintf(temp, "(%3d, %3d)", mx, my);
	putfonts8_asc(buf_back, sht_back->xsize, 0, 0, COL8_FFFFFF, temp);
	sprintf(temp, "memory = %dMB ,   free = %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, temp, 40);
	for (;;) {
		io_cli();
		if (fifo32_status(&fifo_m) == 0) {
			//io_stihlt();
			task_sleep(task_m);
			io_sti();
		} else {
			dat = fifo32_get(&fifo_m);
			io_sti();
			if (256 <= dat && dat <= 511) {		//键盘数据
				sprintf(temp, "%02X", dat - 256);
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, temp, 2);
				if (dat < 0x80 + 256) {	//将按键编码转为字符编码
					if (key_shift == 0) {
						temp[0] = keytable0[dat - 256];
					} else {
						temp[0] = keytable1[dat - 256];
					}
				} else {
					temp[0] = 0;
				}
				if (temp[0] != 0) {
					if (key_to == 0) {
						if (cursor_x < 128) {
							temp[1] = 0;
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, temp, 1);
							cursor_x += 8;
						}
					} else {
						fifo32_put(&task_cons->fifo, temp[0] + 256);
					}
				}
				if (dat == 256 + 0x0e) {	//退格键
					if (key_to == 0) {
						if (cursor_x > 8) {
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
							cursor_x -= 8;
						}
					} else {
						fifo32_put(&task_cons->fifo, 8 + 256);
					}
				}
				if (dat == 256 + 0x0f) {	//tab键
					if (key_to == 0) {
						key_to = 1;
						make_wtitle8(buf_win, sht_win->xsize, "task_a", 0);	//改变window的状态
						make_wtitle8(buf_cons, sht_cons->xsize, "console", 1);
					} else {
						key_to = 0;
						make_wtitle8(buf_win, sht_win->xsize, "task_a", 1);	//改变window的状态
						make_wtitle8(buf_cons, sht_cons->xsize, "console", 0);
					}
					sheet_refresh(sht_win, 0, 0, sht_win->xsize, 21);
					sheet_refresh(sht_cons, 0, 0, sht_cons->xsize, 21);
				}
				if (dat == 256 + 0x2a) {	//左shift on
					key_shift |= 1;
				}
				if (dat == 256 + 0x36) {	//右shift on
					key_shift |= 2;
				}
				if (dat == 256 + 0xaa) {	//左shift off
					key_shift &= ~1;
				}
				if (dat == 256 + 0xb6) {	//右shift off
					key_shift &= ~2;
				}
				boxfill8(sht_win->buf, sht_win->xsize, course_c, cursor_x, 28, cursor_x + 7, 28 + 15);	//显示光标
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 28 + 16);
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
				timer_init(timer, &fifo_m, 0);
				course_c = COL8_000000;
				timer_settime(timer, 50);
				boxfill8(sht_win->buf, sht_win->xsize, course_c, cursor_x, 28, cursor_x + 7, 28 + 15);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 28 + 16);
			} else if (dat == 0) {
				timer_init(timer, &fifo_m, 1);
				course_c = COL8_FFFFFF;
				timer_settime(timer, 50);
				boxfill8(sht_win->buf, sht_win->xsize, course_c, cursor_x, 28, cursor_x + 7, 28 + 15);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 28 + 16);
			}
		}
	}
}

void console_task(SHEET *sht)
{
	TIMER *timer;
	TASK *task = task_now();
	char temp[30];
	int dat, fifobuf[128], cursor_x = 16, cursor_col = COL8_000000;
	fifo32_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);
	putfonts8_asc_sht(sht, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);

	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {
			task_sleep(task);
			io_sti();
		} else {
			dat = fifo32_get(&task->fifo);
			io_sti();
			if (dat <= 1) {	//光标闪烁
				if (dat != 0) {
					timer_init(timer, &task->fifo, 0);
					cursor_col = COL8_FFFFFF;
				} else {
					timer_init(timer, &task->fifo, 1);
					cursor_col = COL8_000000;
				}
				timer_settime(timer, 50);
			}

			if (256 <= dat && dat <= 511) {
				if (dat == 8 + 256) {
					if (cursor_x > 16) {
						putfonts8_asc_sht(sht, cursor_x, 28, COL8_FFFFFF, COL8_000000, " ", 1);
						cursor_x -= 8;
					}
				} else {
					if (cursor_x < 240) {
						temp[0] = dat - 256;
						temp[1] = 0;
						putfonts8_asc_sht(sht, cursor_x, 28, COL8_FFFFFF, COL8_000000, temp, 1);
						cursor_x += 8;
					}
				}
			}
			boxfill8(sht->buf, sht->xsize, cursor_col, cursor_x, 28, cursor_x + 7, 43);
			sheet_refresh(sht, cursor_x, 28, cursor_x + 8, 44);
		}
	}
}
