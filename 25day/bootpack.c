#include "bootpack.h"
#include "stdio.h"
#include "string.h"

#define KEYCMD_LED 0xed

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


int keywin_off(SHEET *key_win, SHEET *sht_win, int cur_c, int cur_x);
int keywin_on(SHEET *key_win, SHEET *sht_win, int cur_c);

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *)ADR_BOOTINFO;
	int fifobuf[128], keycmd_buf[32];
	FIFO32 fifo_a, keycmd;
	int mx, my, dat, cursor_x, cursor_c, key_to = 0, key_shift = 0, key_leds = (binfo->leds >> 4) & 0x07, keycmd_wait = -1;
	char temp[40];
	MOUSE_DESCODE mdecode;
	uint memtotal;
	MEMMAN *memman = (MEMMAN *)MEMMAN_ADDR;	//初始化内存空闲表的地址（注：表大小为32K）
	SHEETCTL *shtctl;
	SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons, *sht = 0, *key_win;
	uchar *buf_back, buf_mouse[16 * 16], *buf_win, *buf_cons;
	TIMER *cursor_timer;
	TASK *task_m, *task_cons;
	CONSOLE *cons;
	int j, x, y, mmx = -1, mmy = -1;

	init_gdtidt();
	init_pic();
	io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
	fifo32_init(&fifo_a, 128, fifobuf, 0);
	init_pit();
	init_keyboard(&fifo_a, 256);	//鼠标键盘主任务共用一个fifo
	enable_mouse(&fifo_a, 512, &mdecode);
	io_out8(PIC0_IMR, 0xf8); /* PIC1とキーボードを許可(11111000) */
	io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */
	memtotal = memtest(0x00400000, 0xffffffff);	//获取最大可管理内存
	memman_init(memman);
	memman_free(memman, 0x004000000, memtotal - 0x00400000);	//将最大空闲内存放入管理表中

	init_palette();
	shtctl = sheetctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	init_mouse_cursor8(buf_mouse, 99);
	task_m = task_init(memman);
	fifo_a.task = task_m;	//设置任务的fifo
	task_run(task_m, 1, 2);
	*((int *)0x0fe4) = (int)shtctl;
	//init screen
	sht_back = sheet_alloc(shtctl);
	buf_back = (uchar *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);
	/*sht console*/
	sht_cons = sheet_alloc(shtctl);
	buf_cons = (uchar *)memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1);
	make_window8(buf_cons, 256, 165, "console", 0);
	make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
	task_cons = task_alloc();
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
	task_cons->tss.eip = (int)&console_task;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	*((int *)(task_cons->tss.esp + 4)) = (int)sht_cons;
	*((int *)(task_cons->tss.esp + 8)) = memtotal;
	task_run(task_cons, 2, 2);
	//init win
	sht_win = sheet_alloc(shtctl);
	buf_win = (uchar *)memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_win, buf_win, 144, 52, -1);
	make_window8(buf_win, 144, 52, "window", 1);
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;
	cursor_timer = timer_alloc();
	timer_init(cursor_timer, &fifo_a, 1);
	timer_settime(cursor_timer, 50);
	key_win = sht_win;
	sht_cons->task = task_cons;
	sht_cons->flags |= 0x20;	//0x20表示有光标
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

	fifo32_init(&keycmd, 32, keycmd_buf, 0);
	fifo32_put(&keycmd, KEYCMD_LED);
	fifo32_put(&keycmd, key_leds);
	for (;;) {
		if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {	//控制键盘的led，使led状态与锁定键的状态保持一致
			keycmd_wait = fifo32_get(&keycmd);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}
		io_cli();
		if (fifo32_status(&fifo_a) == 0) {
			task_sleep(task_m);
			io_sti();
		}
		else {
			dat = fifo32_get(&fifo_a);
			io_sti();
			if (key_win->flags == 0) {	//输入窗口已被关闭
				key_win = shtctl->sheetseq[shtctl->top - 1];
				cursor_c = keywin_on(key_win, sht_win, cursor_c);
			}
			if (256 <= dat && dat <= 511) {		//键盘数据
				if (dat < 0x80 + 256) {	//将按键编码转为字符编码
					if (key_shift == 0) {
						temp[0] = keytable0[dat - 256];
					}
					else {
						temp[0] = keytable1[dat - 256];
					}
				}
				else {
					temp[0] = 0;
				}
				if ('A' <= temp[0] && temp[0] <= 'Z') {
					if (((key_leds & 4) == 0 && key_shift == 0) || ((key_leds & 4) != 0 && key_shift != 0)) {
						temp[0] += 0x20;
					}
				}
				if (temp[0] != 0) {
					if (key_win == sht_win) {  //发送给win窗口的
						if (cursor_x < 128) {
							temp[1] = 0;
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, temp, 1);
							cursor_x += 8;
						}
					}
					else {  //发送给命令行的
						fifo32_put(&key_win->task->fifo, temp[0] + 256);
					}
				}
				if (dat == 256 + 0x0e) {	//退格键
					if (key_win == sht_win) {  //发送给win窗口的
						if (cursor_x > 8) {
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
							cursor_x -= 8;
						}
					}
					else {
						fifo32_put(&key_win->task->fifo, 8 + 256);
					}
				}
				if (dat == 256 + 0x1c) {	//回车键
					if (key_win != sht_win) {  //发送给win窗口的
						fifo32_put(&key_win->task->fifo, 10 + 256);	//enter键的ASCII码为10
					}
				}
				if (dat == 256 + 0x0f) {	//tab键
					cursor_c = keywin_off(key_win, sht_win, cursor_c, cursor_x);
					j = key_win->zindex - 1;
					if (j == 0) {
						j = shtctl->top - 1;
					}
					key_win = shtctl->sheetseq[j];
					cursor_c = keywin_on(key_win, sht_win, cursor_c);
				}
				if (dat == 256 + 0x57 && shtctl->top > 2) { //F11键
					sheet_updown(shtctl->sheetseq[1], shtctl->top - 1);//把最下面（非桌面背景）的图层放到最上面（非鼠标）
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
				if (dat == 256 + 0x3a) {	//CapsLock键
					key_leds ^= 4;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (dat == 256 + 0x45) {	//NumLock键
					key_leds ^= 2;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (dat == 256 + 0x46) {	//ScrollLock键
					key_leds ^= 1;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (dat == 256 + 0x3b && key_shift != 0 && task_cons->tss.ss0 != 0) {  //shift+F1, 强制结束应用程序
					cons = (CONSOLE *)*((int *)0x0fec);
					cons_putstr0(cons, "\nBreak\n");
					io_cli();
					task_cons->tss.eax = (int)&(task_cons->tss.esp0);
					task_cons->tss.eip = (int)asm_end_app;
					io_sti();
				}
				if (dat == 256 + 0xfa) {	//键盘接收到了数据
					keycmd_wait = -1;
				}
				if (dat == 256 + 0xfe) {	//键盘没有接收到数据，重发
					wait_KBC_sendready();
					io_out32(PORT_KEYDAT, keycmd_wait);
				}
				if (cursor_c >= 0) {
					boxfill8(sht_win->buf, sht_win->xsize, cursor_c, cursor_x, 28, cursor_x + 7, 28 + 15);	//显示光标
				}
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 28 + 16);
			}
			else if (512 <= dat && dat <= 767) {	//鼠标数据
				if (mouse_decode(&mdecode, dat - 512) != 0) {
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
					sheet_slide(sht_mouse, mx, my);
					if ((mdecode.btn & 0x01) != 0) {	//左键按下
						if (mmx < 0) {
							for (j = shtctl->top - 1; j > 0; j--) {
								sht = shtctl->sheetseq[j];
								x = mx - sht->lx;
								y = my - sht->ly;
								if (0 <= x&&x < sht->xsize && 0 <= y&&y < sht->ysize) {
									if (sht->buf[y*sht->xsize + x] != sht->col_inv) {
										sheet_updown(sht, shtctl->top - 1);
										if (sht != sht_win) { //鼠标点击切换输入窗口
											cursor_c = keywin_off(key_win, sht_win, cursor_c, cursor_x);
											key_win = sht;
											cursor_c = keywin_on(key_win, sht_win, cursor_c);
										}
										if (3 <= x&&x < sht->xsize - 3 && 3 <= y&&y < 21) { //移动窗口
											mmx = mx;
											mmy = my;
										}
										if (sht->xsize - 21 <= x && x < sht->xsize - 5 && 5 <= y && y < 19) { //点击窗口关闭按钮
											if ((sht->flags & 0x10) != 0) {
												cons = (CONSOLE *) *((int *)0x0fec);
												cons_putstr0(cons, "\nBreak (mouse):\n");
												io_cli();
												task_cons->tss.eax = (int) &(task_cons->tss.esp0);
												task_cons->tss.eip = (int)asm_end_app;
												io_sti();
											}
										}
										break;
									}
								}
							}
						}
						else
						{
							x = mx - mmx;
							y = my - mmy;
							sheet_slide(sht, sht->lx + x, sht->ly + y);
							mmx = mx;
							mmy = my;
						}
					}
					else
					{
						mmx = -1;
					}
				}
			}
			else if (dat <= 1) {	//光标
				if (dat != 0) {
					timer_init(cursor_timer, &fifo_a, 0);
					if (cursor_c >= 0) {
						cursor_c = COL8_000000;
					}
				}
				else {
					timer_init(cursor_timer, &fifo_a, 1);
					if (cursor_c >= 0) {
						cursor_c = COL8_FFFFFF;
					}
				}
				timer_settime(cursor_timer, 50);
				if (cursor_c >= 0) {
					boxfill8(sht_win->buf, sht_win->xsize, cursor_c, cursor_x, 28, cursor_x + 7, 28 + 15);
					sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 28 + 16);
				}
			}
		}
	}
}

int keywin_off(SHEET *key_win, SHEET *sht_win, int cur_c, int cur_x)
{
	change_wtitle8(key_win, 0);
	if (key_win == sht_win) {
		cur_c = -1;  //删除光标
		boxfill8(sht_win->buf, sht_win->xsize, COL8_FFFFFF, cur_x, 28, cur_x + 7, 43);
	}
	else {
		if ((key_win->flags & 0x20) != 0) {
			fifo32_put(&key_win->task->fifo, 3); //命令行窗口光标off
		}
	}
	return cur_c;
}

int keywin_on(SHEET *key_win, SHEET *sht_win, int cur_c)
{
	change_wtitle8(key_win, 1);
	if (key_win == sht_win) {
		cur_c = COL8_000000;  //显示光标
	}
	else {
		if ((key_win->flags & 0x20) != 0) {
			fifo32_put(&key_win->task->fifo, 2); //命令行窗口光标on
		}
	}
	return cur_c;
}
