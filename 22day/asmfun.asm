;汇编函数定义，提供给C语言调用
[FORMAT "WCOFF"]
[INSTRSET "i486p"]	;使用到486为止的指令集
[BITS 32]		;制作32位模式的机器语言

[FILE "asmfun.asm"]	;制作目标文件信息
	GLOBAL	_io_hlt, _io_cli, _io_sti, _io_stihlt
	GLOBAL	_io_in8,  _io_in16,  _io_in32
	GLOBAL	_io_out8, _io_out16, _io_out32
	GLOBAL	_io_load_eflags, _io_store_eflags
	GLOBAL	_load_gdtr, _load_idtr
	GLOBAL	_asm_inthandler20, _asm_inthandler21, _asm_inthandler27, _asm_inthandler2c, _asm_inthandler0c, _asm_inthandler0d
	GLOBAL	_load_cr0, _store_cr0
	GLOBAL	_memtest_sub
	GLOBAL	_load_tr, _taskswitch3, _taskswitch4, _farjmp
	GLOBAL	_asm_mwe_api, _farcall, _start_app, _asm_end_app
	EXTERN	_inthandler20, _inthandler21, _inthandler27, _inthandler2c, _inthandler0d, _inthandler0c
	EXTERN 	_mwe_api
;实际函数
[section .text]
_io_hlt:
	hlt
	ret
_io_cli:	;void io_cli(void)	清除中断标记位
	cli
	ret
_io_sti:	;	设置中断标记位
	sti
	ret
_io_stihlt:
	sti
	hlt
	ret
_io_in8:	;int io_in8(int port)
	mov	edx,	[esp+4]
	mov	eax,	0
	in	al,	dx
	ret
_io_in16:
	mov	edx,	[esp+4]
	mov	eax,	0
	in	ax,	dx
	ret
_io_in32:
	mov	edx,	[esp+4]
	in	eax,	dx
	ret
_io_out8:	;io_out8(int port, int data)
	mov	edx,	[esp+4]
	mov	al,	[esp+8]
	out 	dx,	al
	ret
_io_out16:
	mov	edx,	[esp+4]
	mov	ax,	[esp+8]
	out	dx,	ax
	ret
_io_out32:
	mov	edx,	[esp+4]
	mov	eax,	[esp+8]
	out	dx,	eax
	ret
_io_load_eflags:	;io_load_eflags(void)
	pushfd		;指push eflags寄存器，此操作会将eflags压入栈中
	pop	eax	;将栈顶元素弹出，放入eax中
	ret
_io_store_eflags:	;void io_store_eflags(int eflags)
	mov	eax,	[esp+4]
	push	eax
	popfd		;将栈顶值弹出放入eflags中
	ret
_load_gdtr:		; void load_gdtr(int limit, int addr);
	MOV	AX,[ESP+4]		; limit
	MOV	[ESP+6],AX
	LGDT	[ESP+6]
	RET

_load_idtr:		; void load_idtr(int limit, int addr);
	MOV	AX,[ESP+4]		; LIMIT
	MOV	[ESP+6],AX
	LIDT	[ESP+6]
	RET

_asm_inthandler20:
	PUSH	ES
	PUSH	DS
	PUSHAD
	MOV		EAX,ESP
	PUSH	EAX
	MOV		AX,SS
	MOV		DS,AX
	MOV		ES,AX
	CALL	_inthandler20
	POP		EAX
	POPAD
	POP		DS
	POP		ES
	IRETD

_asm_inthandler21:
	PUSH	ES
	PUSH	DS
	PUSHAD
	MOV		EAX,ESP
	PUSH	EAX
	MOV		AX,SS
	MOV		DS,AX
	MOV		ES,AX
	CALL	_inthandler21
	POP		EAX
	POPAD
	POP		DS
	POP		ES
	IRETD

_asm_inthandler27:
	PUSH	ES
	PUSH	DS
	PUSHAD
	MOV		EAX,ESP
	PUSH	EAX
	MOV		AX,SS
	MOV		DS,AX
	MOV		ES,AX
	CALL	_inthandler27
	POP		EAX
	POPAD
	POP		DS
	POP		ES
	IRETD

_asm_inthandler2c:
	PUSH	ES
	PUSH	DS
	PUSHAD
	MOV		EAX,ESP
	PUSH	EAX
	MOV		AX,SS
	MOV		DS,AX
	MOV		ES,AX
	CALL	_inthandler2c
	POP		EAX
	POPAD
	POP		DS
	POP		ES
	IRETD
_asm_inthandler0d:	;cpu一般保护异常中断
	STI
	PUSH	ES
	PUSH	DS
	PUSHAD
	MOV		EAX,ESP
	PUSH	EAX
	MOV		AX,SS
	MOV		DS,AX
	MOV		ES,AX
	CALL	_inthandler0d
	CMP		eax, 0
	JNE		end_app
	POP		EAX
	POPAD
	POP		DS
	POP		ES
	add		esp,4
	IRETD
_asm_inthandler0c:	;cpu 栈异常
	STI
	PUSH	ES
	PUSH	DS
	PUSHAD
	MOV		EAX,ESP
	PUSH	EAX
	MOV		AX,SS
	MOV		DS,AX
	MOV		ES,AX
	CALL	_inthandler0c
	CMP		eax, 0
	JNE		end_app
	POP		EAX
	POPAD
	POP		DS
	POP		ES
	add		esp,4
	IRETD

_load_cr0:
	MOV		EAX, CR0
	RET
_store_cr0:
	MOV		EAX, [ESP+4]
	MOV		CR0, EAX
	RET
_memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
	PUSH	EDI						; （EBX, ESI, EDI も使いたいので）
	PUSH	ESI
	PUSH	EBX
	MOV		ESI,0xaa55aa55			; pat0 = 0xaa55aa55;
	MOV		EDI,0x55aa55aa			; pat1 = 0x55aa55aa;
	MOV		EAX,[ESP+12+4]			; i = start;
mts_loop:
	MOV		EBX,EAX
	ADD		EBX,0xffc				; p = i + 0xffc;
	MOV		EDX,[EBX]				; old = *p;
	MOV		[EBX],ESI				; *p = pat0;
	XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
	CMP		EDI,[EBX]				; if (*p != pat1) goto fin;
	JNE		mts_fin
	XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
	CMP		ESI,[EBX]				; if (*p != pat0) goto fin;
	JNE		mts_fin
	MOV		[EBX],EDX				; *p = old;
	ADD		EAX,0x1000				; i += 0x1000;
	CMP		EAX,[ESP+12+8]			; if (i <= end) goto mts_loop;
	JBE		mts_loop
	POP		EBX
	POP		ESI
	POP		EDI
	RET
mts_fin:
	MOV		[EBX],EDX				; *p = old;
	POP		EBX
	POP		ESI
	POP		EDI
	RET
_load_tr:	;void load_tr(int tr)
	LTR		[esp+4]
	RET
_taskswitch3:
	JMP		3*8 : 0
_taskswitch4:
	JMP		4*8 : 0
	RET
_farjmp:	; void farjump(int eip, int cs)
	JMP		FAR [ESP+4]
	RET
_asm_mwe_api:	;提供给中断0x40用，中断触发的时候会自动CLI（！！！中断只运行在ring0中，如果当前为用户态，则会涉及栈切换）
	STI			;因为中断机制会自动CLI
	PUSH 	DS	
	PUSH	ES
	PUSHAD			;用于保存
	PUSHAD			;用于传值
	MOV		AX,SS	;ss肯定是内核栈
	MOV		DS,AX
	MOV		ES,AX
	CALL	_mwe_api
	CMP		EAX, 0	;当eax不为0时程序结束
	JNE		end_app
	ADD		ESP,32
	POPAD
	POP		ES
	POP		DS
	IRETD
  end_app:
    MOV		ESP,[EAX]
    POPAD	;弹出的是start_app保存的操作系统寄存器
    RET

_asm_end_app:
	MOV		ESP,[EAX]
	MOV		DWORD [EAX+4], 0
	POPAD
    RET

_farcall:	;void farcall(int eip, int cs)
	CALL	FAR [ESP+4];eip, cs
	RET

_start_app:	;void start_app(int eip, int cs, int esp, int ds, int *tss_esp0);
	PUSHAD	;保护操作系统的寄存器
	MOV		EAX, [ESP+36]	;EIP
	MOV		ECX, [ESP+40]	;CS
	MOV		EDX, [ESP+44]	;ESP
	MOV		EBX, [ESP+48]	;DS/SS
	MOV		EBP, [ESP+52]	;tss.esp0的地址
	MOV   	[EBP], ESP		;把OS的esp放到tss.esp0中
	MOV   	[EBP+4], SS		;把OS的ss放到tss.ss0中
	MOV		ES, BX
	MOV		DS, BX
	MOV		FS, BX
	MOV		GS, BX

	OR		ECX, 3
	OR		EBX, 3
	PUSH	EBX		;应用程序的ss（！！！http://blog.csdn.net/bfboys/article/details/52420211）
	PUSH	EDX		;应用程序的esp
	PUSH	ECX		;应用程序的cs
	PUSH	EAX		;应用程序的eip
	RETF		;利用了该指令的特点（！！！这里执行 由低特权级到高特权级的调用返回）







