;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "_api_alloctimer.asm"]	;制作目标文件信息

	GLOBAL _api_alloctimer

[SECTION .text]
_api_alloctimer:	;int api_alloctimer(void)
	mov		edx,	16
	int		0x40
	ret
