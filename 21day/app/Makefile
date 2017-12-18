toolpath = ../../../30day/tolset/z_tools/
incpath = ../../../30day/tolset/z_tools/haribote/
objp = ./obj/

make = $(toolpath)make.exe -r
#制作img镜像
edimg = $(toolpath)edimg.exe
#将二进制镜像文件转为本os可运行的文件
bim2hrb = $(toolpath)bim2hrb.exe
#将obj文件链接为bim为二进制镜像文件
obj2bim = $(toolpath)obj2bim.exe
#asm汇编
nask = $(toolpath)nask.exe
#将gas汇编转为asm汇编
gas2nask = $(toolpath)gas2nask.exe
#将.c源文件编译为gas汇编
cc1 = $(toolpath)cc1.exe -I$(incpath) -Os -Wall -quiet
#链接规则
rulefile = $(toolpath)haribote/haribote.rul
#制作字体二进制文件
makefont = $(toolpath)makefont.exe
#将字体二进制文件转为可链接的obj文件
bin2obj = $(toolpath)bin2obj.exe

a_objs = $(objp)a.obj $(objp)a_nask.obj
hello3_objs = $(objp)hello3.obj $(objp)a_nask.obj
crack1_objs = $(objp)crack1.obj $(objp)a_nask.obj

default: hello.mwe \
		hello2.mwe \
		crack2.mwe \
		a.mwe \
		hello3.mwe \
		crack1.mwe

hello.mwe:
	$(nask) hello.asm hello.mwe
hello2.mwe:
	$(nask) hello2.asm hello2.mwe
crack2.mwe:
	$(nask) crack2.asm crack2.mwe
	
a.mwe: $(objp)a.bim Makefile
	$(bim2hrb) $(objp)a.bim a.mwe 0
$(objp)a.bim: $(a_objs) Makefile
	$(obj2bim) @$(rulefile) out:$(objp)a.bim map:$(objp)a.map $(a_objs)
	
hello3.mwe: $(objp)hello3.bim Makefile
	$(bim2hrb) $(objp)hello3.bim hello3.mwe 0
$(objp)hello3.bim: $(hello3_objs) Makefile
	$(obj2bim) @$(rulefile) out:$(objp)hello3.bim map:$(objp)hello3.map $(hello3_objs)
	
crack1.mwe: $(objp)crack1.bim Makefile
	$(bim2hrb) $(objp)crack1.bim crack1.mwe 0
$(objp)crack1.bim: $(crack1_objs) Makefile
	$(obj2bim) @$(rulefile) out:$(objp)crack1.bim map:$(objp)crack1.map $(crack1_objs)
	
$(objp)a_nask.obj: a_nask.asm Makefile
	$(nask) a_nask.asm $(objp)a_nask.obj $(objp)a_nask.lst

#一般规则	
$(objp)%.obj: $(objp)%.asm Makefile
	$(nask) $(objp)$*.asm $(objp)$*.obj $(objp)$*.lst
	
$(objp)%.asm: $(objp)%.gas Makefile
	$(gas2nask) $(objp)$*.gas $(objp)$*.asm

$(objp)%.gas: %.c Makefile
	$(cc1) -o $(objp)$*.gas $*.c

clean:
	del /Q .\obj\*.lst
	del /Q .\obj\*.obj
	del /Q .\obj\*.hrb
	del /Q .\obj\*.bim
	del /Q .\obj\*.map
	del /Q .\obj\*.bin
	del /Q .\obj\*.sys
	del /Q .\*.mwe