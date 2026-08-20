//Rtc_v2.c
#include <lpc21xx.h>
#include "Rtc.h"
#include "types.h"
#include "lcddefines.h"
#include "lcd.h"
#include "delay.h"

char week[][4] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};

u32 RTC_Init(void)
{
	u32 needsInit = 0;

	delay_ms(50);   // let the 32.768kHz crystal/CCR register settle right after power-on
	                 // before trusting its state - fixes "sometimes works, sometimes not"

	if((CCR & RTC_ENABLE) == 0)   // RTC not already running -> genuine first boot
	{
		needsInit = 1;
	}
	else
	{
		// CCR says "already running", but on some boards (VBAT/crystal not
		// perfectly stable at power-on) the register can lie. Sanity-check
		// the actual values - if anything is out of valid range, treat it
		// as corrupted and force a proper re-init anyway.
		if(HOUR > 23 || MIN > 59 || SEC > 59 ||
		   DOM == 0  || DOM > 31 ||
		   MONTH == 0 || MONTH > 12 ||
		   YEAR < 2000 || YEAR > 2099)
		{
			needsInit = 1;
		}
	}

	if(needsInit)
	{
		CCR = RTC_RESET;
		CCR = 0x00;
		PREINT  = PREINT_VAL;
		PREFRAC = PREFRAC_VAL;
		SEC = 0; MIN = 0; HOUR = 0;
		DOM = 1; MONTH = 1; YEAR = 2026;
		CCR = RTC_ENABLE;
		return 1;   // caller should now ask for time/date via keypad
	}
	return 0;   // already running with sane values (battery-backed) - leave untouched
}
void GetRTCTimeInfo(s32 *hour,s32 *minute,s32 *second)
{
	*hour = HOUR;
	*minute = MIN;
	*second = SEC;
}
void DisplayRTCTime(u32 hour,u32 minute,u32 second)
{
	CMD_TO_LCD(GOTO_LINE1_POS0);
	DISP_CHAR(hour/10+48);
	DISP_CHAR(hour%10+48);
	DISP_CHAR(':');
	DISP_CHAR(minute/10+48);
	DISP_CHAR(minute%10+48);
	DISP_CHAR(':');
	DISP_CHAR(second/10+48);
	DISP_CHAR(second%10+48);
}
void GetRTCDateInfo(s32 *date,s32 *month,s32 *year)
{
	*date = DOM;
	*month = MONTH;
	*year = YEAR;
}
void DisplayRTCDate(u32 date,u32 month,u32 year)
{
	CMD_TO_LCD(GOTO_LINE2_POS0);
	DISP_CHAR(date/10+48);
	DISP_CHAR(date%10+48);
	DISP_CHAR('/');
	DISP_CHAR(month/10+48);
	DISP_CHAR(month%10+48);
	DISP_CHAR('/');
	DISP_INT(year);
}
void SetRTCTimeInfo(u32 hour,u32 minute,u32 second)
{
	HOUR = hour;
	MIN = minute;
	SEC = second;
}
void SetRTCDateInfo(u32 date,u32 month,u32 year)
{
	DOM = date;
	MONTH = month;
	YEAR = year;
}
void GetRTCDay(s32 *dow)
{
	*dow = DOW;
}
void DisplayRTCDay(u32 day)
{
	CMD_TO_LCD(GOTO_LINE1_POS0 + 10);
	DISP_STRING(week[day]);
}
void SetRTCDay(u32 dow)
{
	DOW = dow;
}
