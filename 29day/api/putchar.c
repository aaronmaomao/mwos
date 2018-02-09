/*
 * putchar.c
 *
 *  Created on: 2018年2月9日
 *      Author: mao-zhengjun
 */

#include "apislib.h"

int putchar(int c)
{
	api_putchar(c);
	return c;
}
