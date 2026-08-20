//eint_sw.c
#include "types.h"
#include "defines.h"
#include "eint_sw.h"
#include <lpc21xx.h>

volatile u32 admin_Edit_Flag=0;

void Init_eint(void)
{
	PINSEL0&=~(3<<(1*2));
	PINSEL0|=EINT0_PIN_FUNC;
	VICIntEnable=1<<EINT0_VIC_CHNO;
	VICVectCntl0=(1<<5)|EINT0_VIC_CHNO;
	VICVectAddr0=(u32 ) eint0_isr;
	EXTMODE=1<<0;
}
void eint0_isr(void) __irq
{
	if(READBIT(IOPIN0,ADMIN_SW)== 0)
	{
		admin_Edit_Flag=1;
	}
	VICVectAddr=0;
	EXTINT=1<<0;
}
