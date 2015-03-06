#ifndef FLASH_H
#define FLASH_H
//---------------------------------------------------------------------------

#include <jendefs.h>
#include "Object.h"
#include "DeviceDescriptor.h"
//---------------------------------------------------------------------------

PUBLIC void Eeprom_init(void);
PUBLIC void Eeprom_saveObject(ObjectId_t ObjectId, void *vObjectContent);
PUBLIC void Eeprom_loadObject(ObjectId_t ObjectId, void *vObjectContent);
PUBLIC void Eeprom_deleteObject(ObjectId_t ObjectId, void *vObjectContent);

PUBLIC void Eeprom_saveAll(ObjectId_t ObjectId, void *vObjectContent);
PUBLIC void Eeprom_deleteAll(ObjectId_t ObjectId, void *vObjectContent);

PUBLIC
PUBLIC void FlashWriteDeviceDescriptor (DeviceDescriptor deviceDescriptor);
PUBLIC void FlashReadDeviceDescriptor (DeviceDescriptor* deviceDescriptor);
PUBLIC void FlashWrite (Object object);
PUBLIC void FlashRead (Object* object);
PUBLIC void FlashErase();

#endif
