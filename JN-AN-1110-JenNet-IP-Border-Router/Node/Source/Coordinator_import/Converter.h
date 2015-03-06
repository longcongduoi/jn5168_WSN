#ifndef CONVERTER_H
#define CONVERTER_H
//---------------------------------------------------------------------------

#include <jendefs.h>
//---------------------------------------------------------------------------

uint8 CStringToAAPString(char const* const str, uint8* val8);

uint16 ntoh16(uint16 v);
uint16 hton16(uint16 v);

uint32 ntoh32(uint32 v);
uint32 hton32(uint32 v);

uint64 ntoh64(uint64 v);
uint64 hton64(uint64 v);

PUBLIC void ConverterUint16ToUint8 (uint16 val, uint8* val8);
PUBLIC void ConverterUint32ToUint8 (uint32 val, uint8* val8);
PUBLIC void ConverterUint64ToUint8 (uint64 val, uint8* val8);
PUBLIC void ConverterFloatToUint8  (float val, uint8* val8);
PUBLIC uint8 ConverterStringToUint8 (char const* const str, uint8* val8);

PUBLIC void ConverterUint8ToUint16 (uint8 const * const val8, uint16* val);
PUBLIC void ConverterUint8ToUint32 (uint8 const * const  val8, uint32* val);
PUBLIC void ConverterUint8ToUint64 (uint8 const * const val8, uint64* val);
PUBLIC void ConverterUint8ToFloat  (uint8 const * const val8, float* val);
PUBLIC void ConverterUint8ToString (uint8 const * const val8, char* str);

PUBLIC char* ConverterDigToStr (uint64 digit, char* str, uint8 u8Base);
PUBLIC char* ConverterDigToStr2(uint64 digit, uint8 u8Base);

PUBLIC void ConverterPackStringToPackString(char const * srcString, char* dstString);
PUBLIC uint8 ConverterStringToPackString (char const * srcString, uint8* dstString);


PUBLIC char* ConverterFloatToString (float val, uint8 precision, char* dstString);

#endif
