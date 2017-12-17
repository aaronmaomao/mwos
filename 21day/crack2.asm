;应用程序直接用汇编改变DS的值
[INSTRSET "i486p"]
[BITS 32]
	MOV		EAX, 1*8
	MOV		DS, AX
	MOV		BYTE[0x102600], 0
	RETF
	mov		edx,4
	int 	0x40
