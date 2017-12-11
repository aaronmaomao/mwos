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

/** 软盘文件 */
typedef struct FILEINFO {
	uchar name[8], ext[3], type;
	char reserve[10];	//保留
	ushort time, date, clustno;	//时间，日期，簇号
	uint size;	//文件大小
} FILEINFO;

extern void putfonts8_asc_sht(SHEET *sht, int lx, int ly, int color, int bcolor, char *str, int length);
void console_task(SHEET *sht, uint memtotal);
int cons_newline(int cursor_y, SHEET *sht);
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img);
void file_readfat(int *fat, uchar *img);

void HariMain(void)
{
	BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;
	int fifobuf[128], keycmd_buf[32];
	FIFO32 fifo_a, keycmd;
	int mx, my, dat, cursor_x, cursor_c, key_to = 0, key_shift = 0, key_leds = (binfo->leds >> 4) & 0x07, keycmd_wait = -1;
	char temp[40];
	MOUSE_DESCODE mdecode;
	uint memtotal;
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;	//初始化内存空闲表的地址（注：表大小为32K）
	SHEETCTL *shtctl;
	SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons;
	uchar *buf_back, buf_mouse[16 * 16], *buf_win, *buf_cons;
	TIMER *cursor_timer;
	TASK *task_m, *task_cons;

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
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
	task_cons->tss.eip = (int) &console_task;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	*((int *) (task_cons->tss.esp + 4)) = (int) sht_cons;
	*((int *) (task_cons->tss.esp + 8)) = memtotal;
	task_run(task_cons, 2, 2);
	//init win
	sht_win = sheet_alloc(shtctl);
	buf_win = (uchar *) memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_win, buf_win, 144, 52, -1);
	make_window8(buf_win, 144, 52, "window", 1);
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;
	cursor_timer = timer_alloc();
	timer_init(cursor_timer, &fifo_a, 1);
	timer_settime(cursor_timer, 50);
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
			//io_stihlt();
			task_sleep(task_m);
			io_sti();
		}
		else {
			dat = fifo32_get(&fifo_a);
			io_sti();
			if (256 <= dat && dat <= 511) {		//键盘数据
				sprintf(temp, "%02X", dat - 256);
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, temp, 2);
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
					if (key_to == 0) {
						if (cursor_x < 128) {
							temp[1] = 0;
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, temp, 1);
							cursor_x += 8;
						}
					}
					else {
						fifo32_put(&task_cons->fifo, temp[0] + 256);
					}
				}
				if (dat == 256 + 0x0e) {	//退格键
					if (key_to == 0) {
						if (cursor_x > 8) {
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
							cursor_x -= 8;
						}
					}
					else {
						fifo32_put(&task_cons->fifo, 8 + 256);
					}
				}
				if (dat == 256 + 0x1c) {	//回车键
					if (key_to != 0) {
						fifo32_put(&task_cons->fifo, 10 + 256);	//enter键的ASCII码为10
					}
				}
				if (dat == 256 + 0x0f) {	//tab键
					if (key_to == 0) {
						key_to = 1;
						make_wtitle8(buf_win, sht_win->xsize, "task_a", 0);	//改变window的状态
						make_wtitle8(buf_cons, sht_cons->xsize, "console", 1);
						cursor_c = -1;
						boxfill8(sht_win->buf, sht_win->xsize, COL8_FFFFFF, cursor_x, 28, cursor_x + 7, 43);
						fifo32_put(&task_cons->fifo, 2);	//2表示光标不闪烁
					}
					else {
						key_to = 0;
						make_wtitle8(buf_win, sht_win->xsize, "task_a", 1);	//改变window的状态
						make_wtitle8(buf_cons, sht_cons->xsize, "console", 0);
						cursor_c = COL8_000000;
						fifo32_put(&task_cons->fifo, 3);	//3表示光标闪烁
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
						sheet_slide(sht_win, mx - 80, my - 8);
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

void console_task(SHEET *sht, uint memtotal)
{
	TIMER *cursor_timer;
	TASK *task = task_now();
	char temp[30], cmdLine[30], *p;
	int dat, fifobuf[128], cursor_x = 16, cursor_y = 28, cursor_col = -1, x, y;
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	FILEINFO *fileinfo = (FILEINFO *) (ADR_DISKIMG + 0x002600);
	int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);

	file_readfat(fat, (uchar *) (ADR_DISKIMG + 0x000200));
	fifo32_init(&task->fifo, 128, fifobuf, task);
	cursor_timer = timer_alloc();
	timer_init(cursor_timer, &task->fifo, 1);
	timer_settime(cursor_timer, 50);
	putfonts8_asc_sht(sht, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);

	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {
			task_sleep(task);
			io_sti();
		}
		else {
			dat = fifo32_get(&task->fifo);
			io_sti();
			if (dat <= 1) {	//光标闪烁
				if (dat != 0) {
					timer_init(cursor_timer, &task->fifo, 0);
					if (cursor_col >= 0) {
						cursor_col = COL8_FFFFFF;
					}
				}
				else {
					timer_init(cursor_timer, &task->fifo, 1);
					if (cursor_col >= 0) {
						cursor_col = COL8_000000;
					}
				}
				timer_settime(cursor_timer, 50);
			}
			if (dat == 2) {	//光标ON
				cursor_col = COL8_FFFFFF;
			}
			if (dat == 3) {	//光标OFF
				boxfill8(sht->buf, sht->xsize, COL8_000000, cursor_x, cursor_y, cursor_x + 7, cursor_y + 16);
				cursor_col = -1;
			}
			if (256 <= dat && dat <= 511) {	//键盘数据
				if (dat == 8 + 256) {	//退格键
					if (cursor_x > 16) {
						putfonts8_asc_sht(sht, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
						cursor_x -= 8;
					}
				}
				else if (dat == 10 + 256) {	//回车键
					putfonts8_asc_sht(sht, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);	//消除上一行的光标
					cmdLine[cursor_x / 8 - 2] = 0;	//0表示命令结束
					cursor_y = cons_newline(cursor_y, sht);	//换行
					/* mem命令 */
					if (strcmp(cmdLine, "mem") == 0) {
						sprintf(temp, "total %dMB", memtotal / (1024 * 1024));
						putfonts8_asc_sht(sht, 8, cursor_y, COL8_FFFFFF, COL8_000000, temp, 30);
						cursor_y = cons_newline(cursor_y, sht);
						sprintf(temp, "free %dKB", memman_total(memman) / 1024);
						putfonts8_asc_sht(sht, 8, cursor_y, COL8_FFFFFF, COL8_000000, temp, 30);
						cursor_y = cons_newline(cursor_y, sht);
						cursor_y = cons_newline(cursor_y, sht);

					}
					else if (strcmp(cmdLine, "cls") == 0) {	//cls命令
						for (y = 28; y < 28 + 128; y++) {
							for (x = 8; x < 8 + 240; x++) {
								sht->buf[x + y * sht->xsize] = COL8_000000;
							}
						}
						sheet_refresh(sht, 8, 28, 8 + 240, 28 + 128);
						cursor_y = 28;
					}
					else if (strcmp(cmdLine, "dir") == 0) {	//dir命令
						for (x = 0; x < 224; x++) {
							if (fileinfo[x].name[0] == 0x00) {	//0x00表示啥也没有
								break;
							}
							if (fileinfo[x].name[0] != 0xe5) {	//0x05表示已删除
								if ((fileinfo[x].type & 0x18) == 0) {
									sprintf(temp, "filename.ext %7d", fileinfo[x].size);
									for (y = 0; y < 8; y++) {	//文件名为8字节
										temp[y] = fileinfo[x].name[y];
									}
									temp[9] = fileinfo[x].ext[0];
									temp[10] = fileinfo[x].ext[1];
									temp[11] = fileinfo[x].ext[2];
									putfonts8_asc_sht(sht, 8, cursor_y, COL8_FFFFFF, COL8_000000, temp, 30);
									cursor_y = cons_newline(cursor_y, sht);
								}
							}
						}
						cursor_y = cons_newline(cursor_y, sht);
					}
					else if (strncmp(cmdLine, "type ", 5) == 0) {	//type 命令（有参数：文件名.扩展名）
						for (y = 0; y < 11; y++) {
							temp[y] = ' ';
						}
						y = 0;
						for (x = 5; y < 11 && cmdLine[x] != 0; x++) {
							if (cmdLine[x] == '.' && y <= 8) {	//文件名部分
								y = 8;
							}
							else {
								temp[y] = cmdLine[x];
								if ('a' <= temp[y] && temp[y] <= 'z') {
									temp[y] -= 0x20;	//将小写字母转为大写
								}
								y++;
							}
						}
						for (x = 0; x < 224;) {
							if (fileinfo[x].name[0] == 0x00) {
								break;
							}
							if ((fileinfo[x].type & 0x18) == 0) {
								for (y = 0; y < 11; y++) {
									if (fileinfo[x].name[y] != temp[y]) {
										goto type_next_file;
									}
								}
								break;
							}
							type_next_file:
							x++;
						}
						if (x < 224 && fileinfo[x].name[0] != 0x00) {	//找到了
							p = (char *) memman_alloc_4k(memman, fileinfo[x].size);
							file_loadfile(fileinfo[x].clustno, fileinfo[x].size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
							cursor_x = 8;
							for (y = 0; y < fileinfo[x].size; y++) {
								temp[0] = p[y];
								temp[1] = 0;
								if (temp[0] == 0x09) {	//制表符
									for (;;) {
										putfonts8_asc_sht(sht, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
										cursor_x += 8;
										if (cursor_x == 8 + 240) {
											cursor_x = 8;
											cursor_y = cons_newline(cursor_y, sht);
										}
										if (((cursor_x - 8) & 0x1f) == 0) {
											break;	//被32整除则跳出，即为4个空格的倍数
										}
									}
								}
								else if (temp[0] == 0x0a) {	//换行符
									cursor_x = 8;
									cursor_y = cons_newline(cursor_y, sht);
								}
								else if (temp[0] == 0x0d) {	//回车符

								}
								else {	//一般字符
									putfonts8_asc_sht(sht, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, temp, 1);
									cursor_x += 8;
									if (cursor_x == 8 + 240) {
										cursor_x = 8;
										cursor_y = cons_newline(cursor_y, sht);
									}
								}
							}
							memman_free_4k(memman, (int) p, fileinfo[x].size);
						}
						else {	//没找到
							putfonts8_asc_sht(sht, 8, cursor_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
							cursor_y = cons_newline(cursor_y, sht);
						}
						cursor_y = cons_newline(cursor_y, sht);
					}
					else if (cmdLine[0] != 0) {	//未知的命令
						putfonts8_asc_sht(sht, 8, cursor_y, COL8_FFFFFF, COL8_000000, "Unknow command.", 15);
						cursor_y = cons_newline(cursor_y, sht);
						cursor_y = cons_newline(cursor_y, sht);
					}
					putfonts8_asc_sht(sht, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">", 1);
					cursor_x = 16;
				}
				else {	//普通字符
					if (cursor_x < 240) {
						temp[0] = dat - 256;
						temp[1] = 0;
						cmdLine[cursor_x / 8 - 2] = dat - 256;	//将字符保存到cmdLine中
						putfonts8_asc_sht(sht, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, temp, 1);
						cursor_x += 8;
					}
				}
			}
			if (cursor_col >= 0) {
				boxfill8(sht->buf, sht->xsize, cursor_col, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
			}
			sheet_refresh(sht, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
		}
	}
}

/**
 * 控制台换行函数
 * cursor_y：在哪一行换
 * sht：要换行的sheet
 * return:返回换行后的cursor_y
 */
int cons_newline(int cursor_y, SHEET *sht)
{
	int x, y;
	if (cursor_y < 28 + 112) {	//不是最下面
		cursor_y += 16;
	}
	else {	//是最下面，需要滚动
		for (y = 28; y < 28 + 112; y++) {	//将所有颜色上移一行
			for (x = 8; x < 8 + 240; x++) {
				sht->buf[x + y * sht->xsize] = sht->buf[x + (y + 16) * sht->xsize];
			}
		}
		for (y = 28 + 112; y < 28 + 128; y++) {	//将最下面的一行抹黑
			for (x = 8; x < 8 + 240; x++) {
				sht->buf[x + y * sht->xsize] = COL8_000000;
			}
		}
		sheet_refresh(sht, 8, 28, 8 + 240, 28 + 128);
	}
	return cursor_y;
}

void file_loadfile(int clustno, int size, char *buf, int *fat, char *img)
{
	int i;
	for (;;) {
		if (size <= 512) {
			for (i = 0; i < size; i++) {
				buf[i] = img[clustno * 512 + i];
			}
			break;
		}
		for (i = 0; i < 512; i++) {
			buf[i] = img[clustno * 512 + i];
		}
		size -= 512;
		buf += 512;
		clustno = fat[clustno];
		return;
	}
}

/*
 * 获取软盘fat信息,解压缩fat信息
 */
void file_readfat(int *fat, uchar *img)
{
	int i, j = 0;
	for (i = 0; i < 2880; i += 2) {
		fat[i + 0] = (img[j + 0]      | img[j + 1] << 8) & 0xfff;
		fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
		j += 3;
	}
	return;
}
