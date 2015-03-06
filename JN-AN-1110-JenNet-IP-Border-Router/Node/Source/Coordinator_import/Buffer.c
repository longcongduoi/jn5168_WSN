#include "Buffer.h"
#include "ErrorHandler.h"
//---------------------------------------------------------------------------

PUBLIC void BufInit(Buffer* buf)
{
	buf->start = 0;
	buf->stop = 0;
	buf->maxSize = BUFF_MAX_SIZE;
	buf->size = 0;
	buf->availSize = buf->maxSize - buf->size;
}
//---------------------------------------------------------------------------

PUBLIC void BufErase(Buffer* buf, uint16 size)
{
	uint16 i;
	uint8 val;

	if (size > buf->size)
		size = buf->size;
	for (i = 0; i < size; i++)
		BufPop(buf, &val);
}
//---------------------------------------------------------------------------

PUBLIC bool BufPush(Buffer* buf, uint8 val)
{
	if (buf->availSize > 0)
	{
		if (buf->size == 0)
			buf->stop = buf->start;
		else
		{
			buf->stop++;
			if (buf->stop >= buf->maxSize)
				buf->stop = 0;
		}

		buf->buf[buf->stop] = val;
		buf->size++;
		buf->availSize--;

		return TRUE;
	}
	else
	{
		EHCall(BUFF_MOD_ID, 0x1);
		return FALSE;
	}
}
//---------------------------------------------------------------------------

PUBLIC bool BufPushFromArray(Buffer* buf, uint8* array, uint16 arraySize)
{
	if (buf->availSize < arraySize)
	{
		EHCall(BUFF_MOD_ID, 0x1);
		return FALSE;
	}

	uint16 i;
	for (i = 0; i < arraySize; i++)
		BufPush(buf, array[i]);

	return TRUE;
}
//---------------------------------------------------------------------------

PUBLIC bool BufPop(Buffer* buf, uint8* val)
{
	if (buf->size > 0)
	{
		(*val) = buf->buf[buf->start];
		buf->start++;
		if (buf->start >= buf->maxSize)
			buf->start = 0;
		buf->size--;
		buf->availSize++;
		return TRUE;
	}
	else
		return FALSE;
}
//---------------------------------------------------------------------------

PUBLIC bool BufTryGet(Buffer const * const  buf, uint16 pos, uint8* val)
{
	if (pos >= buf->size)
		return FALSE;
	pos = (buf->start + pos) % buf->maxSize;
	(*val) = buf->buf[pos];
	return TRUE;
}

PUBLIC uint8 BufGet(Buffer const * const buf, uint16 pos)
{
	uint8 result = 0;

	if(!BufTryGet(buf, pos, & result))
		EHCall(BUFF_MOD_ID, 0x2);

	return result;
}

//---------------------------------------------------------------------------

PUBLIC bool BufGetToArray(Buffer const * const  buf, uint8* array, uint16 arraySize, uint16* getSize)
{
	uint8 val;
	(*getSize) = 0;

	if (buf->size == 0)
		return FALSE;
	while ((*getSize) < arraySize && BufTryGet(buf, (*getSize), &val))
	{
		array[(*getSize)] = val;
		(*getSize)++;
	}
	return TRUE;
}
//---------------------------------------------------------------------------

PUBLIC bool BufPopToArray(Buffer* buf, uint8* array, uint16 arraySize, uint16* getSize)
{
	if (BufGetToArray(buf, array, arraySize, getSize))
	{
		BufErase(buf, *getSize);
		return TRUE;
	}
	return FALSE;
}
//---------------------------------------------------------------------------

PUBLIC uint16 BufMaxSize(Buffer const * const  buf)
{
	return buf->maxSize;
}
//---------------------------------------------------------------------------

PUBLIC uint16 BufSize(Buffer const * const  buf)
{
	return buf->size;
}
//---------------------------------------------------------------------------

PUBLIC uint16 BufAvailSize(Buffer const * const  buf)
{
	return buf->availSize;
}
//---------------------------------------------------------------------------
