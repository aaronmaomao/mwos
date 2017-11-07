toolpath = ../../30day/tolset/z_tools/
incpath = ../../30day/tolset/z_tools/haribote/
objp = ./obj/
font = st

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
bin2obj  = $(toolpath)bin2obj.exe

default :
	$(make) img

img: 
	$(make) mwos.img

mwos.img: ipl10.bin os.sys Makefile
	$(edimg) imgin:$(toolpath)fdimg0at.tek wbinimg src:$(objp)ipl10.bin len:512 from:0 to:0 copy from:$(objp)os.sys to:@: imgout:../mwos.img

ipl10.bin: ipl10.asm Makefile
	$(nask) ipl10.asm $(objp)ipl10.bin

os.sys:	boothead.bin bootpack.hrb Makefile
	copy /B .\obj\boothead.bin+.\obj\bootpack.hrb .\obj\os.sys
	
boothead.bin: boothead.asm Makefile
	$(nask) boothead.asm $(objp)boothead.bin
	
bootpack.hrb: bootpack.bim Makefile
	$(bim2hrb) $(objp)bootpack.bim $(objp)bootpack.hrb 0
	
bootpack.bim: bootpack.obj asmfun.obj font.obj Makefile
	$(obj2bim) @$(rulefile) out:$(objp)bootpack.bim stack:3136k map:$(objp)bootpack.map $(objp)bootpack.obj $(objp)asmfun.obj $(objp)font.obj

bootpack.obj: bootpack.asm Makefile
	$(nask) $(objp)bootpack.asm $(objp)bootpack.obj $(objp)bootpack.lst
	
bootpack.asm: bootpack.gas Makefile
	$(gas2nask) $(objp)bootpack.gas $(objp)bootpack.asm

bootpack.gas: bootpack.c Makefile
	$(cc1) -o $(objp)bootpack.gas bootpack.c
	
asmfun.obj: asmfun.asm Makefile
	$(nask) asmfun.asm $(objp)asmfun.obj $(objp)asmfun.lst
	
font.obj: $(font).font Makefile
	$(bin2obj) $(font).font $(objp)font.obj _font

#consolas16.bin: consolas16.txt Makefile
#	$(makefont) consolas16.txt $(objp)consolas16.bin
	
clean:
	del /Q .\obj\*.lst
	del /Q .\obj\*.obj
	del /Q .\obj\*.hrb
	del /Q .\obj\*.asm
	del /Q .\obj\*.gas
	del /Q .\obj\*.bim
	del /Q .\obj\*.map
	del /Q .\obj\*.bin
	
	
	