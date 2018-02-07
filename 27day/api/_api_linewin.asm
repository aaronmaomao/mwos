;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "_api_linewin.asm"]	;制作目标文件信息

	GLOBAL _api_linewin

[SECTION .text]
_api_linewin:	;void api_linewin(int win, int x0, int y0, int x1, int y1, int col);
	push	edi
	push	esi
	push	ebp
	push	ebx
	mov		edx,	13
	mov		ebx,	[esp+20]
	mov		eax,	[esp+24]
	mov		ecx,	[esp+28]
	mov		esi,	[esp+32]
	mov		edi,	[esp+36]
	mov		ebp,	[esp+40]
	int		0x40
	pop		ebx
	pop		ebp
	pop		esi
	pop		edi
	ret
