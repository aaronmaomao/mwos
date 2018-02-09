/*
 * free.c
 *
 *  Created on: 2018年2月9日
 *      Author: mao-zhengjun
 */

#include "apislib.h"

void free(void *p)
{
	char *q = p;
	int size;
	if (q != 0) {
		q -= 16;
		size = *((int *) q);
		api_free(q, size + 16);
	}
	return;
}
