toolpath = ../../30day/tolset/z_tools/
incpath = ../../30day/tolset/z_tools/haribote/
objpath = ./obj/

make = $(toolpath)make.exe -r
edimg = $(toolpath)edimg.exe
nask = $(toolpath)nask.exe
bim2hrb = $(toolpath)bim2hrb.exe
obj2bim = $(toolpath)obj2bim.exe
gas2nask = $(toolpath)gas2nask.exe
ccl = $(toolpath)ccl.exe

default :
	$(make) img

img: 
	$(make) mwos.img

mwos.img: ipl10.bin boothead.sys Makefile
	$(edimg) imgin:$(toolpath)fdimg0at.tek wbinimg src:$(objpath)ipl10.bin len:512 from:0 to:0 copy from:$(objpath)boothead.sys to:@: imgout:../mwos.img
	
ipl10.bin: ipl10.asm Makefile
	$(nask) ipl10.asm $(objpath)ipl10.bin
	
boothead.sys: boothead.asm Makefile
	$(nask) boothead.asm $(objpath)boothead.sys
	

bootpack.hrb: bootpack.bim Makefile
	$(bim2hrb) $(objpath)bootpack.bim $(objpath)bootpack.hrb 0
	
bootpack.bim: bootpack.obj asmfun.obj Makefile
	