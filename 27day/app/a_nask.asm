;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "a_nask.asm"]	;制作目标文件信息

	GLOBAL	_api_putchar, _api_putstr0, _api_openwin, _api_putstrwin, _api_boxfillwin, _api_point, _api_refreshwin, _api_linewin, _api_closewin, _api_getkey
	GLOBAL	_api_alloctimer, _api_inittimer, _api_settimer, _api_freetimer
	GLOBAL	_api_initmalloc, _api_malloc, _api_free
	GLOBAL	_api_beep
	GLOBAL	_api_end

[SECTION .text]

_api_putchar:
	MOV		EDX,	1
	MOV		AL,		[ESP+4]
	INT		0x40
	RET

_api_putstr0:	;void api_putstr0(char *str)
	PUSH	EBX
	MOV		EDX,	2
	MOV		EBX,	[ESP+8]
	INT		0x40
	POP		EBX
	RET

_api_openwin:	;int api_openwin(char *buf, int xsize, int ysize, int col_inv, char *title);
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	MOV		EDX,	5
	MOV		EBX,	[esp+16]
	MOV		ESI,	[esp+20]
	MOV		EDI,	[esp+24]
	MOV		EAX,	[esp+28]
	MOV		ECX,	[esp+32]
	INT		0x40
	POP		EBX
	POP		ESI
	POP		EDI
	RET

_api_putstrwin:		;void api_putstrwin(int win, int lx, int ly, int col, int len, char *str);
	PUSH	EDI
	PUSH	ESI
	PUSH	EBP
	PUSH	EBX
	MOV		EDX,	6
	MOV		EBX,	[ESP+20]	;win
	MOV		ESI,	[ESP+24]	;x
	MOV		EDI,	[ESP+28]	;y
	MOV		EAX,	[ESP+32]	;col
	MOV		ECX,	[ESP+36]	;len
	MOV		EBP,	[ESP+40]	;*str
	INT		0x40
	POP		EBX
	POP		EBP
	POP		ESI
	POP		EDI
	RET

_api_boxfillwin:		;void api_boxfillwin(int win, int lx0, int ly0, int lx1, int ly1, int col);
	PUSH	EDI
	PUSH	ESI
	PUSH	EBP
	PUSH	EBX
	MOV		EDX,	7
	MOV		EBX,	[ESP+20]	;win
	MOV		EAX,	[ESP+24]	;lx0
	MOV		ECX,	[ESP+28]	;ly0
	MOV		ESI,	[ESP+32]	;lx1
	MOV		EDI,	[ESP+36]	;ly1
	MOV		EBP,	[ESP+40]	;col
	INT		0x40
	POP		EBX
	POP		EBP
	POP		ESI
	POP		EDI
	RET

_api_initmalloc:	;void  api_initmalloc(void);
	PUSH	EBX
	MOV		EDX,	8
	MOV		EBX,	[CS:0x0020];	malloc内存空间的地址
	MOV		EAX,	EBX
	ADD		EAX,	32*1024
	MOV		ECX,	[CS:0x0000];	数据段的大小
	SUB		ECX,	EAX
	INT		0x40
	POP		EBX
	RET

_api_malloc:		;char *api_malloc(int size);
	PUSH	EBX
	MOV		EDX,	9
	MOV		EBX,	[CS:0x0020]
	MOV		ECX,	[ESP+8]
	INT		0x40
	POP		EBX
	RET

_api_free:			;void api_free(char *addr, int size);
	PUSH	EBX
	MOV		EDX,	10
	MOV		EBX,	[CS:0x0020]
	MOV		EAX,	[ESP+8]
	MOV		ECX,	[ESP+12]
	INT		0x40
	POP		EBX
	RET

_api_point:		;void api_point(int win, int x, int y, int col)
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	MOV		EDX,	11
	MOV		EBX,	[esp+16];win
	MOV		ESI,	[esp+20];
	MOV		EDI,	[esp+24]
	MOV		eax,	[esp+28]
	INT		0x40
	pop		ebx
	pop		esi
	pop		edi
	ret

_api_refreshwin:	;void api_refreshwin(int win, int x0, int y0, int x1, int y1);
	PUSH	EDI
	PUSH	ESI
	PUSH	EBX
	MOV		EDX,	12
	MOV		EBX,	[esp+16];win
	MOV		EAX,	[esp+20];
	MOV		ECX,	[esp+24];
	MOV		ESI,	[esp+28]
	MOV		edi,	[esp+32]
	INT		0x40
	pop		ebx
	pop		esi
	pop		edi
	ret

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

_api_closewin:		;void api_closewin(int win);
	push	ebx
	mov		edx,	14
	mov		ebx,	[esp+8];	win
	int		0x40
	pop		ebx
	ret

_api_getkey:	;int api_getkey(int mode); //mode=1休眠直到有键盘输入，mode=0不休眠有键盘输入则返回，无键盘输入则返回-1
	mov		edx,	15
	mov		eax,	[esp+4]
	int		0x40
	ret

_api_alloctimer:	;int api_alloctimer(void)
	mov		edx,	16
	int		0x40
	ret

_api_inittimer:		;void api_inittimer(int timer, int data);
	push	ebx
	mov		edx,	17
	mov		ebx,	[esp+8]
	mov		eax,	[esp+12]
	int		0x40
	pop		ebx
	ret

_api_settimer:		;void api_settimer(int timer, int time);
	push	ebx
	mov		edx,	18
	mov		ebx,	[esp+8]
	mov		eax,	[esp+12]
	int		0x40
	pop		ebx
	ret

_api_freetimer:		;void api_freetimer(int timer)
	push	ebx
	mov		edx,	19
	mov		ebx,	[esp+8]		;timer
	int		0x40
	pop		ebx
	ret

_api_beep:		;void _api_beep(int tone);
	mov		edx,	20
	mov		eax,	[esp+4]
	int		0x40
	ret

_api_end:
	MOV		edx, 4
	INT		0x40
