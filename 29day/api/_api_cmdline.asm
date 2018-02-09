;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "_api_cmdline.asm"]	;制作目标文件信息

	GLOBAL _api_cmdline

[SECTION .text]
_api_cmdline:		; int _api_cmdline(char *buf, int maxsize);
	PUSH	EBX
	MOV		EDX, 26
	MOV		ECX, [esp+12]  ;maxsize 存放的最大长度
	MOV		EBX, [esp+8]   ;buf
	INT		0x40
	POP		EBX
	RET
