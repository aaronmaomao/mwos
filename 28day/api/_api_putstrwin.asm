;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "_api_putstrwin.asm"]	;制作目标文件信息

	GLOBAL _api_putstrwin

[SECTION .text]
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
