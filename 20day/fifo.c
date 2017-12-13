#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001

void fifo32_init(FIFO32 *fifo, int size, int *buf, TASK *task)
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size;
	fifo->flags = 0;
	fifo->p = 0;
	fifo->q = 0;
	fifo->task = task; //有数据写入时要唤醒的任务
	return;
}

/** 向fifo缓冲区添加数据 */
int fifo32_put(FIFO32 *fifo, int data)
{
	if (fifo->free == 0) {
		fifo->flags |= FLAGS_OVERRUN;	//溢出
		return -1;
	}
	fifo->buf[fifo->p] = data;	//放数据
	fifo->p++;
	if (fifo->p == fifo->size) {
		fifo->p = 0;	//调整保存索引
	}
	fifo->free--;
	if (fifo->task != 0) {
		if (fifo->task->flags != 2) {	//非运行状态
			task_run(fifo->task, -1, 0);	//唤醒, 不改变优先级
		}
	}
	return 0;
}

/** 从缓冲区取数据 */
int fifo32_get(FIFO32 *fifo)
{
	int data;
	if (fifo->free == fifo->size) {
		return -1;
	}
	data = fifo->buf[fifo->q];
	fifo->q++;
	if (fifo->q == fifo->size) {
		fifo->q = 0;
	}
	fifo->free++;
	return data;
}

/** 缓冲区的状态 */
int fifo32_status(FIFO32 *fifo)
{
	return fifo->size - fifo->free;
}
