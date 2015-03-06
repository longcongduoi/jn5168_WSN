#include <pdm.h>

#ifdef JN513x
  #include "AppHardwareApi_JN513x.h"
#else
#ifdef JN514x
  #include "AppHardwareApi_JN514x.h"
#endif
#endif


#include "Flash.h"
#include "Converter.h"
#include <CommandHandler.h>
#define FLASH_HAS_DATA      0
#define DEVISEDESCRIPTOR_SIZE 256


//---------------------------------------------------------------------------
PRIVATE uint32 m_FlashStartAddress = 0;

PRIVATE void FlashInit(uint8* data){


}
//---------------------------------------------------------------------------

PUBLIC void FlashWrite(Object object)
{
	uint8 data[CONFIG_OBJECT_SIZE];

// init
	FlashInit(data);

	data[0] = FLASH_HAS_DATA;
	ConverterUint32ToUint8(object.valControl, &data[1]);
	ConverterUint32ToUint8(object.sendDval, &data[5]);
	ConverterUint16ToUint8(object.readTime, &data[9]);
	ConverterUint16ToUint8(object.sendTime, &data[11]);
	ConverterUint32ToUint8(object.config, &data[13]);

	bAHI_FullFlashProgram(m_FlashStartAddress + DEVISEDESCRIPTOR_SIZE + object.objectID * CONFIG_OBJECT_SIZE, CONFIG_OBJECT_SIZE, data);
	vAHI_FlashPowerDown();
}
//----------------------------------------------------------------------------

PUBLIC void FlashRead(Object* object)
{
	uint8 data[CONFIG_OBJECT_SIZE];

	FlashInit(data);

	bAHI_FullFlashRead(m_FlashStartAddress + DEVISEDESCRIPTOR_SIZE + object->objectID * CONFIG_OBJECT_SIZE, CONFIG_OBJECT_SIZE, data);
	if (data[0] == FLASH_HAS_DATA)
	{
		ConverterUint8ToUint32(&data[1], (uint32*) &object->valControl);
		ConverterUint8ToUint32(&data[5], (uint32*) &object->sendDval);
		ConverterUint8ToUint16(&data[9], &object->readTime);
		ConverterUint8ToUint16(&data[11], &object->sendTime);
		ConverterUint8ToUint32(&data[13], (uint32*) &object->config);
	}

	vAHI_FlashPowerDown();
}
//----------------------------------------------------------------------------

PUBLIC void FlashWriteDeviceDescriptor(DeviceDescriptor deviceDescriptor)
{
	uint8 data[DEVISEDESCRIPTOR_SIZE];

// init
	FlashInit(data);
	bAHI_FlashEraseSector(3);

	data[0] = FLASH_HAS_DATA;
	ConverterStringToUint8(deviceDescriptor.name, &data[1]);
	data[33] = deviceDescriptor.netID;
	data[34] = deviceDescriptor.demoMode;
	data[35] = deviceDescriptor.testMode;
	ConverterStringToUint8(deviceDescriptor.descriptor, &data[36]);
	ConverterStringToUint8(deviceDescriptor.xdata, &data[49]); // запись даты поверки в датчиках и цифровой подписи в координаторах
	data[50] = deviceDescriptor.router; // 1-режим работы только с роутерами
//bAHI_FullFlashProgram(m_FlashStartAddress+deviceDescriptor.objectID*CONFIG_OBJECT_SIZE, DEVISEDESCRIPTOR_SIZE, data);
	bAHI_FullFlashProgram(m_FlashStartAddress, DEVISEDESCRIPTOR_SIZE, data);
	vAHI_FlashPowerDown();
}
//----------------------------------------------------------------------------

PUBLIC void FlashReadDeviceDescriptor(DeviceDescriptor* deviceDescriptor)
{
	uint8 data[DEVISEDESCRIPTOR_SIZE];

	FlashInit(data);
	//bAHI_FullFlashRead(m_FlashStartAddress+deviceDescriptor->objectID*CONFIG_OBJECT_SIZE, DEVISEDESCRIPTOR_SIZE, data);
	bAHI_FullFlashRead(m_FlashStartAddress, DEVISEDESCRIPTOR_SIZE, data);

	if (data[0] == FLASH_HAS_DATA)
	{
		ConverterUint8ToString(&data[1], deviceDescriptor->name);
		deviceDescriptor->netID = data[33];
		deviceDescriptor->demoMode = data[34];
		deviceDescriptor->testMode = data[35];
		ConverterUint8ToString(&data[36], deviceDescriptor->descriptor);
		ConverterUint8ToString(&data[49], deviceDescriptor->xdata); // чтение даты поверки в датчиках и цифровой подписи в координаторах
		deviceDescriptor->router = data[50]; // 1-режим работы только с роутерами
	}

	vAHI_FlashPowerDown();
}

PUBLIC void FlashErase()
{
	vAHI_FlashPowerUp();
	bAHI_FlashInit(E_FL_CHIP_AUTO, NULL);

	bAHI_FlashEraseSector(3);

	vAHI_FlashPowerDown();
}


PRIVATE void FlashInit(uint8* data)
{
	uint8 i;

	if (m_FlashStartAddress == 0)
	{
#ifdef JN5148
		m_FlashStartAddress=1024*64*3;
#else
		m_FlashStartAddress = 1024 * 32 * 3;
#endif
	}

	for (i = 0; i < CONFIG_OBJECT_SIZE; i++)
		data[i] = 1;
	vAHI_FlashPowerUp();
	bAHI_FlashInit(E_FL_CHIP_AUTO, NULL);
}
//----------------------------------------------------------------------------
