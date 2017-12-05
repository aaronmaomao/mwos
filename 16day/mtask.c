/*
 * mtask.c
 *
 *  Created on: 2017年12月5日
 *      Author: mao-zhengjun
 */
#include "bootpack.h"

TIMER *task_timer;
TASKCTL *taskctl;

/**
 * 初始化task之后，所有的可运行程序都由任务来管理
 * 返回的TASK即为主程序对应的任务
 */
TASK *task_init(MEMMAN *mem)
{
	int i;
	TASK *task;
	SEGMENT_DESC *gdt = (SEGMENT_DESC *) ADR_GDT;
	taskctl = (TASKCTL *) memman_alloc_4k(mem, sizeof(TASKCTL));
	for (i = 0; i < MAX_TASKS; i++) {
		taskctl->task[i].flags = 0;
		taskctl->task[i].sel = (TASK_BGDT + i) * 8;
		set_segmdesc(gdt + TASK_BGDT + i, 103, (int) &taskctl->task[i].tss, AR_TSS32);
	}
	task = task_alloc();
	task->flags = 2;
	taskctl->runnum = 1;
	taskctl->now = 0;
	taskctl->taskseq[0] = task;
	load_tr(task->sel);
	task_timer = timer_alloc();
	timer_settime(task_timer, 2);
	return task;
}

TASK *task_alloc(void)
{
	int i;
	TASK *task;
	for (i = 0; i < MAX_TASKS; i++) {
		if (taskctl->task[i].flags == 0) {
			task = &taskctl->task[i];
			task->flags = 1;
			task->tss.eflags = 0x00000202;	//IF = 1
			task->tss.eax = 0;
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.ldtr = 0;
			task->tss.iomap = 0x40000000;
			return task;
		}
	}
	return 0;
}

void task_run(TASK *task)
{
	task->flags = 2;
	taskctl->taskseq[taskctl->runnum] = task;
	taskctl->runnum++;
	return;
}

void task_switch(void)
{
	timer_settime(task_timer, 2);
	if (taskctl->runnum >= 2) {
		taskctl->now++;
		if (taskctl->now == taskctl->runnum) {
			taskctl->now = 0;
		}
		farjmp(0, taskctl->taskseq[taskctl->now]->sel);
	}
	return;
}

void task_sleep(TASK *task)
{
	int i;
	char ts = 0;
	if (task->flags == 2) {
		if (task == taskctl->taskseq[taskctl->now]) {
			ts = 1;	//task就是当前运行的任务，即自己睡眠自己
		}

		for (i = 0; i < taskctl->runnum; i++) {	//当前任务睡眠其他的任务
			if (taskctl->taskseq[i] == task) {
				break;
			}
		}
		taskctl->runnum--;
		if (i < taskctl->now) {
			taskctl->now--;
		}
		for (; i < taskctl->runnum; i++) {
			taskctl->taskseq[i] = taskctl->taskseq[i + 1];	//把要睡眠的任务从运行列队中踢掉
		}
		task->flags = 1;	//把当前task设为不工作（睡眠）状态

		if (ts != 0) {	//自己睡眠自己
			if (taskctl->now >= taskctl->runnum) {	//即位于任务列队的最后一位
				taskctl->now = 0; //所以下一个任务为最前面的
			}
			farjmp(0, taskctl->taskseq[taskctl->now]->sel);
		}
	}
	return;
}
