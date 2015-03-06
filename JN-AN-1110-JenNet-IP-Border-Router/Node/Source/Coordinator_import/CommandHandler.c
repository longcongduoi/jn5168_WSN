#include <string.h>
#include <AppApi.h>
#include <Api.h>
#include <jip.h>

#include "CommandHandler.h"
#include "ErrorHandler.h"
#include "Vector.h"
#include "Uart.h"
#include "Utils.h"
#include "Converter.h"
#include "CoordinatorAsDevice.h"

// Функции, учавствующие в протоколе AACP
#define FUNCTION_GET_CONNECTED_DEVICES 		1
#define FUNCTION_ON_CONNECTED_STATE_CHANGED 2
#define FUNCTION_SEND_DATA 					3
#define FUNCTION_ON_DATA_RECEIVED 			4
#define FUNCTION_RESET 						5
#define FUNCTION_START 						6
#define FUNCTION_ON_DEBUG_MESSAGE 			7

// Тип кадра
#define FRAME_TYPE_REQUEST 					1
#define FRAME_TYPE_CONFIRM 					2
#define FRAME_TYPE_COMPLETED 				3
#define FRAME_TYPE_NOTIFICATION 			4
#define FRAME_TYPE_EXCEPTION 				5

// Коды ошибок
#define E_INVALID_FRAME_TYPE				1
#define E_UNKNOWN_FUNCTION					2
#define E_TXBUFFER_TOO_SMALL				3
#define E_ERROR_RX_SEQUENCE 				4
#define E_NOT_ENOUGH_DATA					5
#define E_TOO_MANY_RECEIVED_RADIO_DATA		6

#define E_SEND_FAILED_JENIE_ERR_INVLD_PARAM 10
#define E_SEND_JENIE_DEFERRED				11
#define E_SEND_FAILED_JENIE_ERR_UNKNOWN		12
#define E_SEND_FAILED_JENIE_ERR_STACK_BUSY	14
#define E_SEND_FAILED_UNKNOWN_CODE			15
#define E_SEND_FAILED_NETWORK_DOWN			16
#define E_RX_PACKEGE_BUFFER_FULL			17


// Переменные
PUBLIC bool g_NetworkUp = FALSE;
PUBLIC bool g_Reset = FALSE;

// Флаг что координатор переведен в работу с сервером
PRIVATE bool m_isCoordinatorStarted = FALSE;
// Принимаемый порядковый номер кадра
PRIVATE uint8 m_rCounter = 0;
// Передаваемый порядковый номер кадра
PRIVATE uint8 m_sCounter = 0;
// Буфер под создание кадра для сервера
PRIVATE Vector m_txVector;
// Буфер под создание кадра для сервера
PRIVATE Frame* m_txFrame = (Frame*)&m_txVector.buf[0];
// Буфер под входящий пакет
PRIVATE Buffer m_PackageRxBuf;
// Входящий пакет
PRIVATE Packet m_rxPackage;


// Декларация функций
PRIVATE uint8 TickFrameCounter(uint8 value);
// Отправить кадр
PRIVATE void SendFrame(uint8 function, uint8 frameType, uint8 argsSize);
// Отправить кадр не смотря на флаг работы
PRIVATE void SendFrameForce(uint8 function, uint8 frameType, uint8 argsSize);
// Обработка команды запроса устройств на связи
PRIVATE void ProcessGetConnectedDevices();
// Обработать команду "Отправить данные"
PRIVATE void ProcessSendData(Vector const* const data);
// Обработать входящий пакет
PRIVATE void ProcessRecievedPack();
// Отправить принятые данные серверу
PRIVATE void ProcessSendDataToServer(uint64 mac, uint8 * data, uint8 size);


PUBLIC void CommandHandlerInit()
{
	BufInit(&m_PackageRxBuf);
}

PUBLIC void OnUartData( Vector const* const data )
{
	VectorClear( &m_txVector );

	Frame* frame = (Frame*)&data->buf[0];

	if( frame->type != FRAME_TYPE_REQUEST)
	{
		EHCall(COMMANDHANDLER_MOD_ID, E_INVALID_FRAME_TYPE);
		return;
	}

	// Если нарушена последовательность кадров пока только рапортуем
	if(frame->ns != m_rCounter && frame->function != FUNCTION_RESET)
	{
		EHCall( COMMANDHANDLER_MOD_ID, E_ERROR_RX_SEQUENCE );
	}

	m_rCounter = TickFrameCounter(frame->ns);

	switch(frame->function)
	{
		case FUNCTION_GET_CONNECTED_DEVICES:
			ProcessGetConnectedDevices();
			break;

		case FUNCTION_SEND_DATA:
			ProcessSendData(data);
			break;

		case FUNCTION_RESET:
			m_sCounter = 0;
			m_rCounter = 0;
			m_isCoordinatorStarted = FALSE;
			SendFrameForce(FUNCTION_RESET, FRAME_TYPE_COMPLETED, 0);
			break;

		case FUNCTION_START:
			m_isCoordinatorStarted = TRUE;
			SendFrame(FUNCTION_START, FRAME_TYPE_COMPLETED, 0);
			break;

		default:
			EHCall(COMMANDHANDLER_MOD_ID, E_UNKNOWN_FUNCTION);
			break;
	}
}


PUBLIC void SendDebugMessageForce( char* message )
{
	SendDebugMessage(message);
	UartSend();
}

PUBLIC void SendDebugMessage( char* message )
{
	VectorClear( &m_txVector );
	uint8 len = CStringToAAPString(message, &m_txFrame->args);
	SendFrame(FUNCTION_ON_DEBUG_MESSAGE, FRAME_TYPE_NOTIFICATION, len);
}

PUBLIC void SendEcho( uint8 value )
{
	VectorClear( &m_txVector );

	int i = 0;
	for(i = 0; i < 8; ++i)
		*(&m_txFrame->args + i) = 0;

	*(&m_txFrame->args + i) = value;

	SendFrame(FUNCTION_ON_DATA_RECEIVED, FRAME_TYPE_NOTIFICATION, 9);
}

PUBLIC void SendPackToServer(Packet* p)
{
	if (MAIN_DEVICE_TYPE == dtEndDevice && !MAIN_EDUART_ENABLE) return;

	uint16 packSize = PacketSize(p);
	uint16 sentBytes = 0;

    while(sentBytes < packSize)
    {
    	int frameArgSize = (packSize - sentBytes) >= RADIO_MAXPACK_SIZE ? RADIO_MAXPACK_SIZE : (packSize - sentBytes) % RADIO_MAXPACK_SIZE;

    	ProcessSendDataToServer(*(uint64*)pvAppApiGetMacAddrLocation(), &p->buf[sentBytes], frameArgSize);

    	sentBytes += frameArgSize;
	}
}
/*
PUBLIC void OnRadioData(tsData* data)
{
	if(data->u16Length  > RADIO_MAXPACK_SIZE )
	{
		EHCall(COMMANDHANDLER_MOD_ID,E_TOO_MANY_RECEIVED_RADIO_DATA);
		return;
	}

    ProcessSendDataToServer(data->u64SrcAddress, data->pau8Data, data->u16Length);
}
*/

PUBLIC void OnDeviceConnectedStateChanged(uint64 mac, bool isConnected)
{
	uint8 *args = (uint8*)&m_txFrame->args;

    mac = hton64(mac);
    memcpy(&args[0], &mac, sizeof(uint64) );
    args[sizeof(uint64)] = isConnected;

    SendFrame(FUNCTION_ON_CONNECTED_STATE_CHANGED, FRAME_TYPE_NOTIFICATION, sizeof(uint64) + sizeof(bool) );
}

PUBLIC void OnRadioSendNotification(bool isSent)
{
	m_txFrame->args = isSent;
	SendFrame(FUNCTION_SEND_DATA, FRAME_TYPE_COMPLETED, 1);
}

PRIVATE void SendFrame(uint8 function, uint8 frameType, uint8 argsSize)
{
	if(!m_isCoordinatorStarted)
		return;

	SendFrameForce(function, frameType, argsSize);
}

PRIVATE void SendFrameForce(uint8 function, uint8 frameType, uint8 argsSize)
{
	if( SizeOfFrame + argsSize > m_txVector.maxSize )
	{
		EHCall(COMMANDHANDLER_MOD_ID, E_TXBUFFER_TOO_SMALL);
		return;
	}

	Frame* frame = (Frame*)&m_txVector.buf[0];

	frame->nr = m_rCounter;
	frame->ns = m_sCounter;
	frame->type = frameType;
	frame->function = function;
	frame->size = ntoh16(argsSize);

	m_sCounter = TickFrameCounter(m_sCounter);

	m_txVector.size = SizeOfFrame + argsSize;

	UartSendFrame( &m_txVector );
}

PRIVATE void ProcessGetConnectedDevices()
{
	VectorClear( &m_txVector );

    uint16 i;

    uint8 *args = (uint8*)&m_txFrame->args;
    uint8 count = 0;

    const uint8 arrayHeaderSize = 3;

    // Координатор

    ts6LP_SockAddr sDeviceAddress;
    i6LP_GetOwnDeviceAddress(&sDeviceAddress, FALSE);
    uint64 mac = (sDeviceAddress.sin6_addr.s6_addr32[2])|(sDeviceAddress.sin6_addr.s6_addr32[3]);//hton64(*(uint64*)pvAppApiGetMacAddrLocation());

    memcpy(&args[arrayHeaderSize + sizeof(uint64) * count++], &mac, sizeof(uint64) );

    for (i = 1; i <= u8Api_GetNeighbourTableSize(); i++)
    {
        tsNeighbourEntry neighbourEntry;

    	if (bApi_GetNeighbourTableEntry(i, &neighbourEntry) == TRUE)
    	{
			union{

				uint64 u64Mac;
				uint32 u32Mac[2];
			}UnionMac;

			UnionMac.u32Mac[0] = neighbourEntry.sAddr.u32H;
			UnionMac.u32Mac[1] = neighbourEntry.sAddr.u32L;

    		//hton64(neighbourEntry.u64Addr);
    		memcpy(&args[arrayHeaderSize + sizeof(uint64) * count++], &UnionMac.u64Mac, sizeof(uint64) );
    	}
    }

    for (i = 0; i < u16Api_GetRoutingTableSize(); i++){

        tsRoutingEntry routingEntry;
    	if (bApi_GetRoutingTableEntry(i, &routingEntry) == TRUE){

			union{

				uint64 u64Mac;
				uint32 u32Mac[2];
			}UnionMac;

			UnionMac.u32Mac[0] = routingEntry.sDestAddr.u32H;
			UnionMac.u32Mac[1] = routingEntry.sDestAddr.u32L;

    		//hton64(neighbourEntry.u64Addr);

    		memcpy(&args[arrayHeaderSize + sizeof(uint64) * count++], &UnionMac.u64Mac, sizeof(uint64) );
    	}
    }

    args[0] = pvtUint64;
    uint16 ncount = hton16(count);
    memcpy(&args[1], &ncount, sizeof(uint16));

    SendFrame(FUNCTION_GET_CONNECTED_DEVICES, FRAME_TYPE_COMPLETED, sizeof(uint64) * count + arrayHeaderSize );
}

PRIVATE void ProcessSendData(Vector const* const data)
{
	if(data->size <=SizeOfFrame + sizeof(uint8))
	{
		EHCall(COMMANDHANDLER_MOD_ID, E_NOT_ENOUGH_DATA);
		return;
	}

	SendFrame(FUNCTION_SEND_DATA, FRAME_TYPE_CONFIRM, 0);

	Frame* frame = (Frame*)&data->buf[0];
	uint64 mac = 0;
	memcpy(&mac, &frame->args , sizeof(uint64));
	mac = ntoh64(mac);

	uint8* sendData = &frame->args + sizeof(uint64);
	uint8 sendDataSize = data->size - SizeOfFrame - sizeof(uint64);

	// Если данные для координатора
    ts6LP_SockAddr sDeviceAddress;
    i6LP_GetOwnDeviceAddress(&sDeviceAddress, FALSE);
    uint64 LocalMac = (sDeviceAddress.sin6_addr.s6_addr32[2])|(sDeviceAddress.sin6_addr.s6_addr32[3]);//hton64(*(uint64*)pvAppApiGetMacAddrLocation());

	if(mac == LocalMac)
	{
		SendFrame(FUNCTION_SEND_DATA, FRAME_TYPE_COMPLETED, 1);

		if(BufPushFromArray(&m_PackageRxBuf, sendData, sendDataSize))
			ProcessRecievedPack();
		else
		{
			EHCall(COMMANDHANDLER_MOD_ID, E_RX_PACKEGE_BUFFER_FULL);
			BufInit(&m_PackageRxBuf);
		}

		return;
	}

	// Данные для устройства сети
	if(g_NetworkUp)
	{
		union{

			uint32 u32Mac[2];
			uint64 u64Mac;
		}TempMac;

		TempMac.u64Mac = mac;
		MAC_ExtAddr_s CurrentMac = {TempMac.u32Mac[1], TempMac.u32Mac[0]};

		teJenNetStatusCode status = eApi_SendDataToPeer(&CurrentMac, sendData, sendDataSize, FALSE);

		switch(status)
		{
		default:
			break;
		}

		OnRadioSendNotification(FALSE);
	}
	else
	{
		EHCall(COMMANDHANDLER_MOD_ID, E_SEND_FAILED_NETWORK_DOWN);
		OnRadioSendNotification(FALSE);
	}
}

PRIVATE uint8 TickFrameCounter(uint8 value)
{
	return (uint8) ((value + 1) % 0xFF);
}

PRIVATE void ProcessRecievedPack()
{
	PacketInit(&m_rxPackage);

	if (!PacketPopFromBuf(&m_rxPackage, &m_PackageRxBuf))
		return;

	CoordinatorAsDeviceHandle(&m_rxPackage);

	BufInit(&m_PackageRxBuf);
}

PRIVATE void ProcessSendDataToServer(uint64 mac, uint8 * data, uint8 size)
{
	uint8 *args = (uint8*)&m_txFrame->args;

    mac = hton64(mac);
    memcpy(&args[0], &mac, sizeof(uint64) );
    memcpy(&args[sizeof(uint64)], data, size );

    SendFrame(FUNCTION_ON_DATA_RECEIVED, FRAME_TYPE_NOTIFICATION, size + sizeof(uint64) );
}



