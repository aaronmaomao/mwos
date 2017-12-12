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
