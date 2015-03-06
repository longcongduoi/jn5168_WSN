#include "ObjectUPS.h"
#include "Converter.h"
//---------------------------------------------------------------------------

PUBLIC Object ObjectUPS;

PUBLIC void ObjectUPSInit (void){
ObjectUPS.objectID = OBJECT_UPS;

//ConverterStringToPackString("UPS", ObjectUPS.name);
//ConverterStringToPackString("\0", ObjectUPS.dimension);

ObjectUPS.val = 0;
ObjectUPS.valControl = 0;
ObjectUPS.minval = 0;
ObjectUPS.maxval = 1;
ObjectUPS.sendDval = 1;
ObjectUPS.valType = pvtBool; //pvtUint8;
ObjectUPS.precision = 0;

ObjectUPS.readTime = OBJECT_DISABLE;
ObjectUPS.sendTime = OBJECT_DISABLE;
ObjectUPS.config = 0;
ObjectUPS.quality = 0xC0;
}
//---------------------------------------------------------------------------

