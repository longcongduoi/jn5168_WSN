#ifndef BUFFER_H
#define BUFFER_H
//---------------------------------------------------------------------------

#include <jendefs.h>
#include "Config.h"
//---------------------------------------------------------------------------

//#define BUFF_MAX_SIZE ((CONFIG_OBJECT_MAXCOUNT*CONFIG_OBJECT_SIZE) + CONFIG_DEVDESC_SIZE)

#define BUFF_MAX_SIZE 2048

//---------------------------------------------------------------------------

PUBLIC typedef struct {
	uint8 buf [BUFF_MAX_SIZE];
	uint16 start;
	uint16 stop;
	uint16 maxSize;
	uint16 size;
	uint16 availSize;
} Buffer;

//---------------------------------------------------------------------------

PUBLIC void BufInit (Buffer* buf);
PUBLIC void BufErase (Buffer* buf, uint16 size);
PUBLIC bool BufPush (Buffer* buf, uint8 val);
PUBLIC bool BufPushFromArray (Buffer* buf, uint8* array, uint16 arraySize);
PUBLIC bool BufPop (Buffer* buf, uint8* val);
PUBLIC bool BufTryGet (Buffer const * const  buf, uint16 pos, uint8* val);
PUBLIC uint8 BufGet ( Buffer const * const buf, uint16 pos);
PUBLIC bool BufGetToArray (Buffer const * const  buf, uint8* array, uint16 arraySize, uint16* getSize);
PUBLIC bool BufPopToArray (Buffer* buf, uint8* array, uint16 arraySize, uint16* getSize);
PUBLIC uint16 BufMaxSize (Buffer const * const  buf);
PUBLIC uint16 BufSize (Buffer const * const  buf);
PUBLIC uint16 BufAvailSize (Buffer const * const  buf);

#endif
