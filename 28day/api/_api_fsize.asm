;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "_api_fsize.asm"]	;制作目标文件信息

	GLOBAL _api_fsize

[SECTION .text]
_api_fsize:		;int api_fsize(int fhandle, int mode)
	MOV		EDX,	24
	MOV		EAX,	[ESP+4]
	MOV		ECX,	[ESP+8]
	INT		0x40
	RET
