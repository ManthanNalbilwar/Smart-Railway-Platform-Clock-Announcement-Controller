//scheduler.h
#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "types.h"

// ---- Runtime state of the ONE train currently being handled ----
// Only one train is ever "active" (approaching / at platform) at a time.
// If two trains are due at the same time, the one picked first (lowest
// index / earliest in DB order) is finished before the next is allowed
// to start - no preemption.
typedef enum
{
	TRAIN_STATE_IDLE = 0,
	TRAIN_STATE_APPROACHING,   // counting down to arrival, still before platform
	TRAIN_STATE_AT_PLATFORM,   // arrived, waiting for departure
	TRAIN_STATE_DEPARTING      // one-tick transient: just left, departure alert fires here
} TrainState_t;

#define APPROACHING_THRESHOLD  2   // minutes before arrival to start announcing

extern u8 Line1_Scrolling_Massage[];   // train name - line1 scrolls this

u32          ToMinutes(u32 hour, u32 minute);

// Advances the state machine by one tick (uses current RTC time).
// Returns the index of the currently active train, or -1 if none.
s32          UpdateTrainStates(void);

// Current state of a given train index (only meaningful for the active one).
TrainState_t GetTrainState(u32 index);

// Edge-triggered: returns 1 exactly once, the instant a train transitions
// from APPROACHING to AT_PLATFORM (i.e. "just arrived"). 0 otherwise.
u32          ConsumeArrivalEvent(u32 index);

// Edge-triggered: returns 1 exactly once, the instant a train transitions
// from AT_PLATFORM to DEPARTING (i.e. "just left"). 0 otherwise.
u32          ConsumeDepartureEvent(u32 index);

#endif
