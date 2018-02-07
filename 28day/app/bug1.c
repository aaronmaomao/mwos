void HariMain(void)
{
	char a[100];
	//a[10] = 'A';
	api_putchar(a[10]);
	//a[100] = 'B';
	api_putchar(a[100]);
	//a[100] = 'C';
	api_putchar(a[123]);	//栈异常
	api_end();
}
