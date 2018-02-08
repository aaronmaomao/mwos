;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "_api_fread.asm"]	;制作目标文件信息

	GLOBAL _api_fread

[SECTION .text]
_api_fread:		;int api_fread(char *buf, int size, int fhandle);
	PUSH	EBX
	MOV		EDX,	25
	MOV		EAX,	[esp+16]
	MOV		ECX,	[esp+12]
	MOV		EBX,	[esp+8]
	INT		0x40
	POP		EBX
	RET
