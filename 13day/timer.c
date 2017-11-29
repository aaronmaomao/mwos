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
	timerctl.mintimes = 0xffffffff;
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

void timer_init(TIMER *timer, FIFO32 *fifo, int data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

void timer_settime(TIMER *timer, uint timeout)
{
	int e, i, j;
	TIMER *temp;
	timer->timeout = timerctl.count + timeout;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();

	if (timerctl.usingsum == 0) {
		timerctl.mintimer = timer;
		timer->next = 0;
		timerctl.mintimes = timer->timeout;
		io_store_eflags(e);
		return;
	}
	if (timer->timeout <= timerctl.mintimer->timeout) {
		timerctl.mintimer = timer;
		timer->next = timerctl.mintimer;
		timerctl.mintimes = timer->timeout;
		io_store_eflags(e);
		return;
	}
	temp = timerctl.mintimer;
	for(;;){
		if(temp->next==0){
			temp->next = timer;
			timer->next =0;
			break;
		}
	}


	timerctl.usingsum++;
	timerctl.timerseq[i] = timer;
	timerctl.mintimes = timerctl.timerseq[0]->timeout;
	io_store_eflags(e);
	return;
}

//定时器中断
void inthandler20(int *esp)
{
	int i, j;
	TIMER *timer;
	io_out8(PIC0_OCW2, 0x60); /* IRQ-07受付完了をPICに通知(7-1参照) */
	timerctl.count++;
	if (timerctl.mintimes > timerctl.count) {
		return;
	}
	timer = timerctl.timerseq[0];
	for (i = 0; i < timerctl.usingsum; i++) {	//注：会出现多个定时器同时超时的情况
		if (timerctl.timerseq[i]->timeout > timerctl.count) {	//未超时
			break;
		}
		timerctl.timerseq[i]->flags = TIMER_FLAGS_ALLOC;	//超时
		fifo32_put(timerctl.timerseq[i]->fifo, timerctl.timerseq[i]->data);
		timer->next = timer;
	}
	timerctl.usingsum -= i;	//共i个定时器超时了（一次中断发现多个定时器超时，貌似不是精确定时 ヽ(ﾟДﾟ)ﾉ ）
	timerctl.timerseq[0] = timer;
	if (timerctl.usingsum > 0) {
		timerctl.mintimes = timerctl.timerseq[0]->timeout;
	} else {
		timerctl.mintimes = 0xffffffff;
	}
	return;
}
