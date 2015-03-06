#ifndef RADIO_H
#define RADIO_H
//---------------------------------------------------------------------------

#include <jendefs.h>
#include "Packet.h"
#include "Buffer.h"

#define RADIO_MAXPACK_SIZE 60

typedef struct{
Buffer TxBuf;
Buffer RxBuf;
uint64 devID;
bool dataSent;
} RadioDevice;


PUBLIC void RadioInit (void);
PUBLIC bool RadioSend (Packet pack, uint64 dstAdr, bool obligatory);
//PUBLIC void RadioDataFromDevice(tsData* data);
PUBLIC bool RadioRecievPack(Packet* pack);
PUBLIC void RadioSent(bool succes);
PUBLIC void RadioTick (uint16 count);
PUBLIC bool RadioAllDataSent (void);

#endif
