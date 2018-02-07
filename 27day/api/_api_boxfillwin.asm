;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "_api_boxfillwin.asm"]	;制作目标文件信息

	GLOBAL _api_boxfillwin

[SECTION .text]
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
