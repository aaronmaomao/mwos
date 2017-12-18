/*
 * memory.c
 *
 *  Created on: 2017年11月21日
 *      Author: aaron
 */

#include "bootpack.h"

/*
 * 获取内存大小
 */
uint memtest(uint start, uint end)
{
	char flg486 = 0;
	uint eflag, cr0, i;
	eflag = io_load_eflags();	//开始检查额flag的18位是否有效
	eflag |= EFLAGS_AC_BIT;
	io_store_eflags(eflag);
	eflag = io_load_eflags();
	if ((eflag & EFLAGS_AC_BIT) != 0) {
		flg486 = 1;
	}
	eflag &= ~EFLAGS_AC_BIT;	//将eflag还原
	io_store_eflags(eflag);

	if (flg486 != 0) {	//如果是486
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE;	//禁止缓存
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flg486 != 0) {	//如果是486
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE;	//允许缓存
		store_cr0(cr0);
	}
	return i;
}

void memman_init(MEMMAN *man)
{
	man->frees = 0;
	man->maxfress = 0;
	man->lostsize = 0;
	man->losts = 0;
	return;
}

/**
 * 返回空闲内存大小
 */
uint memman_total(MEMMAN *man)
{
	uint i, t = 0;
	for (i = 0; i < man->frees; i++) {
		t += man->free[i].size;
	}
	return t;
}

/**
 * 申请内存
 */
uint memman_alloc(MEMMAN *man, uint size)
{
	uint i, addr;
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].size >= size) {
			addr = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if (man->free[i].size == 0) {
				man->frees--;
				for (; i < man->frees; i++) {
					man->free[i] = man->free[i + 1];	//将后面的结构体前移
				}
			}
			return addr;
		}
	}
	return 0;
}

/**
 * 释放内存(并且归纳内存)
 */
int memman_free(MEMMAN *man, uint addr, uint size)
{
	int i, j;
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].addr > addr) {
			break;
		}
	}

	if (i > 0) {	//说明在 要释放的内存之前有空闲内存
		if (man->free[i - 1].addr + man->free[i - 1].size == addr) {	//前归纳
			man->free[i - 1].size += size;
			if (i < man->frees) {	//归纳后检查后面的空闲内存是否与前面的接壤，是的话还可以归纳
				if (addr + size == man->free[i].addr) {
					man->free[i - 1].size += man->free[i].size;
					man->frees--;
					for (; i < man->frees; i++) {
						man->free[i] = man->free[i + 1];	//将后面的结构体前移
					}
				}
			}
			return 0;	//释放（归纳）完成
		}
	}

	if (i < man->frees) {
		if (addr + size == man->free[i].addr) {	//后归纳
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0;	//释放（归纳）完成
		}
	}

	//既不能前归纳又不能后归纳
	if (man->frees < MEMMAN_FREES) {	//管理表中还有空位
		for (j = man->frees; j > i; j--) {	//把要释放的内存插在 i 处
			man->free[j] = man->free[j - 1];	//把  i 后面的元素整体往后移一下（把 i 位置腾出来）
		}
		man->frees++;
		if (man->maxfress < man->frees) {
			man->maxfress = man->frees;
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0;	//完成
	}

	//内存空闲管理表已满，只能lost掉了(⊙︿⊙)
	man->losts++;
	man->lostsize += size;
	return -1;
}

/**
 * 以4k为单位分配内存
 */
uint memman_alloc_4k(MEMMAN *man, uint size)
{
	uint addr;
	size = (size + 0xfff) & 0xfffff000;	//+0xfff，然后向下舍入，得到n*4k的大小
	addr = memman_alloc(man, size);
	return addr;
}

/**
 * 以4k为单位释放内存
 */
uint memman_free_4k(MEMMAN *man, uint addr, uint size)
{
	uint result;
	size = (size + 0xfff) & 0xfffff000;
	result = memman_free(man, addr, size);
	return result;
}
