/*
 * crack1.c
 *
 *  Created on: 2017年12月15日
 *      Author: aaron
 */
void api_end(void);

void HariMain(void)
{
	*(char *) (0x00102600) = 0;
	api_end();
}
