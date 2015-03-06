
#include "Vector.h"
#include "ErrorHandler.h"

// Коды ошибок
#define E_VECTOR_MODULE_BUFFER_FULL	1
#define E_VECTOR_MODULE_POSSITION_OUT_OF_RANGE	2


void VectorClear (Vector* const v)
{
	v->maxSize = VECTOR_MAX_SIZE;
	v->size = 0;
}

uint16 VectorAvailSize (Vector const * const  v)
{
	return v->maxSize - v->size;
}

void VectorPushBack(Vector * const  v, uint8 value)
{
	if(VectorAvailSize(v) <= 0)
	{
		EHCall(VECTOR_MOD_ID, E_VECTOR_MODULE_BUFFER_FULL);
		return;
	}

	v->buf[v->size] = value;
	++v->size;
}

uint8 VectorGet(Vector const * const v, uint8 pos)
{
	if(pos >= v->size)
	{
		EHCall(VECTOR_MOD_ID, E_VECTOR_MODULE_POSSITION_OUT_OF_RANGE);
		return 0;
	}

	return v->buf[pos];
}




