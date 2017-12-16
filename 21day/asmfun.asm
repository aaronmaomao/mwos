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
	GLOBAL	_asm_inthandler20, _asm_inthandler21, _asm_inthandler27, _asm_inthandler2c
	GLOBAL	_load_cr0, _store_cr0
	GLOBAL	_memtest_sub
	GLOBAL	_load_tr, _taskswitch3, _taskswitch4, _farjmp
	GLOBAL	_asm_mwe_api, _farcall, _start_app, _asm_inthandler0d
	EXTERN	_inthandler20, _inthandler21, _inthandler27, _inthandler2c, _inthandler0d
	EXTERN 	_mwe_api
;实际函数
[section .text]
_io_hlt:
	hlt
	ret
_io_cli:	;void io_cli(void)	清除中断标记位
	cli
	ret
_io_sti:	;			设置中断标记位
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
	MOV		AX,SS
	CMP		AX, 1*8
	JNE		.from_app	;不相等则是应用程序运行期间发生的中断
	;当操作系统运行时发生中断(无需改变段，只需保存现场即可)
	MOV		EAX, ESP
	PUSH	SS	;保存中断的ss
	PUSH	EAX	;保存中断的esp
	MOV		AX,SS
	MOV		DS,AX
	MOV		ES,AX
	CALL	_inthandler20
	ADD		ESP,8
	POPAD
	POP		DS
	POP		ES
	IRETD

  .from_app:	;当运行的是应用程序时发生中断
	MOV		EAX, 1*8
	MOV		DS,AX	;暂将DS设为操作系统用
	MOV		ECX,[0xfe4]	;系统的ESP
	ADD		ECX,-8
	MOV		[ECX+4],SS	;保存当前中断的SS,（保存到系统的栈中）
	MOV		[ECX],ESP	;保存当前中断的ESP,（保存到系统的栈中）
	MOV		SS,AX
	MOV		ES,AX
	MOV		ESP,ECX	;系统的栈
	CALL	_inthandler20
	POP		ECX
	POP		EAX
	MOV		SS,AX
	MOV		ESP,ECX
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
_asm_inthandler0d:	;cpu异常中断
			STI
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		AX,SS
		CMP		AX,1*8
		JNE		.from_app
;	OSが動いているときに割り込まれたのでほぼ今までどおり，OS运行时产生中断
		MOV		EAX,ESP
		PUSH	SS				; 割り込まれたときのSSを保存
		PUSH	EAX				; 割り込まれたときのESPを保存
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0d
		ADD		ESP,8
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4			; INT 0x0d では、これが必要
		IRETD
.from_app:
;	アプリが動いているときに割り込まれた，app运行时产生中断
		CLI
		MOV		EAX,1*8
		MOV		DS,AX			; とりあえずDSだけOS用にする
		MOV		ECX,[0xfe4]		; OSのESP
		ADD		ECX,-8
		MOV		[ECX+4],SS		; 割り込まれたときのSSを保存
		MOV		[ECX  ],ESP		; 割り込まれたときのESPを保存
		MOV		SS,AX
		MOV		ES,AX
		MOV		ESP,ECX
		STI
		CALL	_inthandler0d
		CLI
		CMP		EAX,0
		JNE		.kill
		POP		ECX
		POP		EAX
		MOV		SS,AX			; SSをアプリ用に戻す
		MOV		ESP,ECX			; ESPもアプリ用に戻す
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4			; INT 0x0d では、これが必要
		IRETD
.kill:
;	アプリを異常終了させることにした，强制结束应用程序，切换回到startapp
		MOV		EAX,1*8			; OS用のDS/SS
		MOV		ES,AX
		MOV		SS,AX
		MOV		DS,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		ESP,[0xfe4]		; start_appのときのESPに無理やり戻す，强制返回到startapp时的esp（强制回到startapp时的状态）
		STI			; 切り替え完了なので割り込み可能に戻す
		POPAD	; 保存しておいたレジスタを回復
		RET
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
_asm_mwe_api:	;提供给中断0x40用，中断触发的时候会自动CLI
	;STI		;因为中断机制会自动CLI
	;PUSHAD
	;PUSH	1	;参数move
	;AND 	EAX, 0xff	;参数 chr
	;PUSH	EAX
	;PUSH 	DWORD [0x0fec]	;参数cons
	;CALL	_cons_putchar
	;ADD		ESP, 12		;丢弃刚才压栈的数据（共4b*3）
	;RETF			;far-call的返回
	;POPAD
	;IRETD	;中断的返回
	;STI
	;PUSHAD	;保护call之前的寄存器
	;PUSHAD	;给_mwe_api的参数
	;CALL	_mwe_api
	;ADD		ESP, 32
	;POPAD
	;IRETD
	PUSH 	DS
	PUSH	ES
	PUSHAD
	MOV		EAX, 1*8
	MOV 	DS, AX
	MOV 	ECX, [0xfe4]	;操作系统的栈
	ADD		ECX, -40
	MOV		[ECX+32], ESP	;将当前ESP保存到操作系统栈
	MOV		[ECX+36], SS	;将当前SS保存到操作系统栈
	;将PUSHAD后的值复制到系统栈
	MOV		EDX, [ESP]
	MOV		EBX, [ESP + 4]
	MOV		[ECX], EDX
	MOV		[ECX + 4], EBX
	MOV		EDX, [ESP + 8]
	MOV 	EBX, [ESP + 12]
	MOV		[ECX +8], EDX
	MOV		[ECX +12], EBX
	MOV		EDX, [ESP+16]
	MOV		EBX, [ESP+20]
	MOV		[ECX+16], EDX
	MOV 	[ECX+20], EBX
	MOV 	EDX, [ESP+24]
	MOV		EBX, [ESP+28]
	MOV		[ECX+24], EDX
	MOV		[ECX+28], EBX

	MOV		ES,	AX
	MOV 	SS, AX
	MOV 	ESP, ECX
	STI

	CALL	_mwe_api

	MOV 	ECX, [ESP+32]
	MOV		EAX, [ESP+36]
	CLI
	MOV		SS, AX
	MOV		ESP, ECX
	POPAD
	POP 	ES
	POP		DS
	IRETD	;中断返回，会自动执行STI

_farcall:	;void farcall(int eip, int cs)
	CALL	FAR [ESP+4];eip, cs
	RET

_start_app:	;void start_app(int eip, int cs, int esp, int ds);
	PUSHAD	;保护操作系统的寄存器
	MOV		EAX, [ESP+36]	;EIP
	MOV		ECX, [ESP+40]	;CS
	MOV		EDX, [ESP+44]	;ESP
	MOV		EBX, [ESP+48]	;DS/SS
	MOV		[0xfe4], ESP	;把操作系统的ESP保护起来
	;设置APP的段寄存器
	CLI
	MOV		ES, BX
	MOV		SS, BX
	MOV		DS, BX
	MOV		FS, BX
	MOV		GS, BX
	MOV		ESP, EDX
	STI
	;准备运行APP
	PUSH	ECX		;APP的CS，给下面的FARCALL使用
	PUSH	EAX		;APP的IP
	CALL	FAR [ESP]	;开始运行APP代码
	;APP运行结束，返回操作系统
	MOV		EAX, 1*8
	CLI
	MOV		ES, AX
	MOV		SS, AX
	MOV		DS, AX
	MOV		FS, AX
	MOV		GS, AX
	MOV		ESP, [0xfe4]
	STI
	POPAD	;恢复之前保存的寄存器
	RET







