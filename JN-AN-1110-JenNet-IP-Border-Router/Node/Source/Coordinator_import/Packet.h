#ifndef PACKET_H
#define PACKET_H
//---------------------------------------------------------------------------

#include <jendefs.h>
#include "Buffer.h"
//---------------------------------------------------------------------------

#define PACKET_VERSION_NUM 1
//---------------------------------------------------------------------------

PUBLIC typedef enum {
	pvtInt8 = 1,
	pvtUint8,
	pvtInt16,
	pvtUint16,
	pvtInt32,
	pvtUint32,
	pvtInt64,
	pvtUint64,
	pvtString,
	pvtFloat,
	pvtBool,
	pvtDouble,
	pvtArray
} ValType;

PUBLIC typedef struct{
	uint8 buf [BUFF_MAX_SIZE];
	uint16 pos;
	uint16 propCountPos;
	uint8 objCount;
	uint8 propCount;
	bool error;
} Packet;

PUBLIC typedef struct{
	uint16 pos;
	uint16 propCountPos;
	uint8 objCount;
	uint8 propCount;
	bool error;
} PacketContext;
//---------------------------------------------------------------------------


PUBLIC void PacketInit (Packet* pack);
PUBLIC void PacketMakeHeader (Packet* pack, uint64 srcAdr, uint64 dstAdr, uint8 type, uint8 state, uint64 timeVal);
PUBLIC bool PacketAddObject (Packet* pack, uint16 objectID);
PUBLIC bool PacketAddProperty (Packet* pack, uint16 propID, ValType valType, uint64 val, uint8 precision, char* strVal);
PUBLIC bool PacketFinalize (Packet* pack);
PUBLIC bool PacketPushToBuf (Packet const * const pack, Buffer* buf);
PUBLIC void PacketSetStateToAppendObject(Packet* pack);
PUBLIC void PacketSetStateToRead(Packet* pack);

PUBLIC bool PacketPopFromBuf (Packet* pack, Buffer* buf);
PUBLIC void PacketGetHeader (Packet const * const pack, uint64* srcAdr, uint64* dstAdr, uint8* type, uint8* state, uint64* timeVal);
PUBLIC bool PacketGetObject (Packet * pack, uint16* objectID);
PUBLIC bool PacketGetProperty (Packet * pack, uint16* propID, ValType* valType, uint64* val, uint8 precision, char* strVal);

PUBLIC uint16 PacketSize (Packet const * const pack);
PUBLIC void PacketSaveContext (Packet const * const pack, PacketContext *packContext);
PUBLIC void PacketRestoreContext (Packet* pack, PacketContext packContext);
PUBLIC void PacketSwapAdr(Packet* pack, uint8 state);

#endif

/*
Add data to exist packet
ObjectConnectionInit();
ObjectLQIInit();
PacketSetStateToAppendObject(&pack);
ObjectToPack (ObjectConnection, &pack, FALSE);
ObjectToPack (ObjectLQI, &pack, FALSE);
PacketFinalize(&pack);
PacketSetStateToRead(&pack);
*/
