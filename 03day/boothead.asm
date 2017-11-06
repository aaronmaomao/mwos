;有关bootinfo的设置
CYLS	equ	0x0ff0
LEDS	equ	0x0ff1
VMODE	equ	0x0ff2	;保存颜色数目的信息。颜色的位数
SCRNX	equ	0x0ff4	;分辨率的x
SCRNY	equ	0x0ff6	;分辨率的y
VRAM	equ	0x0ff8	;图像缓冲区的起始地址，即显存的起始地址

org	0xc200
;设置显卡
mov	al,	0x13	;VGA图形模式，320x200x8位彩色模式，调色板模式
mov	ah,	0x00	;设置显卡模式
int	0x10
mov	byte[VMODE],	8	
mov	word[SCRNX],	320
mov	word[SCRNY],	200
mov	dword[VRAM],	0x000a0000	;显存的起始地址

;用bios取的键盘上各种led指示灯的状态
mov	ah,	0x12
int	0x16
mov	[LEDS],	al

fin:
hlt
jmp	fin