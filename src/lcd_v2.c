//lcd_v2.c
#include <lpc21xx.h>
#include "delay.h"
#include "lcddefines.h"
#include "defines.h"
#include "lcd.h"
#include "types.h"
void INIT_LCD_16X2(void)
{
	IODIR0|=((0XFF<<LCD_DATA)|(1<<LCD_RS)|(1<<LCD_RW)|(1<<LCD_EN));
	delay_ms(15);
	CMD_TO_LCD(0X30);
	delay_ms(4);
	delay_us(100);
	CMD_TO_LCD(0X30);
	delay_ms(100);
	CMD_TO_LCD(0X30);
	CMD_TO_LCD(MODE_8BIT_2LINE);
	CMD_TO_LCD(DISP_ON_CUR_BLK);
	CMD_TO_LCD(CLEAR_LCD);
	CMD_TO_LCD(SHIFT_CUR_RIGHT);	
}
void WRITE_TO_LCD(u8 BYTE)
{
	WRITEBYTE(IOPIN0,LCD_DATA,BYTE);
	IOCLR0=(1<<LCD_RW);
	IOSET0=(1<<LCD_EN);
	delay_us(1);
	IOCLR0=(1<<LCD_EN);
	delay_us(100);
}
void CMD_TO_LCD(u8 cmd)
{
	IOCLR0=(1<<LCD_RS);
	WRITE_TO_LCD(cmd);
	if(cmd == CLEAR_LCD || cmd == RETURN_CUR_HOME)
	{
		delay_ms(2);
	}
}
void DISP_CHAR(u8 asscival)
{
	IOSET0=(1<<LCD_RS);
	WRITE_TO_LCD(asscival);
}
void DISP_STRING(s8 *str)
{
	while(*str!='\0')
	{
		DISP_CHAR(*str++);
	}
}
void DISP_INT(u32 Num)
{
	u8 a[10];
	s32 i=0;
	if(Num == 0) 
		DISP_CHAR('0');
	else
	{
		while(Num>0)
		{
			a[i]=(Num%10)+48;
			Num/=10;
			i++;
		}
		for(--i;i>=0;i--)
		{
			DISP_CHAR(a[i]);
		}
	}
}
void DISP_NGTVE_INT(s32 Num)
{
	if(Num<0)
	{
		DISP_CHAR('-');
		Num = -Num;
	}
	DISP_INT(Num);
}
void DISP_FLOAT(f32 FNum,u32 dp)
{
	u32 n;
	s32 i;
	if(FNum <0.0)
	{
		DISP_CHAR('-');
		FNum = -FNum;
	}
	n=FNum;
	DISP_INT(n);
	DISP_CHAR('.');
	for(i=0;i<dp;i++)
	{
		FNum=((FNum-n)*10);
		n=FNum;
		DISP_CHAR(n+48);
	}
}
void BUILD_CGRAM(u8 *P, u8 BYTES)
{
    u8 i;
    CMD_TO_LCD(GOTO_CGRAM_START);
    IOSET0=(1<<LCD_RS);
    for(i=0;i<BYTES;i++)
    {
        WRITE_TO_LCD(P[i]);
    }
    CMD_TO_LCD(GOTO_LINE1_POS0); 
}
