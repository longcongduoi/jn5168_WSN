#include <AppHardwareApi_JN516x.h>

#include "DriverI2C.h"
#include "ErrorHandler.h"
#include "StringSimple.h"
#include "Config.h"

//---------------------------------------------------------------------------

PRIVATE void DriverI2CSendSequence(DriverI2CInfo* info, char* sdascl);
PRIVATE void DriverI2ClearClock(DriverI2CInfo* info);
PRIVATE void DriverI2SetClock(DriverI2CInfo* info);
PRIVATE void DriverI2ClearData(DriverI2CInfo* info);
PRIVATE void DriverI2SetData(DriverI2CInfo* info);
PRIVATE void DriverI2ClearDataDirection(DriverI2CInfo* info);
PRIVATE void DriverI2SetDataDirection(DriverI2CInfo* info);
//---------------------------------------------------------------------------

PUBLIC void DriverI2CInit(DriverI2CInfo* info, uint32 sdaDIO, uint32 sclDIO, uint8 address, char* start, char* stop)
{
	info->sdaDIO = sdaDIO;
	info->sclDIO = sclDIO;
	info->address = address;
	sstrcpy(info->start, start);
	sstrcpy(info->stop, stop);

	DriverI2ClearClock(info);
	DriverI2ClearData(info);
	DriverI2SetDataDirection(info);
	vAHI_DioSetDirection(0, info->sclDIO);
}
//---------------------------------------------------------------------------

PUBLIC bool DriverI2CStart(DriverI2CInfo* info, bool write)
{
	DriverI2CSendSequence(info, info->start);
	return DriverI2CSend(info, (info->address << 1) | ((write) ? 0 : 1));
}
//---------------------------------------------------------------------------

PUBLIC void DriverI2CStop(DriverI2CInfo* info)
{
	DriverI2CSendSequence(info, info->stop);
	vAHI_DioSetOutput(info->sdaDIO | info->sclDIO, 0);
}
//---------------------------------------------------------------------------

PUBLIC bool DriverI2CSend(DriverI2CInfo* info, uint8 data)
{
	uint8 i;
	// send byte MSB first
	for (i = 0; i < 8; i++)
	{
		if (data & (1 << (7 - i)))
			DriverI2SetData(info);
		else
			DriverI2ClearData(info);
		DriverI2SetClock(info);
		DriverI2ClearClock(info);
	}

	// get ack
	DriverI2ClearDataDirection(info);
	DriverI2SetClock(info);
	if (u32AHI_DioReadInput() & info->sdaDIO)
	{
		DriverI2ClearClock(info);
		DriverI2SetDataDirection(info);
		vAHI_DioSetOutput(info->sdaDIO | info->sclDIO, 0);
		EHCall(DRIVERI2C_MOD_ID, 1);
		return FALSE;
	}
	DriverI2ClearClock(info);
	DriverI2SetDataDirection(info);
	return TRUE;
}
//---------------------------------------------------------------------------

PUBLIC uint8 DriverI2CRead(DriverI2CInfo* info)
{
	uint8 i;
	uint8 data = 0;

	// read byte MSB first
	DriverI2ClearDataDirection(info);
	for (i = 128; i > 0; i >>= 1)
	{
		DriverI2SetClock(info);
		if (u32AHI_DioReadInput() & info->sdaDIO)
			data |= i;
		DriverI2ClearClock(info);
	}

	// send nack
	DriverI2SetDataDirection(info);
//#if  (HARDTYPE == 0001 || HARDTYPE == 1400 || HARDTYPE == 1401 || HARDTYPE == 1410 ||
//    HARDTYPE == 150120 || HARDTYPE == 151120 || HARDTYPE == 150140 || HARDTYPE == 151140 ||
//    HARDTYPE == 15012015 || HARDTYPE == 15112015 || HARDTYPE == 15012018 || HARDTYPE == 15112018 || HARDTYPE == 1501221 || HARDTYPE == 1511221)

	DriverI2CSendSequence(info, "1110");
//#else
//    DriverI2CSendSequence(info, "0100");
//#endif
	return data;
}
//--------для ТСП ---------

PUBLIC uint8 DriverI2CReadSeq(DriverI2CInfo* info, bool ack)
{
	uint8 i;
	uint8 data = 0;

	// read byte MSB first
	DriverI2ClearDataDirection(info);
	for (i = 128; i > 0; i >>= 1)
	{
		DriverI2SetClock(info);
		if (u32AHI_DioReadInput() & info->sdaDIO)
			data |= i;
		DriverI2ClearClock(info);
	}

	//send nack
	DriverI2SetDataDirection(info);
	if (ack)
		DriverI2CSendSequence(info, "0100"); //смотреть AG-1511(AG_PTv1.0)_OLED
	else
		DriverI2CSendSequence(info, "1110");

	return data;
}
//---------------------------------------------------------------------------

PRIVATE void DriverI2CSendSequence(DriverI2CInfo* info, char* sdascl)
{
	uint8 i = 0;

	while (sdascl[i] != '\0')
	{
		if (sdascl[i] == '0')
		{
			if (i % 2 == 0)
				DriverI2ClearData(info);
			else
				DriverI2ClearClock(info);
		}
		else
		{
			if (i % 2 == 0)
				DriverI2SetData(info);
			else
				DriverI2SetClock(info);
		}
		i++;
	}
}
//---------------------------------------------------------------------------

PRIVATE void DriverI2ClearClock(DriverI2CInfo* info)
{
	vAHI_DioSetOutput(0, info->sclDIO);
}
//---------------------------------------------------------------------------

PRIVATE void DriverI2SetClock(DriverI2CInfo* info)
{
	vAHI_DioSetOutput(info->sclDIO, 0);
}
//---------------------------------------------------------------------------

PRIVATE void DriverI2ClearData(DriverI2CInfo* info)
{
	vAHI_DioSetOutput(0, info->sdaDIO);
}
//---------------------------------------------------------------------------

PRIVATE void DriverI2SetData(DriverI2CInfo* info)
{
	vAHI_DioSetOutput(info->sdaDIO, 0);
}
//---------------------------------------------------------------------------

PRIVATE void DriverI2ClearDataDirection(DriverI2CInfo* info)
{
	vAHI_DioSetDirection(info->sdaDIO, 0);
}
//---------------------------------------------------------------------------

PRIVATE void DriverI2SetDataDirection(DriverI2CInfo* info)
{
	vAHI_DioSetDirection(0, info->sdaDIO);
}
//---------------------------------------------------------------------------
