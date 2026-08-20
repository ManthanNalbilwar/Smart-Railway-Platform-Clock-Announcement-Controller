//scheduler.c
#include "types.h"
#include "scheduler.h"
#include "Rtc.h"
#include "train_db.h"

// Line1 buffer: holds train NAME (line1 shows fixed train number then scrolls this)
u8 Line1_Scrolling_Massage[30];

static TrainState_t trainState[TOTAL_TRAINS];
static u8            arrivalEventPending[TOTAL_TRAINS];
static u8            departureEventPending[TOTAL_TRAINS];
static s32            activeTrainIndex = -1;

u32 ToMinutes(u32 hour, u32 minute)
{
	u32 TotalMin;
	TotalMin = hour*60 + minute;
	return TotalMin;
}

// Works out what a train would be doing RIGHT NOW, purely from RTC vs its
// (possibly delayed / admin-updated) arrival & departure time. No side effects.
static u32 ComputeCategory(u32 index)
{
	TrainInfo_t *t;
	s32 curHH, curMM, curSS;
	u32 curTotalMin, arrTotalMin, depTotalMin;
	s32 diffArr, diffDep;

	t = Get_Record(index);
	if(t == 0)
		return TRAIN_STATE_IDLE;

	GetRTCTimeInfo(&curHH, &curMM, &curSS);
	curTotalMin = ToMinutes((u32)curHH, (u32)curMM);
	arrTotalMin = ToMinutes(t->updatedArrivalHour,   t->updatedArrivalMinute);
	depTotalMin = ToMinutes(t->updatedDepartureHour, t->updatedDepartureMinute);

	diffArr = (s32)arrTotalMin - (s32)curTotalMin;
	diffDep = (s32)depTotalMin - (s32)curTotalMin;

	if(diffArr > 0 && diffArr <= APPROACHING_THRESHOLD)
		return TRAIN_STATE_APPROACHING;

	if(diffArr <= 0 && diffDep > 0)
		return TRAIN_STATE_AT_PLATFORM;

	return TRAIN_STATE_IDLE;
}

s32 UpdateTrainStates(void)
{
	u32 i;
	u32 cat;

	if(activeTrainIndex == -1)
	{
		// Nobody being handled right now - look for the first (earliest)
		// train that needs attention. Others just wait their turn even if
		// they are also within their approaching window.
		for(i=0; i<GetTotalTrains(); i++)
		{
			cat = ComputeCategory(i);
			if(cat != TRAIN_STATE_IDLE)
			{
				activeTrainIndex        = (s32)i;
				trainState[i]           = (TrainState_t)cat;
				arrivalEventPending[i]  = (cat == TRAIN_STATE_AT_PLATFORM) ? 1 : 0;
				break;
			}
		}
	}
	else
	{
		i   = (u32)activeTrainIndex;
		cat = ComputeCategory(i);   // raw RTC-vs-schedule category: IDLE/APPROACHING/AT_PLATFORM

		if(cat == TRAIN_STATE_IDLE)
		{
			if(trainState[i] == TRAIN_STATE_AT_PLATFORM)
			{
				// Just crossed past departure time. Don't free the slot yet -
				// park one tick in DEPARTING so the app layer gets a chance
				// to fire the departure buzzer/alert exactly once.
				trainState[i]            = TRAIN_STATE_DEPARTING;
				departureEventPending[i] = 1;
			}
			else if(trainState[i] == TRAIN_STATE_APPROACHING)
			{
				// Edge case: the clock moved straight past BOTH arrival and
				// departure without us ever seeing AT_PLATFORM in between -
				// e.g. admin jumped the RTC forward across the whole window,
				// or a train's dwell time is shorter than one loop tick.
				// Don't silently drop both alerts: treat it as "arrived"
				// right now (fires the arrival beep + shows the train once),
				// and the very next tick will naturally fall into the
				// AT_PLATFORM->DEPARTING branch above and fire the
				// departure beep too - so neither alert gets lost.
				trainState[i]          = TRAIN_STATE_AT_PLATFORM;
				arrivalEventPending[i] = 1;
			}
			else
			{
				// Already DEPARTING (departure alert already fired) - free
				// the slot for the next train.
				trainState[i]            = TRAIN_STATE_IDLE;
				arrivalEventPending[i]   = 0;
				departureEventPending[i] = 0;
				activeTrainIndex         = -1;
			}
		}
		else
		{
			if(trainState[i] == TRAIN_STATE_APPROACHING && cat == TRAIN_STATE_AT_PLATFORM)
				arrivalEventPending[i] = 1;   // just crossed into the platform window

			trainState[i] = (TrainState_t)cat;
		}
	}

	// Keep the Line1 scrolling name buffer in sync with whichever train is active.
	if(activeTrainIndex != -1)
	{
		TrainInfo_t *t = Get_Record((u32)activeTrainIndex);
		u32 k = 0;

		if(t != 0)
		{
			while(t->trainName[k] != '\0' && k < 28)
			{
				Line1_Scrolling_Massage[k] = t->trainName[k];
				k++;
			}
		}
		Line1_Scrolling_Massage[k] = '\0';
	}

	return activeTrainIndex;
}

TrainState_t GetTrainState(u32 index)
{
	if(index < TOTAL_TRAINS)
		return trainState[index];
	return TRAIN_STATE_IDLE;
}

u32 ConsumeArrivalEvent(u32 index)
{
	if(index < TOTAL_TRAINS && arrivalEventPending[index])
	{
		arrivalEventPending[index] = 0;
		return 1;
	}
	return 0;
}

u32 ConsumeDepartureEvent(u32 index)
{
	if(index < TOTAL_TRAINS && departureEventPending[index])
	{
		departureEventPending[index] = 0;
		return 1;
	}
	return 0;
}
