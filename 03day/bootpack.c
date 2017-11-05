void io_hlt(void);

void HariMain(void)
{
fin:
	io_hlt();	//执行汇编里定义的方法
	goto fin;
}