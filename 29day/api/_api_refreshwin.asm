;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "_api_refreshwin.asm"]	;制作目标文件信息

	GLOBAL _api_refreshwin

[SECTION .text]
_api_refreshwin:	;void api_refreshwin(int win, int x0, int y0, int x1, int y1);
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	MOV		EDX,	12
	MOV		EBX,	[esp+16];win
	MOV		EAX,	[esp+20];
	MOV		ECX,	[esp+24];
	MOV		ESI,	[esp+28]
	MOV		edi,	[esp+32]
	INT		0x40
	pop		ebx
	pop		esi
	pop		edi
	ret
