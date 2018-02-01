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
	ctl = (SHEETCTL *)memman_alloc_4k(memman, sizeof(SHEETCTL));
	if (ctl == 0) {
		goto err;
	}
	ctl->map = (uchar *)memman_alloc_4k(memman, xsize * ysize);
	if (ctl->map == 0) {
		memman_free_4k(memman, (int)ctl, sizeof(SHEETCTL));
		goto err;
	}
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1;	//一个图层都没有
	for (i = 0; i < MAX_SHEETS; i++) {
		ctl->sheets[i].flags = 0;	//标记为未使用
		ctl->sheets[i].ctl = ctl;
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
			sheet->task = 0;
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
void sheet_updown(SHEET *sht, int zindex)
{
	int tempindex, oldindex = sht->zindex;
	SHEETCTL *ctl = sht->ctl;
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
			sheet_refreshmap(ctl, sht->lx, sht->ly, sht->lx + sht->xsize, sht->ly + sht->ysize, zindex + 1);
			sheet_refreshsub(ctl, sht->lx, sht->ly, sht->lx + sht->xsize, sht->ly + sht->ysize, zindex + 1, oldindex);
		}
		else {	//隐藏
			if (ctl->top > oldindex) {
				for (tempindex = oldindex; tempindex < ctl->top; tempindex++) {	//去掉隐藏图层
					ctl->sheetseq[tempindex] = ctl->sheetseq[tempindex + 1];	//调整图层位置（类似于列队中走了一个人）
					ctl->sheetseq[tempindex]->zindex = tempindex;
				}
			}
			ctl->top--;
			sheet_refreshmap(ctl, sht->lx, sht->ly, sht->lx + sht->xsize, sht->ly + sht->ysize, 0);
			sheet_refreshsub(ctl, sht->lx, sht->ly, sht->lx + sht->xsize, sht->ly + sht->ysize, 0, oldindex - 1);
		}
	}
	else if (zindex > oldindex) {		//比以前高
		if (oldindex >= 0) {	//之前是非隐藏状态
			for (tempindex = oldindex; tempindex < zindex; tempindex++) {
				ctl->sheetseq[tempindex] = ctl->sheetseq[tempindex + 1];	//调整队伍，类似于前面的人往后调位置
				ctl->sheetseq[tempindex]->zindex = tempindex;
			}
			ctl->sheetseq[zindex] = sht;	//调位成功
		}
		else {	//之前是隐藏状态
			for (tempindex = ctl->top; tempindex >= zindex; tempindex--) {
				ctl->sheetseq[tempindex + 1] = ctl->sheetseq[tempindex];	//我靠，有人在插队￣へ￣
				ctl->sheetseq[tempindex + 1]->zindex = tempindex + 1;
			}
			ctl->sheetseq[zindex] = sht;	//没办法，插就插吧 (T＿T)
			ctl->top++;	//队伍加一
		}
		sheet_refreshmap(ctl, sht->lx, sht->ly, sht->lx + sht->xsize, sht->ly + sht->ysize, zindex);
		sheet_refreshsub(ctl, sht->lx, sht->ly, sht->lx + sht->xsize, sht->ly + sht->ysize, zindex, zindex);
	}
	return;
}

/**
 * 刷新所有图层(从最里面的图层开始，将每个图层的非透明的像素放到对应显存位置，类似于"千手观音"原理)
 */
void sheet_refresh(SHEET *sht, int vx0, int vy0, int vx1, int vy1)
{
	if (sht->zindex >= 0) {
		sheet_refreshsub(sht->ctl, sht->lx + vx0, sht->ly + vy0, sht->lx + vx1, sht->ly + vy1, sht->zindex, sht->zindex);
	}
	return;
}

/**
 * 只刷新指定大小位置的区域
 * zindex: 刷新该层级以上的图层
 */
void sheet_refreshsub(SHEETCTL *ctl, int vx0, int vy0, int vx1, int vy1, int zindex0, int zindex1)
{
	int index, bufy, bufx, ly, lx, tbufx0, tbufy0, tbufx1, tbufy1, tbufx2, sid4, i, i1, *p, *q, *r;
	SHEET *sht;
	uchar *buf, sid, *vram = ctl->vram, *map = ctl->map;
	if (vx0 < 0) {
		vx0 = 0;
	}
	if (vy0 < 0) {
		vy0 = 0;
	}
	if (vx1 > ctl->xsize) {
		vx1 = ctl->xsize;
	}
	if (vy1 > ctl->ysize) {
		vy1 = ctl->ysize;
	}
	for (index = zindex0; index <= zindex1; index++) {
		sht = ctl->sheetseq[index];
		buf = sht->buf;
		sid = sht - ctl->sheets;
		tbufx0 = vx0 - sht->lx;
		tbufy0 = vy0 - sht->ly;
		tbufx1 = vx1 - sht->lx;
		tbufy1 = vy1 - sht->ly;
		if (tbufx0 < 0) {	//把要刷新部分的像素从图层中括出来
			tbufx0 = 0;
		}
		if (tbufy0 < 0) {
			tbufy0 = 0;
		}
		if (tbufx1 > sht->xsize) {
			tbufx1 = sht->xsize;
		}
		if (tbufy1 > sht->ysize) {
			tbufy1 = sht->ysize;
		}

		if ((sht->lx & 3) == 0) {  //4字节型
			i = (tbufx0 + 3) / 4;  //小数进位
			i1 = tbufx1 / 4;   //小数舍去
			i1 = i1 - i;
			sid4 = sid | sid << 8 | sid << 16 | sid << 24;
			for (bufy = tbufy0; bufy < tbufy1; bufy++) {
				ly = sht->ly + bufy;
				for (bufx = tbufx0; bufx < tbufx1 && (bufx & 3) != 0; bufx++) {
					lx = sht->lx + bufx;
					if (map[ly * ctl->xsize + lx] == sid) {		//如果当前像素点包含在当前图层内，则刷新
						vram[ly * ctl->xsize + lx] = buf[bufy * sht->xsize + bufx];	//把像素颜色放到显存对应位置处
					}
				}
				lx = sht->lx + bufx;
				p = (int *)&map[ly * ctl->xsize + lx];
				q = (int *)&vram[ly * ctl->xsize + lx];
				r = (int *)&buf[bufy * sht->xsize + bufx];
				for (i = 0; i < i1; i++) {							/* 4の倍数部分 */
					if (p[i] == sid4) {
						q[i] = r[i];
					}
					else {
						tbufx2 = bufx + i * 4;
						lx = sht->lx + tbufx2;
						if (map[ly * ctl->xsize + lx + 0] == sid) {
							vram[ly * ctl->xsize + lx + 0] = buf[bufy * sht->xsize + tbufx2 + 0];
						}
						if (map[ly * ctl->xsize + lx + 1] == sid) {
							vram[ly * ctl->xsize + lx + 1] = buf[bufy * sht->xsize + tbufx2 + 1];
						}
						if (map[ly * ctl->xsize + lx + 2] == sid) {
							vram[ly * ctl->xsize + lx + 2] = buf[bufy * sht->xsize + tbufx2 + 2];
						}
						if (map[ly * ctl->xsize + lx + 3] == sid) {
							vram[ly * ctl->xsize + lx + 3] = buf[bufy * sht->xsize + tbufx2 + 3];
						}
					}
				}
				for (bufx += i1 * 4; bufx < tbufx1; bufx++) {				/* 後ろの端数を1バイトずつ */
					lx = sht->lx + bufx;
					if (map[ly * ctl->xsize + lx] == sid) {
						vram[ly * ctl->xsize + lx] = buf[bufy * sht->xsize + bufx];
					}
				}
			}
		}
		else {    //1字节型
			for (bufy = tbufy0; bufy < tbufy1; bufy++) {
				ly = sht->ly + bufy;
				for (bufx = tbufx0; bufx < tbufx1; bufx++) {
					lx = sht->lx + bufx;
					if (map[ly * ctl->xsize + lx] == sid) {
						vram[ly * ctl->xsize + lx] = buf[bufy * sht->xsize + bufx];
					}
				}
			}
		}

	}
	return;
}

void sheet_refreshmap(SHEETCTL *ctl, int vx0, int vy0, int vx1, int vy1, int zindex)
{
	int index, bufy, bufx, ly, lx, tbufx0, tbufy0, tbufx1, tbufy1, *p, sid4;
	SHEET *sht;
	uchar *buf, sid, *map = ctl->map;
	if (vx0 < 0) {
		vx0 = 0;
	}
	if (vy0 < 0) {
		vy0 = 0;
	}
	if (vx1 > ctl->xsize) {
		vx1 = ctl->xsize;
	}
	if (vy1 > ctl->ysize) {
		vy1 = ctl->ysize;
	}
	for (index = zindex; index <= ctl->top; index++) {
		sht = ctl->sheetseq[index];
		buf = sht->buf;
		sid = sht - ctl->sheets;	//得到的是 当前图层地址 基于图层数组的 地址增量
		tbufx0 = vx0 - sht->lx;
		tbufy0 = vy0 - sht->ly;
		tbufx1 = vx1 - sht->lx;
		tbufy1 = vy1 - sht->ly;
		if (tbufx0 < 0) {	//把要刷新部分的像素从图层中括出来
			tbufx0 = 0;
		}
		if (tbufy0 < 0) {
			tbufy0 = 0;
		}
		if (tbufx1 > sht->xsize) {
			tbufx1 = sht->xsize;
		}
		if (tbufy1 > sht->ysize) {
			tbufy1 = sht->ysize;
		}
		if (sht->col_inv == -1) {	//无透明色
			if ((sht->lx & 3) == 0 && (tbufx0 & 3) == 0 && (tbufx1 & 3) == 0) {
				tbufx1 = (tbufx1 - tbufx0) / 4;	//4字节型
				sid4 = sid | sid << 8 | sid << 16 | sid << 24;
				for (bufy = tbufy0; bufy < tbufy1; bufy++) {
					lx = sht->lx + tbufx0;
					ly = sht->ly + bufy;
					p = (int *)&map[ly * ctl->xsize + lx];
					for (bufx = 0; bufx < tbufx1; bufx++) {
						p[bufx] = sid4;
					}
				}
			}
			else {
				for (bufy = tbufx0; bufy < tbufy1; bufy++) {	//1字节型
					ly = sht->ly + bufy;
					for (bufx = tbufx0; bufx < tbufx1; bufx++) {
						lx = sht->lx + bufx;
						map[ly * ctl->xsize + lx] = sid;
					}
				}
			}
		}
		else {
			for (bufy = tbufy0; bufy < tbufy1; bufy++) {
				ly = sht->ly + bufy;
				for (bufx = tbufx0; bufx < tbufx1; bufx++) {
					lx = sht->lx + bufx;
					if (buf[bufy * sht->xsize + bufx] != sht->col_inv) {		//如果不是透明色
						map[ly * ctl->xsize + lx] = sid;	//把sid与像素点对应起来
					}
				}
			}
		}
	}
	return;
}

/**
 * 处理图层滑动
 */
void sheet_slide(SHEET *sht, int lx, int ly)
{
	int oldlx = sht->lx, oldly = sht->ly;
	sht->lx = lx;
	sht->ly = ly;
	if (sht->zindex >= 0) {
		sheet_refreshmap(sht->ctl, oldlx, oldly, oldlx + sht->xsize, oldly + sht->ysize, 0);
		sheet_refreshmap(sht->ctl, lx, ly, lx + sht->xsize, ly + sht->ysize, sht->zindex);
		sheet_refreshsub(sht->ctl, oldlx, oldly, oldlx + sht->xsize, oldly + sht->ysize, 0, sht->zindex - 1);
		sheet_refreshsub(sht->ctl, lx, ly, lx + sht->xsize, ly + sht->ysize, sht->zindex, sht->zindex);
	}
	return;
}

/**
 * 释放图层
 */
void sheet_free(SHEET *sht)
{
	if (sht->zindex >= 0) {
		sheet_updown(sht, -1);	//先隐藏起来
	}
	sht->flags = 0;
	return;
}
