//indicator.c
#include <lpc21xx.h>
#include "indicator.h"
#include "delay.h"

void Init_LED_Buzzer(void)
{
	IODIR0|=(15<<GREEN_LED);
}
void LED_GreenOn(void)
{
	IOSET0=(1<<GREEN_LED);
}
void LED_YellowOn(void)
{
	IOSET0=(1<<YELLOW_LED);
}
void LED_RedOn(void)
{
	IOSET0=(1<<RED_LED);
}
void LED_AllOff(void)
{
	IOCLR0=(1<<GREEN_LED)|(1<<YELLOW_LED)|(1<<RED_LED);
}
void LED_GreenOff(void)
{
	IOCLR0=(1<<GREEN_LED);
}
void LED_YellowOff(void)
{
	IOCLR0=(1<<YELLOW_LED);
}
void LED_RedOff(void)
{
	IOCLR0=(1<<RED_LED);
}
// IOSET0/IOCLR0 only set/clear the bits you name - they never touch the
// other pins on the port. That means "turn Green on" by itself does NOT
// turn Yellow/Red off, so every state change MUST clear-then-set in one
// place, or a missed AllOff() anywhere leaves old LEDs stuck lit ("always
// all on, not following the flow"). Route every LED change through here.
void SetIndicator(u32 green, u32 yellow, u32 red)
{
	LED_AllOff();
	if(green)  LED_GreenOn();
	if(yellow) LED_YellowOn();
	if(red)    LED_RedOn();
}
void Buzzer_On(void)
{
	IOSET0=(1<<BUZZER);
}
void Buzzer_Off(void)
{
	IOCLR0=(1<<BUZZER);
}
void Buzzer_Beep(u32 onMs, u32 offMs)
{
	Buzzer_On();
	delay_ms(onMs);
	Buzzer_Off();
	delay_ms(offMs);
}
