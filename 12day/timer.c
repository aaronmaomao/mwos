/*
 * timer.c
 *
 *  Created on: 2017年11月28日
 *      Author: mao-zhengjun
 */

#include "bootpack.h"

TIMERCTL timerctl;

/**
 * 初始化可编程中断定时器（如8254）
 */
void init_pit()
{
	int i;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);	//设置定时器中断周期的高8位
	io_out8(PIT_CNT0, 0x2e);	//设置定时器中断周期的低8位
	timerctl.count = 0;
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
	timer->timeout = timeout;
	timer->flags = TIMER_FLAGS_USING;
	return;
}

//定时器中断
void inthandler20(int *esp)
{
	int i;
	io_out8(PIC0_OCW2, 0x60); /* IRQ-07受付完了をPICに通知(7-1参照) */
	timerctl.count++;
	for (i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timers[i].flags == TIMER_FLAGS_USING) {
			timerctl.timers[i].timeout--;
			if (timerctl.timers[i].timeout == 0) {
				timerctl.timers[i].flags = TIMER_FLAGS_ALLOC;
				fifo8_put(timerctl.timers[i].fifo, timerctl.timers[i].data);
			}
		}
	}
	return;
}
