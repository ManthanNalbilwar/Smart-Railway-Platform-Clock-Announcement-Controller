#ifndef INDICATOR_H
#define INDICATOR_H
#include "types.h"
#define GREEN_LED   2
#define YELLOW_LED  3
#define RED_LED     4
#define BUZZER      5
void Init_LED_Buzzer(void);
void LED_GreenOn(void);
void LED_YellowOn(void);
void LED_RedOn(void);
void LED_AllOff(void);
void LED_GreenOff(void);
void LED_YellowOff(void);
void LED_RedOff(void);
// ---- Atomically sets the 3 indicator LEDs to an exact combo (clears the
//      other two first) - use this instead of individual On() calls so no
//      code path can ever leave a stale LED lit ("always all on" bug). ----
void SetIndicator(u32 green, u32 yellow, u32 red);
void Buzzer_On(void);
void Buzzer_Off(void);
void Buzzer_Beep(u32 onMs, u32 offMs);   // blocking: buzzer ON for onMs, OFF for offMs
#endif
