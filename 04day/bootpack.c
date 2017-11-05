void io_hlt(void);
void write_mem8(int addr, int data);

void HariMain(void)
{
	int i;
	char *p;	//�ñ�����֪�������ַ����Ҫ���byte��С������
	for(i = 0xa0000; i <= 0xaffff; i++)
	{
		p = (char *)i;	
		*p = i & 0x0f;	//����д��*((char *)i) = i & 0x0f, �ô������Ч��write_mem8
		write_mem8(i, i&0x0f);	//��15д����ַΪ0x0a0000~0x0affff��
	}
	
	for(;;)
	{
		io_hlt();
	}
}
