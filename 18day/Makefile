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
		   $(objp)mtask.obj

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

mwos.img: ipl10.bin os.sys Makefile
	$(edimg) imgin:$(toolpath)fdimg0at.tek wbinimg src:$(objp)ipl10.bin len:512 from:0 to:0 \
	copy from:$(objp)os.sys to:@: \
	copy from:ipl10.asm to:@: \
	copy from:run.bat to:@: \
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
	del /Q .\obj\*.asm
	del /Q .\obj\*.gas
	del /Q .\obj\*.bim
	del /Q .\obj\*.map
	del /Q .\obj\*.bin
	del /Q .\obj\*.sys
	
	