//train_db.c
#include "types.h"
#include "train_db.h"
TrainInfo_t TrainDB[TOTAL_TRAINS] =
{
    {
        12627,
        "Karnataka Express",
        "New Delhi",
        6,30,
        6,35,
        6,30,
        6,35,
        1,
        0
    },
    {
        12028,
        "Shatabdi Express",
        "Chennai",
        7,15,
        7,20,
        7,15,
        7,20,
        2,
        0
    },
    {
        12785,
        "Kacheguda Express",
        "Hyderabad",
        8,00,
        8,05,
        8,20,
        8,25,
        3,
        20
    }
};
TrainInfo_t* Get_Record(u32 index)
{
    if(index < TOTAL_TRAINS)
        return &TrainDB[index];
    else
        return 0;
}
void Updated_Train_Arrival(u32 index, u32 hour, u32 minute)
{
    if(index < TOTAL_TRAINS)
    {
        TrainDB[index].updatedArrivalHour   = hour;
        TrainDB[index].updatedArrivalMinute = minute;
    }
}
void  Updated_Train_Departure(u32 index, u32 hour, u32 minute)
{
    if(index < TOTAL_TRAINS)
    {
        TrainDB[index].updatedDepartureHour   = hour;
        TrainDB[index].updatedDepartureMinute = minute;
    }
}
void  Update_Platform(u32 index, u32 platform)
{
    if(index < TOTAL_TRAINS)
        TrainDB[index].platform = platform;
}
void Set_Train_Delay(u32 index, u32 delayMin)
{
    if(index < TOTAL_TRAINS)
        TrainDB[index].delayMinutes = delayMin;
}
u32 GetTotalTrains(void)
{
    return TOTAL_TRAINS;
}
