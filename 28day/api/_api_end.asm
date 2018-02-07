;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "_api_end.asm"]	;制作目标文件信息

	GLOBAL _api_end

[SECTION .text]
_api_end:
	MOV		edx, 4
	INT		0x40
