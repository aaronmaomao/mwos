;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "_api_free.asm"]	;制作目标文件信息

	GLOBAL _api_free

[SECTION .text]
_api_free:			;void api_free(char *addr, int size);
	PUSH	EBX
	MOV		EDX,	10
	MOV		EBX,	[CS:0x0020]
	MOV		EAX,	[ESP+8]
	MOV		ECX,	[ESP+12]
	INT		0x40
	POP		EBX
	RET
