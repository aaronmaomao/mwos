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
	MEMMAN *memman = (MEMMAN *)MEMMAN_ADDR;
	int *fat = (int *)memman_alloc_4k(memman, 4 * 2880);

	cons.cur_x = 8;
	cons.cur_y = 28;
	cons.cur_c = -1;
	cons.sht = sht;
	*((int *)0x0fec) = (int)&cons;

	fifo32_init(&task->fifo, 128, fifobuf, task);
	cursor_timer = timer_alloc();
	timer_init(cursor_timer, &task->fifo, 1);
	timer_settime(cursor_timer, 50);

	file_readfat(fat, (uchar *)(ADR_DISKIMG + 0x000200));
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
	else if (cmdLine[0] != 0) {
		if (cmd_app(cons, fat, cmdLine) == 0) { //不是命令也不是应用程序
			cons_putstr0(cons, "Unknow command.\n\n");
		}
	}
	return;
}

/* 运行app 命令 */
int cmd_app(CONSOLE *cons, int *fat, char *cmdLine)
{
	char *p, *q;
	char name[18];
	int i, segsize, datsize, esp, dathrb;
	FILEINFO *fileinfo;
	MEMMAN *memman = (MEMMAN *)MEMMAN_ADDR;
	SEGMENT_DESC *gdt = (SEGMENT_DESC *)ADR_GDT;
	TASK *task = task_now();

	for (i = 0; i < 13; i++) {
		if (cmdLine[i] <= ' ') {
			break;
		}
		name[i] = cmdLine[i];
	}
	name[i] = 0;
	fileinfo = file_search(name, (FILEINFO *)(ADR_DISKIMG + 0x002600), 224); //不加后缀名查找
	if (fileinfo == 0 && name[i - 1] != '.') {
		name[i] = '.';
		name[i + 1] = 'M';
		name[i + 2] = 'W';
		name[i + 3] = 'E';
		name[i + 4] = 0;
		fileinfo = file_search(name, (FILEINFO *)(ADR_DISKIMG + 0x002600), 224);  //加上后缀名重新查找
	}
	if (fileinfo != 0) {
		p = (char *)memman_alloc_4k(memman, fileinfo->size);
		file_loadfile(fileinfo->clustno, fileinfo->size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
		if (fileinfo->size >= 36 && strncmp(p + 4, "Hari", 4) == 0 && *p == 0) {
			segsize = *((int *)(p + 0x0000));	//数据区（包括ss和ds）大小
			esp = *((int *)(p + 0x000c));	//esp寄存器的初始值()
			datsize = *((int *)(p + 0x0010));	//向数据段传送的部分的字节数（即“helloworld”的大小）
			dathrb = *((int *)(p + 0x0014));	//向数据段传送的的部分在hrb中的位置（即“helloworld”的偏移地址）
			q = (char *)memman_alloc_4k(memman, segsize);
			*((int *)0x0fe8) = (int)q;	//把数据段的地址存起来
			set_segmdesc(gdt + 1003, fileinfo->size - 1, (int)p, AR_CODE32_ER + 0x60);	//代码段：注：1003之前的都被用了,0x60意思是这个段是应用程序用
			set_segmdesc(gdt + 1004, segsize - 1, (int)q, AR_DATA32_RW + 0x60);		//数据段
			for (i = 0; i < datsize; i++) {
				q[esp + i] = p[dathrb + i];
			}
			start_app(0x1b, 1003 * 8, esp, 1004 * 8, &(task->tss.esp0));
			memman_free_4k(memman, (int)q, segsize);
		}
		else {
			cons_putstr0(cons, "This is not an Executable file");
		}

		memman_free_4k(memman, (int)p, fileinfo->size);
		cons_newline(cons);
		return 1;
	}
	return 0;
}

/* type 命令 */
void cmd_type(CONSOLE *cons, int *fat, char *cmdLine)
{
	char *p;
	FILEINFO *fileinfo = file_search(cmdLine + 5, (FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
	MEMMAN *memman = (MEMMAN *)MEMMAN_ADDR;

	if (fileinfo != 0) {
		p = (char *)memman_alloc_4k(memman, fileinfo->size);
		file_loadfile(fileinfo->clustno, fileinfo->size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
		cons_putstr1(cons, p, fileinfo->size);
		memman_free_4k(memman, (int)p, fileinfo->size);
	}
	else {	//没找到
		cons_putstr0(cons, "File not found.\n");
	}
	cons_newline(cons);
	return;
}

/* dir 命令 */
void cmd_dir(CONSOLE *cons)
{
	int i, j;
	char temp[30];
	FILEINFO *fileinfo = (FILEINFO *)(ADR_DISKIMG + 0x002600);
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
				cons_putstr0(cons, temp);
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
	char temp[60];
	MEMMAN *memman = (MEMMAN *)MEMMAN_ADDR;
	sprintf(temp, "total %dMB\nfree %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	cons_putstr0(cons, temp);
	cons_newline(cons);
	return;
}

void cons_putstr0(CONSOLE *cons, char *str)
{
	for (; *str != 0; str++) {
		cons_putchar(cons, *str, 1);
	}
	return;
}

void cons_putstr1(CONSOLE *cons, char *str, int len)
{
	int i;
	for (i = 0; i < len; i++) {
		cons_putchar(cons, str[i], 1);
	}
	return;
}

/**
 * 这算是个系统功能： 输出一个字符到控制台
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

/**
 * api
 */
int *mwe_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
	CONSOLE *cons = (CONSOLE *) *((int *)0x0fec);
	int ds_base = *((int *)0x0fe8);	//应用程序的数据段
	TASK *task = task_now();
	SHEETCTL *shtctl = (SHEETCTL *) *((int *)0x0fe4);
	SHEET *sht;
	int *reg = &eax + 1;
	if (edx == 1) {
		cons_putchar(cons, eax & 0xff, 1);
	}
	else if (edx == 2) {
		cons_putstr0(cons, (char *)ebx + ds_base);
	}
	else if (edx == 3) {
		cons_putstr1(cons, (char *)ebx + ds_base, ecx);
	}
	else if (edx == 4) {	//结束应用程序的api
		return &(task->tss.esp0);
	}
	else if (edx == 5) {
		sht = sheet_alloc(shtctl);
		sheet_setbuf(sht, (char *)ebx + ds_base, esi, edi, eax);
		make_window8((char *)ebx + ds_base, esi, edi, (char *)ecx + ds_base, 0);
		sheet_slide(sht, 100, 50);
		sheet_updown(sht, 3);
		reg[7] = (int)sht;
	}
	else if (edx == 6) {
		sht = (SHEET *)(ebx & 0xfffffffe);
		putfonts8_asc(sht->buf, sht->xsize, esi, edi, eax, (char *)ebp + ds_base);
		if ((ebx & 1) == 0) {  //偶地址即为真实的地址
			sheet_refresh(sht, esi, edi, esi + ecx * 8, edi + 16);
		}
	}
	else if (edx == 7) {
		sht = (SHEET *)(ebx & 0xfffffffe);
		boxfill8(sht->buf, sht->xsize, ebp, eax, ecx, esi, edi);
		if ((ebx & 1) == 0) {
			sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
		}
	}
	else if (edx == 8) {  //初始化应用程序的memman
		memman_init((MEMMAN *)(ebx + ds_base));
		ecx &= 0xfffffff0;  //以16字节取整
		memman_free((MEMMAN *)(ebx + ds_base), eax, ecx);
	}
	else if (edx == 9) {  //应用程序的malloc
		ecx = (ecx + 0x0f) & 0xfffffffe;
		reg[7] = memman_alloc((MEMMAN *)(ebx + ds_base), ecx);
	}
	else if (edx == 10) {  //应用程序的free
		ecx = (ecx + 0x0f) & 0xfffffff0;
		memman_free((MEMMAN *)(ebx + ds_base), eax, ecx);
	}
	else if (edx == 11) {  //在窗口上画点
		sht = (SHEET *)(ebx & 0xfffffffe);
		sht->buf[sht->xsize*edi + esi] = eax;
		if ((ebx & 1) == 0) {
			sheet_refresh(sht, esi, edi, esi + 1, edi + 1);
		}
	}
	else if (edx == 12) //刷新图层
	{
		sht = (SHEET *)ebx;
		sheet_refresh(sht, eax, ecx, esi, edi);
	}
	else if (edx == 13) {  //画直线
		sht = (SHEET *)(ebx & 0xfffffffe);
	}
	return 0;
}

/* 在win上画直线 */
void mw_api_linewin(SHEET *sht, int x0, int y0, int x1, int y1) {

}

/**
 * cpu一般异常的中断
 */
int *inthandler0d(int *esp)
{
	CONSOLE *cons = (CONSOLE *) *((int *)0x0fec);
	TASK *task = task_now();
	cons_putstr0(cons, "\nINT 0D : General Protected Exception.\n");
	return &(task->tss.esp0);
}

/**
 * cpu栈异常的中断
 */
int *inthandler0c(int *esp)
{
	CONSOLE *cons = (CONSOLE *) *((int *)0x0fec);
	char temp[30];
	TASK *task = task_now();
	cons_putstr0(cons, "\nINT 0D : Stack Exception.\n");
	sprintf(temp, "EIP = %08X\n", esp[11]);
	cons_putstr0(cons, temp);
	return &(task->tss.esp0);
}

