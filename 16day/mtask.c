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

	for (i = 0; i < MAX_TASKLEVELS; i++) {
		taskctl->level[i].now = 0;
		taskctl->level[i].runnum = 0;
	}
	task = task_alloc();
	task->flags = 2;
	task->priority = 2;
	task->level = 0;
	task_add(task);
	task_switchsub();
	load_tr(task->sel);
	task_timer = timer_alloc();
	timer_settime(task_timer, task->priority);
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

/**
 * 将task添加到运行队列
 */
void task_run(TASK *task, int level, int priority)
{
	if (level < 0) {
		level = task->level;
	}
	if (priority > 0) {
		task->priority = priority;
	}
	if (task->flags == 2 && level != task->level) {
		task_remove(task);	//改变当前运行任务的优先级组
	}
	if (task->flags != 2) {
		task->level = level;
		task_add(task);
	}
	taskctl->lv_change = 1;	//下次切换任务时检查level
	return;
}

void task_switch(void)
{
	TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	TASK *newtask, *nowtask = tl->tasks[tl->now];
	tl->now++;
	if (tl->now == tl->runnum) {
		tl->now = 0;
	}
	if (taskctl->lv_change != 0) {
		task_switchsub();
		tl = &taskctl->level[taskctl->now_lv];
	}
	newtask = tl->tasks[tl->now];
	timer_settime(task_timer, newtask->priority);
	if (newtask != nowtask) {
		farjmp(0, newtask->sel);
	}
	return;
}

void task_sleep(TASK *task)
{
	TASK *nowtask;
	if (task->flags == 2) {
		nowtask = task_now();
		task_remove(task);
		if (task == nowtask) {
			task_switchsub();
			nowtask = task_now();
			farjmp(0, nowtask->sel);
		}
	}
	return;
}

TASK *task_now(void)
{
	TASKLEVEL *tl = &(taskctl->level[taskctl->now_lv]);
	return tl->tasks[tl->now];
}
/**
 *	向优先级组中添加一个任务
 */
void task_add(TASK *task)
{
	TASKLEVEL *tl = &(taskctl->level[task->level]);
	tl->tasks[tl->runnum] = task;
	tl->runnum++;
	task->flags = 2;
	return;
}

/**
 * 从优先级组中删除一个任务
 */
void task_remove(TASK *task)
{
	int i;
	TASKLEVEL *tl = &(taskctl->level[task->level]);
	for (i = 0; i < tl->runnum; i++) {
		if (tl->tasks[i] == task) {
			break;
		}
	}
	tl->runnum--;
	if (i < tl->now) {
		tl->now--;
	}
	if (tl->now >= tl->runnum) {
		tl->now = 0;
	}
	task->flags = 1;
	for (; i < tl->runnum; i++) {
		tl->tasks[i] = tl->tasks[i + 1];
	}
	return;
}

/**
 * 判断下一个运行的优先级组
 */
void task_switchsub(void)
{
	int i;
	for (i = 0; i < MAX_TASKLEVELS; i++) {
		if (taskctl->level[i].runnum > 0) {
			break;
		}

	}
	taskctl->now_lv = i;
	taskctl->lv_change = 0;
	return;
}
