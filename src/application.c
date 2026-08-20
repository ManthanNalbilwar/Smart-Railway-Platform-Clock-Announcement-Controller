//application.c
#include <lpc21xx.h>
#include <string.h>
#include "types.h"
#include "defines.h"
#include "delay.h"
#include "lcddefines.h"
#include "lcd.h"
#include "KPM_defines.h"
#include "kpm_v2.h"
#include "eint_sw.h"
#include "Rtc.h"
#include "train_db.h"
#include "indicator.h"
#include "scheduler.h"
#include "menu.h"

extern u8 Line1_Scrolling_Massage[];

/* ---------------------------------------------------------------------
 * Tunable timings - all buzzer/alert durations live here so they're easy
 * to retune without hunting through the logic below.
 * ------------------------------------------------------------------- */
#define ARRIVAL_BEEP_ON_MS     900     // each of the 3 arrival beeps
#define ARRIVAL_BEEP_OFF_MS    400     // gap between the 3 arrival beeps
#define ARRIVAL_BEEP_COUNT     3

#define DEPARTURE_BEEP_ON_MS   2500    // single long beep when train leaves

#define LINE2_INFO_MS          3000    // "P.. A.. D.." shown for this long
#define LINE2_CLOCK_MS         3000    // then RTC clock shown for this long
#define LINE1_SCROLL_STEP_MS   300     // one scroll step of the train name
#define LINE1_SCROLL_STEPS     40      // total steps per Announce* call

// ---- Prints a number left-padded with zeros to a fixed width, so a
//      shorter value (e.g. platform "1") can never leave stray characters
//      from a previous, longer value (e.g. platform "12") behind on the
//      LCD - that stray-character overlap was the root of the "data
//      overlaps" complaint when Line2 content changed length. ----
static void PrintPadded(u32 val, u32 width)
{
	u32 count = 1;
	u32 t = val;
	u32 i;

	while(t >= 10) { t /= 10; count++; }

	for(i = count; i < width; i++)
		DISP_CHAR('0');

	DISP_INT(val);
}

static void ClearLine2(void)
{
	u32 i;
	CMD_TO_LCD(GOTO_LINE2_POS0);
	for(i=0; i<16; i++) DISP_CHAR(' ');
}

// ---- Draws "P<pp>A<hh:mm> D<hh:mm>" on line 2 - EXACTLY 16 characters,
//      matching the LCD's visible 16 columns. The earlier "P: .. A: .. D: .."
//      layout was 20 characters long; anything past column 16 on a 16x2
//      HD44780 gets silently pushed off the visible area, which is what
//      was actually causing fields to look cut off / overlapped. This
//      layout is budgeted char-by-char so it can never overflow:
//      P(1) + platform(2) + A(1) + hh:mm(5) + space(1) + D(1) + hh:mm(5) = 16 ----
static void DrawLine2(TrainInfo_t *t)
{
	ClearLine2();
	CMD_TO_LCD(GOTO_LINE2_POS0);
	DISP_CHAR('P');
	PrintPadded(t->platform, 2);
	DISP_CHAR('A');
	DISP_CHAR((t->updatedArrivalHour/10)+48);
	DISP_CHAR((t->updatedArrivalHour%10)+48);
	DISP_CHAR(':');
	DISP_CHAR((t->updatedArrivalMinute/10)+48);
	DISP_CHAR((t->updatedArrivalMinute%10)+48);
	DISP_CHAR(' ');
	DISP_CHAR('D');
	DISP_CHAR((t->updatedDepartureHour/10)+48);
	DISP_CHAR((t->updatedDepartureHour%10)+48);
	DISP_CHAR(':');
	DISP_CHAR((t->updatedDepartureMinute/10)+48);
	DISP_CHAR((t->updatedDepartureMinute%10)+48);
}

// ---- Draws "HH:MM:SS" perfectly centred on line 2 (8 chars centred in
//      16 columns -> 4 spaces of padding either side). Line is cleared
//      first so it never overlaps the platform-info text it replaces. ----
static void DrawLine2ClockCentered(void)
{
	s32 hh, mm, ss;
	GetRTCTimeInfo(&hh, &mm, &ss);

	ClearLine2();
	CMD_TO_LCD(GOTO_LINE2_POS0 + 4);
	DISP_CHAR((u32)hh/10+48);
	DISP_CHAR((u32)hh%10+48);
	DISP_CHAR(':');
	DISP_CHAR((u32)mm/10+48);
	DISP_CHAR((u32)mm%10+48);
	DISP_CHAR(':');
	DISP_CHAR((u32)ss/10+48);
	DISP_CHAR((u32)ss%10+48);
}

// ---- Line1 fixed part: train number, no wasted space -----------------
// cols 0-4  : 5 digit train number
// col  5    : single space
// cols 6-15 : (10 cols) scrolling window for the train name
static void DrawLine1Number(TrainInfo_t *t)
{
	CMD_TO_LCD(GOTO_LINE1_POS0);
	DISP_INT(t->trainNumber);
	DISP_CHAR(' ');
}

// ---- Runs for LINE1_SCROLL_STEPS steps. Line1 keeps scrolling the train
//      name the WHOLE time (never cleared/replaced by anything else -
//      this is the fix for "we don't want transition of the train info
//      line to RTC"). Line2 alternates every few seconds between the
//      platform/arrival/departure info and the centred RTC clock, each
//      redraw fully clearing line2 first so nothing overlaps. ----
static void RunInfoClockLoop(TrainInfo_t *t)
{
	u32 nameLen = strlen((char*)Line1_Scrolling_Massage);
	u32 offset = 0;
	u32 step;
	u32 cycleMs = 0;
	u32 showingClock = 0;
	u32 j;

	if(nameLen == 0) nameLen = 1;

	for(step = 0; step < LINE1_SCROLL_STEPS; step++)
	{
		// --- Line1: scroll one step, window only (cols 6-15), untouched otherwise ---
		CMD_TO_LCD(GOTO_LINE1_POS0 + 6);
		for(j = 0; j < 10; j++)
			DISP_CHAR(Line1_Scrolling_Massage[(offset+j) % nameLen]);
		offset++;

		// --- Line2: flip between info/clock on its own timer ---
		if(cycleMs >= (showingClock ? LINE2_CLOCK_MS : LINE2_INFO_MS))
		{
			showingClock = !showingClock;
			cycleMs = 0;
		}

		if(showingClock) DrawLine2ClockCentered();
		else              DrawLine2(t);

		delay_ms(LINE1_SCROLL_STEP_MS);
		cycleMs += LINE1_SCROLL_STEP_MS;

		if(admin_Edit_Flag == 1) return;
	}
}

// ---- APPROACHING phase: train is due within APPROACHING_THRESHOLD mins,
//      hasn't reached the platform yet. Line1 = fixed number + scrolling
//      name (kept alive throughout). Line2 = info/clock alternation,
//      BLINKING first if this train is delayed. ----
static void AnnounceApproaching(u32 index, u32 delayed)
{
	TrainInfo_t *t = Get_Record(index);
	if(t == 0) return;

	CMD_TO_LCD(CLEAR_LCD);
	delay_ms(2);

	DrawLine1Number(t);

	if(delayed)
	{
		u32 b;
		for(b=0; b<5; b++)
		{
			DrawLine2(t);
			delay_ms(400);
			if(admin_Edit_Flag == 1) return;
			ClearLine2();
			delay_ms(400);
			if(admin_Edit_Flag == 1) return;
		}
	}

	DrawLine2(t);
	RunInfoClockLoop(t);
}

// ---- AT_PLATFORM phase: train has arrived, waiting to depart. Line1
//      keeps showing the train name; Line2 alternates platform-info and
//      the live RTC clock, centred, without ever overwriting Line1. ----
static void AnnounceAtPlatform(u32 index)
{
	TrainInfo_t *t = Get_Record(index);
	if(t == 0) return;

	CMD_TO_LCD(CLEAR_LCD);
	delay_ms(2);

	DrawLine1Number(t);
	DrawLine2(t);

	RunInfoClockLoop(t);
}

// ---- Fired once, the instant a train reaches the platform:
//      three beeps + Yellow LED. Base LED colour (green/red on-time/
//      delayed indicator) is left exactly as it was. ----
static void AlertArrival(void)
{
	u32 i;
	for(i=0; i<ARRIVAL_BEEP_COUNT; i++)
	{
		LED_YellowOn();
		Buzzer_On();
		delay_ms(ARRIVAL_BEEP_ON_MS);
		Buzzer_Off();
		LED_YellowOff();
		delay_ms(ARRIVAL_BEEP_OFF_MS);

		if(admin_Edit_Flag == 1) return;
	}
}

// ---- Fired once, the instant a train leaves the platform: one long
//      continuous beep (departure = single long tone, arrival = 3 short/
//      medium beeps, so the two are audibly distinct). ----
static void AlertDeparture(void)
{
	SetIndicator(0, 1, 0);   // Yellow on for the duration of the departure tone
	Buzzer_On();
	delay_ms(DEPARTURE_BEEP_ON_MS);
	Buzzer_Off();
	SetIndicator(0, 0, 0);
}

int main(void)
{
	u32 firstBoot;
	s32 activeIdx;
	TrainInfo_t *t;
	u32 delayed;
	TrainState_t state;

	Init_KPM();
	Init_eint();
	firstBoot = RTC_Init();
	INIT_LCD_16X2();
	Init_LED_Buzzer();

	// ---- One-time boot self-test: proves each LED/buzzer is physically wired ----
	CMD_TO_LCD(CLEAR_LCD); delay_ms(2);
	CMD_TO_LCD(GOTO_LINE1_POS0);
	DISP_STRING("LED/BUZ TEST");
	LED_AllOff();
	LED_GreenOn();  delay_ms(400); LED_AllOff();
	LED_YellowOn(); delay_ms(400); LED_AllOff();
	LED_RedOn();    delay_ms(400); LED_AllOff();
	Buzzer_On();    delay_ms(300); Buzzer_Off();
	delay_ms(300);

	if(firstBoot)
	{
		FirstTimeSetup();
	}

	admin_Edit_Flag = 0;   // discard any spurious interrupt captured during boot/self-test/setup

	while(1)
	{
		activeIdx = UpdateTrainStates();

		if(admin_Edit_Flag == 1)
		{
			AdminMenu();
			continue;
		}

		if(activeIdx == -1)
		{
			// ---- Nothing due: plain, simple clock display, Green = all clear ----
			SetIndicator(1, 0, 0);

			CMD_TO_LCD(CLEAR_LCD);
			delay_ms(2);
			{
				s32 hh, mm, ss, dd, mo, yy, dow;
				u32 k;
				for(k=0; k<6; k++)
				{
					GetRTCTimeInfo(&hh, &mm, &ss);
					DisplayRTCTime((u32)hh, (u32)mm, (u32)ss);
					GetRTCDay(&dow);
					DisplayRTCDay((u32)dow);
					GetRTCDateInfo(&dd, &mo, &yy);
					DisplayRTCDate((u32)dd, (u32)mo, (u32)yy);
					delay_ms(500);
					if(admin_Edit_Flag == 1) break;
				}
			}
			continue;
		}

		t = Get_Record((u32)activeIdx);
		if(t == 0) continue;

		delayed = (t->delayMinutes > 0) ? 1 : 0;
		state   = GetTrainState((u32)activeIdx);

		if(state == TRAIN_STATE_APPROACHING)
		{
			SetIndicator(!delayed, 0, delayed);   // Green if on time, Red if delayed

			AnnounceApproaching((u32)activeIdx, delayed);
		}
		else if(state == TRAIN_STATE_AT_PLATFORM)
		{
			if(ConsumeArrivalEvent((u32)activeIdx))
			{
				AlertArrival();
			}

			SetIndicator(!delayed, 1, delayed);   // Yellow (at platform) + Green/Red base

			AnnounceAtPlatform((u32)activeIdx);
		}
		else if(state == TRAIN_STATE_DEPARTING)
		{
			if(ConsumeDepartureEvent((u32)activeIdx))
			{
				AlertDeparture();
			}
		}

		if(admin_Edit_Flag == 1)
		{
			AdminMenu();
		}
	}
}
