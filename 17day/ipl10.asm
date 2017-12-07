CYLS	EQU	10

ORG	0x7C00

JMP 	entry
DB	0X90
DB	"HELLOIPL"		; ブートセクタの名前を自由に書いてよい（8バイト）
DW	512				; 1セクタの大きさ（512にしなければいけない）
DB	1				; クラスタの大きさ（1セクタにしなければいけない）
DW	1				; FATがどこから始まるか（普通は1セクタ目からにする）
DB	2				; FATの個数（2にしなければいけない）
DW	224				; ルートディレクトリ領域の大きさ（普通は224エントリにする）
DW	2880			; このドライブの大きさ（2880セクタにしなければいけない）
DB	0xf0			; メディアのタイプ（0xf0にしなければいけない）
DW	9				; FAT領域の長さ（9セクタにしなければいけない）
DW	18				; 1トラックにいくつのセクタがあるか（18にしなければいけない）
DW	2				; ヘッドの数（2にしなければいけない）
DD	0				; パーティションを使ってないのでここは必ず0
DD	2880			; このドライブ大きさをもう一度書く
DB	0,0,0x29		; よくわからないけどこの値にしておくといいらしい
DD	0xffffffff		; たぶんボリュームシリアル番号
DB	"HELLO-OS   "	; ディスクの名前（11バイト）
DB	"FAT12   "		; フォーマットの名前（8バイト）
RESB	18				; とりあえず18バイトあけておく

entry:
MOV	AX,	0
MOV	SS,	AX
MOV	SP,	0x7C00
MOV	DS,	AX

MOV	AX,	0x0820
MOV	ES,	AX
MOV	CH,	0	;0柱面
MOV	DH,	0	;0磁头
MOV	CL,	2	;2扇区

readloop:
MOV	SI,	0	;记录失败次数的寄存器

retry:
MOV	AH,	0x02	;read disk
MOV	AL,	1	;
MOV	BX,	0
MOV	DL,	0x00
INT	0X13
JNC	next
ADD	SI,	1
CMP	SI, 	5
JAE	err
MOV	AH,	0x00
MOV	DL,	0x00
INT	0x13
JMP	retry

next:
mov 	ax,	es
add	ax,	0x20	;段加0x20，即地址域增加512Byte
mov	es,	ax
add	cl,	1
cmp	cl,	18
jbe	readloop
mov	cl,	1
add	dh,	1
cmp	dh,	2
jb	readloop
mov	dh,	0
add	ch,	1
cmp	ch,	CYLS
jb	readloop

mov	[0x0ff0],	ch
jmp	0xc200	;正确的读完了所有的扇区

fin:
HLT
JMP	fin

err:
MOV	SI,	msg

printf:
MOV	AL,	[SI]
ADD	SI,	1
CMP	AL,	0
JE	fin
MOV	AH,	0x0E
MOV	BX,	15
INT	0X10
JMP	printf

msg:
DB	0x0A, 0x0A
DB	"READ DISK ERROR!"
DB	0x0A
DB	0

RESB	0x7dfe-$
DB	0x55, 0xAA

;填充部分
DB	0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
RESB	4600
DB	0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
RESB	1469432


