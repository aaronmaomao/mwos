;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "_api_initmalloc.asm"]	;制作目标文件信息

	GLOBAL _api_initmalloc

[SECTION .text]
_api_initmalloc:	;void  api_initmalloc(void);
	PUSH	EBX
	MOV		EDX,	8
	MOV		EBX,	[CS:0x0020];	malloc内存空间的地址
	MOV		EAX,	EBX
	ADD		EAX,	32*1024
	MOV		ECX,	[CS:0x0000];	数据段的大小
	SUB		ECX,	EAX
	INT		0x40
	POP		EBX
	RET
