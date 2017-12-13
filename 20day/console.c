/*
 * console.c
 *
 *  Created on: 2017年12月12日
 *      Author: mao-zhengjun
 */

#include "bootpack.h"
#include "string.h"
#include "stdio.h"

void console_task(SHEET *sht, uint memtotal)
{
	TIMER *cursor_timer;
	TASK *task = task_now();
	char cmdLine[30];
	CONSOLE cons;
	int dat, fifobuf[128];
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);

	cons.cur_x = 8;
	cons.cur_y = 28;
	cons.cur_c = -1;
	cons.sht = sht;
	*((int *) 0x0fec) = (int) &cons;

	fifo32_init(&task->fifo, 128, fifobuf, task);
	cursor_timer = timer_alloc();
	timer_init(cursor_timer, &task->fifo, 1);
	timer_settime(cursor_timer, 50);

	file_readfat(fat, (uchar *) (ADR_DISKIMG + 0x000200));
	cons_putchar(&cons, '>', 1);

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
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_FFFFFF;
					}
				}
				else {
					timer_init(cursor_timer, &task->fifo, 1);
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_000000;
					}
				}
				timer_settime(cursor_timer, 50);
			}
			if (dat == 2) {	//光标ON
				cons.cur_c = COL8_FFFFFF;
			}
			if (dat == 3) {	//光标OFF
				boxfill8(sht->buf, sht->xsize, COL8_000000, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 16);
				cons.cur_c = -1;
			}
			if (256 <= dat && dat <= 511) {	//键盘数据
				if (dat == 8 + 256) {	//退格键
					if (cons.cur_x > 16) {
						cons_putchar(&cons, ' ', 0);
						cons.cur_x -= 8;
					}
				}
				else if (dat == 10 + 256) {	//回车键
					cons_putchar(&cons, ' ', 0);
					cmdLine[cons.cur_x / 8 - 2] = 0;	//0表示命令结束
					cons_newline(&cons);	//换行
					cons_runcmd(cmdLine, &cons, fat, memtotal);
					cons_putchar(&cons, '>', 1);
				}
				else {	//普通字符
					if (cons.cur_x < 240) {
						cmdLine[cons.cur_x / 8 - 2] = dat - 256;	//将字符保存到cmdLine中
						cons_putchar(&cons, dat - 256, 1);
					}
				}
			}
			if (cons.cur_c >= 0) {
				boxfill8(sht->buf, sht->xsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
			}
			sheet_refresh(sht, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
		}
	}
}

void cons_runcmd(char *cmdLine, CONSOLE *cons, int *fat, uint memtotal)
{
	/* mem命令 */
	if (strcmp(cmdLine, "mem") == 0) {
		cmd_mem(cons, memtotal);
	}
	else if (strcmp(cmdLine, "cls") == 0) {	//cls命令
		cmd_cls(cons);
	}
	else if (strcmp(cmdLine, "dir") == 0) {	//dir命令
		cmd_dir(cons);
	}
	else if (strncmp(cmdLine, "type ", 5) == 0) {	//type 命令（有参数：文件名.扩展名）
		cmd_type(cons, fat, cmdLine);
	}
	else if (strcmp(cmdLine, "hlt") == 0) {	//hlt命令
		cmd_hlt(cons, fat);
	}
	else if (cmdLine[0] != 0) {	//未知的命令
		putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "Unknow command.", 15);
		cons_newline(cons);
		cons_newline(cons);
	}
	return;
}

/* hlt 命令 */
void cmd_hlt(CONSOLE *cons, int *fat)
{
	char *p;
	FILEINFO *fileinfo = file_search("HLT.MWE", (FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	SEGMENT_DESC *gdt = (SEGMENT_DESC *) ADR_GDT;

	if (fileinfo != 0) {
		p = (char *) memman_alloc_4k(memman, fileinfo->size);
		file_loadfile(fileinfo->clustno, fileinfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		set_segmdesc(gdt + 1003, fileinfo->size - 1, (int) p, AR_CODE32_ER);	//注：1003之前的都被用了
		farcall(0, 1003 * 8);
		memman_free_4k(memman, (int) p, fileinfo->size);
	}
	else {
		putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
		cons_newline(cons);
	}
	cons_newline(cons);
	return;
}

/* type 命令 */
void cmd_type(CONSOLE *cons, int *fat, char *cmdLine)
{
	int i;
	char *p;
	FILEINFO *fileinfo = file_search(cmdLine + 5, (FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;

	if (fileinfo != 0) {
		p = (char *) memman_alloc_4k(memman, fileinfo->size);
		file_loadfile(fileinfo->clustno, fileinfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		for (i = 0; i < fileinfo->size; i++) {
			cons_putchar(cons, p[i], 1);

		}
		memman_free_4k(memman, (int) p, fileinfo->size);
	}
	else {	//没找到
		putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
		cons_newline(cons);
	}
	cons_newline(cons);
	return;
}

/* dir 命令 */
void cmd_dir(CONSOLE *cons)
{
	int i, j;
	char temp[30];
	FILEINFO *fileinfo = (FILEINFO *) (ADR_DISKIMG + 0x002600);
	for (i = 0; i < 224; i++) {
		if (fileinfo[i].name[0] == 0x00) {	//0x00表示啥也没有
			break;
		}
		if (fileinfo[i].name[0] != 0xe5) {	//0x05表示已删除
			if ((fileinfo[i].type & 0x18) == 0) {
				sprintf(temp, "filename.ext %7d", fileinfo[i].size);
				for (j = 0; j < 8; j++) {	//文件名为8字节
					temp[j] = fileinfo[i].name[j];
				}
				temp[9] = fileinfo[i].ext[0];
				temp[10] = fileinfo[i].ext[1];
				temp[11] = fileinfo[i].ext[2];
				putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, temp, 30);
				cons_newline(cons);
			}
		}
	}
	cons_newline(cons);
	return;
}

/** cls命令 */
void cmd_cls(CONSOLE *cons)
{
	int x, y;
	for (y = 28; y < 28 + 128; y++) {
		for (x = 8; x < 8 + 240; x++) {
			cons->sht->buf[x + y * cons->sht->xsize] = COL8_000000;
		}
	}
	sheet_refresh(cons->sht, 8, 28, 8 + 240, 28 + 128);
	cons->cur_y = 28;
}

/** mem命令 */
void cmd_mem(CONSOLE *cons, uint memtotal)
{
	char temp[30];
	MEMMAN *memman = (MEMMAN *) MEMMAN_ADDR;
	sprintf(temp, "total %dMB", memtotal / (1024 * 1024));
	putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, temp, 30);
	cons_newline(cons);
	sprintf(temp, "free %dKB", memman_total(memman) / 1024);
	putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, temp, 30);
	cons_newline(cons);
	cons_newline(cons);
	return;
}

/** 输出一个字符到控制台
 *  move : 是否移动光标
 */
void cons_putchar(CONSOLE *cons, int chr, char move)
{
	char temp[2];
	temp[0] = chr;
	temp[1] = 0;
	if (temp[0] == 0x09) {	//制表符
		for (;;) {
			putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
			cons->cur_x += 8;
			if (cons->cur_x == 8 + 240) {
				cons_newline(cons);
			}
			if (((cons->cur_x - 8) & 0x1f) == 0) {
				break;	//被32整除则跳出，即为4个空格的倍数
			}
		}
	}
	else if (temp[0] == 0x0a) {	//换行符
		cons_newline(cons);
	}
	else if (temp[0] == 0x0d) {	//回车符

	}
	else {	//一般字符
		putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, temp, 1);
		if (move != 0) {
			cons->cur_x += 8;
			if (cons->cur_x == 8 + 240) {
				cons_newline(cons);
			}
		}
	}
	return;
}

/**
 * 控制台换行函数
 * cursor_y：在哪一行换
 * sht：要换行的sheet
 * return:返回换行后的cursor_y
 */
void cons_newline(CONSOLE *cons)
{
	int x, y;
	if (cons->cur_y < 28 + 112) {	//不是最下面
		cons->cur_y += 16;
	}
	else {	//是最下面，需要滚动
		for (y = 28; y < 28 + 112; y++) {	//将所有颜色上移一行
			for (x = 8; x < 8 + 240; x++) {
				cons->sht->buf[x + y * cons->sht->xsize] = cons->sht->buf[x + (y + 16) * cons->sht->xsize];
			}
		}
		for (y = 28 + 112; y < 28 + 128; y++) {	//将最下面的一行抹黑
			for (x = 8; x < 8 + 240; x++) {
				cons->sht->buf[x + y * cons->sht->xsize] = COL8_000000;
			}
		}
		sheet_refresh(cons->sht, 8, 28, 8 + 240, 28 + 128);
	}
	cons->cur_x = 8;
	return;
}
