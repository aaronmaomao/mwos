/*
 * malloc.c
 *
 *  Created on: 2018年2月9日
 *      Author: mao-zhengjun
 */

#include "apislib.h"

void *malloc(int size)
{
	char *p = api_malloc(size + 16);
	if (p != 0) {
		*((int *) p) = size; //把所申请的大小放在前面16个字节处
		p += 16;
	}
	return p;
}
