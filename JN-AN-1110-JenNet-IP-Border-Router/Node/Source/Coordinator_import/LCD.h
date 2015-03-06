#ifndef LCD_H
#define LCD_H
//---------------------------------------------------------------------------

#include "object.h"
#include <jendefs.h>
//---------------------------------------------------------------------------
#define LCD_MOD_ID 15

PUBLIC void LCDInit (void);
PUBLIC void LCDPower(bool on);
PUBLIC void LCDSetNet(uint8 netID);
PUBLIC void LCDSetDevStat(uint8 GetNeighbourTableSize, uint8 GetRoutingTableSize, uint8 ErrorDescModuleId, uint8 ErrorDescErrorCode);
PUBLIC void LCDSetDevName (char* name);
PUBLIC void LCDSetBattaryVal (uint32 val, uint32 min, uint32 max);
PUBLIC void LCDSetLinkVal (uint32 val, uint32 min, uint32 max);
PUBLIC void LCDSetConnectionVal (bool online);

//to print measured points

PUBLIC void LCDSetTempreture (float val, char* dimension);//(float val, char* dimension);
PUBLIC void LCDSetHumidity (uint32 val, char* dimension);
//PUBLIC void LCD_SetSHT10Temperature (Object* object, char* dimension);
//PUBLIC void LCD_SetSHT10Humidity (Object* object, char* dimension);
//PUBLIC void LCD_SetBMP085Pressure (Object* object, char* dimension);

PUBLIC void LCDSetTempreturePt1 (float val, char* dimension);
PUBLIC void LCDSetTempreturePt2 (float val, char* dimension);
PUBLIC void LCDSetTempreturePt3 (float val, char* dimension);
PUBLIC void LCDSetTempreturePt4 (float val, char* dimension);
PUBLIC void LCDSetDIO(bool val, int id);
PUBLIC void LCDSetLinkDrawAsText(bool asText);
PUBLIC void LCDSetText (char* str);
PUBLIC void LCDUpdate(void);
PUBLIC void LCDDebug(char* str, uint8 line);
PUBLIC void LCDDebugWithDig (char* str, uint64 dig, uint8 line);
PUBLIC void LCDTick();

// Для отладочных целей. Перетерает строку информации о количестве устройств
PUBLIC void LCDOutError(uint8 module, uint8 code);

#endif
