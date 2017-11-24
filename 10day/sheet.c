/*
 * sheet.c
 *
 *  Created on: 2017年11月22日
 *      Author: aaron
 */

#include "bootpack.h"

/**
 * 生成一个图层管理表
 */
SHEETCTL *sheetctl_init(MEMMAN *memman, uchar *vram, int xsize, int ysize)
{
	SHEETCTL *ctl;
	int i;
	ctl = (SHEETCTL *) memman_alloc_4k(memman, sizeof(SHEETCTL));
	if (ctl == 0) {
		goto err;
	}
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1;	//一个图层都没有
	for (i = 0; i < MAX_SHEETS; i++) {
		ctl->sheets[i].flags = 0;	//标记为未使用
	}
	err: return ctl;
}

#define SHEET_USE 1

/**
 * 申请一个空闲的图层
 */
SHEET *sheet_alloc(SHEETCTL *ctl)
{
	SHEET *sheet;
	int i;
	for (i = 0; i < MAX_SHEETS; i++) {
		if (ctl->sheets[i].flags == 0) {
			sheet = &(ctl->sheets[i]);	//取得空闲图层的地址
			sheet->flags = SHEET_USE;	//标记为该图层正在使用
			sheet->zindex = -1;	//隐藏
			return sheet;
		}
	}
	return 0;	//没图层了，所有图层都在使用中
}

/**
 * 设置图层参数
 */
void sheet_setbuf(SHEET *sht, uchar *buf, int xsize, int ysize, int col_inv)
{
	sht->buf = buf;
	sht->xsize = xsize;
	sht->ysize = ysize;
	sht->col_inv = col_inv;
	return;
}

/**
 * 移动图层位置
 */
void sheet_updown(SHEETCTL *ctl, SHEET *sht, int zindex)
{
	int tempindex, oldindex = sht->zindex;
	if (zindex > ctl->top + 1) {
		zindex = ctl->top + 1;
	}
	if (zindex < -1) {
		zindex = -1;
	}
	sht->zindex = zindex;	//给图层设置高度
	/** 对所有图层进行排列 */
	if (oldindex > zindex) {	//说明是降低图层的zindex
		if (zindex >= 0) {	//非隐藏
			for (tempindex = oldindex; tempindex > zindex; tempindex--) {
				ctl->sheetseq[tempindex] = ctl->sheetseq[tempindex - 1];	//（调整队伍，类似于后面的人往前调位置）
				ctl->sheetseq[tempindex]->zindex = tempindex;	//重设调整后图层的zindex
			}
			ctl->sheetseq[zindex] = sht;
		} else {	//隐藏
			if (ctl->top > oldindex) {
				for (tempindex = oldindex; tempindex < ctl->top; tempindex++) {	//去掉隐藏图层
					ctl->sheetseq[tempindex] = ctl->sheetseq[tempindex + 1];	//调整图层位置（类似于列队中走了一个人）
					ctl->sheetseq[tempindex]->zindex = tempindex;
				}
			}
			ctl->top--;
		}
		sheet_refresh(ctl);
	} else if (zindex > oldindex) {		//比以前高
		if (oldindex >= 0) {	//之前是非隐藏状态
			for (tempindex = oldindex; tempindex < zindex; tempindex++) {
				ctl->sheetseq[tempindex] = ctl->sheetseq[tempindex + 1];	//调整队伍，类似于前面的人往后调位置
				ctl->sheetseq[tempindex]->zindex = tempindex;
			}
			ctl->sheetseq[zindex] = sht;	//调位成功
		} else {	//之前是隐藏状态
			for (tempindex = ctl->top; tempindex >= zindex; tempindex--) {
				ctl->sheetseq[tempindex + 1] = ctl->sheetseq[tempindex];	//我靠，有人在插队￣へ￣
				ctl->sheetseq[tempindex + 1]->zindex = tempindex + 1;
			}
			ctl->sheetseq[zindex] = sht;	//没办法，插就插吧 (T＿T)
			ctl->top++;	//队伍加一
		}
		sheet_refresh(ctl);
	}
	return;
}

/**
 * 刷新所有图层(从最里面的图层开始，将每个图层的非透明的像素放到对应显存位置，类似于"千手观音"原理)
 */
void sheet_refresh(SHEETCTL *ctl)
{
	int zindex, bufy, bufx, ly, lx;
	SHEET *sht;
	uchar *buf, color, *vram = ctl->vram;
	for (zindex = 0; zindex <= ctl->top; zindex++) {
		sht = ctl->sheetseq[zindex];
		buf = sht->buf;
		for (bufy = 0; bufy < sht->ysize; bufy++) {	//循环像素列
			ly = bufy + sht->ly;
			for (bufx = 0; bufx < sht->xsize; bufx++) {	//循环像素行
				lx = bufx + sht->lx;
				color = buf[bufy * sht->xsize + bufx];	//获取该图层该位置处的颜色
				if (color != sht->col_inv) {		//如果不是透明色
					vram[ly * ctl->xsize + lx] = color;	//把像素颜色放到显存对应位置处
				}
			}
		}
	}
	return;
}

/**
 * 处理图层滑动
 */
void sheet_slide(SHEETCTL *ctl, SHEET *sht, int lx, int ly)
{
	sht->lx = lx;
	sht->ly = ly;
	if (sht->zindex >= 0) {
		sheet_refresh(ctl);
	}
	return;
}

/**
 * 释放图层
 */
void sheet_free(SHEETCTL *ctl, SHEET *sht)
{
	if (sht->zindex >= 0) {
		sheet_updown(ctl, sht, -1);	//先隐藏起来
	}
	sht->flags = 0;
	return;
}
