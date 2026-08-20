//menu.c
#include "types.h"
#include "defines.h"
#include "kpm_v2.h"
#include "KPM_defines.h"
#include "lcd.h"
#include "lcddefines.h"
#include "Rtc.h"
#include "train_db.h"
#include "eint_sw.h"
#include "menu.h"
#include "delay.h"

// ---- Small helper: show a message on line1 for ~1s ----
static void ShowMsg(s8 *msg)
{
	CMD_TO_LCD(CLEAR_LCD);
	delay_ms(2);
	CMD_TO_LCD(GOTO_LINE1_POS0);
	DISP_STRING(msg);
	delay_ms(1000);
}

// ---- Shown ONCE per edit session (not repeated on every field) ----
static void ShowControlsHint(void)
{
	CMD_TO_LCD(CLEAR_LCD);
	delay_ms(2);
	CMD_TO_LCD(GOTO_LINE1_POS0);
	DISP_STRING("#next *save C=X");
	CMD_TO_LCD(GOTO_LINE2_POS0);
	DISP_STRING("D=backspace");
	delay_ms(1300);
}

// ---- Prints a number left-padded with zeros to a fixed width (e.g. "08") ----
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

// ---- Reads digits with LIVE ECHO on LCD line2 ("new: xx"), backspace ('D').
//      Line1 always shows "<fieldName>: <currentValue>" for context.
//      Cross-validated: value must be within [minVal,maxVal] and at most
//      maxDigits long, else rejected and the SAME field re-asked.
//      Terminator key ('#','*','C') returned via *termKey. ----
static u32 InputField(s8 *fieldName, u32 defaultVal, u32 minVal, u32 maxVal, u32 maxDigits, u8 *termKey)
{
	u8 buf[7];
	u8 idx;
	u8 key;
	u32 i;
	u32 result;

	if(maxDigits > 6) maxDigits = 6;

	while(1)
	{
		idx = 0;

		CMD_TO_LCD(CLEAR_LCD);
		delay_ms(2);

		CMD_TO_LCD(GOTO_LINE1_POS0);
		DISP_STRING(fieldName);
		DISP_STRING(": ");
		PrintPadded(defaultVal, maxDigits);

		CMD_TO_LCD(GOTO_LINE2_POS0);
		DISP_STRING("new: ");

		while(1)
		{
			key = keyscan();

			if(key >= '0' && key <= '9' && idx < maxDigits)
			{
				buf[idx++] = key;
				DISP_CHAR(key);
			}
			else if(key == 'D')
			{
				if(idx > 0)
				{
					idx--;
					CMD_TO_LCD(GOTO_LINE2_POS0 + 5);         // right after "new: "
					for(i=0; i<11; i++) DISP_CHAR(' ');       // clear just the digit area
					CMD_TO_LCD(GOTO_LINE2_POS0 + 5);
					for(i=0; i<idx; i++) DISP_CHAR(buf[i]);   // redraw remaining digits
				}
			}
			else
			{
				*termKey = key;
				break;
			}
		}

		if(*termKey == 'C')
			return defaultVal;

		if(idx == 0)
			return defaultVal;   // nothing typed - keep current value

		result = 0;
		for(i=0; i<idx; i++)
			result = result*10 + (buf[i]-48);

		if(result < minVal || result > maxVal)
		{
			CMD_TO_LCD(CLEAR_LCD);
			delay_ms(2);
			CMD_TO_LCD(GOTO_LINE1_POS0);
			DISP_STRING("Invalid! Range:");
			CMD_TO_LCD(GOTO_LINE2_POS0);
			DISP_INT(minVal);
			DISP_CHAR('-');
			DISP_INT(maxVal);
			delay_ms(1200);
			continue;   // re-ask the SAME field
		}

		return result;
	}
}

void AdminMenu(void)
{
	u8 choice;
	u8 termKey;

	CMD_TO_LCD(CLEAR_LCD);
	delay_ms(2);
	CMD_TO_LCD(GOTO_LINE1_POS0);
	DISP_STRING("Enter The Choice");
	CMD_TO_LCD(GOTO_LINE2_POS0);
	DISP_STRING("A:EdtTm B:RschT");

	choice = keyscan();

	if(choice == 'A')
	{
		s32 curHH, curMM, curSS, curDOW;
		s32 curDD, curMO, curYY;
		u32 hh, mm, ss, dow, DD, MO, YYYY;
		u8  sub;
		u8  cancelled;

		GetRTCTimeInfo(&curHH, &curMM, &curSS);
		GetRTCDateInfo(&curDD, &curMO, &curYY);
		GetRTCDay(&curDOW);
		hh = (u32)curHH; mm = (u32)curMM; ss = (u32)curSS; dow = (u32)curDOW;
		DD = (u32)curDD; MO = (u32)curMO; YYYY = (u32)curYY;

		CMD_TO_LCD(CLEAR_LCD);
		delay_ms(2);
		CMD_TO_LCD(GOTO_LINE1_POS0);
		DISP_STRING("1:DD 2:MO 3:YR");
		CMD_TO_LCD(GOTO_LINE2_POS0);
		DISP_STRING("4:HH 5:MM 6:ALL");

		sub = keyscan();
		ShowControlsHint();   // shown ONCE, before the field(s) for this session

		switch(sub)
		{
			case '1':
				DD = InputField("Date", DD, 1, 31, 2, &termKey);
				if(termKey != 'C') { SetRTCDateInfo(DD, MO, YYYY); ShowMsg("DD Updated!"); }
				else                 ShowMsg("Cancelled");
				break;

			case '2':
				MO = InputField("Month", MO, 1, 12, 2, &termKey);
				if(termKey != 'C') { SetRTCDateInfo(DD, MO, YYYY); ShowMsg("MO Updated!"); }
				else                 ShowMsg("Cancelled");
				break;

			case '3':
				YYYY = InputField("Year", YYYY, 2000, 2026, 4, &termKey);
				if(termKey != 'C') { SetRTCDateInfo(DD, MO, YYYY); ShowMsg("YR Updated!"); }
				else                 ShowMsg("Cancelled");
				break;

			case '4':
				hh = InputField("Hours", hh, 0, 23, 2, &termKey);
				if(termKey != 'C') { SetRTCTimeInfo(hh, mm, ss); ShowMsg("HH Updated!"); }
				else                 ShowMsg("Cancelled");
				break;

			case '5':
				mm = InputField("Minutes", mm, 0, 59, 2, &termKey);
				if(termKey != 'C') { SetRTCTimeInfo(hh, mm, ss); ShowMsg("MM Updated!"); }
				else                 ShowMsg("Cancelled");
				break;

			case '6':
				// ---- ALL: walk Time -> Day -> Date, one field at a time ----
				// Hour, Minute, Sec, Day(of week), Date, Month, Year.
				// '*' on any field saves everything entered so far and exits
				// early; '#' saves the field and moves to the next; 'C'
				// cancels the WHOLE sequence (nothing is written to the RTC).
				cancelled = 0;

				hh = InputField("Hours", hh, 0, 23, 2, &termKey);
				if(termKey == 'C') cancelled = 1;

				if(!cancelled && termKey != '*')
				{
					mm = InputField("Minutes", mm, 0, 59, 2, &termKey);
					if(termKey == 'C') cancelled = 1;
				}

				if(!cancelled && termKey != '*')
				{
					ss = InputField("Seconds", ss, 0, 59, 2, &termKey);
					if(termKey == 'C') cancelled = 1;
				}

				if(!cancelled && termKey != '*')
				{
					dow = InputField("Day(0=Sun)", dow, 0, 6, 1, &termKey);
					if(termKey == 'C') cancelled = 1;
				}

				if(!cancelled && termKey != '*')
				{
					DD = InputField("Date", DD, 1, 31, 2, &termKey);
					if(termKey == 'C') cancelled = 1;
				}

				if(!cancelled && termKey != '*')
				{
					MO = InputField("Month", MO, 1, 12, 2, &termKey);
					if(termKey == 'C') cancelled = 1;
				}

				if(!cancelled && termKey != '*')
				{
					YYYY = InputField("Year", YYYY, 2000, 2026, 4, &termKey);
					if(termKey == 'C') cancelled = 1;
				}

				if(cancelled)
				{
					ShowMsg("Cancelled");
				}
				else
				{
					SetRTCTimeInfo(hh, mm, ss);
					SetRTCDay(dow);
					SetRTCDateInfo(DD, MO, YYYY);
					ShowMsg("All Updated!");
				}
				break;

			default:
				ShowMsg("Invalid Choice");
				break;
		}
	}
	else if(choice == 'B')
	{
		u32 trainNo, idx;
		TrainInfo_t *t;
		u8 fieldChoice;

		CMD_TO_LCD(CLEAR_LCD);
		delay_ms(2);
		CMD_TO_LCD(GOTO_LINE1_POS0);
		DISP_STRING("SELECT TRAIN NO");
		CMD_TO_LCD(GOTO_LINE2_POS0);
		DISP_STRING("T:1 T:2 T:3");

		trainNo = keyscan() - '0';
		idx = trainNo - 1;

		if(trainNo < 1 || trainNo > GetTotalTrains())
		{
			ShowMsg("Invalid Train No");
		}
		else
		{
			u8 cancelled;
			u32 platform, delayMin, arrHH, arrMM, depHH, depMM;

			t = Get_Record(idx);
			platform = t->platform;
			delayMin = t->delayMinutes;
			arrHH    = t->updatedArrivalHour;
			arrMM    = t->updatedArrivalMinute;
			depHH    = t->updatedDepartureHour;
			depMM    = t->updatedDepartureMinute;

			CMD_TO_LCD(CLEAR_LCD);
			delay_ms(2);
			CMD_TO_LCD(GOTO_LINE1_POS0);
			DISP_STRING("1:PNo 2:DlyMin");
			CMD_TO_LCD(GOTO_LINE2_POS0);
			DISP_STRING("3:ArTm 4:DpTm");

			fieldChoice = keyscan();
			ShowControlsHint();   // shown ONCE, before the field(s) for this session

			switch(fieldChoice)
			{
				case '1':
					platform = InputField("Platform", platform, 1, 20, 2, &termKey);
					if(termKey != 'C') { Update_Platform(idx, platform); ShowMsg("Platform Updt!"); }
					else                 ShowMsg("Cancelled");
					break;

				case '2':
					delayMin = InputField("Delay-min", delayMin, 0, 180, 3, &termKey);
					if(termKey != 'C') { Set_Train_Delay(idx, delayMin); ShowMsg("Delay Updated!"); }
					else                 ShowMsg("Cancelled");
					break;

				case '3':
					cancelled = 0;

					arrHH = InputField("Arr Hour", arrHH, 0, 23, 2, &termKey);
					if(termKey == 'C') cancelled = 1;

					if(!cancelled && termKey != '*')
					{
						arrMM = InputField("Arr Min", arrMM, 0, 59, 2, &termKey);
						if(termKey == 'C') cancelled = 1;
					}

					if(cancelled) ShowMsg("Arr Cancelled");
					else          { Updated_Train_Arrival(idx, arrHH, arrMM); ShowMsg("ArrTime Updt!"); }
					break;

				case '4':
					cancelled = 0;

					depHH = InputField("Dep Hour", depHH, 0, 23, 2, &termKey);
					if(termKey == 'C') cancelled = 1;

					if(!cancelled && termKey != '*')
					{
						depMM = InputField("Dep Min", depMM, 0, 59, 2, &termKey);
						if(termKey == 'C') cancelled = 1;
					}

					if(cancelled) ShowMsg("Dep Cancelled");
					else          { Updated_Train_Departure(idx, depHH, depMM); ShowMsg("DepTime Updt!"); }
					break;

				default:
					ShowMsg("Invalid Choice");
					break;
			}
		}
	}

	// ---- Back to normal mode ----
	CMD_TO_LCD(CLEAR_LCD);
	delay_ms(2);
	admin_Edit_Flag = 0;
}

// ==== Called ONCE on power-up, only if RTC_Init() reports first-time boot ====
void FirstTimeSetup(void)
{
	u32 hh, mm, ss, dow, DD, MO, YYYY;
	u8 termKey;

	CMD_TO_LCD(CLEAR_LCD); delay_ms(2);
	CMD_TO_LCD(GOTO_LINE1_POS0);
	DISP_STRING("FIRST TIME SETUP");
	delay_ms(1000);
	ShowControlsHint();   // shown ONCE for the whole first-time setup sequence

	hh   = InputField("Hours",   0,    0,    23,   2, &termKey);
	mm   = InputField("Minutes", 0,    0,    59,   2, &termKey);
	ss   = InputField("Seconds", 0,    0,    59,   2, &termKey);
	SetRTCTimeInfo(hh, mm, ss);

	dow  = InputField("Day(0=Sun)", 0, 0,    6,    1, &termKey);
	SetRTCDay(dow);

	DD   = InputField("Date",    1,    1,    31,   2, &termKey);
	MO   = InputField("Month",   1,    1,    12,   2, &termKey);
	YYYY = InputField("Year",    2026, 2000, 2026, 4, &termKey);
	SetRTCDateInfo(DD, MO, YYYY);

	ShowMsg("Setup Done!");
	CMD_TO_LCD(CLEAR_LCD);
	delay_ms(2);
}
