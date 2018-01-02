;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "a_nask.asm"]	;制作目标文件信息

	GLOBAL	_api_putchar, _api_putstr0, _api_openwin, _api_putstrwin, _api_boxfillwin
	GLOBAL	_api_end

[SECTION .text]

_api_putchar:
	MOV		EDX,	1
	MOV		AL,		[ESP+4]
	INT		0x40
	RET

_api_putstr0:	;void api_putstr0(char *str)
	PUSH	EBX
	MOV		EDX,	2
	MOV		EBX,	[ESP+8]
	INT		0x40
	POP		EBX
	RET

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

_api_putstrwin:		;void api_putstrwin(int win, int lx, int ly, int col, int len, char *str);
	PUSH	EDI
	PUSH	ESI
	PUSH	EBP
	PUSH	EBX
	MOV		EDX,	6
	MOV		EBX,	[ESP+20]	;win
	MOV		ESI,	[ESP+24]	;x
	MOV		EDI,	[ESP+28]	;y
	MOV		EAX,	[ESP+32]	;col
	MOV		ECX,	[ESP+36]	;len
	MOV		EBP,	[ESP+40]	;*str
	INT		0x40
	POP		EBX
	POP		EBP
	POP		ESI
	POP		EDI
	RET

_api_boxfillwin:		;void api_boxfillwin(int win, int lx0, int ly0, int lx1, int ly1, int col);
	PUSH	EDI
	PUSH	ESI
	PUSH	EBP
	PUSH	EBX
	MOV		EDX,	7
	MOV		EBX,	[ESP+20]	;win
	MOV		EAX,	[ESP+24]	;lx0
	MOV		ECX,	[ESP+28]	;ly0
	MOV		ESI,	[ESP+32]	;lx1
	MOV		EDI,	[ESP+36]	;ly1
	MOV		EBP,	[ESP+40]	;col
	INT		0x40
	POP		EBX
	POP		EBP
	POP		ESI
	POP		EDI
	RET

_api_end:
	MOV		edx, 4
	INT		0x40
