;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "a_nask.asm"]	;制作目标文件信息

	GLOBAL	_api_putchar, _api_putstr0
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

_api_openwin:	;void api_putstr0(char *str)
	PUSH	EBX
	MOV		EDX,	2
	MOV		EBX,	[ESP+8]
	INT		0x40
	POP		EBX
	RET

_api_end:
	MOV		edx, 4
	INT		0x40
