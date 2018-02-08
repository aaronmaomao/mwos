;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "_api_fseek.asm"]	;制作目标文件信息

	GLOBAL _api_fseek

[SECTION .text]
_api_fseek:		;void api_fseek(int fhandle,int offset, int mode);
	PUSH	EBX
	MOV		EDX, 23
	MOV		EAX, [ESP+8]
	MOV		ECX, [ESP+16]
	MOV		EBX, [ESP+12]
	INT		0x40
	POP		EBX
	RET
