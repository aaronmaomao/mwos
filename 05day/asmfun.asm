;汇编函数定义，提供给C语言调用
[format "WCOFF"]
[bits 32]		;制作32位模式的机器语言
[INSTRSET "i486p"]	;使用到486为止的指令集

[file "asmfun.asm"]	;制作目标文件信息
	global	_io_hlt, _write_mem8, _io_cli
	global	_io_in8, _io_in16, io_in32
	global	_io_out8, io_out16, io_out32
	global	_io_load_eflags, _io_store_eflags
	global _load_gdtr, _load_idtr
;实际函数
[section .text]
_io_hlt:
	hlt
	ret
	
_write_mem8;	;_write_mem8(int addr, int data)
	mov	ecx,	[esp+4]
	mov	al,		[esp+8]
	mov	[ecx],	al
	ret
_io_cli:	;void io_cli(void)	清除中断标记位
	cli
	ret
	
_io_in8:	;int io_in8(int port)
	mov	edx,	[esp+4]
	mov	eax,	0
	in	al,	dx
	ret
_io_in16:
	mov	edx,	[esp+4]
	mov	eax,	0
	in	ax,	dx
	ret
_io_in32:
	mov	edx,	[esp+4]
	in	eax,	dx
	ret
_io_out8:	;io_out8(int port, int data)
	mov	edx,	[esp+4]
	mov	al,	[esp+8]
	out 	dx,	al
	ret
_io_out16:
	mov	edx,	[esp+4]
	mov	ax,	[esp+8]
	out	dx,	ax
	ret
_io_out32:
	mov	edx,	[esp+4]
	mov	eax,	[esp+8]
	out	dx,	eax
	ret
_io_load_eflags:	;io_load_eflags(void)
	pushfd		;指push eflags寄存器，此操作会将eflags压入栈中
	pop	eax	;将栈顶元素弹出，放入eax中
	ret
_io_store_eflags:	;void io_store_eflags(int eflags)
	mov	eax,	[esp+4]
	push	eax
	popfd		;将栈顶值弹出放入eflags中
	ret
_load_gdtr:		; void load_gdtr(int limit, int addr);
	MOV	AX,[ESP+4]		; limit
	MOV	[ESP+6],AX
	LGDT	[ESP+6]
	RET

_load_idtr:		; void load_idtr(int limit, int addr);
	MOV	AX,[ESP+4]		; limit
	MOV	[ESP+6],AX
	LIDT	[ESP+6]
	RET



