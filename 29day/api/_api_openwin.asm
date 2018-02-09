;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "_api_openwin.asm"]	;制作目标文件信息

	GLOBAL _api_openwin

[SECTION .text]
_api_openwin:	;int api_openwin(char *buf, int xsize, int ysize, int col_inv, char *title);
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	MOV		EDX,	5
	MOV		EBX,	[esp+16]
	MOV		ESI,	[esp+20]
	MOV		EDI,	[esp+24]
	MOV		EAX,	[esp+28]
	MOV		ECX,	[esp+32]
	INT		0x40
	POP		EBX
	POP		ESI
	POP		EDI
	RET
