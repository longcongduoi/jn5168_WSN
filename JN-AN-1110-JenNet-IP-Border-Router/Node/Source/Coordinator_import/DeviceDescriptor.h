#ifndef DEVICEDESCRIPTOR_H
#define DEVICEDESCRIPTOR_H
//---------------------------------------------------------------------------

#include <jendefs.h>
#include "Packet.h"
//---------------------------------------------------------------------------

#define DEVICEDESCRIPTOR_ID 1
#define DEVICEDESCRIPTOR_PROP_MAC        	1
#define DEVICEDESCRIPTOR_PROP_DEVNAME    	2
#define DEVICEDESCRIPTOR_PROP_DEVICETYPE	3
#define DEVICEDESCRIPTOR_PROP_NETID  		4
#define DEVICEDESCRIPTOR_PROP_RELATION   	5
#define DEVICEDESCRIPTOR_PROP_ERROR     	6
#define DEVICEDESCRIPTOR_PROP_DEMOMODE     	7
#define DEVICEDESCRIPTOR_PROP_VERSION     	8
#define DEVICEDESCRIPTOR_PROP_SERVICEMODE  	9
#define DEVICEDESCRIPTOR_PROP_TESTMODE  	10
#define DEVICEDESCRIPTOR_PROP_DESCRIPTOR	11
#define DEVICEDESCRIPTOR_PROP_XDATA		12	/////////
#define DEVICEDESCRIPTOR_PROP_ROUTER		14

#define ERROR_HISTORY_BUF 4
//---------------------------------------------------------------------------

PUBLIC typedef enum{
	dddtCoordinator = 0,
	dddtRouter,
	dddtEndDevice
} DeviceDescriptorDevType;

PUBLIC typedef struct{
//********************************считаем длину пакета***************************
uint16 objectID;
char name [7];
uint8 netID;
//10
uint64 mac;
uint64 routeBy;
//26
uint16 error;
//28
uint8 demoMode;// включается любой цифрой, отличной от 0
uint32 version;
//33
uint8 testMode;// 0-выключен, 1-передача данных 1 раз в секунду и так далее
uint16 errorsHist [ERROR_HISTORY_BUF];
//36
char descriptor [12];//32// имя модели устройства
char xdata [10]; 	/////////// пишем в датчиках дату поверки в формате 2012-01-29, в координаторах цифровую подпись в формате xX23Ys
//58
uint8 router;
uint8 deviceType;	//0 - C, 1 - R, 2 - E
uint8 serviceMode;
//----------------------------------------------------------
//---------------------------------------------------------------------------
} DeviceDescriptor;

extern PUBLIC DeviceDescriptor deviceDescriptor;
//---------------------------------------------------------------------------

PUBLIC void DeviceDescriptorInit ();
PUBLIC void DeviceDescriptorInitExtDevice (DeviceDescriptor* devDescriptor);
PUBLIC void DeviceDescriptorToPack (DeviceDescriptor deviceDescriptor, Packet* pack, uint8 propID);
PUBLIC bool DeviceDescriptorFromPack (DeviceDescriptor* deviceDescriptor, Packet* pack);
PUBLIC void DeviceDescriptorSetProp (DeviceDescriptor* deviceDescriptor, uint8 propID, uint64 val, char* strVal);
PUBLIC bool DeviceDescriptorErrorToPack (DeviceDescriptor* deviceDescriptor, Packet* pack);
PUBLIC void DeviceDescriptorSetError (DeviceDescriptor* deviceDescriptor, uint16 val);
PUBLIC void DeviceDescriptorDecodeVersion(DeviceDescriptor deviceDescriptor, uint8* version1, uint8* version2, uint8* version3);

#endif
