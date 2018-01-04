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

bug1_objs = $(objp)bug1.obj $(objp)a_nask.obj
bug2_objs = $(objp)bug2.obj $(objp)a_nask.obj
bug3_objs = $(objp)bug3.obj $(objp)a_nask.obj
hello4_objs = $(objp)hello4.obj $(objp)a_nask.obj
winhello_objs = $(objp)winhello.obj $(objp)a_nask.obj

default: bug1.mwe bug2.mwe bug3.mwe hello4.mwe winhello.mwe
	
bug1.mwe: $(objp)bug1.bim Makefile
	$(bim2hrb) $(objp)bug1.bim bug1.mwe 0
$(objp)bug1.bim: $(bug1_objs) Makefile
	$(obj2bim) @$(rulefile) out:$(objp)bug1.bim map:$(objp)bug1.map $(bug1_objs)

bug2.mwe: $(objp)bug2.bim Makefile
	$(bim2hrb) $(objp)bug2.bim bug2.mwe 0
$(objp)bug2.bim: $(bug2_objs) Makefile
	$(obj2bim) @$(rulefile) out:$(objp)bug2.bim map:$(objp)bug2.map $(bug2_objs)

bug3.mwe: $(objp)bug3.bim Makefile
	$(bim2hrb) $(objp)bug3.bim bug3.mwe 0
$(objp)bug3.bim: $(bug3_objs) Makefile
	$(obj2bim) @$(rulefile) out:$(objp)bug3.bim map:$(objp)bug3.map $(bug3_objs)

hello4.mwe: $(objp)hello4.bim Makefile
	$(bim2hrb) $(objp)hello4.bim hello4.mwe 0
$(objp)hello4.bim: $(hello4_objs) Makefile
	$(obj2bim) @$(rulefile) out:$(objp)hello4.bim stack:1k map:$(objp)hello4.map $(hello4_objs)	

winhello.mwe: $(objp)winhello.bim Makefile
	$(bim2hrb) $(objp)winhello.bim winhello.mwe 40k
$(objp)winhello.bim: $(winhello_objs) Makefile
	$(obj2bim) @$(rulefile) out:$(objp)winhello.bim stack:1k map:$(objp)winhello.map $(winhello_objs)	
	
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