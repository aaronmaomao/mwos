/*
 * mtask.c
 *
 *  Created on: 2017年12月5日
 *      Author: mao-zhengjun
 */
#include "bootpack.h"

TIMER *mt_timer;
int mt_tr;

void mt_init()
{
	mt_timer = timer_alloc(); //用来切换任务的定时器，不需要fifo
	timer_settime(mt_timer, 2);
	mt_tr = 3 * 8;
	return;
}

void mt_taskswitch()
{
	if (mt_tr == 3 * 8) {
		mt_tr = 4 * 8;
	} else {
		mt_tr = 3 * 8;
	}
	timer_settime(mt_timer, 2);
	farjmp(0, mt_tr);
	return;
}
