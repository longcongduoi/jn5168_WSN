#include "Converter.h"
//---------------------------------------------------------------------------

typedef union
{
	float valFloat;
	uint32 valUint32;
} FloatUint32Val;
//---------------------------------------------------------------------------

static char converterBuffer[32];


PUBLIC uint8 CStringToAAPString(char const* const src, uint8* dst)
{
	uint8 i = 0;

	while (src[i] != '\0')
	{
		dst[i + 1] = src[i];
		i++;
	}
	dst[0] = i;

	return i+1;
}

PUBLIC uint16 ntoh16(uint16 v)
{
	uint8 res[2];
	uint8 *src = (uint8*)&v;

	res[0] = src[1];
	res[1] = src[0];

	return *(uint16*)res;
}

PUBLIC uint16 hton16(uint16 v)
{
	return ntoh16(v);
}

PUBLIC uint32 ntoh32(uint32 v)
{
	uint8 res[4];
	uint8 *src = (uint8*)&v;

	res[0] = src[3];
	res[1] = src[2];
	res[2] = src[1];
	res[3] = src[0];

	return *(uint32*)res;
}

PUBLIC uint32 hton32(uint32 v)
{
	return ntoh32(v);
}

PUBLIC uint64 ntoh64(uint64 v)
{
	uint8 res[8];
	uint8 *src = (uint8*)&v;

	res[0] = src[7];
	res[1] = src[6];
	res[2] = src[5];
	res[3] = src[4];
	res[4] = src[3];
	res[5] = src[2];
	res[6] = src[1];
	res[7] = src[0];

	return *(uint64*)res;
}

PUBLIC uint64 hton64(uint64 v)
{
	return ntoh64(v);
}


//======================= old functions ====================================== //

PRIVATE char* ConverterDig32ToStr(uint32 digit, char* str, uint8 u8Base);
//---------------------------------------------------------------------------

PUBLIC void ConverterUint16ToUint8(uint16 val, uint8* val8)
{
	val8[0] = val;
	val8[1] = val >> 8;
}
//---------------------------------------------------------------------------

PUBLIC void ConverterUint32ToUint8(uint32 val, uint8* val8)
{
	val8[0] = val;
	val8[1] = val >> 8;
	val8[2] = val >> 16;
	val8[3] = val >> 24;
}
//---------------------------------------------------------------------------

PUBLIC void ConverterUint64ToUint8(uint64 val, uint8* val8)
{
	val8[0] = val;
	val8[1] = val >> 8;
	val8[2] = val >> 16;
	val8[3] = val >> 24;
	val8[4] = val >> 32;
	val8[5] = val >> 40;
	val8[6] = val >> 48;
	val8[7] = val >> 56;
}
//---------------------------------------------------------------------------

PUBLIC void ConverterFloatToUint8(float val, uint8* val8)
{
	FloatUint32Val floatUint32Val;
	floatUint32Val.valFloat = val;
	ConverterUint32ToUint8(floatUint32Val.valUint32, val8);
}
//---------------------------------------------------------------------------

PUBLIC uint8 ConverterStringToUint8 (char const* const str, uint8* val8)
{
	uint8 i;
	uint8 length = str[0] + 1;

	for (i = 0; i < length; i++)
		val8[i] = ((uint8*) str)[i];

	return length;
}
//---------------------------------------------------------------------------

PUBLIC void ConverterUint8ToUint16(uint8 const* const val8, uint16* val)
{
	(*val) = 0;
	(*val) |= (((uint16) val8[0]));
	(*val) |= (((uint16) val8[1]) << 8);
}
//---------------------------------------------------------------------------

PUBLIC void ConverterUint8ToUint32(uint8 const * const val8, uint32* val)
{
	(*val) = 0;
	(*val) |= (((uint32) val8[0]));
	(*val) |= (((uint32) val8[1]) << 8);
	(*val) |= (((uint32) val8[2]) << 16);
	(*val) |= (((uint32) val8[3]) << 24);
}
//---------------------------------------------------------------------------

PUBLIC void ConverterUint8ToUint64(uint8 const * const val8, uint64* val)
{
	(*val) = 0;
	(*val) |= (((uint64) val8[0]));
	(*val) |= (((uint64) val8[1]) << 8);
	(*val) |= (((uint64) val8[2]) << 16);
	(*val) |= (((uint64) val8[3]) << 24);
	(*val) |= (((uint64) val8[4]) << 32);
	(*val) |= (((uint64) val8[5]) << 40);
	(*val) |= (((uint64) val8[6]) << 48);
	(*val) |= (((uint64) val8[7]) << 56);
}
//---------------------------------------------------------------------------

PUBLIC void ConverterUint8ToFloat(uint8 const * const val8, float* val)
{
	FloatUint32Val floatUint32Val;
	ConverterUint8ToUint32(val8, &floatUint32Val.valUint32);
	(*val) = floatUint32Val.valFloat;
}
//---------------------------------------------------------------------------

PUBLIC void ConverterUint8ToString(uint8 const * const val8, char* str)
{
	uint8 i;
	uint8 length = val8[0] + 1;

	for (i = 0; i < length; i++)
		str[i] = ((char*) val8)[i];
	str[i] = '\0';
}
//---------------------------------------------------------------------------

PUBLIC char* ConverterDigToStr2(uint64 digit, uint8 u8Base)
{
	return ConverterDigToStr(digit, converterBuffer, u8Base);
}

PUBLIC char* ConverterDigToStr(uint64 digit, char* str, uint8 u8Base)
{
	char high[33], low[33];
	uint32 val32;
	uint8 length = 0, lowLength = 0;
	uint8 i, strPos = 0;
	high[0] = '\0';
	low[0] = '\0';
	str[0] = '\0';

// convert
	val32 = (uint32) (digit >> 32);
	if (val32 != 0)
		ConverterDig32ToStr((uint32) (digit >> 32), high, u8Base);
	ConverterDig32ToStr((uint32) digit, low, u8Base);

// add '0' to second part of string presentation of digit
	if (val32 != 0)
	{
		while (low[length] != '\0')
			length++;
		switch (u8Base)
		{
			case 2:
				lowLength = 32;
				break;
			case 10:
				lowLength = 10;
				break;
			case 16:
				lowLength = 8;
				break;
		}
		i = 0;
		while (high[i] != '\0')
			str[strPos++] = high[i++];
		for (i = 0; i < lowLength - length; i++)
			str[strPos++] = '0';
	}
	i = 0;
	while (low[i] != '\0')
		str[strPos++] = low[i++];
	str[strPos] = '\0';

	return str;
}
//----------------------------------------------------------------------------

PUBLIC uint8 ConverterStringToPackString(char const * const srcString, uint8 * dstString)
{
	uint8 i = 0;

	while (srcString[i] != '\0')
	{
		dstString[i + 1] = srcString[i];
		i++;
	}
	dstString[0] = i;
	dstString[i + 1] = '\0';

	return i+1;
}
//---------------------------------------------------------------------------
PUBLIC void ConverterPackStringToPackString(char const * srcString, char* dstString)
{
	uint8 i;

	dstString[0] = srcString[0];
	for (i = 1; i <= srcString[0]; i++)
		dstString[i] = srcString[i];
	dstString[i] = '\0';
}
//---------------------------------------------------------------------------

PUBLIC char* ConverterFloatToString(float val, uint8 precision, char* dstString)
{
	uint32 powVal = 1;
	uint32 left;
	uint32 right;
	uint8 pos;
	uint8 i;

	for (i = 0; i < precision; i++)
		powVal *= 10;

	pos = 0;
	if (val < 0)
	{
		val = -val;
		dstString[pos++] = '-';
	}

	left = val;
	right = (val - left) * powVal;

	ConverterDigToStr(left, &dstString[pos], 10);
	if (precision != 0)
	{
		while (dstString[pos] != '\0')
			pos++;
		dstString[pos++] = '.';
		if (right == 0)
		{
			for (i = 0; i < precision; i++)
				dstString[pos++] = '0';
			dstString[pos++] = '\0';
		}
		else
			ConverterDigToStr(right, &dstString[pos], 10);
	}

	return dstString;
}
//---------------------------------------------------------------------------

PRIVATE char* ConverterDig32ToStr(uint32 digit, char* str, uint8 u8Base)
{
	char buf[33];
	char *p = buf + 33;
	uint32 c, n;
	uint8 i;

	*--p = '\0';
	do
	{
		n = digit / u8Base;
		c = digit - (n * u8Base);
		if (c < 10)
		{
			*--p = '0' + c;
		}
		else
		{
			*--p = 'a' + (c - 10);
		}
		digit /= u8Base;
	} while (digit != 0);

	i = 0;
	while (*p)
	{
		str[i++] = (*p);
		p++;
	}
	str[i] = '\0';

	return str;
}
//----------------------------------------------------------------------------
