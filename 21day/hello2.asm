;汇编版本的应用程序
[INSTRSET "i486p"]
[BITS 32]
	MOV	EDX, 2
	MOV	EBX, msg
	INT	0x40
	RETF
msg:	;此时的msg地址是相对地址，在进入内存后要加上base地址才可以读取到
	DB	"hello world", 0
