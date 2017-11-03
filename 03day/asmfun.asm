;汇编函数定义，提供给C语言调用
[format "WCOFF"]
[bits 32]

;制作目标文件信息
[file "naskfunc.nas"]
	global _io_hlt
;实际函数
[section .text]
_io_hlt:
	hlt
	ret