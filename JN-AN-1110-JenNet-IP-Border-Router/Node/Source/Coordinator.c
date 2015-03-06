
#include <string.h>
#include <AppApi.h>
#include <Api.h>
#include <AppHardwareApi.h>
#include <AppHardwareApi_JN516x.h>
#include <pdm.h>
#include <6LP.h>
#include <jip.h>

#include "DebugP.h"
#include "Coordinator.h"
#include "Config.h"
#include "DeviceDescriptor.h"
#include "Uart.h"
#include "Watchdog.h"
#include "Flash.h"
#include "LCD.h"
#include "Led.h"
#include "CommandHandler.h"
#include "Converter.h"
#include "ErrorHandler.h"
#include "Exceptions.h"

//---------------------------------------------------------------------------
// DEFINES

#define PDM_START_SECTOR            0
#define PDM_NUM_SECTORS             63
#define PDM_SECTOR_SIZE             64

#define E_AHI_DEVICE_TICK_TIMER		15

#define INFINITE_LOOP 				while(TRUE)

//---------------------------------------------------------------------------
// LOCAL FUNCTIONS AND VARIABLES

//PUBLIC extern tsJenieRoutingTable MainRoutingTable[];
PUBLIC bool MAIN_EDUART_ENABLE;
PUBLIC uint8 MAIN_HARD_VERSION;

PUBLIC bool g_bReset;

PRIVATE bool g_bWarmStart = FALSE;
PRIVATE uint32 g_u32NetworkId = 0;

PRIVATE void vInitPeripherals(void);
PRIVATE void vStartNetwork(void);

PRIVATE void UpdateLCD(void);
PRIVATE void UpdateLED(void);
PRIVATE void ProcessMessages(void);

PRIVATE bool_t vJenNetAuthoriseCallback(MAC_ExtAddr_s *psAddr, uint8 u8DataLength, uint8 *pu8Data);
PRIVATE bool_t vJenNetBeaconUserAuthoriseCallback(tsScanElement *psBeaconInfo, uint16 u16ProtocolVersion);

PRIVATE void vInitPeripherals(){

	while(bAHI_Clock32MHzStable() == FALSE);

	v6LP_InitHardware();
}

PUBLIC void AppColdStart(void){

	// Base init
	vInitPeripherals();

	vAHI_WatchdogStop();

	vLed_init();
	vUart_init();

	vDebug_enable();

	vExceptions_register();

	if(g_bWarmStart){

		//-- Warm start
		MSG("Warm start\n\r");

		g_bWarmStart = FALSE;
	}
	else{

		//-- Cold start
		MSG("Cold start\n\r");
	}
	if (bAHI_WatchdogResetEvent()){

		MSG("Watchdog restart\n\r");
	}

    // Always initialise the PDM
	PDM_vInit(PDM_START_SECTOR, PDM_NUM_SECTORS, PDM_SECTOR_SIZE, (OS_thMutex) 1, NULL, NULL, NULL);

    ObjectInit(NULL, NULL, NULL);
    MSG("Object init...OK\n\r");

    vFlashSettings_init(&deviceDescriptor, sizeof(deviceDescriptor));
    MSG("Flash settings init...OK\n\r");

    // ������������� ���������
    DeviceDescriptorInit();// ������
    MSG("Device descriptor init...OK\n\r");

    // ������������� ����������� ������
	CommandHandlerInit();
	MSG("Command handler init...OK\n\r");

	// Set up routing table
	//gJenie_RoutingEnabled = TRUE;
	//gJenie_RoutingTableSize = MAIN_MAX_DEVICE_COUNT + 1;
	//gJenie_RoutingTableSpace = (void *) MainRoutingTable;

	// ����������� ������� �������
	//gJenie_EndDeviceChildActivityTimeout = CONFIG_LOST_CONNECTIONTIME * 10;

	// � ������
	//gJenie_MaxFailedPkts = 10;
	//gJenie_RouterPingPeriod = (CONFIG_LOST_CONNECTIONTIME * 1000) / (((uint32) gJenie_MaxFailedPkts) * 100);

	vAHI_DioSetDirection (DIP_1, 0); // ���������� DIO4 ��� ����
	vAHI_DioSetDirection (DIP_2, 0); // ���������� DIO5 ��� ����
	//vAHI_DioSetDirection(0, WDTIMER); // ���������� ��� �����

	// ��������� ������ ����
	uint8 valDIP_1;
	uint8 valDIP_2;

	if (u32AHI_DioReadInput() & DIP_1) // �������� 1 ���, ���� == 0
		valDIP_1=0;
	else
		valDIP_1=1;

	if (u32AHI_DioReadInput() & DIP_2) // �������� 1 ���, ���� == 0
		valDIP_2=0;
	else
		valDIP_2=1;
	if (deviceDescriptor.demoMode == 0){ // �� ��������� �������� ������������� �����// ���� ����� ��������, �� ������ ����� ����� ���������������� � ���� ������������, �������� �����

		if (valDIP_1 ==0 && valDIP_2 == 0)// ��� �����
		{
			g_u32NetworkId = 19;
			//gJenie_Channel = (valNet % 15) + 11; // ��������� ������ ���� � ����
			//gJenie_NetworkApplicationID = valNet; // ��������� ������ ���� � ����
			//gJenie_PanID = valNet; // ��������� ������ ���� � ����
			deviceDescriptor.netID = g_u32NetworkId;
		}
		if (valDIP_1 ==1 && valDIP_2 == 0)// ����� �����
		{
			g_u32NetworkId = 24;
			//gJenie_Channel = (valNet % 15) + 11; // ��������� ������ ���� � ����
			//gJenie_NetworkApplicationID = valNet; // ��������� ������ ���� � ����
			//gJenie_PanID = valNet; // ��������� ������ ���� � ����
			deviceDescriptor.netID = g_u32NetworkId;
		}
		if (valDIP_1 ==0 && valDIP_2 == 1)// ������ �����
		{
			g_u32NetworkId = 29;
			//gJenie_Channel = (valNet % 15) + 11; // ��������� ������ ���� � ���
			//gJenie_NetworkApplicationID = valNet; // ��������� ������ ���� � ����
			//gJenie_PanID = valNet; // ��������� ������ ���� � ����
			deviceDescriptor.netID = g_u32NetworkId;
		}
		if (valDIP_1 ==1 && valDIP_2 == 1)// ��� ������
		{
			g_u32NetworkId = 34;
			//gJenie_Channel = (valNet % 15) + 11; // ��������� ������ ���� � ����
			//gJenie_NetworkApplicationID = valNet; // ��������� ������ ���� � ����
			//gJenie_PanID = valNet; // ��������� ������ ���� � ����
			deviceDescriptor.netID = g_u32NetworkId;
		}
	}
	else // ���� deviceDescriptor.demoMode != 0, �� ����� ������ ���� �� ��� � ���� �� ��������
	{
		//gJenie_Channel = (deviceDescriptor.netID % 15) + 11;
		//gJenie_NetworkApplicationID = deviceDescriptor.netID;
		//gJenie_PanID = deviceDescriptor.netID;
		LCDSetNet(deviceDescriptor.netID);// ���������� ������ ���� �� ������
	}

	PRINTF("Network ID = %d\n\r", g_u32NetworkId);

	//------------------------------------------------------------------------------
	// ������������ ������ ������ ������������

	if (deviceDescriptor.router == 0) // ������� ��������� �����
	{
		//����� ����� ���������, ������������ � ������������
		//gJenie_MaxChildren = 16;
		//����� ��������
		//gJenie_MaxSleepingChildren = 12;
	}
	if (deviceDescriptor.router == 1) // ������ ������ � ���������
	{
		// ����� ��������
		//gJenie_MaxChildren = 6;
		// ����� ��������
		//gJenie_MaxSleepingChildren = 0;
	}

	//-------------------------------------------------------------------
	//gJenie_RecoverFromJpdm = FALSE;

	LCDInit();
	MSG("Lcd init...OK\n\r");

	g_NetworkUp = FALSE;

	vStartNetwork();

	// �������� ����
	INFINITE_LOOP{

		vWatchdog_restart();

		//v6LP_Tick();
		ProcessMessages();
	}
}

PUBLIC void AppWarmStart(void){

	g_bWarmStart = TRUE;

	AppColdStart();
}

PUBLIC void vStartNetwork(void){

	tsJIP_InitData sJIP_InitData;

    memset(&sJIP_InitData, 0, sizeof(sJIP_InitData));

    sJIP_InitData.eDeviceType 				= E_JIP_DEVICE_COORDINATOR;// ��� ����������� ����������
    sJIP_InitData.u32DeviceId           	= 0x08010074;// ID ���������� � ����, ������������ ��� ������������� ������// ��������: Manufacturer ID = 0x0801 � Product ID = 0x0074
    sJIP_InitData.pcVersion             	= VERSION;// ���������� ��� Node MIB// ������ � �������, ���������� Version// ��������� ��� �������
    sJIP_InitData.u32Channel            	= NETWORK_GATEWAY_ALL_CHANNELS_BITMAP;// ������������ ��������� ������
    sJIP_InitData.u16PanId              	= g_u32NetworkId;// ���������� ID ���� (0xFFFF - �������������� �����)
    sJIP_InitData.u64AddressPrefix      	= NETWORK_ADDRESS_PREFIX;// ������� (������� 64 ����) ������ �� ��������� 6LoWPAN ��� ���� ��������� � ����// ������� ������ ��� ������������

    sJIP_InitData.u16MaxIpPacketSize		= 0;// ����������� ����� ������ ������������ IP ������� (������� ��������: payload+40)// �������� ��-��������� 1280 ����

    // Set resource allocation
    sJIP_InitData.u16NumPacketBuffers   	= PACKET_BUFFERS;// ���������� �������� �������
    sJIP_InitData.u8UdpSockets          	= 2;// ���������� ������� �� ����// ���� ����� ��� JIP
    sJIP_InitData.u32RoutingTableEntries   	= ROUTE_TABLE_ENTRIES;// ���������� ������� � ������� ���������

    sJIP_InitData.u16Port                  = 1873;// UDP ���� ��� JenNet-IP �������// ��-��������� = 1873
    sJIP_InitData.u8UniqueWatchers         = 0;// ����������� ���������� Traps ��� �������� �����
    sJIP_InitData.u8MaxTraps               = 0;// ������������ ���������� ��������� Traps
    sJIP_InitData.u8MaxNameLength          = 0;// ���������� ��� Node MIB// ���c�������� ����� ���������� DescriptiveName// ��������� ��� �������
    sJIP_InitData.u8QueueLength            = 0;// ���������� ��� ������ � MIBs// ������������ ����� ������� ��� ��������� JIP-���������

    // ������������� �� ��������� � ������ callback-������� v6LP_ConfigureNetwork
    if (eJIP_Init(&sJIP_InitData) != E_JIP_OK){

    	MSG("JneNet-IP is uninitialised\n\r");
    	vLed_selectColor(cRed);
    	INFINITE_LOOP;
    }
    else{

    	MSG("JneNet-IP is initialised\n\r");
    }

    // Set up our JIP Device Types// ���-�� ������� � MIB
/*
    {
        PRIVATE const uint16 au16DeviceType[] = {0x0001};
        vJIP_SetDeviceTypes(sizeof(au16DeviceType)/sizeof(uint16), (uint16 *)au16DeviceType);
    }

    // Set our descriptive name// ������ � ����������� MIB
    vJIP_SetNodeName("Coordinator");
*/
    if ((u8AHI_PowerStatus() & (1 << 3)) == 0)
    {
        // Power domain is inactive - writing to the register will cause a bus exception
    	MSG("Power domain is inactive\n\r");
    	vLed_selectColor(cRed);
    	INFINITE_LOOP;
    }

    //-- ���������� ��������// ����������� ����� eJIP_Init

    // Standard power
    {

    	vAppApiSetHighPowerMode(APP_API_MODULE_STD, TRUE);
    	vAHI_HighPowerModuleEnable(FALSE, FALSE);
    	MSG("Module standard power...OK\n\r");
    }

    // High power
    {
    	//vAHI_HighPowerModuleEnable(TRUE, TRUE);
    	//MSG("Module high power...OK\n\r");
    }

    // Set short packet fragmentation timeout// timeout ����� �������� ������������� ����� �������������// �� ��������� 6LoWPAN ������������� 60���// ����������� 1 �� ������������� � JenNet-IP
    v6LP_SetPacketDefragTimeout(1);
}

PUBLIC void v6LP_ConfigureNetwork(tsNetworkConfigData *psNetworkConfigData){

	if ((bJnc_SetRunProfile  (CURRENT_NETWORK_JOIN_PROFILE, NULL) != TRUE) || (bJnc_SetJoinProfile (CURRENT_NETWORK_JOIN_PROFILE, NULL) != TRUE)){

		vLed_selectColor(cRed);

		INFINITE_LOOP;
	}

	// Set up network ID / User data stuff
	//if (sConfig.u32NetworkID != 0){

		// Set beacon filtering callback
		//vApi_RegBeaconNotifyCallback(vJenNetBeaconUserAuthoriseCallback);

		// Set establish route payload
		//v6LP_SetUserData(sizeof(uint32), (uint8*)&sConfig.u32NetworkID);
	//}

	/* Set beacon filtering callback */
	vApi_RegBeaconNotifyCallback(vJenNetBeaconUserAuthoriseCallback);

	v6LP_SetUserData(sizeof(uint32), (uint8*)(&g_u32NetworkId));
}

PUBLIC void vJIP_StackEvent(te6LP_StackEvent eEvent, uint8 *pu8Data, uint8 u8DataLen){

    switch (eEvent)
    {
    case E_STACK_NODE_JOINED_NWK:
		{

			MSG("Stack event: E_STACK_NODE_JOINED_NWK\n\r");
		}
		break;

    case E_STACK_NODE_JOINED:
		{
			tsAssocNodeInfo *psAssocNodeInfo = (tsAssocNodeInfo *)pu8Data;

			MSG("Stack event: E_STACK_NODE_JOINED\n\r");

			union{

				uint64 u64Mac;
				uint32 u32Mac[2];
			}UnionMac;

			UnionMac.u32Mac[0] = psAssocNodeInfo->sMacAddr.u32H;
			UnionMac.u32Mac[1] = psAssocNodeInfo->sMacAddr.u32L;

			OnDeviceConnectedStateChanged(UnionMac.u64Mac, TRUE);
		}
    	break;

    case E_STACK_STARTED:
		{

			MSG("Stack event: E_STACK_STARTED\n\r");
		}

    case E_STACK_JOINED:
		{

			MSG("Stack event: E_STACK_JOINED\n\r");
			/* Stack has started */
			//tsNwkInfo *psNwkInfo = (tsNwkInfo *)pu8Data;

			//vLog_Printf(TRACE_BR, LOG_INFO,
					//"\nNetwork Started. Channel: %d, PAN ID: 0x%x",
					//psNwkInfo->u8Channel & 0xFF, psNwkInfo->u16PanID & 0xFFFF);

			/* Update network configuration information */
			//sConfig.u8Channel	= psNwkInfo->u8Channel;
			//sConfig.u16PanID	= psNwkInfo->u16PanID;

			//if (sConfig.u32NetworkID != 0){

				/* Create array containing network ID */
				//uint8 au8NwkId[MAX_BEACON_USER_DATA];
				//memset(au8NwkId, 0, MAX_BEACON_USER_DATA);
				//memcpy(au8NwkId, (uint8 *)&sConfig.u32NetworkID, sizeof(uint32));

				/* Set beacon payload */
				//vApi_SetUserBeaconBits(au8NwkId);

				/* Set up establish route callback */
				//v6LP_SetNwkCallback(vJenNetAuthoriseCallback);
			//}

			// Start polling device IDs
			//vJIP_StartDeviceIDPoll(2000, 2);

			// Flag that network is up and running
			g_NetworkUp = TRUE;
		}
		break;
    case E_STACK_RESET:
		{

			MSG("Stack event: E_STACK_RESET\n\r");

			// Flag that network is down
			g_NetworkUp = TRUE;
		}
    	break;

    case E_STACK_NODE_LEFT:
		{
			tsAssocNodeInfo *psAssocNodeInfo = (tsAssocNodeInfo *)pu8Data;

			MSG("Stack event: E_STACK_NODE_LEFT\n\r");

			union{

				uint64 u64Mac;
				uint32 u32Mac[2];
			}UnionMac;

			UnionMac.u32Mac[0] = psAssocNodeInfo->sMacAddr.u32H;
			UnionMac.u32Mac[1] = psAssocNodeInfo->sMacAddr.u32L;

			OnDeviceConnectedStateChanged(UnionMac.u64Mac, FALSE);
		}
		break;

    case E_STACK_NODE_LEFT_NWK:
		{

			MSG("Stack event: E_STACK_NODE_LEFT_NWK\n\r");
		}
		break;

    case E_STACK_TABLES_RESET:
		{

			MSG("Stack event: E_STACK_TABLES_RESET\n\r");
		}
    	break;

    case E_STACK_NODE_AUTHORISE:
		{

			MSG("Stack event: E_STACK_NODE_AUTHORISE\n\r");
		}
    	break;

    default:
		{

			MSG("Stack event: unknown\n\r");
		}
        break;
    }
}

PUBLIC void v6LP_DataEvent(int iSocket, te6LP_DataEvent eEvent, ts6LP_SockAddr *psAddr, uint8 u8AddrLen){

    switch(eEvent){

    case E_DATA_SENT:
		{

			MSG("Data event: E_DATA_SENT\n\r");
		}
        //DBG_vPrintf(TRACE_BR_COMMS, "<Sent>");
        break;

    case E_DATA_SEND_FAILED:
		{

			MSG("Data event: E_DATA_SEND_FAILED\n\r");
		}
        //DBG_vPrintf(TRACE_BR_COMMS, "<Send Fail>");
        break;

    case E_DATA_RECEIVED:
		{

			MSG("Data event: E_DATA_RECEIVED\n\r");
		}
        //DBG_vPrintf(TRACE_BR_COMMS, "<RXD>");
    	/* Discard 6LP packets as only interested in JIP communication */

        // �������� ������!!!
        //OnRadioData(psData);

    	i6LP_RecvFrom(iSocket, NULL, 0, 0, NULL, NULL);
    	break;

    case E_6LP_ICMP_MESSAGE:
		{

			MSG("Data event: E_6LP_ICMP_MESSAGE\n\r");
		}
        //DBG_vPrintf(TRACE_BR_COMMS, "<RXICMP>");
	    /* Discard 6LP packets as only interested in JIP communication */
        i6LP_RecvFrom(iSocket, NULL, 0, 0, NULL, NULL);
        break;

    case E_IP_DATA_RECEIVED:
		{

			MSG("Data event: E_IP_DATA_RECEIVED\n\r");
		}
        //DBG_vPrintf(TRACE_BR_COMMS, "<GRX %d>", u16_6LP_GetNumberOfAvailableIPv6Buffers());
        break;

    default:
		{

			MSG("Data event: unknown\n\r");
		}
        //DBG_vPrintf(TRACE_BR_COMMS, "\n? in v6LP_DataEvent");
    	break;
    }
}

/****************************************************************************
 *
 * NAME: vJenNetAuthoriseCallback
 *
 * DESCRIPTION:
 * Callback function from JenNet allowing us to decide if a node is allowed
 * to join the network or not
 *
 * PARAMETERS Name          RW Usage
 *            psAddr        R  MAC address of node attempting to join
 *            u8DataLength  R  Length of establish route payload
 *            pu8Data       R  Pointer to establish route payload data
 *
 * RETURNS:
 * TRUE if node is allowed to join, FALSE otherwise
 *
 ****************************************************************************/
PRIVATE bool_t vJenNetAuthoriseCallback(MAC_ExtAddr_s *psAddr, uint8 u8DataLength, uint8 *pu8Data){

    return TRUE;
}


/****************************************************************************
 *
 * NAME: vJenNetBeaconUserAuthoriseCallback
 *
 * DESCRIPTION:
 * Callback function from JenNet allowing us to decide if we should try to join
 * a beacon
 *
 * PARAMETERS Name               RW Usage
 *            psBeaconInfo       R  Pointer to the beacon information
 *            u16ProtocolVersion R  JenNet protocol version of beaconing device
 *
 * RETURNS:
 * TRUE if we should attempt to join, FALSE otherwise
 *
 ****************************************************************************/
PRIVATE bool_t vJenNetBeaconUserAuthoriseCallback(tsScanElement *psBeaconInfo, uint16 u16ProtocolVersion){

	return TRUE;
}

PUBLIC void v6LP_PeripheralEvent(uint32 u32Device, uint32 u32ItemBitmap){

    // DBG_vPrintf(FALSE, "%s(%u, %x)\n", __FUNCTION__, u32Device, u32ItemBitmap);

	// The Tick timer is used to time events
	if (u32Device == E_AHI_DEVICE_TICK_TIMER){

		//#define ACTIVITY_LED_RATE_RUNNING (100) 	/* Toggle every second when running */
		//static uint8 u8ActivityLedCountDown = ACTIVITY_LED_RATE_RUNNING;

		//if (--u8ActivityLedCountDown == 0){

			//vToggleActivityLED();

			// Set next timeout dependent on what mode we are currently in
			//u8ActivityLedCountDown = ACTIVITY_LED_RATE_RUNNING;
		//}
	}
}

PRIVATE void ProcessUartData()
{
	// ������������ �������� �� UART �����
	Vector* data = UartGetFrameBuffer();
	if (data == NULL)
		return;

	OnUartData(data);

	UartIncommingFrameHandled();
}

PRIVATE void ProcessMessages(void)
{
	static uint32 tickTimer = 0;
	tickTimer++;
	static uint32 allDevInfoTimer = 0;

	// ���������� ������������ ��� UART ������
	UartSend();

	if (g_NetworkUp && g_bReset)
	{
		// �������������� ������������ ��� ����� ����
		vAHI_SwReset();
	}

	// ��������� ������� �� PC
	ProcessUartData();

	// ��� � ������� (��� �������� � ������������ ������ ������ �������� 32000 ��� � �������)
	if (tickTimer >= 32000)
	{
		allDevInfoTimer++;

		LCDTick();

		UpdateLCD();

		UpdateLED();

		//WDTPulse();

		tickTimer = 0;
	}
}

PRIVATE void UpdateLCD(void)
{
	// ���������� CoordGUI
	//LCDSetNet(gJenie_NetworkApplicationID);

	LCDSetDevName(&deviceDescriptor.name[1]);

	// ��������� ������� ���� 1-1 Ex-x
	LCDSetDevStat(u8Api_GetNeighbourTableSize(), u16Api_GetRoutingTableSize(), g_ErrorDesc.ModuleId, g_ErrorDesc.ErrorCode);

	LCDUpdate();
}

PRIVATE void UpdateLED(void)
{
	static bool blinc = TRUE;
	blinc = !blinc;
	if ((u32AHI_DioReadInput() & EXTERN_VCC_ON_OFF) != 0){

		vLed_on();
	}
	else{

		if (blinc) vLed_selectColor(cRed);
		else vLed_off();
	}
}
