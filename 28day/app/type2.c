/*
 * type2.c
 *
 *  Created on: 2018年2月8日
 *      Author: mao-zhengjun
 */

#include "../api/apislib.h"

void HariMain()
{
	int fhandle;
	char c, cmdline[30], *p;
	api_cmdline(cmdline, 30);
	for (p = cmdline; *p > ' '; p++) {

	}

	for (; *p == ' '; p++) {

	}
	fhandle = api_fopen(p);
	if (fhandle != 0) {
		for (;;) {
			if (api_fread(&c, 1, fhandle) == 0) {
				break;
			}
			api_putchar(c);
		}
	} else {
		api_putstr0("File not found!");
	}
	api_end();
}

/*
 void HariMain()
 {
 int fhandle;
 char c;
 fhandle = api_fopen("Makefile");
 if (fhandle != 0) {
 for (;;) {
 if (api_fread(&c, 1, fhandle) == 0) {
 break;
 }
 api_putchar(c);
 }
 }
 api_end();
 }
 */
