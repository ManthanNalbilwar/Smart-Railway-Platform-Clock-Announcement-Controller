#ifndef TRAIN_DB_H
#define TRAIN_DB_H
#include "types.h"
#define TOTAL_TRAINS 3
typedef struct
{
    u32  trainNumber;
    u8   trainName[25];
    u8   destination[20];
    u8   arrivalHour;
    u8   arrivalMinute;
    u8   departureHour;
    u8   departureMinute;
    u8   updatedArrivalHour;
    u8   updatedArrivalMinute;
    u8   updatedDepartureHour;
    u8   updatedDepartureMinute;
    u8   platform;
    u8   delayMinutes;
} TrainInfo_t;
extern TrainInfo_t TrainDB[TOTAL_TRAINS];
TrainInfo_t* Get_Record(u32 index);
void Updated_Train_Arrival(u32 index, u32 hour, u32 minute);
void Updated_Train_Departure(u32 index, u32 hour, u32 minute);
void Update_Platform(u32 index, u32 platform);
void Set_Train_Delay(u32 index, u32 delayMin);
u32  GetTotalTrains(void);
#endif
