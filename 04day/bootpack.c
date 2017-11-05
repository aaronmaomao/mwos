void io_hlt(void);
void write_mem8(int addr, int data);

void HariMain(void)
{
	int i;
	char *p;	//让编译器知道这个地址处将要存放byte大小的数据
	for(i = 0xa0000; i <= 0xaffff; i++)
	{
		p = (char *)i;	
		*p = i & 0x0f;	//可以写成*((char *)i) = i & 0x0f, 该处代码等效于write_mem8
		write_mem8(i, i&0x0f);	//把15写到地址为0x0a0000~0x0affff中
	}
	
	for(;;)
	{
		io_hlt();
	}
}
