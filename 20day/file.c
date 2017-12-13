/*
 * file.c
 *
 *  Created on: 2017年12月12日
 *      Author: mao-zhengjun
 */

#include "bootpack.h"

/**
 * max：文件数的最大值
 */
FILEINFO *file_search(char *name, FILEINFO *fileinfo, int max)
{
	int i, j;
	char temp[12];
	for (j = 0; j < 11; j++) {
		temp[j] = ' ';
	}
	j = 0;
	for (i = 0; name[i] != 0; i++) {
		if (j >= 11) {
			return 0;
		} 
		if (name[i] == '.'&&j <= 8) {
			j = 8;
		}
		else {
			temp[j] = name[i];
			if ('a' <= temp[j] && temp[j] <= 'z') {
				temp[j] -= 0x20;
			}
			j++;
		}
	}
	for (i = 0; i < max;) {
		if (fileinfo[i].name[0] == 0x00) {
			break;
		}
		if ((fileinfo[i].type & 0x18) == 0) {
			for (j = 0; j < 11; j++) {
				if (fileinfo[i].name[j] != temp[j]) {
					goto next_file;
				}
			}
			return fileinfo + i; //返回fileinfo的地址
		}
	next_file:
		i++;
	}
	return 0; //没找到
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
	}
	return;
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
