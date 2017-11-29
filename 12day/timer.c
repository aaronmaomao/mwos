/*
 * timer.c
 *
 *  Created on: 2017年11月28日
 *      Author: mao-zhengjun
 */

#include "bootpack.h"

TIMERCTL timerctl;

/**
 * 初始化可编程中断定时器（如8254）,初始定时单位为10ms
 */
void init_pit()
{
	int i;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);	//设置定时器中断周期的高8位
	io_out8(PIT_CNT0, 0x2e);	//设置定时器中断周期的低8位
	timerctl.count = 0;
	timerctl.mintimer = 0xffffffff;
	timerctl.usingsum = 0;	//起初没有定时器
	for (i = 0; i < MAX_TIMER; i++) {
		timerctl.timers[i].flags = 0;
	}
	return;
}

/**
 * 申请一个定时器
 */
TIMER *timer_alloc(void)
{
	int i;
	for (i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timers[i].flags == 0) {
			timerctl.timers[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timers[i];
		}
	}
	return 0;
}

void timer_free(TIMER *timer)
{
	timer->flags = 0;
	return;
}

void timer_init(TIMER *timer, FIFO8 *fifo, uchar data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

void timer_settime(TIMER *timer, uint timeout)
{
	int e, i, j;
	timer->timeout = timerctl.count + timeout;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	for (i = 0; i < timerctl.usingsum; i++) {
		if (timerctl.timerseq[i]->timeout >= timer->timeout) {
			break;
		}
	}
	for (j = timerctl.usingsum; j > i; j--) {
		timerctl.timerseq[j] = timerctl.timerseq[j - 1];
	}
	timerctl.usingsum++;
	timerctl.timerseq[i] = timer;
	timerctl.mintimer = timerctl.timerseq[0]->timeout;
	io_store_eflags(e);
	return;
}

//定时器中断
void inthandler20(int *esp)
{
	int i, j;
	io_out8(PIC0_OCW2, 0x60); /* IRQ-07受付完了をPICに通知(7-1参照) */
	timerctl.count++;
	if (timerctl.mintimer > timerctl.count) {
		return;
	}
	for (i = 0; i < timerctl.usingsum; i++) {
		if (timerctl.timerseq[i]->timeout > timerctl.count) {	//未超时
			break;
		}
		timerctl.timerseq[i]->flags = TIMER_FLAGS_ALLOC;	//超时
		fifo8_put(timerctl.timerseq[i]->fifo, timerctl.timerseq[i]->data);
	}
	timerctl.usingsum--;
	for (j = 0; j < timerctl.usingsum; j++) {	//将未超时的定时器逐个前移
		timerctl.timerseq[j] = timerctl.timerseq[j + 1];
	}
	if (timerctl.usingsum > 0) {
		timerctl.mintimer = timerctl.timerseq[0]->timeout;
	} else {
		timerctl.mintimer = 0xffffffff;
	}
	return;
}
