#ifndef DRIVERI2C_H
#define DRIVERI2C_H
//---------------------------------------------------------------------------

#include <jendefs.h>
//---------------------------------------------------------------------------

#define DRIVERI2C_MOD_ID 12

PUBLIC typedef struct{
    uint32 sdaDIO;
    uint32 sclDIO;
    uint8 address;
    char start[8];
    char stop[8];
} DriverI2CInfo;
//---------------------------------------------------------------------------

PUBLIC void DriverI2CInit(DriverI2CInfo* info, uint32 sdaDIO, uint32 sclDIO, uint8 address, char* start, char* stop);
PUBLIC bool DriverI2CStart(DriverI2CInfo* info, bool write);
PUBLIC void DriverI2CStop(DriverI2CInfo* info);
PUBLIC bool DriverI2CSend(DriverI2CInfo* info, uint8 data);
PUBLIC uint8 DriverI2CRead(DriverI2CInfo* info);
PUBLIC uint8 DriverI2CReadSeq(DriverI2CInfo* info, bool ack);// добавлено

#endif
