;汇编函数定义，提供给C语言调用
[format "WCOFF"]
[bits 32]
[INSTRSET "i486p"]

[file "asmfun.asm"]	;制作目标文件信息
	global _io_hlt,_write_mem8
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
	