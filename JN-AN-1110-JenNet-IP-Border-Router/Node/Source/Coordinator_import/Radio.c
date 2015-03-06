#include <Api.h>

#include "Radio.h"
#include "Config.h"
#include "ErrorHandler.h"

//---------------------------------------------------------------------------

PUBLIC extern RadioDevice MainRadioDevices[];
PRIVATE uint8 m_RDevCount;
PRIVATE RadioDevice* m_RCurDevice;
PRIVATE uint8 m_LastTimeSend;
PRIVATE RadioDevice* RadioGetDevice(uint64 devID);
//----------------------------------------------------------------------------

PUBLIC void RadioInit(void)
{
	m_RDevCount = 0;
	m_RCurDevice = NULL;
	m_LastTimeSend = 0;
}
//----------------------------------------------------------------------------

PUBLIC bool RadioSend(Packet pack, uint64 dstAdr, bool obligatory)
{
	RadioDevice* device;

	device = RadioGetDevice(dstAdr);
	if (device == NULL)
		return FALSE;

	if (PacketSize(&pack) <= BufAvailSize(&device->TxBuf))
		PacketPushToBuf(&pack, &device->TxBuf);
	else if (obligatory)
	{
		BufErase(&device->TxBuf, PacketSize(&pack) - BufAvailSize(&device->TxBuf));
		PacketPushToBuf(&pack, &device->TxBuf);
	}
	else
	{
		EHCall(RADIO_MOD_ID, 4);
		return FALSE;
	}

	device->dataSent = FALSE;
	if (m_RCurDevice == NULL)
		RadioSent(TRUE);

	return TRUE;
}
//----------------------------------------------------------------------------

PUBLIC void RadioSent(bool succes)
{
	uint8 data[RADIO_MAXPACK_SIZE];
	uint16 size;
	uint8 i;

	m_LastTimeSend = 0;
	if (m_RDevCount == 0)
		return;

	if (m_RCurDevice != NULL && !succes)
		EHCall(RADIO_MOD_ID, 5);

	if ((m_RCurDevice != NULL) && (BufSize(&m_RCurDevice->TxBuf) == 0))
		m_RCurDevice->dataSent = TRUE;

	if (m_RCurDevice == NULL || m_RCurDevice->dataSent)
	{
		m_RCurDevice = NULL;
		for (i = 0; i < m_RDevCount; i++)
			if (!MainRadioDevices[i].dataSent)
			{
				m_RCurDevice = &MainRadioDevices[i];
				break;
			}
	}

	if (m_RCurDevice != NULL)
	{
		BufPopToArray(&m_RCurDevice->TxBuf, data, RADIO_MAXPACK_SIZE, &size);

		// Разделить MAC на две части
		union{

			uint32 u32Mac[2];
			uint64 u64Mac;
		}TempMac;

		TempMac.u64Mac = m_RCurDevice->devID;
		MAC_ExtAddr_s CurrentMac = {TempMac.u32Mac[1], TempMac.u32Mac[0]};

		// Отпрвить данные
		eApi_SendDataToPeer(&CurrentMac, data, size, FALSE);
	}
}
//----------------------------------------------------------------------------
/*
PUBLIC void RadioDataFromDevice(tsData* data)
{
	RadioDevice* device;
	uint16 i;

	device = RadioGetDevice(data->u64SrcAddress);
	if (device == NULL)
		return;

	if (BufAvailSize(&device->RxBuf) < data->u16Length)
	{
		EHCall(RADIO_MOD_ID, 1);
		BufErase(&device->RxBuf, data->u16Length - BufAvailSize(&device->RxBuf));
	}

	for (i = 0; i < data->u16Length; i++)
		BufPush(&device->RxBuf, data->pau8Data[i]);
}
*/
//----------------------------------------------------------------------------

PUBLIC bool RadioRecievPack(Packet* pack)
{
	uint8 i;
	PacketInit(pack);
	uint16 prevSize;
	uint16 prevStart;

	for (i = 0; i < m_RDevCount; i++)
	{
		prevSize = BufSize(&MainRadioDevices[i].RxBuf);
		prevStart = MainRadioDevices[i].RxBuf.start;
		if (PacketPopFromBuf(pack, &MainRadioDevices[i].RxBuf))
			return TRUE;
	}
	return FALSE;
}
//----------------------------------------------------------------------------

PUBLIC void RadioTick(uint16 count)
{
	m_LastTimeSend += count;
	if (m_LastTimeSend > 7) // 7 из стека было 16=CONFIG_COMMAND_ACK_TIME)
		RadioSent(FALSE);
}
//----------------------------------------------------------------------------

PUBLIC bool RadioAllDataSent(void)
{
	uint8 i;
	for (i = 0; i < m_RDevCount; i++)
		if (!MainRadioDevices[i].dataSent)
			return FALSE;

	return TRUE;
}
//----------------------------------------------------------------------------

PRIVATE RadioDevice* RadioGetDevice(uint64 devID)
{
	uint8 i;

	if (MAIN_DEVICE_TYPE != dtCoordinator && devID != 0) devID = 0;

	for (i = 0; i < m_RDevCount; i++)
		if (MainRadioDevices[i].devID == devID)
			return &MainRadioDevices[i];

	if (m_RDevCount == MAIN_MAX_DEVICE_COUNT)
	{
		EHCall(RADIO_MOD_ID, 2);
		return NULL;
	}

	MainRadioDevices[m_RDevCount].devID = devID;
	MainRadioDevices[m_RDevCount].dataSent = TRUE;
	BufInit(&MainRadioDevices[m_RDevCount].RxBuf);
	BufInit(&MainRadioDevices[m_RDevCount].TxBuf);
	m_RDevCount++;
	return &MainRadioDevices[m_RDevCount - 1];
}
//----------------------------------------------------------------------------
