#include <Api.h>
#include <AppApi.h>
#include <AppHardwareApi_JN516x.h>

#include "CoordinatorAsDevice.h"
#include "CommandHandlerDef.h"
#include "ObjectConnection.h"
#include "ObjectUPS.h"
#include "ObjectChildren1.h"
#include "CommandHandler.h"

#include "ErrorHandler.h"
#include "Flash.h"
#include "Uart.h"

// Коды ошибки
#define E_INVALID_MAC 1
#define E_GET_PROP_UNKNOWN 2
#define E_UNKNOWN_PACKAGE_TYPE 3




extern bool g_Reset;

PRIVATE void SendCoordDevInfo(void);

PRIVATE void SendCoordConnectionVal(void);

//---------------------------------------------------------------------------

PUBLIC void CoordinatorAsDeviceHandle(Packet * pack)
{
	uint64 srcAdr, dstAdr;
	uint8 type, state;
	uint64 timeVal;

	PacketGetHeader(pack, &srcAdr, &dstAdr, &type, &state, &timeVal);

	if (dstAdr != deviceDescriptor.mac)
	{
		EHCall(COORDINATOR_AS_DEVICE_MOD_ID, E_INVALID_MAC);
		return;
	}

	switch (type)
	{
		/*
		 case COMMAND_TYPE_RESET :{
		 PacketInit(&reqPack);
		 PacketMakeHeader (&reqPack, deviceDescriptor.mac, srcAdr, COMMAND_TYPE_RESET, COMMAND_STATE_OK, 0);
		 PacketFinalize (&reqPack);
		 UartSendPack(reqPack);
		 vAHI_SwReset ();
		 }
		 break;
		 */
		// from enddevice
		case COMMAND_TYPE_DEVINFO:
		{
			if (state == COMMAND_STATE_COMMAND) // получение описания устройства
				SendCoordDevInfo();

			break;
		}


		case COMMAND_SET_PROP:
		{
			if (state == COMMAND_STATE_COMMAND)
			{
				uint16 objectID;
				uint8 netID = deviceDescriptor.netID;
				PacketContext packContext;
				PacketSaveContext(pack, &packContext);

				PacketGetObject(pack, &objectID);
				if (objectID == DEVICEDESCRIPTOR_ID)
				{
					if (DeviceDescriptorFromPack(&deviceDescriptor, pack))
					{
						vFlashSettings_write();

						PacketMakeHeader(pack, deviceDescriptor.mac, srcAdr, COMMAND_SET_PROP, COMMAND_STATE_OK, 0);
						PacketRestoreContext(pack, packContext);
						PacketFinalize(pack);
						SendPackToServer(pack);
					}
				}
				else
					EHCall(COORDINATOR_AS_DEVICE_MOD_ID, 3);


				// При изменении номера сети перезагружаем координатор
				if (netID != deviceDescriptor.netID)
					g_Reset = TRUE;
			}
			break;
		}

		case COMMAND_GET_PROP:
		{
			if (state == COMMAND_STATE_COMMAND)
			{
				uint16 objectID;
				PacketGetObject(pack, &objectID);

				if (objectID == OBJECT_CONNECTION)
					SendCoordConnectionVal();
				else
				{
					EHCall(COORDINATOR_AS_DEVICE_MOD_ID, E_GET_PROP_UNKNOWN);

					Packet reqPack;
					PacketInit(&reqPack);
					PacketMakeHeader(&reqPack, dstAdr, srcAdr, COMMAND_GET_PROP, COMMAND_STATE_ERROR, 0);
					PacketFinalize(&reqPack);
					SendPackToServer(&reqPack);
				}
			}
			break;
		}

		default:
		{
			EHCall(COORDINATOR_AS_DEVICE_MOD_ID, E_UNKNOWN_PACKAGE_TYPE);
		}
		break;
	}
}
//---------------------------------------------------------------------------

PRIVATE void SendCoordDevInfo(void)
{
	Packet pack;
	PacketInit(&pack);
	PacketMakeHeader(&pack, deviceDescriptor.mac, 0, COMMAND_TYPE_DEVINFO, COMMAND_STATE_OK, 0);
	DeviceDescriptorToPack(deviceDescriptor, &pack, -1);
	ObjectConnectionInit();
	ObjectConnection.val = 1;
	ObjectToPack(ObjectConnection, &pack, FALSE);
// объявление UPS
		ObjectUPSInit();
		if ((u32AHI_DioReadInput() & EXTERN_VCC_ON_OFF) != 0)
			ObjectUPS.val = 0;
		else
			ObjectUPS.val = 1;
		ObjectToPack(ObjectUPS, &pack, FALSE);


	// ****************объекты, указывающие на состояние сети*********************
	ObjectChildren1Init();
	ObjectChildren1.val = u8Api_GetNeighbourTableSize(); // обнаружено устройств в сети
	ObjectToPack(ObjectChildren1, &pack, TRUE);

	PacketFinalize(&pack);
	SendPackToServer(&pack);

}
//---------------------------------------------------------------------------
PUBLIC void SendCoordConnectionVal(void)
{
	Packet pack;
	PacketInit(&pack);
	PacketMakeHeader(&pack, deviceDescriptor.mac, 0, COMMAND_GET_PROP, COMMAND_STATE_OK, 0);
	ObjectConnectionInit();
	ObjectConnection.val = 1;
	ObjectToPack(ObjectConnection, &pack, TRUE);
// объявление UPS
		ObjectUPSInit();
		if ((u32AHI_DioReadInput() & EXTERN_VCC_ON_OFF) != 0)
			ObjectUPS.val = 0;
		else
			ObjectUPS.val = 1;
		ObjectToPack(ObjectUPS, &pack, TRUE);


// ****************объекты, указывающие на состояние сети*********************
	ObjectChildren1Init();
	ObjectChildren1.val = u8Api_GetNeighbourTableSize(); // обнаружено устройств в сети
	ObjectToPack(ObjectChildren1, &pack, TRUE);

	//****************************************************************************
	PacketFinalize(&pack);
	SendPackToServer(&pack);
}
