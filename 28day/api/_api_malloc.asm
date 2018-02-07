;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "_api_malloc.asm"]	;制作目标文件信息

	GLOBAL _api_malloc

[SECTION .text]
_api_malloc:		;char *api_malloc(int size);
	PUSH	EBX
	MOV		EDX,	9
	MOV		EBX,	[CS:0x0020]
	MOV		ECX,	[ESP+8]
	INT		0x40
	POP		EBX
	RET
