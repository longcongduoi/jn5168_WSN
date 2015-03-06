
#include "UPS.h"
#include "Config.h"
#include "Converter.h"
//---------------------------------------------------------------------------

PUBLIC ObjectHandler ObjectHandlerUPS;
PRIVATE void MesureUPS (void);
PRIVATE bool HwEventUPS (uint32 deviceId, uint32 itemBitmap);
PRIVATE bool SetValueUPS (uint32 valControl);
PRIVATE bool SetConfigUPS (int32 config);
//---------------------------------------------------------------------------

PUBLIC void ObjectHandlerUPSInit (void){
ObjectHandlerUPS.Mesure = &MesureUPS;
ObjectHandlerUPS.HwEvent = &HwEventUPS;
ObjectHandlerUPS.SetValue = &SetValueUPS;
ObjectHandlerUPS.SetConfig = &SetConfigUPS;

//ConverterStringToPackString("UPS", ObjectHandlerUPS.object.name);
//ConverterStringToPackString("\0", ObjectHandlerUPS.object.dimension);

ObjectHandlerUPS.object.val = 0;
ObjectHandlerUPS.object.valControl = 0;
ObjectHandlerUPS.object.minval = 0;
ObjectHandlerUPS.object.maxval = 1;
ObjectHandlerUPS.object.sendDval = 1;
ObjectHandlerUPS.object.valType = pvtBool; //pvtUint8;
ObjectHandlerUPS.object.precision = 0;

ObjectHandlerUPS.object.readTime = OBJECT_ONWAKE;
ObjectHandlerUPS.object.sendTime = OBJECT_DISABLE;
ObjectHandlerUPS.object.config = 0;
ObjectHandlerUPS.object.quality = 0xC0;

vAHI_DioSetDirection (EXTERN_VCC_ON_OFF, 0);
}
//---------------------------------------------------------------------------

PRIVATE void MesureUPS (void){
if((u32AHI_DioReadInput() & EXTERN_VCC_ON_OFF)!= 0)
	ObjectHandlerUPS.object.val = 0;
else
	ObjectHandlerUPS.object.val = 1;
}
//---------------------------------------------------------------------------

PRIVATE bool HwEventUPS (uint32 deviceId, uint32 itemBitmap){
//deviceId; itemBitmap;	// disable warning
return FALSE;
}
//---------------------------------------------------------------------------

PRIVATE bool SetValueUPS (uint32 valControl){
//valControl;	// disable warning
return FALSE;
}
//---------------------------------------------------------------------------

PRIVATE bool SetConfigUPS (int32 config){
//config;	// disable warning
return FALSE;
}
//---------------------------------------------------------------------------
