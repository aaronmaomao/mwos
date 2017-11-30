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
	TIMER *timer;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);	//设置定时器中断周期的高8位
	io_out8(PIT_CNT0, 0x2e);	//设置定时器中断周期的低8位
	timerctl.count = 0;
	timerctl.mintimes = 0xffffffff;
	//timerctl.usingsum = 0;	//起初没有定时器
	for (i = 0; i < MAX_TIMER; i++) {
		timerctl.timers[i].flags = 0;
	}
	timer = timer_alloc();	//哨兵定时器
	timer->next = 0;
	timer->timeout = 0xffffffff;
	timer->flags = TIMER_FLAGS_USING;
	timerctl.mintimer = timer;
	timerctl.mintimes = 0xffffffff;
	//timerctl.usingsum = 1;
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

void timer_init(TIMER *timer, FIFO32 *fifo, int data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

void timer_settime(TIMER *timer, uint timeout)
{
	int e;
	TIMER *t1, *t2;
	timer->timeout = timerctl.count + timeout;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	//timerctl.usingsum++;
	t1 = timerctl.mintimer;
	if (timer->timeout <= t1->timeout) {
		timerctl.mintimer = timer;
		timer->next = t1;
		timerctl.mintimes = timer->timeout;
		io_store_eflags(e);
		return;
	}

	for (;;) {
		t2 = t1;
		t2 = t2->next;
		if (timer->timeout <= t2->timeout) {	//因为有哨兵timer，所以肯定会满足
			t1->next = timer;
			timer->next = t2;
			io_store_eflags(e);
			return;
		}
	}
}

//定时器中断
void inthandler20(int *esp)
{
	int i;
	TIMER *timer;
	io_out8(PIC0_OCW2, 0x60); /* IRQ-07受付完了をPICに通知(7-1参照) */
	timerctl.count++;
	if (timerctl.mintimes > timerctl.count) {
		return;
	}
	timer = timerctl.mintimer;
	for (;;) {	//注：会出现多个定时器同时超时的情况
		if (timer->timeout > timerctl.count) {	//未超时
			break;
		}
		timer->flags = TIMER_FLAGS_ALLOC;	//超时
		fifo32_put(timer->fifo, timer->data);
		timer = timer->next;
	}
	//timerctl.usingsum -= i;	//共i个定时器超时了（一次中断发现多个定时器超时，貌似不是精确定时 ヽ(ﾟДﾟ)ﾉ ）
	timerctl.mintimer = timer;
	timerctl.mintimes = timerctl.mintimer->timeout;
	return;
}
