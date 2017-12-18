void api_putchar(int c);
void api_end(void);

void HariMain(void)
{
	char a[100];
	//a[10] = 'A';
	api_putchar(a[10]);
	//a[100] = 'B';
	api_putchar(a[100]);
	//a[100] = 'C';
	api_putchar(a[104]);	//栈异常
	api_putchar(a[120] + 'A');	//栈异常
	api_end();
}
