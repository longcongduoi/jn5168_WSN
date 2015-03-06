#include <AppApi.h>
#include "Config.h"
#include "Flash.h"
#include "DeviceDescriptor.h"
#include "Converter.h"
#include "StringSimple.h"
#include "ErrorHandler.h"
#include "Converter.h"
#include "DebugP.h"



PUBLIC DeviceDescriptor deviceDescriptor;
//---------------------------------------------------------------------------

PRIVATE void DeviceDescriptorPropToPack(DeviceDescriptor deviceDescriptor, Packet* pack, uint8 propID);

//---------------------------------------------------------------------------

PUBLIC void DeviceDescriptorInit()
{
	uint8 i;

	SettingsReadStatus_e SettingsReadStatus = eFlashSettings_read();

	if((SettingsReadStatus == SETTINGS_READ_CRC_FAILED)||(SettingsReadStatus == SETTINGS_READ_NO_DATA)){

		deviceDescriptor.objectID = DEVICEDESCRIPTOR_ID;
		deviceDescriptor.version = (SOFT_VERSION1 << 24) | (SOFT_VERSION2 << 16)	| (SOFT_VERSION3 << 8) | 0;

		uint8 length, delta;
		char name[32];

		//***************формирование начального ID**********************
		// add '0' to mac name
		name[1] = 'M';
		name[2] = 'A';
		name[3] = 'C';
		name[4] = '_';
		ConverterDigToStr(deviceDescriptor.mac, &name[5], 16);
		length = 0;
		while (name[length + 4 + 1] != '\0')
			length++;
		delta = 16 - length;
		for (i = 16; i > delta; i--)
			name[i + 4] = name[i + 4 - delta];
		for (i = delta; i > 0; i--)
			name[i + 4] = '0';
		name[0] = 16 + 4;
		name[16 + 4 + 1] = '\0';
		//***************формирование ID с префиксом*********************
		deviceDescriptor.name[0] = 5; // новая длина строки
		deviceDescriptor.name[1] = PREFNAME;
		deviceDescriptor.name[2] = name[17];
		deviceDescriptor.name[3] = name[18];
		deviceDescriptor.name[4] = name[19];
		deviceDescriptor.name[5] = name[20];
		deviceDescriptor.name[6] = '\0';
		//***************************************************************
		deviceDescriptor.netID = CONFIG_DEFAULT_NETID;
		deviceDescriptor.demoMode = 0;
		deviceDescriptor.testMode = 0;
		deviceDescriptor.router = 0; // 1-режим работы только с роутерами
		deviceDescriptor.deviceType = MAIN_DEVICE_TYPE;

		//------------------формирование модели устройства-----------------------
		char dig[16];
		sstrcpy(&deviceDescriptor.descriptor[1], PREFMODEL);
		sstrcat(&deviceDescriptor.descriptor[1], ConverterDigToStr(HARDTYPE, dig, 10));
		deviceDescriptor.descriptor[0] = sstrlen(&deviceDescriptor.descriptor[1]);
	}
	else if(SettingsReadStatus == SETTINGS_READ_STATUS_OK){

		for (i = 0; i < ERROR_HISTORY_BUF; i++) deviceDescriptor.errorsHist[i] = 0;
		deviceDescriptor.mac = *((uint64*) pvAppApiGetMacAddrLocation());
		deviceDescriptor.routeBy = 0;
		deviceDescriptor.error = 0;
		deviceDescriptor.serviceMode = 0;
	}
	else{

		MSG("Unknown device descriptor read status\n\r");
	}
}

//***************************************************************
//---------------------------------------------------------------------------

PUBLIC void DeviceDescriptorInitExtDevice(DeviceDescriptor* devDescriptor)
{
	uint8 i;
	devDescriptor->objectID = DEVICEDESCRIPTOR_ID;
	sstrcpy(devDescriptor->name, "blank");
	devDescriptor->netID = 0;
	devDescriptor->mac = 0;
	devDescriptor->routeBy = 0;
	devDescriptor->error = 0;
	devDescriptor->demoMode = 0;
	devDescriptor->version = (SOFT_VERSION1 << 24) | (SOFT_VERSION2 << 16)
			| (SOFT_VERSION3 << 8) | 0;
	devDescriptor->serviceMode = 0;
	devDescriptor->testMode = 0;
	devDescriptor->router = 0; // 1-режим работы только с роутерами
	sstrcpy(devDescriptor->descriptor, "blank");
	for (i = 0; i < ERROR_HISTORY_BUF; i++)
		devDescriptor->errorsHist[i] = 0;
}
//---------------------------------------------------------------------------

PUBLIC void DeviceDescriptorToPack(DeviceDescriptor deviceDescriptor,
		Packet* pack, uint8 propID)
{

	if (propID >= DEVICEDESCRIPTOR_PROP_MAC
			&& propID <= DEVICEDESCRIPTOR_PROP_ROUTER) //XDATA)
	{ ///////
		PacketAddObject(pack, deviceDescriptor.objectID);
		DeviceDescriptorPropToPack(deviceDescriptor, pack, propID);
	}
//    	if (propID >= DEVICEDESCRIPTOR_PROP_MAC && propID
//    		<= DEVICEDESCRIPTOR_PROP_DESCRIPTOR)
//    	    {
//    	    PacketAddObject(pack, deviceDescriptor.objectID);
//    	    DeviceDescriptorPropToPack(deviceDescriptor, pack, propID);
//    	    }
	else if (propID == (uint8) -1)
	{
		PacketAddObject(pack, deviceDescriptor.objectID);
		DeviceDescriptorPropToPack(deviceDescriptor, pack,
				DEVICEDESCRIPTOR_PROP_MAC);
		DeviceDescriptorPropToPack(deviceDescriptor, pack,
				DEVICEDESCRIPTOR_PROP_DEVNAME);
		DeviceDescriptorPropToPack(deviceDescriptor, pack,
				DEVICEDESCRIPTOR_PROP_DEVICETYPE);
		DeviceDescriptorPropToPack(deviceDescriptor, pack,
				DEVICEDESCRIPTOR_PROP_NETID);
		DeviceDescriptorPropToPack(deviceDescriptor, pack,
				DEVICEDESCRIPTOR_PROP_RELATION);
		DeviceDescriptorPropToPack(deviceDescriptor, pack,
				DEVICEDESCRIPTOR_PROP_ERROR);
		DeviceDescriptorPropToPack(deviceDescriptor, pack,
				DEVICEDESCRIPTOR_PROP_DEMOMODE);
		DeviceDescriptorPropToPack(deviceDescriptor, pack,
				DEVICEDESCRIPTOR_PROP_VERSION);
		DeviceDescriptorPropToPack(deviceDescriptor, pack,
				DEVICEDESCRIPTOR_PROP_SERVICEMODE);
		DeviceDescriptorPropToPack(deviceDescriptor, pack,
				DEVICEDESCRIPTOR_PROP_TESTMODE);
		DeviceDescriptorPropToPack(deviceDescriptor, pack,
				DEVICEDESCRIPTOR_PROP_DESCRIPTOR);
		DeviceDescriptorPropToPack(deviceDescriptor, pack,
				DEVICEDESCRIPTOR_PROP_XDATA);
		DeviceDescriptorPropToPack(deviceDescriptor, pack,
				DEVICEDESCRIPTOR_PROP_ROUTER); // 1-режим работы только с роутерами
	} else
		EHCall(DEVICEDESCRIPTOR_MOD_ID, 1);
}
//---------------------------------------------------------------------------

PUBLIC bool DeviceDescriptorFromPack(DeviceDescriptor* deviceDescriptor,
		Packet* pack)
{
	uint16 propID;
	ValType valType;
	uint64 val;
	char strVal[32];
	bool error = TRUE;
	while (PacketGetProperty(pack, &propID, &valType, &val, 0, strVal))
	{
		switch (propID)
		{
		case DEVICEDESCRIPTOR_PROP_DEVNAME:
		case DEVICEDESCRIPTOR_PROP_NETID:
		case DEVICEDESCRIPTOR_PROP_DEMOMODE:
		case DEVICEDESCRIPTOR_PROP_TESTMODE:
		case DEVICEDESCRIPTOR_PROP_DESCRIPTOR:
		case DEVICEDESCRIPTOR_PROP_XDATA:
		case DEVICEDESCRIPTOR_PROP_ROUTER://///////////
		{
			DeviceDescriptorSetProp(deviceDescriptor, propID, val, strVal);
			error = FALSE;
			break;
		}
		default:
			EHCall(DEVICEDESCRIPTOR_MOD_ID, 2);
			break;
		}
	}
	return !error;
}

//---------------------------------------------------------------------------
PUBLIC void DeviceDescriptorSetProp(DeviceDescriptor* deviceDescriptor,
		uint8 propID, uint64 val, char* strVal)
{
	switch (propID)
	{
	case DEVICEDESCRIPTOR_PROP_MAC:
		deviceDescriptor->mac = val;
		break;
	case DEVICEDESCRIPTOR_PROP_DEVNAME:
		ConverterPackStringToPackString(strVal, (char*) deviceDescriptor->name); // если закоментировать, lank
		break;
	case DEVICEDESCRIPTOR_PROP_DEVICETYPE:
		deviceDescriptor->deviceType = (uint8) val;
		break;
	case DEVICEDESCRIPTOR_PROP_NETID:
		deviceDescriptor->netID = (uint8) val;
		break;
	case DEVICEDESCRIPTOR_PROP_RELATION:
		deviceDescriptor->routeBy = val;
		break;
	case DEVICEDESCRIPTOR_PROP_ERROR:
		deviceDescriptor->error = (uint16) val;
		break;
	case DEVICEDESCRIPTOR_PROP_DEMOMODE:
		deviceDescriptor->demoMode = (uint8) val;
		//-------------------включение демо-режима любой цифрой, отличной от 0, 5 попыток соединиться-------------------------------
		if (deviceDescriptor->demoMode != 0)
			deviceDescriptor->demoMode = 5;
		break;
		//--------------------------------------------------------------------------------------------------------------------------
	case DEVICEDESCRIPTOR_PROP_VERSION:
		deviceDescriptor->version = (uint32) val;
		break;
	case DEVICEDESCRIPTOR_PROP_SERVICEMODE:
		deviceDescriptor->serviceMode = (uint8) val;
		break;
	case DEVICEDESCRIPTOR_PROP_TESTMODE:
		deviceDescriptor->testMode = (uint8) val;
		break;
	case DEVICEDESCRIPTOR_PROP_DESCRIPTOR:
		ConverterPackStringToPackString(strVal, deviceDescriptor->descriptor);
		break;
	case DEVICEDESCRIPTOR_PROP_XDATA:
		ConverterPackStringToPackString(strVal, deviceDescriptor->xdata);
		break;
	case DEVICEDESCRIPTOR_PROP_ROUTER:
		deviceDescriptor->router = (uint8) val;
		break;
	default:
		EHCall(DEVICEDESCRIPTOR_MOD_ID, 1);
		break;
	}
}
//---------------------------------------------------------------------------

PUBLIC void DeviceDescriptorSetError(DeviceDescriptor* deviceDescriptor,
		uint16 val)
{
	uint8 i;
	for (i = 0; i < ERROR_HISTORY_BUF; i++)
		if (deviceDescriptor->errorsHist[i] == 0)
		{
			deviceDescriptor->errorsHist[i] = val;
			break;
		}
}
////---------------------------------------------------------------------------
//
PUBLIC bool DeviceDescriptorErrorToPack(DeviceDescriptor* deviceDescriptor,
		Packet* pack)
{
	uint8 i;
	for (i = 0; i < ERROR_HISTORY_BUF; i++)
		if (deviceDescriptor->errorsHist[i] != 0)
		{
			deviceDescriptor->error = deviceDescriptor->errorsHist[i];
			DeviceDescriptorToPack(*deviceDescriptor, pack,
					DEVICEDESCRIPTOR_PROP_ERROR);
			deviceDescriptor->errorsHist[i] = 0;
			return TRUE;
		}
	return FALSE;
}
//---------------------------------------------------------------------------

PRIVATE void DeviceDescriptorPropToPack(DeviceDescriptor deviceDescriptor,
		Packet* pack, uint8 propID)
{
	switch (propID)
	{
	case DEVICEDESCRIPTOR_PROP_MAC:
		PacketAddProperty(pack, propID, pvtUint64, deviceDescriptor.mac, 1, "");
		break;
	case DEVICEDESCRIPTOR_PROP_DEVNAME:
		PacketAddProperty(pack, propID, pvtString, 0, 0, deviceDescriptor.name);
		break; // если закомментировать, то вместо директории с именем будет Device_mac-адрес и имени не будет в свойствах
	case DEVICEDESCRIPTOR_PROP_DEVICETYPE:
		PacketAddProperty(pack, propID, pvtUint8, deviceDescriptor.deviceType,
				0, "");
		break;
	case DEVICEDESCRIPTOR_PROP_NETID:
		PacketAddProperty(pack, propID, pvtUint8, deviceDescriptor.netID, 0,
				"");
		break;
	case DEVICEDESCRIPTOR_PROP_RELATION:
		PacketAddProperty(pack, propID, pvtUint64, deviceDescriptor.routeBy, 0,
				"");
		break;
	case DEVICEDESCRIPTOR_PROP_ERROR:
		PacketAddProperty(pack, propID, pvtUint16, deviceDescriptor.error, 0,
				"");
		break;
	case DEVICEDESCRIPTOR_PROP_DEMOMODE:
		if (deviceDescriptor.demoMode != 0)
			PacketAddProperty(pack, propID, pvtUint8, 1, 0, "");
		else
			PacketAddProperty(pack, propID, pvtUint8, deviceDescriptor.demoMode,
					0, "");
		break;
		//---------------------------------------------------------------------------------------------------------------------
	case DEVICEDESCRIPTOR_PROP_VERSION:
		PacketAddProperty(pack, propID, pvtUint32, deviceDescriptor.version, 0,
				"");
		break;
	case DEVICEDESCRIPTOR_PROP_SERVICEMODE:
		PacketAddProperty(pack, propID, pvtUint8, deviceDescriptor.serviceMode,
				0, "");
		break;
	case DEVICEDESCRIPTOR_PROP_TESTMODE:
		PacketAddProperty(pack, propID, pvtUint8, deviceDescriptor.testMode, 0,
				"");
		break;
	case DEVICEDESCRIPTOR_PROP_DESCRIPTOR:
		PacketAddProperty(pack, propID, pvtString, 0, 0,
				deviceDescriptor.descriptor);
		break;
	case DEVICEDESCRIPTOR_PROP_XDATA:
		PacketAddProperty(pack, propID, pvtString, 0, 0,
				deviceDescriptor.xdata);
		break;
	case DEVICEDESCRIPTOR_PROP_ROUTER:
		PacketAddProperty(pack, propID, pvtUint8, deviceDescriptor.router, 0,
				"");
		break;
	}
}
//---------------------------------------------------------------------------
PUBLIC void DeviceDescriptorDecodeVersion(DeviceDescriptor deviceDescriptor,
		uint8* version1, uint8* version2, uint8* version3)
{
	(*version1) = (uint8) (deviceDescriptor.version >> 24);
	(*version2) = (uint8) (deviceDescriptor.version >> 16);
	(*version3) = (uint8) (deviceDescriptor.version >> 8);
}
//---------------------------------------------------------------------------
