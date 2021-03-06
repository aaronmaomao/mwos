/*
 * sosu.c
 *
 *  Created on: 2018年2月7日
 *      Author: aaron
 */

#include "stdio.h"
#include "../api/apislib.h"

#define MAX 10000

void HariMain(void)
{
	//char flag[MAX], s[8];
	char *flag, s[8];
	int i, j;
	api_initmalloc();
	flag = api_malloc(MAX);
	for (i = 0; i < MAX; i++) {
		flag[i] = 0;
	}

	for (i = 2; i < MAX; i++) {
		if (flag[i] == 0) {
			sprintf(s, "%d ", i);
			api_putstr0(s);
			for (j = i * 2; j < MAX; j += i) {
				flag[j] = 1;
			}
		}
	}
	api_end();
}
