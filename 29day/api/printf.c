/*
 * printf.c
 *
 *  Created on: 2018年2月9日
 *      Author: mao-zhengjun
 */

#include "apislib.h"
#include "stdarg.h"
#include "stdio.h"

int printf(char *format, ...)
{
	va_list ap;
	char s[1000];
	int i;
	va_start(ap, format);
	i = vsprintf(s, format, ap);
	api_putstr0(s);
	va_end(ap);
	return i;
}
