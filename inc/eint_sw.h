//eint_sw.h
#ifndef EINT_SW_H
#define EINT_SW_H
#include "types.h"
#define ADMIN_SW 				1
#define EINT0_VIC_CHNO 	14
#define EINT0_PIN_FUNC  0x0000000C
extern volatile u32 admin_Edit_Flag;
void Init_eint(void);
void eint0_isr(void) __irq;
#endif
