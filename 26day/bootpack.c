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

void keywin_off(SHEET *key_win);
void keywin_on(SHEET *key_win);

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	int fifobuf[128], keycmd_buf[32];
	FIFO32 fifo_a, keycmd;
	int mx, my, dat, key_shift = 0, key_leds = (binfo->leds >> 4) & 0x07, keycmd_wait = -1, *cons_fifo[2];
	char temp[40];
	MOUSE_DESCODE mdecode;
	uint memtotal;
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;	//初始化内存空闲表的地址（注：表大小为32K）
	SHEETCTL *shtctl;
	SHEET *sht_back, *sht_mouse, *sht_cons[2], *sht = 0, *key_win;
	uchar *buf_back, buf_mouse[16 * 16], *buf_cons[2];
	TASK *task_m, *task_cons[2], *task;
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
	*((int *) 0x0fe4) = (int) shtctl;
	//init screen
	sht_back = sheet_alloc(shtctl);
	buf_back = (uchar *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);

	/*sht console*/

	for (dat = 0; dat < 2; dat++) {
		sht_cons[dat] = sheet_alloc(shtctl);
		buf_cons[dat] = (uchar *) memman_alloc_4k(memman, 256 * 165);
		sheet_setbuf(sht_cons[dat], buf_cons[dat], 256, 165, -1);
		make_window8(buf_cons[dat], 256, 165, "console", 0);
		make_textbox8(sht_cons[dat], 8, 28, 240, 128, COL8_000000);
		task_cons[dat] = task_alloc();
		task_cons[dat]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
		task_cons[dat]->tss.eip = (int) &console_task;
		task_cons[dat]->tss.es = 1 * 8;
		task_cons[dat]->tss.cs = 2 * 8;
		task_cons[dat]->tss.ss = 1 * 8;
		task_cons[dat]->tss.ds = 1 * 8;
		task_cons[dat]->tss.fs = 1 * 8;
		task_cons[dat]->tss.gs = 1 * 8;
		*((int *) (task_cons[dat]->tss.esp + 4)) = (int) sht_cons[dat];
		*((int *) (task_cons[dat]->tss.esp + 8)) = memtotal;
		task_run(task_cons[dat], 2, 2);
		sht_cons[dat]->task = task_cons[dat];
		sht_cons[dat]->flags |= 0x20;	//0x20表示有光标
		cons_fifo[dat] = (int *) memman_alloc_4k(memman, 128 * 4);
		fifo32_init(&task_cons[dat]->fifo, 128, cons_fifo[dat], task_cons[dat]);
	}

	//init mouse
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	mx = (binfo->scrnx - 16) / 2; /* 画面中央になるように座標計算 */
	my = (binfo->scrny - 28 - 16) / 2;

	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_cons[0], 56, 6);
	sheet_slide(sht_cons[1], 8, 2);
	sheet_slide(sht_mouse, mx, my);
	sheet_updown(sht_back, 0);
	sheet_updown(sht_cons[0], 1);
	sheet_updown(sht_cons[1], 2);
	sheet_updown(sht_mouse, 4);
	key_win = sht_cons[0];
	keywin_on(key_win);

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
					fifo32_put(&key_win->task->fifo, temp[0] + 256);
				}
				if (dat == 256 + 0x0e) {	//退格键
					fifo32_put(&key_win->task->fifo, 8 + 256);
				}
				if (dat == 256 + 0x1c) {	//回车键
					fifo32_put(&key_win->task->fifo, 10 + 256);	//enter键的ASCII码为10
				}
				if (dat == 256 + 0x0f) {	//tab键
					keywin_off(key_win);
					j = key_win->zindex - 1;
					if (j == 0) {
						j = shtctl->top - 1;
					}
					key_win = shtctl->sheetseq[j];
					keywin_on(key_win);
				}
				if (dat == 256 + 0x57 && shtctl->top > 2) { //F11键
					sheet_updown(shtctl->sheetseq[1], shtctl->top - 1); //把最下面（非桌面背景）的图层放到最上面（非鼠标）
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
				if (dat == 256 + 0x3b && key_shift != 0) {  //shift+F1, 强制结束应用程序
					task = key_win->task;
					if (task != 0 && task->tss.ss0 != 0) {
						cons_putstr0(task->cons, "\nBreak(by key)\n");
						io_cli();
						task->tss.eax = (int) &(task->tss.esp0);
						task->tss.eip = (int) asm_end_app;
						io_sti();
					}
				}
				if (dat == 256 + 0xfa) {	//键盘接收到了数据
					keycmd_wait = -1;
				}
				if (dat == 256 + 0xfe) {	//键盘没有接收到数据，重发
					wait_KBC_sendready();
					io_out32(PORT_KEYDAT, keycmd_wait);
				}
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
								if (0 <= x && x < sht->xsize && 0 <= y && y < sht->ysize) {
									if (sht->buf[y * sht->xsize + x] != sht->col_inv) {
										sheet_updown(sht, shtctl->top - 1);
										if (sht != key_win) {
											keywin_off(key_win);
											key_win = sht;
											keywin_on(key_win);
										}
										if (3 <= x && x < sht->xsize - 3 && 3 <= y && y < 21) { //移动窗口
											mmx = mx;
											mmy = my;
										}
										if (sht->xsize - 21 <= x && x < sht->xsize - 5 && 5 <= y && y < 19) { //点击窗口关闭按钮
											if ((sht->flags & 0x10) != 0) {
												task = sht->task;
												cons_putstr0(task->cons, "\nBreak (mouse):\n");
												io_cli();
												task->tss.eax = (int) &(task->tss.esp0);
												task->tss.eip = (int) asm_end_app;
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
		}
	}
}

void keywin_off(SHEET *key_win)
{
	change_wtitle8(key_win, 0);
	if ((key_win->flags & 0x20) != 0) {
		fifo32_put(&key_win->task->fifo, 3); //命令行窗口光标off
	}
	return;
}

void keywin_on(SHEET *key_win)
{
	change_wtitle8(key_win, 1);
	if ((key_win->flags & 0x20) != 0) {
		fifo32_put(&key_win->task->fifo, 2); //命令行窗口光标on
	}
	return;
}
