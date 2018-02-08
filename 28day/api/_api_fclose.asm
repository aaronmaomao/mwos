;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "_api_fclose.asm"]	;制作目标文件信息

	GLOBAL _api_fclose

[SECTION .text]
_api_fclose:		;void api_fclose(int fhandle);
	MOV		EDX,	22
	MOV		EAX,	[ESP+4]
	INT		0x40
	RET
