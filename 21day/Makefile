toolpath = ../../30day/tolset/z_tools/
incpath = ../../30day/tolset/z_tools/haribote/
objp = ./obj/
font = st
bootobjs = $(objp)bootpack.obj \
		   $(objp)asmfun.obj \
		   $(objp)$(font).obj \
		   $(objp)graphic.obj \
		   $(objp)dsctbl.obj \
		   $(objp)int.obj \
		   $(objp)fifo.obj \
		   $(objp)mouse.obj \
		   $(objp)keyboard.obj \
		   $(objp)memory.obj \
		   $(objp)sheet.obj \
		   $(objp)window.obj \
		   $(objp)timer.obj \
		   $(objp)mtask.obj \
		   $(objp)console.obj \
		   $(objp)file.obj

a_objs = $(objp)a.obj $(objp)a_nask.obj
hello3_objs = $(objp)hello3.obj $(objp)a_nask.obj
crack1_objs = $(objp)crack1.obj $(objp)a_nask.obj
		   
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

default:
	$(make) img

img: 
	$(make) mwos.img

mwos.img: ipl10.bin os.sys hello.mwe hello2.mwe hello3.mwe a.mwe crack1.mwe crack2.mwe Makefile
	$(edimg) imgin:$(toolpath)fdimg0at.tek wbinimg src:$(objp)ipl10.bin len:512 from:0 to:0 \
	copy from:$(objp)os.sys to:@: \
	copy from:ipl10.asm to:@: \
	copy from:Makefile to:@: \
	copy from:hello2.mwe to:@: \
	copy from:hello.mwe to:@: \
	copy from:hello3.mwe to:@: \
	copy from:a.mwe to:@: \
	copy from:crack1.mwe to:@: \
	copy from:crack2.mwe to:@: \
	imgout:../mwos.img

ipl10.bin: ipl10.asm Makefile
	$(nask) ipl10.asm $(objp)ipl10.bin

os.sys:	boothead.bin bootpack.hrb Makefile
	copy /B .\obj\boothead.bin+.\obj\bootpack.hrb .\obj\os.sys
	
boothead.bin: boothead.asm Makefile
	$(nask) boothead.asm $(objp)boothead.bin
	
bootpack.hrb: bootpack.bim Makefile
	$(bim2hrb) $(objp)bootpack.bim $(objp)bootpack.hrb 0
	
bootpack.bim: $(bootobjs) Makefile
	$(obj2bim) @$(rulefile) out:$(objp)bootpack.bim stack:3136k map:$(objp)bootpack.map $(bootobjs)

#生成汇编函数库目标文件
$(objp)asmfun.obj: asmfun.asm Makefile	
	$(nask) asmfun.asm $(objp)asmfun.obj $(objp)asmfun.lst

#生成字体目标文件	
$(objp)$(font).obj: $(font).font Makefile
	$(bin2obj) $(font).font $(objp)$(font).obj _font
	
#####应用程序相关的编译

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
	
#####################

#一般规则	
$(objp)%.obj: $(objp)%.asm Makefile
	$(nask) $(objp)$*.asm $(objp)$*.obj $(objp)$*.lst
	
$(objp)%.asm: $(objp)%.gas Makefile
	$(gas2nask) $(objp)$*.gas $(objp)$*.asm

$(objp)%.gas: %.c Makefile
	$(cc1) -o $(objp)$*.gas $*.c

#consolas16.bin: consolas16.txt Makefile
#	$(makefont) consolas16.txt $(objp)consolas16.bin

run1: 
	F:\vbox\install\VirtualBox.exe --startvm F:\vbox\mwos\mwos.vbox
debug1:
	F:\vbox\install\VirtualBox.exe --startvm F:\vbox\mwos\mwos.vbox --debug
run2:
	F:\install\vbox\install\VirtualBox.exe --startvm C:\Users\mao-zhengjun\vbox\mwos\mwos.vbox
debug2:
	F:\install\vbox\install\VirtualBox.exe --startvm C:\Users\mao-zhengjun\vbox\mwos\mwos.vbox --debug
	
clean:
	del /Q .\obj\*.lst
	del /Q .\obj\*.obj
	del /Q .\obj\*.hrb
	del /Q .\obj\*.bim
	del /Q .\obj\*.map
	del /Q .\obj\*.bin
	del /Q .\obj\*.sys
	
a1:
	del /Q .\obj\*.mwe
	del /Q .\*.mwe
	$(make) default
	$(make) run1
a2:
	del /Q .\obj\*.mwe
	del /Q .\*.mwe
	$(make) clean
	$(make) default
	$(make) run2
