;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "_api_point.asm"]	;制作目标文件信息

	GLOBAL _api_point

[SECTION .text]
_api_point:		;void api_point(int win, int x, int y, int col)
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	MOV		EDX,	11
	MOV		EBX,	[esp+16];win
	MOV		ESI,	[esp+20];
	MOV		EDI,	[esp+24]
	MOV		eax,	[esp+28]
	INT		0x40
	pop		ebx
	pop		esi
	pop		edi
	ret
