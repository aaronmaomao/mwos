;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "_api_freetimer.asm"]	;制作目标文件信息

	GLOBAL _api_freetimer

[SECTION .text]
_api_freetimer:		;void api_freetimer(int timer)
	push	ebx
	mov		edx,	19
	mov		ebx,	[esp+8]		;timer
	int		0x40
	pop		ebx
	ret
