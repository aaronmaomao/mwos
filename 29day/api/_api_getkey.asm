;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "_api_getkey.asm"]	;制作目标文件信息

	GLOBAL _api_getkey

[SECTION .text]
_api_getkey:	;int api_getkey(int mode); //mode=1休眠直到有键盘输入，mode=0不休眠有键盘输入则返回，无键盘输入则返回-1
	mov		edx,	15
	mov		eax,	[esp+4]
	int		0x40
	ret
