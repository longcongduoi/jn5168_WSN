#include "Packet.h"
#include "ErrorHandler.h"
#include "Converter.h"
#include "MathSimple.h"
//---------------------------------------------------------------------------
#define PACKET_START		0
#define PACKET_CRC			1
#define PACKET_HEADER_SIZE	2
#define PACKET_VERSION		3
#define PACKET_NUMBER		4
#define PACKET_SRC			5
#define PACKET_DST			13
#define PACKET_TIME			21
#define PACKET_BODY_SIZE	29
#define PACKET_TYPE			31
#define PACKET_STATE		32
#define PACKET_OBJ_COUNT	33
#define PACKET_START_ID		254
#define PACKET_STOP_ID		255


// Коды ошибок
#define E_DATA_TYPE_NOT_SUPPORTED 25

//---------------------------------------------------------------------------
PRIVATE uint8 ValTypeSize(void* val, ValType valType);
PRIVATE void ValToUint8(void* val, uint8* val8, ValType valType);
PRIVATE void Uint8ToVal(uint8 const * const val8, void* val, ValType valType);
PRIVATE uint8 CRC(uint8* buf, uint16 start, uint16 stop);
PRIVATE void PacketSkipToEnd(Packet *pack);
//---------------------------------------------------------------------------

PUBLIC void PacketInit(Packet* pack)
{
	pack->buf[0] = 0;
	pack->pos = PACKET_OBJ_COUNT + 1;
	pack->propCountPos = 0;
	pack->objCount = 0;
	pack->propCount = 0;
	pack->error = FALSE;
}
//---------------------------------------------------------------------------

PUBLIC void PacketMakeHeader(Packet* pack, uint64 srcAdr, uint64 dstAdr, uint8 type, uint8 state, uint64 timeVal)
{
	ValToUint8(&srcAdr, &pack->buf[PACKET_SRC], pvtUint64);
	ValToUint8(&dstAdr, &pack->buf[PACKET_DST], pvtUint64);
	ValToUint8(&type, &pack->buf[PACKET_TYPE], pvtUint8);
	ValToUint8(&state, &pack->buf[PACKET_STATE], pvtUint8);
	ValToUint8(&timeVal, &pack->buf[PACKET_TIME], pvtUint64);
}
//---------------------------------------------------------------------------

PUBLIC bool PacketAddObject(Packet* pack, uint16 objectID)
{
	pack->buf[0] = 0;
	if (pack->pos + 2 >= BUFF_MAX_SIZE - 1)
	{
		EHCall(PACKET_MOD_ID, 1);
		pack->error = TRUE;
		return FALSE;
	}

	if (pack->propCountPos != 0)
		ValToUint8(&pack->propCount, &pack->buf[pack->propCountPos], pvtUint8);
	ValToUint8(&objectID, &pack->buf[pack->pos], pvtUint16);
	pack->pos += 2;
	pack->propCountPos = pack->pos;
	pack->pos++;
	pack->objCount++;
	pack->propCount = 0;
	return TRUE;
}
//---------------------------------------------------------------------------

PUBLIC bool PacketAddProperty(Packet* pack, uint16 propID, ValType valType, uint64 val, uint8 precision, char* strVal)
{
	pack->buf[0] = 0;
	uint8 valTypeSize = (valType == pvtString) ? ValTypeSize(strVal, valType) : ValTypeSize(&val, valType);

	if ((pack->pos + 2 + valTypeSize) >= BUFF_MAX_SIZE - 1)
	{
		EHCall(PACKET_MOD_ID, 1);
		pack->error = TRUE;
		return FALSE;
	}
	if (propID > 0xFFF)
	{
		EHCall(PACKET_MOD_ID, 4);
		pack->error = TRUE;
		return FALSE;
	}

	propID = (propID << 4) | valType;
	ValToUint8(&propID, &pack->buf[pack->pos], pvtUint16);
	pack->pos += 2;

	uint8 val8;
	uint16 val16;
	uint32 val32;
	uint64 val64;
	int32 val32signed;
	float valFloat;
	switch (valType)
	{
		case pvtBool:
		case pvtUint8:
		case pvtInt8:
			val8 = val;
			ValToUint8(&val8, &pack->buf[pack->pos], valType);
			break;
		case pvtUint16:
		case pvtInt16:
			val16 = val;
			ValToUint8(&val16, &pack->buf[pack->pos], valType);
			break;
		case pvtUint32:
		case pvtInt32:
			val32 = val;
			ValToUint8(&val32, &pack->buf[pack->pos], valType);
			break;
		case pvtUint64:
		case pvtInt64:
			val64 = val;
			ValToUint8(&val64, &pack->buf[pack->pos], valType);
			break;
		case pvtFloat:
			val32signed = (int32) val;
			valFloat = val32signed * MathSimplePowInt8(0.1, precision);
			ValToUint8(&valFloat, &pack->buf[pack->pos], valType);
			break;
		case pvtString:
			ValToUint8(strVal, &pack->buf[pack->pos], valType);
			break;

		default:
			EHCall(PACKET_MOD_ID, E_DATA_TYPE_NOT_SUPPORTED);
			pack->error = TRUE;
			return FALSE;
	}

	pack->pos += valTypeSize;
	pack->propCount++;
	return TRUE;
}
//---------------------------------------------------------------------------

PUBLIC bool PacketFinalize(Packet* pack)
{
	uint8 val8;
	uint16 val16, packet_stop;

	if (pack->error)
		return FALSE;

	if (pack->buf[0] != 0)
		PacketSetStateToAppendObject(pack);

	val8 = PACKET_START_ID;
	ValToUint8(&val8, &pack->buf[PACKET_START], pvtUint8);
	val8 = PACKET_BODY_SIZE - PACKET_VERSION;
	ValToUint8(&val8, &pack->buf[PACKET_HEADER_SIZE], pvtUint8);
	val8 = PACKET_VERSION_NUM;
	ValToUint8(&val8, &pack->buf[PACKET_VERSION], pvtUint8);
	packet_stop = pack->pos;
	val16 = packet_stop - PACKET_TYPE;
	ValToUint8(&val16, &pack->buf[PACKET_BODY_SIZE], pvtUint16);
	ValToUint8(&pack->objCount, &pack->buf[PACKET_OBJ_COUNT], pvtUint8);
	if (pack->propCountPos != 0)
		ValToUint8(&pack->propCount, &pack->buf[pack->propCountPos], pvtUint8);
	val8 = PACKET_STOP_ID;
	ValToUint8(&val8, &pack->buf[packet_stop], pvtUint8);
	val8 = 0;
	ValToUint8(&val8, &pack->buf[PACKET_NUMBER], pvtUint8);
	val8 = CRC(pack->buf, PACKET_START + 2, packet_stop - 1);
	ValToUint8(&val8, &pack->buf[PACKET_CRC], pvtUint8);
	return TRUE;
}
//---------------------------------------------------------------------------

PUBLIC bool PacketPushToBuf(Packet const * const pack, Buffer* buf)
{
	uint16 i;
	uint16 packSize;

	packSize = PacketSize(pack);
	if (BufAvailSize(buf) < packSize)
	{
		EHCall(PACKET_MOD_ID, 2);
		return FALSE;
	}

	for (i = 0; i < packSize; i++)
		BufPush(buf, pack->buf[i]);

	return TRUE;
}
//---------------------------------------------------------------------------

PUBLIC void PacketSetStateToAppendObject(Packet* pack)
{
	pack->pos = PacketSize(pack) - 1;
}
//---------------------------------------------------------------------------

PUBLIC void PacketSetStateToRead(Packet* pack)
{
	pack->pos = PACKET_OBJ_COUNT + 1;
	pack->propCountPos = 0;
	Uint8ToVal(&pack->buf[PACKET_OBJ_COUNT], &pack->objCount, pvtUint8);
	pack->propCount = 0;
}
//---------------------------------------------------------------------------

PUBLIC bool PacketPopFromBuf(Packet* pack, Buffer* buf)
{
	uint8 val8;
	uint16 bufSize = BufSize(buf);
	uint16 packSize;
	uint16 i, j;
	bool packFound = FALSE;

	if (bufSize == 0)
		return FALSE;

	pack->error = FALSE;
	for (i = 0; i < bufSize; i++)
	{
		val8 = BufGet(buf, i);
		if (val8 == PACKET_START_ID)
		{
			// Выделение пакета
			if (i + PACKET_OBJ_COUNT + 2 > bufSize)
				return FALSE;
			for (j = 0; j < PACKET_OBJ_COUNT + 2; j++)
			{
				val8 = BufGet(buf, i + j);
				pack->buf[j] = val8;
			}
			packSize = PacketSize(pack);
			if (packSize > bufSize - i)
				continue;

			val8 = BufGet(buf, i + packSize - 1);
			if (val8 != PACKET_STOP_ID)
				continue;

			if (i != 0)
				EHCall(PACKET_MOD_ID, 7);
			BufErase(buf, i);
			i = -1;
			val8 = BufGet(buf, PACKET_VERSION);
			/*
			 if ((val8 & 0xE0) != (PACKET_VERSION_NUM & 0xE0)){
			 EHCall (PACKET_MOD_ID, 5);
			 BufErase (buf, packSize);
			 bufSize = BufSize (*buf);
			 continue;
			 }
			 */
			/*
			 if (packSize > BUFF_BUF_SIZE){
			 EHCall (PACKET_MOD_ID, 3);
			 BufErase (buf, packSize);
			 bufSize = BufSize (*buf);
			 continue;
			 }
			 */
			for (j = 0; j < packSize; j++)
			{
				BufPop(buf, &val8);
				pack->buf[j] = val8;
			}
			Uint8ToVal(&pack->buf[PACKET_CRC], &val8, pvtUint8);
			if (val8 != CRC(pack->buf, PACKET_START + 2, packSize - 2))
			{
				EHCall(PACKET_MOD_ID, 6);
				BufErase(buf, packSize);
				bufSize = BufSize(buf);
				continue;
			}
			packFound = TRUE;
			break;
		}
	}
	if (!packFound)
		return FALSE;

	PacketSetStateToRead(pack);
	pack->error = FALSE;
	return TRUE;
}
//---------------------------------------------------------------------------

PUBLIC void PacketGetHeader(Packet const * const pack, uint64* srcAdr, uint64* dstAdr, uint8* type, uint8* state, uint64* timeVal)
{
	Uint8ToVal(&pack->buf[PACKET_SRC], (void*) srcAdr, pvtUint64);
	Uint8ToVal(&pack->buf[PACKET_DST], (void*) dstAdr, pvtUint64);
	Uint8ToVal(&pack->buf[PACKET_TYPE], (void*) type, pvtUint8);
	Uint8ToVal(&pack->buf[PACKET_STATE], (void*) state, pvtUint8);
	Uint8ToVal(&pack->buf[PACKET_TIME], (void*) timeVal, pvtUint64);
}
//---------------------------------------------------------------------------

PUBLIC bool PacketGetObject(Packet * pack, uint16* objectID)
{
	if (pack->objCount == 0)
		return FALSE;

	Uint8ToVal(&pack->buf[pack->pos], objectID, pvtUint16);

	pack->pos += 2;
	pack->propCountPos = pack->pos;
	Uint8ToVal(&pack->buf[pack->propCountPos], &pack->propCount, pvtUint8);
	pack->pos++;
	pack->objCount--;

	return TRUE;
}
//---------------------------------------------------------------------------

PUBLIC bool PacketGetProperty(Packet * pack, uint16* propID, ValType* valType, uint64* val, uint8 precision, char* strVal)
{
	if (pack->propCount == 0)
		return FALSE;

	Uint8ToVal(&pack->buf[pack->pos], propID, pvtUint16);
	(*valType) = (ValType) ((*propID) & 0xF);
	(*propID) = (*propID) >> 4;
	pack->pos += 2;
	pack->propCount--;

	uint8 valTypeSize = ValTypeSize(&pack->buf[pack->pos], *valType);
	uint8 val8;
	uint16 val16;
	uint32 val32;
	uint64 val64;
	float valFloat;
	(*val) = 0;
	switch (*valType)
	{
		case pvtBool:
		case pvtUint8:
		case pvtInt8:
			Uint8ToVal(&pack->buf[pack->pos], &val8, (*valType));
			(*val) = val8;
			break;
		case pvtUint16:
		case pvtInt16:
			Uint8ToVal(&pack->buf[pack->pos], &val16, (*valType));
			(*val) = val16;
			break;
		case pvtUint32:
		case pvtInt32:
			Uint8ToVal(&pack->buf[pack->pos], &val32, (*valType));
			(*val) = val32;
			break;
		case pvtUint64:
		case pvtInt64:
			Uint8ToVal(&pack->buf[pack->pos], &val64, (*valType));
			(*val) = val64;
			break;
		case pvtFloat:
			Uint8ToVal(&pack->buf[pack->pos], &valFloat, (*valType));
			(*val) = (uint32) (valFloat * MathSimplePowInt8(10, precision));
			break;
		case pvtString:
			Uint8ToVal(&pack->buf[pack->pos], strVal, (*valType));
			break;

		default:
			EHCall(PACKET_MOD_ID, E_DATA_TYPE_NOT_SUPPORTED);
			break;
	}

	pack->pos += valTypeSize;
	return TRUE;
}
//---------------------------------------------------------------------------

PUBLIC uint16 PacketSize(Packet const * const pack)
{
	uint8 headerSize;
	uint16 bodySize;
	Uint8ToVal(&pack->buf[PACKET_HEADER_SIZE], &headerSize, pvtUint8);
	Uint8ToVal(&pack->buf[PACKET_BODY_SIZE], &bodySize, pvtUint16);
	return ((uint16) headerSize) + bodySize + 6;
}
//---------------------------------------------------------------------------

PUBLIC void PacketSaveContext(Packet const * const pack, PacketContext *packContext)
{
	packContext->pos = pack->pos;
	packContext->propCountPos = pack->propCountPos;
	packContext->objCount = pack->objCount;
	packContext->propCount = pack->propCount;
	packContext->error = pack->error;
}
//---------------------------------------------------------------------------

PUBLIC void PacketRestoreContext(Packet* pack, PacketContext packContext)
{
	pack->pos = packContext.pos;
	pack->propCountPos = packContext.propCountPos;
	pack->objCount = packContext.objCount;
	pack->propCount = packContext.propCount;
	pack->error = packContext.error;
}
//---------------------------------------------------------------------------

PUBLIC void PacketSwapAdr(Packet* pack, uint8 state)
{
	uint64 srcAdr, dstAdr;
	uint8 type, prevState;
	uint64 timeVal;

	PacketGetHeader(pack, &srcAdr, &dstAdr, &type, &prevState, &timeVal);
	PacketMakeHeader(pack, dstAdr, srcAdr, type, state, 0);
	PacketSkipToEnd(pack);
}
//---------------------------------------------------------------------------

PRIVATE uint8 ValTypeSize(void* val, ValType valType)
{
	switch (valType)
	{
		case pvtBool:
		case pvtInt8:
		case pvtUint8:
			return 1;
		case pvtInt16:
		case pvtUint16:
			return 2;
		case pvtInt32:
		case pvtUint32:
			return 4;
		case pvtInt64:
		case pvtUint64:
			return 8;
		case pvtFloat:
			return 4;
		case pvtString:
			return ((uint8*) val)[0] + 1;

		default:
			EHCall(PACKET_MOD_ID, E_DATA_TYPE_NOT_SUPPORTED);
			return 0;
	}
}
//---------------------------------------------------------------------------

PRIVATE void ValToUint8(void* val, uint8* val8, ValType valType)
{
	switch (valType)
	{
		case pvtBool:
		case pvtInt8:
		case pvtUint8:
			val8[0] = (*((uint8*) val));
			break;
		case pvtInt16:
		case pvtUint16:
			ConverterUint16ToUint8((*((uint16*) val)), val8);
			break;
		case pvtInt32:
		case pvtUint32:
			ConverterUint32ToUint8((*((uint32*) val)), val8);
			break;
		case pvtInt64:
		case pvtUint64:
			ConverterUint64ToUint8((*((uint64*) val)), val8);
			break;
		case pvtFloat:
			ConverterFloatToUint8((*((float*) val)), val8);
			break;
		case pvtString:
			ConverterStringToUint8((char*) val, val8);
			break;

		 default:
			EHCall(PACKET_MOD_ID, E_DATA_TYPE_NOT_SUPPORTED);
			break;
	}
}
//---------------------------------------------------------------------------

PRIVATE void Uint8ToVal(uint8 const* const val8, void* val, ValType valType)
{

	switch (valType)
	{
		case pvtBool:
		case pvtInt8:
		case pvtUint8:
			(*((uint8*) val)) = val8[0];
			break;
		case pvtInt16:
		case pvtUint16:
			ConverterUint8ToUint16(val8, (uint16*) val);
			break;
		case pvtInt32:
		case pvtUint32:
			ConverterUint8ToUint32(val8, (uint32*) val);
			break;
		case pvtInt64:
		case pvtUint64:
			ConverterUint8ToUint64(val8, (uint64*) val);
			break;
		case pvtFloat:
			ConverterUint8ToFloat(val8, (float*) val);
			break;
		case pvtString:
			ConverterUint8ToString(val8, (char*) val);
			break;

		default:
			EHCall(PACKET_MOD_ID, E_DATA_TYPE_NOT_SUPPORTED);
			break;
	}
}
//---------------------------------------------------------------------------

PRIVATE uint8 CRC(uint8* buf, uint16 start, uint16 stop)
{
	uint16 i;
	uint8 crc = 0;

	for (i = start; i <= stop; i++)
		crc += buf[i];
	return crc;
}
//---------------------------------------------------------------------------

PRIVATE void PacketSkipToEnd(Packet *pack)
{
	uint16 objectID, propID;
	ValType valType;
	uint64 val;
	char strVal[32];

	while (PacketGetObject(pack, &objectID))
		while (PacketGetProperty(pack, &propID, &valType, &val, 0, strVal))
			;
}
//---------------------------------------------------------------------------
