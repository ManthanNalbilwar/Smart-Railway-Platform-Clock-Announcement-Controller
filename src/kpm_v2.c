//kpm_v2.c
#include "types.h"
#include "defines.h"
#include "KPM_defines.h"
#include <lpc21xx.h>

u8 KPMLUT[4][4]={	 {'1','2','3','A'},
									 {'4','5','6','B'},
									 {'7','8','9','C'},
									 {'*','0','#','D'} };

void Init_KPM(void)
{
	WRITENIBBLE(IODIR1,ROW0,15);
}
u32 colscan(void)
{
	if(READNIBBLE(IOPIN1,COL0)<15)
		return 0;
	else
		return 1;
}
u32 rowcheck(void)
{
	u32 rno;
	for(rno=0; rno<4; rno++)
	{
		WRITENIBBLE(IOPIN1,ROW0,(~(1<<rno)));
		if(colscan()==0)
			break;
	}
	IOCLR1=15<<ROW0;
	return rno;
}
u32 colcheck(void)
{
	u32 cno;
	for(cno=0; cno<4; cno++)
	{
		if(READBIT(IOPIN1,(cno+COL0))==0)
			break;
	}
	return cno;
}
u32 keyscan(void)
{
	u32 rno,cno,key;
	while(colscan());
	rno=rowcheck();
	cno=colcheck();
	key=KPMLUT[rno][cno];
	while(!colscan());
	return key;
}
u32 ReadNum(u8 *SpKey)
{
	u8 key;
	u32 num=0;
	while(1)
	{
		key=keyscan();
		if( (key>='0') && (key<='9') )
		{
			num=(num*10)+(key-48);
		}
		else
		{
			*SpKey=key;
			break;
		}
	}
	return num;
}
