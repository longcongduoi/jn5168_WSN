#include "StringSimple.h"
//---------------------------------------------------------------------------

PUBLIC void sstrcpy(char* dst, char* src)
{
	uint8 pos = 0;

	do
	{
		dst[pos] = src[pos];
	} while (src[pos++] != '\0');
}
//---------------------------------------------------------------------------

PUBLIC void sstrcat(char* dst, char* src)
{
	uint8 posDst = 0, posSrc = 0;

	while (dst[posDst] != '\0')
		posDst++;

	do
	{
		dst[posDst++] = src[posSrc];
	} while (src[posSrc++] != '\0');
}
//---------------------------------------------------------------------------

PUBLIC uint8 sstrlen(char* str)
{
	uint8 length = 0;

	while (str[length] != '\0')
		length++;
	return length;
}
//---------------------------------------------------------------------------
