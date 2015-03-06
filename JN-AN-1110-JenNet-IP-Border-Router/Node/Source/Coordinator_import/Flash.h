#ifndef FLASH_H
#define FLASH_H

//---------------------------------------------------------------------------
#include <jendefs.h>
#include "Object.h"
#include "DeviceDescriptor.h"
//---------------------------------------------------------------------------

PUBLIC typedef enum{

	SETTINGS_READ_STATUS_OK,
	SETTINGS_READ_CRC_FAILED,
	SETTINGS_READ_NO_DATA
}SettingsReadStatus_e;

PUBLIC void vFlashSettings_init(void *SettingsContents, uint8 SettingsLength);
PUBLIC void vFlashSettings_write(void);

PUBLIC SettingsReadStatus_e eFlashSettings_read(void);

#endif
