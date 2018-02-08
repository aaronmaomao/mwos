;提供给C语言版的应用程序调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言
[FILE "__alloca.asm"]	;制作目标文件信息

	GLOBAL __alloca

[SECTION .text]
__alloca:
	ADD		EAX,-4
	SUB		ESP,EAX
	JMP		DWORD [ESP+EAX] ;代替ret
