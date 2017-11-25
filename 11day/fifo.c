#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001

void fifo8_init(FIFO8 *fifo, int size, unsigned char *buf)
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size;
	fifo->flags = 0;
	fifo->p = 0;
	fifo->q = 0;
	return;
}

/** 向fifo缓冲区添加数据 */
int fifo8_put(FIFO8 *fifo, unsigned char data)
{
	if(fifo->free == 0) {
		fifo->flags |= FLAGS_OVERRUN;	//溢出
		return -1;
	}
	fifo->buf[fifo->p] = data;	//放数据
	fifo->p++;
	if(fifo->p == fifo->size) {
		fifo->p = 0;	//调整保存索引
	}
	fifo->free--;
	return 0;
}

/** 从缓冲区取数据 */
int fifo8_get(FIFO8 *fifo)
{
	int data;
	if(fifo->free == fifo->size) {
		return -1;
	}
	data = fifo->buf[fifo->q];
	fifo->q++;
	if(fifo->q == fifo->size) {
		fifo->q = 0;
	}
	fifo->free++;
	return data;
}

/** 缓冲区的状态 */
int fifo8_status(FIFO8 *fifo)
{
	return fifo->size - fifo->free;
}
