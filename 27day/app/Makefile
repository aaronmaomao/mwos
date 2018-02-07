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
#将多个obj文件封装为lib静态库
golib = $(toolpath)golib00.exe

apis_objs = $(objp)_api_putchar.obj, $(objp)_api_putstr0.obj, \
			$(objp)_api_openwin.obj, $(objp)_api_putstrwin.obj, $(objp)_api_boxfillwin.obj, $(objp)_api_point.obj, $(objp)_api_refreshwin.obj, $(objp)_api_linewin.obj, $(objp)_api_closewin.obj, $(objp)_api_getkey.obj \
			$(objp)_api_alloctimer.obj, $(objp)_api_inittimer.obj, $(objp)_api_settimer.obj, $(objp)_api_freetimer.obj \
			$(objp)_api_initmalloc.obj, $(objp)_api_malloc.obj, $(objp)_api_free.obj \
			$(objp)_api_beep.obj \
			$(objp)_api_end.obj

apislib = ../api/apislib.lib

default: hello4.mwe winhello.mwe noodle.mwe color.mwe
	
hello4.mwe: $(objp)hello4.bim Makefile
	$(bim2hrb) $(objp)hello4.bim hello4.mwe 0
$(objp)hello4.bim: $(objp)hello4.obj $(apislib) Makefile
	$(obj2bim) @$(rulefile) out:$(objp)hello4.bim stack:1k map:$(objp)hello4.map $(objp)hello4.obj $(apislib)

winhello.mwe: $(objp)winhello.bim Makefile
	$(bim2hrb) $(objp)winhello.bim winhello.mwe 48k
$(objp)winhello.bim: $(objp)winhello.obj $(apislib) Makefile
	$(obj2bim) @$(rulefile) out:$(objp)winhello.bim stack:1k map:$(objp)winhello.map $(objp)winhello.obj $(apislib)

noodle.mwe: $(objp)noodle.bim Makefile
	$(bim2hrb) $(objp)noodle.bim noodle.mwe 40k
$(objp)noodle.bim: $(objp)noodle.obj $(apislib) Makefile
	$(obj2bim) @$(rulefile) out:$(objp)noodle.bim stack:1k map:$(objp)noodle.map $(objp)noodle.obj $(apislib)

color.mwe: $(objp)color.bim Makefile
	$(bim2hrb) $(objp)color.bim color.mwe 56K
$(objp)color.bim: $(objp)color.obj $(apislib) Makefile
	$(obj2bim) @$(rulefile) out:$(objp)color.bim stack:1k map:$(objp)color.map $(objp)color.obj $(apislib)

$(apislib):
	$(make) -C ../api/

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
	