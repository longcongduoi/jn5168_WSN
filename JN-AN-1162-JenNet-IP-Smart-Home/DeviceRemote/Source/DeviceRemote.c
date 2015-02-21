/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         Device Remote - Main Remote Source File
 */
/****************************************************************************/
/*
 * This software is owned by NXP B.V. and/or its supplier and is protected
 * under applicable copyright laws. All rights are reserved. We grant You,
 * and any third parties, a license to use this software solely and
 * exclusively on NXP products [NXP Microcontrollers such as JN5148, JN5142, JN5139].
 * You, and any third parties must reproduce the copyright and warranty notice
 * and any other legend of ownership on each copy or partial copy of the
 * software.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright NXP B.V. 2012. All rights reserved
 */
/****************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <jendefs.h>
#include <AppHardwareApi.h>
#include "Config.h"
#include "AppApi.h"
#include <Api.h>

#include "dbg.h"
#include "dbg_uart.h"
#include <os.h>
#include <pdm.h>

#include <string.h>

#include "JIP.h"
#include "6LP.h"

#include "Key.h"
#include "DriverCapTouch.h"
#include "DriverLed.h"
#include "ModeCommission.h"
#include "Exception.h"
#include "Security.h"
#include "Mib.h"

#include "RemoteDefault.h"
#include "MibRemote.h"
#include "MibRemoteConfigGroup.h"
#include "MibCommon.h"
#include "Uart.h"
#if (MK_JIP_DEVICE_ID == 0x0801035C) || (MK_JIP_DEVICE_ID == 0x0801035E)	/* NXP RD6035 Colour ? */
#include "MibColour.h"
#endif

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
/* Debugging defines */
#ifdef DBG_ENABLE
#define TRACE_RM    				TRUE
#define DBG_CALLBACKS               TRUE
#else
#define TRACE_RM					FALSE
#define DBG_CALLBACKS				FALSE
#endif

#define UART_TO_DBG                 DBG_E_UART_0        /* Uart to terminal     */
#define BAUD_RATE_DBG               DBG_E_UART_BAUD_RATE_115200 /* Baud rate to use     */

/* Sleep paramaters */
#define SLEEP_TIME_MS         10
#define TICKS_PER_MS          32
#define CPU_CAL_CYCLES     10000
#define APP_WAKE_TIMER     E_AHI_WAKE_TIMER_0
#define MAX_WAKE_TIMER     0x7ffffffffULL

/* Masks to wake from sleep */
#if (MK_JIP_DEVICE_ID == 0x08016035) || (MK_JIP_DEVICE_ID == 0x0801035F)	/* NXP RD6035 ? */
#define DIO_WAKEMASK       0x1UL		/* DIO0 Wakes from sleep */
#else
#define DIO_WAKEMASK	   0x0UL		/* Disable wake from sleep */
#endif

#define  ALL_DIO_MASK       0xffffffffUL
/* 15 minutes user inactivity before slowing down key-scan  */
#define USER_INACTIVE_TIME_MS       (1000*60*15)

//#define USER_INACTIVE_TIME_MS       (1000*10)   /* 10SEC TEST ONLY !! */

/* JN514x Chip family ? */
#ifdef JENNIC_CHIP_FAMILY_JN514x
/* JenOS PDM config */
#define PDM_START_SECTOR			5
#define PDM_NUM_SECTORS				2
#define PDM_SECTOR_SIZE				0x10000
/* Other chip family ? */
#else
/* JenOS PDM config */
#define PDM_START_SECTOR			0
#define PDM_NUM_SECTORS			   63
#define PDM_SECTOR_SIZE		       64
#endif

// #define MEASURE_SLEEP_CURRENT

// TODO 0 Requires R&D stack fix to allow callback to occur after leaving network
// TODO 1 Resolve occasional crash issue (Safe to sleep determination?)
// TODO 3 Check current profiles
// TODO 4 Inherit lamp MIB index from makefile? (allow working with NXPA/C/bleeding edge)

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
/* Stack modes */
#define STACK_MODE_STANDALONE 	0x1
#define STACK_MODE_COMMISSION 	0x2
/* Up mode values */
#define UP_MODE_NONE 		0
#define UP_MODE_GATEWAY 	1
#define UP_MODE_STANDALONE  2
/* Security key indicies */
#define SECURITY_KEY_NETWORK					0
#define SECURITY_KEY_GATEWAY_COMMISSIONING		1
#define SECURITY_KEY_STANDALONE_COMMISSIONING	2

typedef struct
{
	uint8   		 u8StackModeInit;	/* Mode at initialisation */
	uint8   		 u8DeviceType;
	uint8   		 u8UpMode;
	uint8  	 		 u8Channel;
	uint16 			u16PanId;
	uint32 			u32FrameCounter;
	tsSecurityKey	 asSecurityKey[3];
} tsRemotePdm;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
extern void vNwk_DeleteChildVect(MAC_ExtAddr_s *psNodeAddr);

PRIVATE void vInitPeripherals(void);
PRIVATE void vStartStack(void);
PRIVATE void vAppMainLoop(bool_t bWarmStart);
PRIVATE void vAppSleep(bool_t bSleepType);
PRIVATE void vTouchChecker(void);
PRIVATE bool_t bCheckPowerButton(void);
PRIVATE void  Remote_vPdmErase(void);
PUBLIC  void  Remote_vSetUserData(void);
PUBLIC bool_t Remote_bBeaconNotifyCallback(tsScanElement *psBeaconInfo, uint16 u16ProtocolVersion);
PUBLIC bool_t Remote_bNwkCallback(MAC_ExtAddr_s *psAddr, uint8 u8DataLength, uint8 *pu8Data);

PUBLIC void Remote_vResumeGateway(tsNetworkConfigData *psNetworkConfigData);
PUBLIC void Remote_vResumeStandalone(tsNetworkConfigData *psNetworkConfigData);
PUBLIC void Remote_vSetSecurityKey(uint8 u8Key);
PUBLIC void Remote_vSetProfile(bool_t bStandalone);
PUBLIC void vAppSave(void);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/

extern tSPIflashFncTable *pSPIflashFncTable;
extern uint32 u32GoodSelectKeyPress;

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
#ifdef MIBREMOTECONFIGGROUP_H_INCLUDED
extern tsMibRemoteConfigGroup	 sMibRemoteConfigGroup;
extern thJIP_Mib		 		 hMibRemoteConfigGroup;
#endif

/* Packet buffers */
//PRIVATE uint8 au8PacketBuffer[PACKET_BUFFER_SIZE] __attribute__ ((aligned (4)));
/* Routing table */
//PRIVATE uint8 au8RoutingTableSpace[ROUTING_TABLE_SPACE] __attribute__ ((aligned (4)));

PRIVATE volatile bool_t bNwkJoined = FALSE;

PRIVATE volatile bool_t bSafeToSleep = FALSE;

PRIVATE uint32 u32CalibaratedSleepTime = SLEEP_TIME_MS*TICKS_PER_MS;

PRIVATE bool_t bStandaloneBeacon;

/* main module data structures
 * types in ModeCommisioning header
 */

PRIVATE tsDevice sDevice;
PRIVATE tsAuthorise sAuthorise;
PRIVATE teSysState eSysState;

/* Remote PDM data */
PRIVATE tsRemotePdm sRemotePdm;

/* PDM record descriptor */
PDM_tsRecordDescriptor   sDesc;

/* Sleep Mode */

PRIVATE bool_t bStackSleep;
PRIVATE uint32 u32ActivityTimer = USER_INACTIVE_TIME_MS;

PRIVATE uint8 u8GatewayRejoin;
PRIVATE bool_t bSave;
PRIVATE uint32 u32EnableFilterLed = 0x01;

PRIVATE eTouchButtonEvent eLastTouchButtonEvent;

PRIVATE volatile bool_t bTickEvent;

PRIVATE uint16 u16DeviceType = MK_JIP_DEVICE_TYPE;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PUBLIC void AppColdStart(void)
{
	PDM_teStatus   ePdmStatus;

	/* Not JN514x Chip family ? */
	#ifndef JENNIC_CHIP_FAMILY_JN514x
		/* Disable JTAG */
		vAHI_SetJTAGdebugger(FALSE, FALSE);
		/* Wait for 32Khz clock to run */
    	while(bAHI_Clock32MHzStable() == FALSE);
    #endif

	Exception_vInit();/* install exception handlers */

	/* Initialise all DIO as outputs and drive low */
	vAHI_DioSetDirection(0, ALL_DIO_MASK);
	vAHI_DioSetOutput(0,  ALL_DIO_MASK);

     vInitPeripherals();

    DBG_vPrintf(TRACE_RM, "\nAppColdStart");

    memset(&sDevice,0,sizeof(sDevice));
    /* Initialise peripherals */

	/* Initialise PDM */
	PDM_vInit(PDM_START_SECTOR,
			  PDM_NUM_SECTORS,
			  PDM_SECTOR_SIZE,
			  (OS_thMutex) 1,	/* Mutex */
			  NULL,
			  NULL,
			  NULL);

	/* Initialise PDM data */
	memset(&sRemotePdm, 0, sizeof(sRemotePdm));
	/* Load NodeStatus mib data */
	ePdmStatus = PDM_eLoadRecord(&sDesc,
#ifdef JENNIC_CHIP_FAMILY_JN514x
								 "Remote",
#else
								 (uint16) MK_JIP_DEVICE_TYPE,
#endif
								 (void *) &sRemotePdm,
								 sizeof(sRemotePdm),
								 FALSE);
	/* Valid parameters ? */
	if (PDM_E_STATUS_OK == ePdmStatus)
	{
		/* Record was not recovered from flash ? */
		if (PDM_RECOVERY_STATE_RECOVERED != sDesc.eState)
		{
			/* Default non-zero permanent status data */
			sRemotePdm.u8DeviceType = E_6LP_ROUTER;
			sRemotePdm.u8StackModeInit = 0;
		}
	}

	/* Debug */
	DBG_vPrintf(TRACE_RM, "\n\t u8DeviceType   = %d", sRemotePdm.u8DeviceType);
	DBG_vPrintf(TRACE_RM, "\n\t u8UpMode       = %d", sRemotePdm.u8UpMode);
	DBG_vPrintf(TRACE_RM, "\n\t u8Channel      = %d", sRemotePdm.u8Channel);
	DBG_vPrintf(TRACE_RM, "\n\tu16PanId        = %x", sRemotePdm.u16PanId);
	DBG_vPrintf(TRACE_RM, "\n\tu32FrameCounter = %d", sRemotePdm.u32FrameCounter);
	DBG_vPrintf(TRACE_RM, "\n\tasSecurityKey[0]   = %x:%x:%x:%x",
		sRemotePdm.asSecurityKey[0].u32KeyVal_1,
		sRemotePdm.asSecurityKey[0].u32KeyVal_2,
		sRemotePdm.asSecurityKey[0].u32KeyVal_3,
		sRemotePdm.asSecurityKey[0].u32KeyVal_4);
	DBG_vPrintf(TRACE_RM, "\n\tasSecurityKey[1]   = %x:%x:%x:%x",
		sRemotePdm.asSecurityKey[1].u32KeyVal_1,
		sRemotePdm.asSecurityKey[1].u32KeyVal_2,
		sRemotePdm.asSecurityKey[1].u32KeyVal_3,
		sRemotePdm.asSecurityKey[1].u32KeyVal_4);
	DBG_vPrintf(TRACE_RM, "\n\tasSecurityKey[2]   = %x:%x:%x:%x",
		sRemotePdm.asSecurityKey[2].u32KeyVal_1,
		sRemotePdm.asSecurityKey[2].u32KeyVal_2,
		sRemotePdm.asSecurityKey[2].u32KeyVal_3,
		sRemotePdm.asSecurityKey[2].u32KeyVal_4);

	/* Initialise RemoteConfigGroup MIB */
	MibRemoteConfigGroup_vInit(hMibRemoteConfigGroup, &sMibRemoteConfigGroup);

	/* Not JN514x family ? */
	#ifndef  JENNIC_CHIP_FAMILY_JN514x
	{
		/* PDM device data */
		PDM_tsRecordDescriptor   sDeviceDesc;
		tsDevicePdm			     sDevicePdm;

		/* Load Device PDM data */
		ePdmStatus = PDM_eLoadRecord(&sDeviceDesc,
#if defined(JENNIC_CHIP_FAMILY_JN514x)
									 "Device",
#else
									 0xFFFF,
#endif
									 (void *) &sDevicePdm,
									 sizeof(sDevicePdm),
									 FALSE);

		/* Record was recovered from flash ? */
		if (PDM_RECOVERY_STATE_RECOVERED == sDeviceDesc.eState)
		{
			/* Is the JIP Device ID unexpected ? */
			if (MK_JIP_DEVICE_ID != sDevicePdm.u32JipDeviceId)
			{
				/* Delete the PDM data and reset to start again */
				PDM_vDelete();
				/* Reset */
				vAHI_SwReset();
			}
		}
		/* Record was not recovered from flash */
		else
		{
			/* Set correct JIP Device ID */
			sDevicePdm.u32JipDeviceId = MK_JIP_DEVICE_ID;
			sDevicePdm.u32Spare       = 0xffffffff;
			/* Make sure permament data is saved */
			PDM_vSaveRecord(&sDeviceDesc);
		}
	}
	#endif

#ifdef MEASURE_SLEEP_CURRENT
    volatile uint32 i;
    vAHI_DioSetOutput(0,LED_DIO_MASK);
    for(i=0;i<2000000;i++);
    vAHI_DioSetOutput(LED_DIO_MASK,0);
    vAHI_UartDisable(E_AHI_UART_0);
    vAHI_UartDisable(E_AHI_UART_1);
    vAHI_FlashPowerDown();
    vAHI_Sleep(E_AHI_SLEEP_OSCON_RAMON);
#endif
    /* Initialise network */
    eSysState = E_STATE_NO_NWK;

	/* Start stack */
	vStartStack();

	/* Register RemoteConfigGroup MIB */
	MibRemoteConfigGroup_vRegister();
	/* Colour control instead of bulb ? */
	#if (MK_JIP_DEVICE_ID == 0x0801035C) || (MK_JIP_DEVICE_ID == 0x0801035E)	/* NXP RD6035 Colour ? */
	{
		/* Override all bulb group with all colour group address */
		MibRemoteConfigGroup_vBuildAddr(&sMibRemoteConfigGroup.sPerm.asAddr[0],
										(MAC_ExtAddr_s *) NULL,
										(uint16)(MIB_ID_COLOUR_CONTROL & 0xffff));
		/* Make sure permament data is saved */
		PDM_vSaveRecord(&sMibRemoteConfigGroup.sDesc);
	}
	#endif
	/* Save PDM data */
//	DBG_vPrintf(TRACE_RM, "\nvAppSave() 1");
	bSave = TRUE;
	vAppSave();

	/* Get background capacitance */
    while(eTouchProcess() ==  TOUCH_STATUS_DONT_SLEEP);

    vSetLedState(E_LED_STATE_JOINING);

    vAppMainLoop(FALSE);

    vAppSleep(TRUE);
}

PUBLIC void AppWarmStart(void)
{
	uint32 u32WakeTimerUnderRun  = 0;

		/* Not JN514x Chip family ? */
	#ifndef JENNIC_CHIP_FAMILY_JN514x
		/* Disable JTAG */
		vAHI_SetJTAGdebugger(FALSE, FALSE);
		/* Wait for 32Khz clock to run */
    	while(bAHI_Clock32MHzStable() == FALSE);
    #endif

	/* Initialise hardware interfaces */

	v6LP_InitHardware();

	#ifdef DBG_ENABLE
		DBG_vUartInit(UART_TO_DBG, BAUD_RATE_DBG);
	#else
		UART_vInit();
	#endif

	eTouchWake();

    /* stop wake timer (if running) & restart for a wake event 10ms in the future */
	vAHI_WakeTimerEnable(APP_WAKE_TIMER,TRUE);

	/* Account for any delay getting to here from Hardware Wake event.  The counter    */
	/* under-runs from 0 to MAX_WAKE_TIMER so we form the 2's complement of a negative */
	/* to get a positive number of additional cycles after the wake event to factor in */
	u32WakeTimerUnderRun = (uint32)((MAX_WAKE_TIMER ^ u64AHI_WakeTimerReadLarge(APP_WAKE_TIMER))+1);

	/* Schedule wake event 10ms in future using calibrated #cycles and event processing latency */
	vAHI_WakeTimerStartLarge(APP_WAKE_TIMER,(u32CalibaratedSleepTime-u32WakeTimerUnderRun));

	/* Default is to sleep without using stack function. If we go into a non-controlling mode  */
	/* eg commissioning we stay awake with the stack resumed and set the stack SleepFlag which */
	/* determines the execution termination sleep call                                         */
     bStackSleep = FALSE;
     vAppMainLoop(TRUE);
     vAppSleep(bStackSleep);
}

/****************************************************************************
 *
 * NAME: vAppMainLoop
 *
 * DESCRIPTION:
 * Main processing Loop. Executes repeatedly while stack is starting or
 * program is in non-control mode.  In control mode a single time slice
 * is given to the thread and wake events schedule the next slice.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vAppMainLoop(bool_t bWarmStart)
{
	bTickEvent = bWarmStart;
	do
    {
        if (bTickEvent == TRUE)
        {

            bTickEvent = FALSE;
            vLedTick();
            vKeyTick();
            vTouchChecker();
            /*Provide processing time to any active special remote modes */
            vDecommissionMode(&eSysState,E_EVENT_DECMSNG_TICK);
            (eSysState &  ANY_STATE_COMMISSIONING) ? vCommissionMode(&sDevice,&sAuthorise,&eSysState) : 0;
        }
        else
        {
           vAHI_CpuDoze();
           v6LP_Tick();
        }

    } while (eSysState != E_STATE_CONTROLLING);
}
/****************************************************************************
 *
 * NAME: vInitPeripherals
 *
 * DESCRIPTION:
 * Initialises peripherals and calibrates the wake timer
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vInitPeripherals(void)
{
    /* Initialise stack and hardware interfaces */

    v6LP_InitHardware();
    vLedInit();

#ifdef DBG_ENABLE
    /* Initialise debugging */
      DBG_vUartInit(UART_TO_DBG, BAUD_RATE_DBG);
      DBG_vPrintf(TRACE_RM, "\nnwk ID 0x%x\r\n", CONFIG_NETWORK_ID);
#else
      UART_vInit();
#endif

      eTouchInit();

    vAHI_WakeTimerStop(E_AHI_WAKE_TIMER_0);
    u32CalibaratedSleepTime = (uint32)(SLEEP_TIME_MS*TICKS_PER_MS*CPU_CAL_CYCLES)/u32AHI_WakeTimerCalibrate();

}

/****************************************************************************
 *
 * NAME: vStartStack
 *
 * DESCRIPTION:
 * Starts the 6LoWPAN stack, gets a socket and forms a binding. Any errors
 * at this point will lead to an error message and a lock-up.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vStartStack(void)
{
   // DBG_vPrintf(TRACE_RM, "\nRunning on chan %d", READ_REG32(0x10000E08));
  //  DBG_vPrintf(TRACE_RM, "\nGot %d buffers", sStackData.u16NumPacketBuffers);
  //  DBG_vPrintf(TRACE_RM, "\nNeed %d bytes for a buffer", u16_6LP_GetSizeOfIPv6record());

    /* Make sure we handle unknown protocols correctly (ie, we still route them onwards) */
//  v6LP_setAllowUnrecognisedNextHeaderFlag(TRUE);

    tsJIP_InitData sJipInitData;

	/* Configure jip */
	sJipInitData.eDeviceType               = sRemotePdm.u8DeviceType;
	sJipInitData.u64AddressPrefix          = CONFIG_ADDRESS_PREFIX; 		/* IPv6 address prefix (C only) */
	sJipInitData.u32Channel				= CONFIG_SCAN_CHANNELS;     	/* Channel 'bitmap' */
	sJipInitData.u16PanId					= CONFIG_PAN_ID;				/* PAN ID to use or look for (0xffff to search/generate) */
	sJipInitData.u16MaxIpPacketSize		= 0; /*CONFIG_PACKET_BUFFER_LEN-216;*/ /* Max IP packet size, 0 defaults to 1280 */
	sJipInitData.u16NumPacketBuffers		= 2;    						/* Number of IP packet buffers */
	sJipInitData.u8UdpSockets				= 2;           					/* Number of UDP sockets supported */
	sJipInitData.u32RoutingTableEntries	= 8;/*CONFIG_ROUTING_TABLE_ENTRIES;  Routing table size (not ED) */
	sJipInitData.u32DeviceId				= MK_JIP_DEVICE_ID;
	sJipInitData.u8UniqueWatchers			= CONFIG_UNIQUE_WATCHERS;
	sJipInitData.u8MaxTraps				= CONFIG_MAX_TRAPS;
	sJipInitData.u8QueueLength 			= CONFIG_QUEUE_LENGTH;
	sJipInitData.u8MaxNameLength			= CONFIG_MAX_NAME_LEN;
	sJipInitData.u16Port					= JIP_DEFAULT_PORT;
	sJipInitData.pcVersion 				= MK_VERSION;


	UART_vChar('i');


    if (eJIP_Init(&sJipInitData) == E_JIP_OK)
    {
	    v6LP_SetPacketDefragTimeout(1);
        // Do stuff
        vJIP_SetNodeName("Remote");
        DBG_vPrintf(TRACE_RM,"\nStarted JIP");
    }



}

PUBLIC void v6LP_ConfigureNetwork(tsNetworkConfigData *psNetworkConfigData)
{
	/* Debug */
	DBG_vPrintf(TRACE_RM, "\nv6LP_ConfigureNetwork()");

	UART_vChar('g');

	/* Set network id */
	Remote_vSetUserData();

	/* Security enabled ? */
	#if (MK_SECURITY != 0)
	{
		/* Set up data structures ready for key retrieval and insertion */
		v6LP_EnableSecurity();
	}
	#endif

	/* Were we in a gateway network and
	   have a valid network key ? */
	if ((sRemotePdm.u8UpMode                        						 	     == UP_MODE_GATEWAY) &&
		(sRemotePdm.asSecurityKey[SECURITY_KEY_NETWORK].u32KeyVal_1 != 0  ||
		 sRemotePdm.asSecurityKey[SECURITY_KEY_NETWORK].u32KeyVal_2 != 0  ||
		 sRemotePdm.asSecurityKey[SECURITY_KEY_NETWORK].u32KeyVal_3 != 0  ||
		 sRemotePdm.asSecurityKey[SECURITY_KEY_NETWORK].u32KeyVal_4 != 0))
	{
		/* Resume gateway operation */
		Remote_vResumeGateway(psNetworkConfigData);
	}
	/* Are we in a standalone network and
	   had a valid network key and
	   channel and
	   pan id */
	else if ((sRemotePdm.u8UpMode == UP_MODE_STANDALONE) &&
			 (sRemotePdm.asSecurityKey[SECURITY_KEY_NETWORK].u32KeyVal_1 != 0   ||
			  sRemotePdm.asSecurityKey[SECURITY_KEY_NETWORK].u32KeyVal_2 != 0   ||
			  sRemotePdm.asSecurityKey[SECURITY_KEY_NETWORK].u32KeyVal_3 != 0   ||
			  sRemotePdm.asSecurityKey[SECURITY_KEY_NETWORK].u32KeyVal_4 != 0)  &&
			  sRemotePdm.u8Channel      				 						  >= 11  &&
			  sRemotePdm.u8Channel					  	 						  <= 26  &&
			  sRemotePdm.u16PanId                       						  >  0   &&
			  sRemotePdm.u16PanId                       						  <  0xffff)
	{
		/* Resume standalone operation */
		Remote_vResumeStandalone(psNetworkConfigData);
	}
	/* Not restoring any security data ? */
	else
	{
		/* Revert to none up mode */
		sRemotePdm.u8UpMode = UP_MODE_NONE;
		/* Debug */
		DBG_vPrintf(TRACE_RM, "\n\tsRemotePdm.u8UpMode=%d", sRemotePdm.u8UpMode);
		/* Apply initialisation stack mode */
		vApi_SetStackMode(sRemotePdm.u8StackModeInit);
		/* Debug */
		DBG_vPrintf(TRACE_RM, "\n\tvApi_SetStackMode(%x)", sRemotePdm.u8StackModeInit);
		/* Set profile for stackmode */
		Remote_vSetProfile((sRemotePdm.u8StackModeInit & STACK_MODE_STANDALONE) ? TRUE : FALSE);

		/* Build default security keys as required */
		uint8 u8Key;
		/* Loop through keys */
		for (u8Key = 0; u8Key < 3; u8Key++)
		{
			/* Did we not load a valid commissioning key from flash ? */
			if ((sRemotePdm.asSecurityKey[u8Key].u32KeyVal_1 |
				 sRemotePdm.asSecurityKey[u8Key].u32KeyVal_2 |
				 sRemotePdm.asSecurityKey[u8Key].u32KeyVal_3 |
				 sRemotePdm.asSecurityKey[u8Key].u32KeyVal_4) == 0)
			{
				/* Commissioning key ? */
				if (u8Key != 0)
				{
					/* Generate commissioning key from MAC address */
					Security_vBuildCommissioningKey((uint8 *) pvAppApiGetMacAddrLocation(),
													(uint8 *) &sRemotePdm.asSecurityKey[u8Key]);
				}
				/* Network key ? */
				else
				{
					/* Production build ? */
					#if MK_PRODUCTION
					{
						/* Generate random key */
					    vAHI_StartRandomNumberGenerator(E_AHI_RND_SINGLE_SHOT,E_AHI_INTS_DISABLED);
					    while(!bAHI_RndNumPoll());
					    sRemotePdm.asSecurityKey[u8Key].u32KeyVal_1  = ((uint32) u16AHI_ReadRandomNumber() << 16);
					    vAHI_StartRandomNumberGenerator(E_AHI_RND_SINGLE_SHOT,E_AHI_INTS_DISABLED);
					    while(!bAHI_RndNumPoll());
					    sRemotePdm.asSecurityKey[u8Key].u32KeyVal_1 |= ((uint32) u16AHI_ReadRandomNumber());
					    vAHI_StartRandomNumberGenerator(E_AHI_RND_SINGLE_SHOT,E_AHI_INTS_DISABLED);
					    while(!bAHI_RndNumPoll());
					    sRemotePdm.asSecurityKey[u8Key].u32KeyVal_2  = ((uint32) u16AHI_ReadRandomNumber() << 16);
					    vAHI_StartRandomNumberGenerator(E_AHI_RND_SINGLE_SHOT,E_AHI_INTS_DISABLED);
					    while(!bAHI_RndNumPoll());
					    sRemotePdm.asSecurityKey[u8Key].u32KeyVal_2 |= ((uint32) u16AHI_ReadRandomNumber());
					    vAHI_StartRandomNumberGenerator(E_AHI_RND_SINGLE_SHOT,E_AHI_INTS_DISABLED);
					    while(!bAHI_RndNumPoll());
					    sRemotePdm.asSecurityKey[u8Key].u32KeyVal_3  = ((uint32) u16AHI_ReadRandomNumber() << 16);
					    vAHI_StartRandomNumberGenerator(E_AHI_RND_SINGLE_SHOT,E_AHI_INTS_DISABLED);
					    while(!bAHI_RndNumPoll());
					    sRemotePdm.asSecurityKey[u8Key].u32KeyVal_3 |= ((uint32) u16AHI_ReadRandomNumber());
					    vAHI_StartRandomNumberGenerator(E_AHI_RND_SINGLE_SHOT,E_AHI_INTS_DISABLED);
					    while(!bAHI_RndNumPoll());
					    sRemotePdm.asSecurityKey[u8Key].u32KeyVal_4  = ((uint32) u16AHI_ReadRandomNumber() << 16);
					    vAHI_StartRandomNumberGenerator(E_AHI_RND_SINGLE_SHOT,E_AHI_INTS_DISABLED);
					    while(!bAHI_RndNumPoll());
					    sRemotePdm.asSecurityKey[u8Key].u32KeyVal_4 |= ((uint32) u16AHI_ReadRandomNumber());
					}
					#else
					{
						/* Set default fixed network key - makes it hard to debug though */
						sRemotePdm.asSecurityKey[u8Key].u32KeyVal_1 =
						sRemotePdm.asSecurityKey[u8Key].u32KeyVal_2 =
						sRemotePdm.asSecurityKey[u8Key].u32KeyVal_3 =
						sRemotePdm.asSecurityKey[u8Key].u32KeyVal_4 = 0x00010001;
					}
					#endif
				}
				DBG_vPrintf(TRACE_RM, "\n\tasSecurityKey[%d]   = %x:%x:%x:%x",
					u8Key,
					sRemotePdm.asSecurityKey[u8Key].u32KeyVal_1,
					sRemotePdm.asSecurityKey[u8Key].u32KeyVal_2,
					sRemotePdm.asSecurityKey[u8Key].u32KeyVal_3,
					sRemotePdm.asSecurityKey[u8Key].u32KeyVal_4);
			}
		}

		/* Not a coordinator ? */
		if (sRemotePdm.u8DeviceType != E_6LP_COORDINATOR)
		{
			/* Standalone mode ? */
			if (sRemotePdm.u8StackModeInit & STACK_MODE_STANDALONE)
			{
				/* Use standalone commissioning key */
				Remote_vSetSecurityKey(SECURITY_KEY_STANDALONE_COMMISSIONING);
			}
			else
			{
				/* Use gateway commissioning key */
				Remote_vSetSecurityKey(SECURITY_KEY_GATEWAY_COMMISSIONING);
			}
		}
		/* Coordinator ? */
		else
		{
			/* Use network key */
			Remote_vSetSecurityKey(SECURITY_KEY_NETWORK);
		}
	}
}

/****************************************************************************
 *
 * NAME: Remote_vResumeGateway
 *
 * DESCRIPTION:
 * REsumes running in a gateway system
 *
 ****************************************************************************/
PUBLIC void Remote_vResumeGateway(tsNetworkConfigData *psNetworkConfigData)
{
	MAC_DeviceDescriptor_s sDeviceDescriptor;

	/* Debug */
	DBG_vPrintf(TRACE_RM, "\nRemote_u8ResumeGateway()");

	/* Use network key */
	Remote_vSetSecurityKey(SECURITY_KEY_NETWORK);
	/* Advance our frame counter */
	sRemotePdm.u32FrameCounter += CONFIG_FRAME_COUNTER_DELTA;
	/* Build security descriptor for this node */
	sDeviceDescriptor.u16PanId  	  = sRemotePdm.u16PanId;
	sDeviceDescriptor.u16Short  	  = 0xfffe;
	memcpy(&sDeviceDescriptor.sExt, pvAppApiGetMacAddrLocation(), sizeof(MAC_ExtAddr_s));
	sDeviceDescriptor.u32FrameCounter = sRemotePdm.u32FrameCounter;
	sDeviceDescriptor.bExempt 		  = FALSE;
	/* Restore security descriptor for this node */
	(void) bSecuritySetDescriptor(0, &sDeviceDescriptor);
	/* Debug */
	DBG_vPrintf(TRACE_RM, "\n\tbSecuritySetDescriptor(0, %x:%x, %d)",
		sDeviceDescriptor.sExt.u32H,
		sDeviceDescriptor.sExt.u32L,
		sDeviceDescriptor.u32FrameCounter);
	/* Revert to none up mode */
	sRemotePdm.u8UpMode = UP_MODE_NONE;
	/* Debug */
	DBG_vPrintf(TRACE_RM, "\n\tsRemotePdm.u8UpMode=%d", sRemotePdm.u8UpMode);

	/* Were we a coordinator ? */
	if (sRemotePdm.u8DeviceType == E_6LP_COORDINATOR)
	{
		/* Need to override scan mask to force us back onto previous channel */
		psNetworkConfigData->u32ScanChannels = (1<<sRemotePdm.u8Channel);
		/* Need to override PAN ID to force us back into previous PAN */
		psNetworkConfigData->u16PanID = sRemotePdm.u16PanId;
	}
	/* Not a coordinator ? */
	else
	{
		/* Make three attempts to rejoin gateway with current network key */
		u8GatewayRejoin = 3;
	}
}

/****************************************************************************
 *
 * NAME: Remote_vResumeStandalone
 *
 * DESCRIPTION:
 * Resumes running in a standalone system
 *
 ****************************************************************************/
PUBLIC void Remote_vResumeStandalone(tsNetworkConfigData *psNetworkConfigData)
{
	MAC_DeviceDescriptor_s sDeviceDescriptor;
	uint8 u8Restored = 0;

	/* Debug */
	DBG_vPrintf(TRACE_RM, "\nRemote_u8ResumeStandalone()");

	/* Use network key */
	Remote_vSetSecurityKey(SECURITY_KEY_NETWORK);
	/* Advance our frame counter */
	sRemotePdm.u32FrameCounter += CONFIG_FRAME_COUNTER_DELTA;
	/* Build security descriptor for this node */
	sDeviceDescriptor.u16PanId  	  = sRemotePdm.u16PanId;
	sDeviceDescriptor.u16Short  	  = 0xfffe;
	memcpy(&sDeviceDescriptor.sExt, pvAppApiGetMacAddrLocation(), sizeof(MAC_ExtAddr_s));
	sDeviceDescriptor.u32FrameCounter = sRemotePdm.u32FrameCounter;
	sDeviceDescriptor.bExempt 		  = FALSE;
	/* Restore security descriptor for this node */
	(void) bSecuritySetDescriptor(u8Restored, &sDeviceDescriptor);
	/* Debug */
	DBG_vPrintf(TRACE_RM, "\n\tbSecuritySetDescriptor(%d, %x:%x, %d)",
		u8Restored,
		sDeviceDescriptor.sExt.u32H,
		sDeviceDescriptor.sExt.u32L,
		sDeviceDescriptor.u32FrameCounter);
	/* Set profile for standalone system */
	Remote_vSetProfile(TRUE);
	/* Start in standalone mode */
	vApi_SetStackMode(STACK_MODE_STANDALONE);
	/* Debug */
	DBG_vPrintf(TRACE_RM, "\n\tvApi_SetStackMode(%x)", STACK_MODE_STANDALONE);
	/* Were we a coordinator ? */
	if (sRemotePdm.u8DeviceType == E_6LP_COORDINATOR)
	{
		/* Need to override scan mask to force us back onto previous channel */
		psNetworkConfigData->u32ScanChannels = (1<<sRemotePdm.u8Channel);
		/* Need to override PAN ID to force us back into previous PAN */
		psNetworkConfigData->u16PanID = sRemotePdm.u16PanId;
	}
	/* Not a coordinator ? */
	else
	{
		/* Note network is up */
		bNwkJoined = TRUE;
		/* Skip the normal joining process */
		vApi_SkipJoin(sRemotePdm.u16PanId, sRemotePdm.u8Channel);
		/* Debug */
		DBG_vPrintf(TRACE_RM, "\n\tvApi_SkipJoin(%x, %d)", sRemotePdm.u16PanId, sRemotePdm.u8Channel);
		/* Update system state and LEDs */
		eSysState = E_STATE_CONTROLLING;
		vSetLedState(E_LED_STATE_JOINED);
	}
}

/****************************************************************************
 *
 * NAME: Remote_vSetSecurityKey
 *
 * DESCRIPTION:
 * Set security key to apply
 *
 ****************************************************************************/
PUBLIC void Remote_vSetSecurityKey(uint8 u8Key)
{
	/* Security enabled ? */
	#if (MK_SECURITY != 0)
	{
		/* Debug */
		DBG_vPrintf(TRACE_RM, "\nRemote_vSetSecurityKey(%d)", u8Key);
		/* Valid key ? */
		if (u8Key < 3)
		{
			/* Setting network key ? */
			if (u8Key == SECURITY_KEY_NETWORK)
			{
				/* Invalidate commissioning key */
				vSecurityInvalidateKey(1);
				/* Debug */
				DBG_vPrintf(TRACE_RM, "\n\tvSecurityInvalidateKey(1)");
				/* Apply network key */
				vApi_SetNwkKey(0, &sRemotePdm.asSecurityKey[u8Key]);
				/* Debug */
				DBG_vPrintf(TRACE_RM, "\n\tvApi_SetNwkKey(0, %x:%x:%x:%x)",
					sRemotePdm.asSecurityKey[u8Key].u32KeyVal_1,
					sRemotePdm.asSecurityKey[u8Key].u32KeyVal_2,
					sRemotePdm.asSecurityKey[u8Key].u32KeyVal_3,
					sRemotePdm.asSecurityKey[u8Key].u32KeyVal_4);
			}
			/* Setting a commissioning key ? */
			else
			{
				/* Invalidate network key */
				vSecurityInvalidateKey(0);
				/* Debug */
				DBG_vPrintf(TRACE_RM, "\n\tvSecurityInvalidateKey(0)");
				/* Apply commissioning key */
				vApi_SetNwkKey(1, &sRemotePdm.asSecurityKey[u8Key]);
				/* Debug */
				DBG_vPrintf(TRACE_RM, "\n\tvApi_SetNwkKey(1, %x:%x:%x:%x)",
					sRemotePdm.asSecurityKey[u8Key].u32KeyVal_1,
					sRemotePdm.asSecurityKey[u8Key].u32KeyVal_2,
					sRemotePdm.asSecurityKey[u8Key].u32KeyVal_3,
					sRemotePdm.asSecurityKey[u8Key].u32KeyVal_4);
			}
		}
	}
	#endif
}

/****************************************************************************
 *
 * NAME: Remote_vSetProfile
 *
 * DESCRIPTION:
 * Set network operating profile according to network mode
 *
 ****************************************************************************/
PUBLIC void Remote_vSetProfile(bool_t bStandalone)
{
	/* Debug */
	DBG_vPrintf(TRACE_RM, "\nRemote_vSetProfile(%d)", bStandalone);

	/* Standalone system ? */
	if (bStandalone)
	{
		tsNwkProfile sNwkProfile;

		/* Read network profile */
		vJnc_GetNwkProfile(&sNwkProfile);
		/* Inhibit pings */
		sNwkProfile.u8MaxFailedPkts = 0;
		sNwkProfile.u16RouterPingPeriod = 0;
		/* Apply as user profile */
		(void) bJnc_SetRunProfile(PROFILE_USER, &sNwkProfile);
		/* Debug */
		DBG_vPrintf(TRACE_RM, "\n\tbJnc_SetRunProfile(USER)");
		/* Inhibit End Device activity timeout */
		//psMibNwkSecurity->psNetworkConfigData->u32EndDeviceActivityTimeout	= 0;
	}
	/* Gateway system ? */
	else
	{
		/* Apply default run profile for joining gateway system - will be updated if necessary upon joining */
		(void) bJnc_SetRunProfile(0, NULL);
		/* Debug */
		DBG_vPrintf(TRACE_RM, "\n\tbJnc_SetRunProfile(0)");
		/* Apply gateway end device activity timeout settings */
		//psMibNwkSecurity->psNetworkConfigData->u32EndDeviceActivityTimeout	= psMibNwkSecurity->u32EndDeviceActivityTimeout;
	}
}

/****************************************************************************
 *
 * NAME: v6LP_DataEvent
 *
 * DESCRIPTION:
 * Deals with any incoming data events. In this application we only expect
 * to receive packets (E_DATA_RECEIVED). When a packet arrives, it is placed
 * in a queue for retrieval later int he application idle loop.
 *
 * PARAMETERS: Name        RW  Usage
 *             iSocket     R   Socket on which packet received
 *             eEvent      R   Data event
 *             psAddr      R   Source address (for RX) or destination (for TX)
 *             u8AddrLen   R   Length of address
 *
 ****************************************************************************/
PUBLIC void v6LP_DataEvent(int iSocket,
                           te6LP_DataEvent eEvent,
                           ts6LP_SockAddr *psAddr,
                           uint8 u8AddrLen)
{
    switch(eEvent)
    {
    case E_DATA_SENT:
        DBG_vPrintf(TRACE_RM, "<Sent>");
        break;

    case E_DATA_SEND_FAILED:
        DBG_vPrintf(TRACE_RM, "<Send Fail>");
        break;

    case E_DATA_RECEIVED:
        DBG_vPrintf(TRACE_RM, "<RXD>");
        /* Discard 6LP packets as only interested in JIP communication */
        i6LP_RecvFrom(iSocket, NULL, 0, 0, NULL, NULL);
        break;

    case E_6LP_ICMP_MESSAGE:
        DBG_vPrintf(TRACE_RM, "<RXICMP>");
        /* Discard 6LP packets as only interested in JIP communication */
        i6LP_RecvFrom(iSocket, NULL, 0, 0, NULL, NULL);
        break;

    case E_IP_DATA_RECEIVED:
        DBG_vPrintf(TRACE_RM, "<GRX %d>", u16_6LP_GetNumberOfAvailableIPv6Buffers());
        /* Discard 6LP packets as only interested in JIP communication */
        i6LP_RecvFrom(iSocket, NULL, 0, 0, NULL, NULL);
        break;

    default:
        DBG_vPrintf(TRACE_RM, "\n? in v6LP_DataEvent");
        break;
    }
}

/****************************************************************************
 *
 * NAME: v6LP_PeripheralEvent
 *
 * DESCRIPTION:
 * Processes any incoming peripheral events to see if the event tick timer
 * as this is the only peripheral event we respond to. If the
 * timer has fired (every 10ms) update system timers to
 *
 * a) Decrement the user commissioning duration Timer
 * b) Decrement the Node commissioning Timeout Timer
 *
 * PARAMETERS: Name          RW Usage
 *             u32Device     R  Device that caused peripheral event
 *             u32ItemBitmap R  Events within that peripheral
 *
 ****************************************************************************/
PUBLIC void v6LP_PeripheralEvent(uint32 u32Device, uint32 u32ItemBitmap)
{
    if (u32Device == E_AHI_DEVICE_TICK_TIMER)
    {
    	bTickEvent = TRUE;
    	if (++sDevice.sTimers.u8Ticks>9)
    	{
    		sDevice.sTimers.u8Ticks = 0;
    		if(++sDevice.sTimers.u8Tenths>9)	/* decrement any active seconds-based guard timers */
    		{
    			sDevice.sTimers.u8Tenths = 0;

    			vAHI_WatchdogRestart();

    			if (&sDevice.sTimers.u16CmsngDuration>0)     /* 5min (300 second) User Commissioning duration timeout */
    			{
    				sDevice.sTimers.u16CmsngDuration--;
    				if (sDevice.sTimers.u16CmsngDuration ==0)
    				{
    					bStackSleep = TRUE;
    				}
    			}

    			if (sDevice.sTimers.u8CmsngTimeout>0)      /* 15 second Individual Node commissioning Timeout      */
    			{
    				sDevice.sTimers.u8CmsngTimeout--;
    				if (sDevice.sTimers.u8CmsngTimeout==0)
    				{
    					sAuthorise.eAuthState = E_STATE_CMSNG_IDLE;  /* Reset Node Commissioning State machine */
						sDevice.sTimers.u8SetVarTimeout = 0;
						bStackSleep = TRUE;
						DBG_vPrintf(TRACE_RM, "\nTIMEOUT CMSNG_IDLE");
    				}
    			}
    		}
    	}
    	/* Set var timer running ? */
    	if (sDevice.sTimers.u8SetVarTimeout > 0)
    	{
    		/* Decrement */
    		sDevice.sTimers.u8SetVarTimeout--;
    		/* Timer expired ? */
    		if (sDevice.sTimers.u8SetVarTimeout == 0)
    		{
    			/* Group send in progress ? */
    			if (sAuthorise.eAuthState == E_STATE_CMSNG_SENDGROUP_INPRG)
    			{
    				/* Try again */
    				sAuthorise.eAuthState = E_STATE_CMSNG_SENDGROUP_START;
    				/* Debug */
					DBG_vPrintf(TRACE_RM, "\nSENDGROUP timeout");
    			}
    			/* Finish send in progress ? */
    			else if (sAuthorise.eAuthState == E_STATE_CMSNG_FINISH_INPRG)
    			{
    				/* Try again */
    				sAuthorise.eAuthState = E_STATE_CMSNG_FINISH_START;
    				/* Debug */
					DBG_vPrintf(TRACE_RM, "\nFINISH timeout");
    			}
    		}
    	}
    	/* Learning timer running ? */
    	if (sDevice.sTimers.u16LearningTimeout > 0)
    	{
    		/* Decrement */
    		sDevice.sTimers.u16LearningTimeout--;
   			/* Learning in progress ? */
   			if (eSysState == E_STATE_LEARNING)
   			{
	    		/* Timer expired ? */
	    		if (sDevice.sTimers.u16LearningTimeout == 0)
	    		{

					UART_vChar('f');

					/* Joined a gateway network ? */
					if ((u16Api_GetStackMode() & STACK_MODE_STANDALONE) == 0)
					{
						/* Update state */
						eSysState = E_STATE_CONTROLLING;
						/* Debug */
						DBG_vPrintf(TRACE_RM, "\nLearning finished");
						vSetLedState(E_LED_STATE_JOINED);
					}
					/* Joined a standlone system ? */
					else
					{
						/* Finished learning ? */
						if (sMibRemoteConfigGroup.sTemp.u8Finish != 0)
						{
							/* Update state */
							eSysState = E_STATE_CONTROLLING;
							/* Debug */
							DBG_vPrintf(TRACE_RM, "\nLearning finished");
							vSetLedState(E_LED_STATE_JOINED);
							/* Reset to coordinator */
							sRemotePdm.u8DeviceType = E_6LP_COORDINATOR;
							bSave = TRUE;
							vAppSave();
						}
						else
						{
							/* Debug */
							DBG_vPrintf(TRACE_RM, "\nLearning timeout");
							/* Erase persistent data */
							//PDM_vDelete();
							Remote_vPdmErase();
						}
						/* Debug */
						DBG_vPrintf(TRACE_RM, "\nvAHI_SwReset()        ");
						/* Reset */
						vAHI_SwReset();
					}
	   			}
    		}
    	}
    }
}

/****************************************************************************
 *
 * NAME: vJIP_StackEvent
 *
 * DESCRIPTION:
 * Processes any incoming stack events. This is involved with 802.15.4 MAC
 * stack activity, and is normally not important to the application on a
 * coordinator.
 *
 * PARAMETERS: Name          RW Usage
 *             eEvent        R  Stack event
 *             pu8Data       R  Additional information associated with event
 *             u8DataLen     R  Length of additional information
 *
 ****************************************************************************/
PUBLIC void vJIP_StackEvent(te6LP_StackEvent eEvent, uint8 *pu8Data, uint8 u8DataLen)
{
 // DBG_vPrintf(DBG_CALLBACKS, "%s(%s)\n", __FUNCTION__, apcStackEvents[eEvent]);

    switch (eEvent)
    {
		/* Started or joined ? */
		case E_STACK_STARTED:
		case E_STACK_JOINED:
		{

			tsNwkInfo *psNwkInfo;
			/* Cast data pointer to correct type */
			psNwkInfo = (tsNwkInfo *) pu8Data;

			UART_vChar('S');
			/* Started (created a network) ? */
			if (eEvent == E_STACK_STARTED)
			{
			UART_vChar('s');

				/* Debug */
				DBG_vPrintf(TRACE_RM, "\nvJIP_StackEvent(STARTED)");
				/* Update state */
				eSysState = E_STATE_CONTROLLING;
   				/* Debug */
				DBG_vPrintf(TRACE_RM, "\nCONTROLLING");
				vSetLedState(E_LED_STATE_JOINED);
			}
			/* Joined (an existing network) ? */
			else
			{
				UART_vChar('j');

				/* Debug */
				DBG_vPrintf(TRACE_RM, "\nvJIP_StackEvent(JOINED)");
				/* Joined a gateway network ? */
				if ((u16Api_GetStackMode() & STACK_MODE_STANDALONE) == 0)
				{
					/* Update state */
					eSysState = E_STATE_LEARNING;
    				/* Debug */
					DBG_vPrintf(TRACE_RM, "\nLEARNING");
					/* Set learning timer 40 seconds */
					sDevice.sTimers.u16LearningTimeout = 4000;
		          	/* Don't take on children */
				  	vApi_SetAssociationState(FALSE);
				  	/* Set LED state */
					vSetLedState(E_LED_STATE_LEARNING);
				}
				/* Joined a standalone system ? */
				else
				{
					/* Update state */
					eSysState = E_STATE_LEARNING;
    				/* Debug */
					DBG_vPrintf(TRACE_RM, "\nLEARNING");
					/* Set learning timer 10 seconds */
					sDevice.sTimers.u16LearningTimeout = 1000;
				}
			}

			/* Set network id */
			Remote_vSetUserData();

			/* Note the channel and PAN ID */
			sRemotePdm.u8Channel = psNwkInfo->u8Channel;
			sRemotePdm.u16PanId  = psNwkInfo->u16PanID;
			/* Not a coordinator ? */
			if (sRemotePdm.u8DeviceType != E_6LP_COORDINATOR)
			{
				/* Take a copy of the network key */
				memcpy(&sRemotePdm.asSecurityKey[SECURITY_KEY_NETWORK], psApi_GetNwkKey(), sizeof(tsSecurityKey));
				/* Invalidate commissioning key */
				vSecurityInvalidateKey(1);
				/* Debug */
				DBG_vPrintf(TRACE_RM, "\n\tvSecurityInvalidateKey(1)");
			}
			/* Have we joined in standalone mode ? */
			if (u16Api_GetStackMode() & STACK_MODE_STANDALONE)
			{
				/* Make sure we are not in commissioning mode */
				vApi_SetStackMode(STACK_MODE_STANDALONE);
				/* Debug */
				DBG_vPrintf(TRACE_RM, "\n\tvApi_SetStackMode(%x)", STACK_MODE_STANDALONE);
				/* Note that we are in a standalone network */
				sRemotePdm.u8UpMode = UP_MODE_STANDALONE;
			}
			/* Must be in gateway mode ? */
			else
			{
				/* Note that we are in a standalone network */
				sRemotePdm.u8UpMode = UP_MODE_GATEWAY;
			}
			/* Debug */
			DBG_vPrintf(TRACE_RM, "\n\tsRemotePdm.u8UpMode=%d", 	   sRemotePdm.u8UpMode);

			/* Save PDM data */
//			DBG_vPrintf(TRACE_RM, "\nvAppSave() 2");
			bSave = TRUE;
			vAppSave();
		}
	  	break;

		case E_STACK_RESET:
		{
			UART_vChar('r');

			/* Debug */
			DBG_vPrintf(TRACE_RM, "\nvJIP_StackEvent(RESET)");

			/* Not a coordinator ? */
			if (sRemotePdm.u8DeviceType != E_6LP_COORDINATOR)
			{
				/* Not joined ? */
				if (eSysState == E_STATE_NO_NWK)
				{
					/* Trying to rejoin a gateway network ? */
					if (u8GatewayRejoin > 0)
					{
						/* Decrement rejoin counter */
						u8GatewayRejoin--;
						/* Debug */
						DBG_vPrintf(TRACE_RM, "\n\tu8GatewayRejoin=%d", u8GatewayRejoin);
					}

					/* Not trying to rejoin a gateway network ? */
					if (u8GatewayRejoin == 0)
					{
						/* Were we trying to join in standalone mode ? */
						if (u16Api_GetStackMode() & STACK_MODE_STANDALONE)
						{
							/* Set profile for gateway system */
							Remote_vSetProfile(FALSE);
							/* Use gateway commissioning key */
							Remote_vSetSecurityKey(SECURITY_KEY_GATEWAY_COMMISSIONING);
							/* Swap to gateway mode */
							vApi_SetStackMode(0);
							/* Debug */
							DBG_vPrintf(TRACE_RM, "\n\tSwap to gateway stack reset");
							/* Debug */
							DBG_vPrintf(TRACE_RM, "\n\tvApi_SetStackMode(%x)", 0);
						}
					}
				}
				/* Network was up so we've lost it ? */
				else
				{
					/* Lost standalone system (shouldn't happen!!!) ? */
					if (u16Api_GetStackMode() & STACK_MODE_STANDALONE)
					{
						/* Unexpected event - do nothing */
						;
					}
					/* Lost gateway system ? */
					else
					{
						/* Make three attempts to rejoin gateway with current network key */
						u8GatewayRejoin = 3;
					}
				}
			}
			/* Coordinator (shouldn't happen!!!) ? */
			else
			{
				/* Return unexpected event - do nothing */
				;
			}

			/* Update state */
			eSysState = E_STATE_NO_NWK;
			/* Revert to none up mode */
			sRemotePdm.u8UpMode = UP_MODE_NONE;
			/* Debug */
			DBG_vPrintf(TRACE_RM, "\n\tsRemotePdm.8UpMode=%d", 	   sRemotePdm.u8UpMode);

			/* Save PDM data */
//			DBG_vPrintf(TRACE_RM, "\nvAppSave() 3");
			bSave = TRUE;
			vAppSave();
		}
		break;

		case E_STACK_NODE_JOINED:
        DBG_vPrintf(TRACE_RM, "\nE:Node Joined");
        if(sAuthorise.eAuthState == E_STATE_CMSNG_INPRG)	// Commissioning in progress
		{
        	if((((tsAssocNodeInfo*)pu8Data)->sMacAddr.u32H == sAuthorise.sAddr.u32H)&&
	           (((tsAssocNodeInfo*)pu8Data)->sMacAddr.u32L == sAuthorise.sAddr.u32L))	// And the child joining was being commissioned
	        {
	        	/* Commissioning bulbs ? */
	        	if (eSysState == E_STATE_COMMISSION_BULB)
	        	{
					/* Start with first group */
					sAuthorise.u8Group = 0;
					/* Start sending groups */
				   	sAuthorise.eAuthState = E_STATE_CMSNG_SENDGROUP_START;	// Commissioning done, now inject group
				   	DBG_vPrintf(TRACE_RM, "\nCOMMISSION_BULB CMSNG_SENDGROUP_START");
				   	/* Reset timer for actual commissioning */
				   	sDevice.sTimers.u8CmsngTimeout = CMSNG_TIMEOUT_S;
	        	}
	        	/* Commissioning border routers ? */
	        	else if (eSysState == E_STATE_COMMISSION_BR)
	        	{
					/* End commissioning */
					sAuthorise.eAuthState = E_STATE_CMSNG_IDLE;
					DBG_vPrintf(TRACE_RM, "\nCMSNG_IDLE 2");
					 // If we are running in non gateway mode break the link
					if(u16Api_GetStackMode() & STACK_MODE_STANDALONE)
					{
						vSetLedState(E_LED_STATE_ON);
					  /* CAUTION! After issuing this we can't unicast to device */
					   vNwk_DeleteChildVect(&sAuthorise.sAddr);
					   DBG_vPrintf(TRACE_RM,"\nvNwk_DeleteChildVect(%s)", __FUNCTION__);
					}
	        	}
	        	/* Commissioning remotes ? */
				else if (eSysState == E_STATE_COMMISSION_REMOTE)
	        	{
					/* Start with first group */
					sAuthorise.u8Group = 0;
					/* No groups to send just finish commissioning */
				   	sAuthorise.eAuthState = E_STATE_CMSNG_FINISH_START;	// Commissioning done
				   	DBG_vPrintf(TRACE_RM, "\nCLONE_REMOTE CMSNG_FINISH_START");
				   	/* Reset timer for actual commissioning */
				   	sDevice.sTimers.u8CmsngTimeout = CMSNG_TIMEOUT_S;
	        	}
	        	/* Cloning remotes ? */
				else if (eSysState == E_STATE_CLONE_REMOTE)
	        	{
					/* Start with first group */
					sAuthorise.u8Group = 0;
					/* Start sending groups */
				   	sAuthorise.eAuthState = E_STATE_CMSNG_SENDGROUP_START;	// Commissioning done, now inject group
				   	DBG_vPrintf(TRACE_RM, "\nCOMMISSION_REMOTE CMSNG_SENDGROUP_START");
				   	/* Reset timer for actual commissioning */
				   	sDevice.sTimers.u8CmsngTimeout = CMSNG_TIMEOUT_S;
	        	}
				/* Something else ? */
				else
				{
					/* End commissioning */
					sAuthorise.eAuthState = E_STATE_CMSNG_IDLE;
					DBG_vPrintf(TRACE_RM, "\nUNKNOWN DEVICE CMSNG_IDLE 3 %x", ((tsAssocNodeInfo*)pu8Data)->u32DeviceClass);
				   	/* Cancel commissioning timer */
				   	sDevice.sTimers.u8CmsngTimeout = 0;
					 // If we are running in non gateway mode break the link
					if(u16Api_GetStackMode() & STACK_MODE_STANDALONE)
					{
						vSetLedState(E_LED_STATE_ON);
					  /* CAUTION! After issuing this we can't unicast to device */
					   vNwk_DeleteChildVect(&sAuthorise.sAddr);
					   DBG_vPrintf(TRACE_RM,"\nvNwk_DeleteChildVect(%s)", __FUNCTION__);
					}
				}
			}
		}
        break;

      case E_STACK_NODE_AUTHORISE:
      {
    	  DBG_vPrintf(TRACE_RM, "\nE:Auth request sAuthorise.eAuthState=%d", sAuthorise.eAuthState);
    	  MAC_ExtAddr_s sAddr;

    	  memcpy((uint8*)&sAddr,pu8Data,sizeof(MAC_ExtAddr_s));	// Get address
    	  if(sAuthorise.eAuthState == E_STATE_CMSNG_IDLE)// Commission this node
    	  {
    		  vSetLedState(E_LED_STATE_OFF);
    		  memcpy((uint8*)&sAuthorise.sAddr,&sAddr,sizeof(MAC_ExtAddr_s));
    		  sAuthorise.eAuthState = E_STATE_CMSNG_START;
    		  DBG_vPrintf(TRACE_RM, "\nCMSNG_START 1");
	  	  }
    	  else
    	  {
    		  if(memcmp(&sAddr,&sAuthorise.sAddr,sizeof(MAC_ExtAddr_s))==0) // Is this a duplicate request
    		  {
    			  // Retry commissioning. Assume key didn't get there
    			  sAuthorise.eAuthState = E_STATE_CMSNG_START;
	    		  DBG_vPrintf(TRACE_RM, "\nCMSNG_START 2");
    		  }
    	  }
      }
      break;
      default:
    	  break;
    }
}

/****************************************************************************
 *
 * NAME: vJIP_Remote_DataSent
 *
 * DESCRIPTION:
 *
 *
 * PARAMETERS: Name                     RW  Usage
 *             *psAddr
 *             u8Handle
 *             eStatus                  R   Response status
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vJIP_Remote_DataSent(ts6LP_SockAddr *psAddr,
                                 teJIP_Status eStatus)
{
    DBG_vPrintf(TRACE_RM, "\nCB: RemoteDataSent=%d", eStatus);

    if (eStatus != E_JIP_OK)
	{
    	uint32 u32ErrCode;
		u32ErrCode = u32_6LP_GetErrNo();
		DBG_vPrintf(TRACE_RM,  "\ncode: %d",(u32ErrCode & 0xff));
		DBG_vPrintf(TRACE_RM,  "\ninfo: %d",((u32ErrCode >>8) & 0xff));
	}

	UART_vChar('J'+eStatus);
    vSetSafetoSleep();
    bStackSleep = TRUE;
}


/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: vAppSleep
 *
 * DESCRIPTION: Sends the unit to sleep
 *
 ****************************************************************************/
PRIVATE void vAppSleep(bool_t bSleepType)
{

	/* Save PDM data if necessary */
//	DBG_vPrintf(TRACE_RM, "\nvAppSave() 4");
	vAppSave();

#if DBG_ENABLE
	//let any debug out
	while ((u8AHI_UartReadLineStatus(E_AHI_UART_0) & E_AHI_UART_LS_THRE) == 0);
	while ((u8AHI_UartReadLineStatus(E_AHI_UART_0) & E_AHI_UART_LS_TEMT) == 0);
	/* Disable UART */
	vAHI_UartDisable(E_AHI_UART_0);
#endif
    /* Decrement the activity timer, Any key press resets to inactive timeout */
    /* If non-zero we sleep between scanning the keypads, otherwise we sleep  */
    /* until the user presses the power on button                             */

    if (u32ActivityTimer >= SLEEP_TIME_MS)
    {
    	/* Only want to wake up from the timer not the dio! */
		vAHI_DioWakeEnable(0,DIO_WAKEMASK);
		u32ActivityTimer -= (SLEEP_TIME_MS * DIO_WAKEMASK);

    	if (bSleepType)
		{

			/* Get stack to sleep */
    		UART_vChar('Z');
    		vAHI_UartDisable(E_AHI_UART_0);

			v6LP_Sleep(TRUE, SLEEP_TIME_MS);
		}
		else
		{
			eTouchSleep();

			/* If we fired timer while awake reschedule a future wake event */
			if ((u8AHI_WakeTimerFiredStatus() & E_AHI_WAKE_TIMER_MASK_0) !=0)
			{
				vAHI_WakeTimerEnable(APP_WAKE_TIMER,TRUE);
				vAHI_WakeTimerStartLarge(APP_WAKE_TIMER,u32CalibaratedSleepTime);
			}
			UART_vChar('z');
			vAHI_UartDisable(E_AHI_UART_0);


			vAHI_Sleep(E_AHI_SLEEP_OSCON_RAMON);
		}
    }
    else /* go to sleep and await wake button pressed */
    {

        DBG_LED_OFF;
        /* turn the LED DIO into an input */
        vAHI_DioSetDirection(DIO_WAKEMASK,0);
        vAHI_DioSetPullup(DIO_WAKEMASK,0);
        vAHI_DioWakeEdge(0,DIO_WAKEMASK);
        vAHI_DioWakeEnable(DIO_WAKEMASK,0);

         /* Make sure we clear any legacy timer/DIO wakes as only interested in NEXT DIO wake */
        vAHI_WakeTimerStop(APP_WAKE_TIMER);
        (void) u8AHI_WakeTimerFiredStatus();
        (void) u32AHI_DioWakeStatus();

		UART_vChar('x');
		vAHI_UartDisable(E_AHI_UART_0);

        u32ActivityTimer = USER_INACTIVE_TIME_MS; /* reset the timer if we press a key */

        vAHI_Sleep(E_AHI_SLEEP_OSCON_RAMON);
    }
}

/****************************************************************************
 *
 * NAME: vAppSave
 *
 * DESCRIPTION: Saves PDM data
 *
 ****************************************************************************/
PUBLIC void vAppSave(void)
{
	MAC_Pib_s *psPib;

	/* Get pointer to pib */
	psPib = MAC_psPibGetHandle(pvAppApiGetMacHandle());

	/* Has our frame counter advanced since we last saved it ? */
	if (psPib->u32MacFrameCounter > sRemotePdm.u32FrameCounter)
	{
		/* Has it advanced far enough to be worth saving ? */
		if (psPib->u32MacFrameCounter - sRemotePdm.u32FrameCounter >= CONFIG_FRAME_COUNTER_DELTA)
		{
			/* Flag we want to save data */
			bSave = TRUE;
		}
	}
	/* Want to save data */
	if (bSave)
	{
		/* Clear flag */
		bSave = FALSE;
		/* Update PDM frame counter */
		sRemotePdm.u32FrameCounter = psPib->u32MacFrameCounter;
		/* Save PDM data */
		PDM_vSaveRecord(&sDesc);
		/* Debug */
	    DBG_vPrintf(TRACE_RM, "\nPDM_vSaveRecord() fc=%d", sRemotePdm.u32FrameCounter);
	}
}

/****************************************************************************
 *
 * NAME: vCbTouchEventButton
 *
 * DESCRIPTION:
 *
 * Callback  function for the touch driver to called when
 * a button event is detected.
 *
 * PARAMETERS:      Name            RW  Usage
 *                  eEvent          R   The event that has been detected
 *                  u8ButtonNumber  R   The button number which experienced the event
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/

PUBLIC void vCbTouchEventButton(eTouchButtonEvent eEvent, uint8 u8ButtonNumber)
{

	sDevice.u8ButtonNumber = E_KEY_NONE;
    sDevice.bTouchPosted = (eEvent == TOUCH_BUTTON_EVENT_PRESSED) || (eEvent == TOUCH_BUTTON_EVENT_ALL_RELEASED);

	if  (eEvent == TOUCH_BUTTON_EVENT_PRESSED)
	{
		sDevice.u8ButtonNumber = u8ButtonNumber;

		/* only provide visual user feedback if changing modes or controlling */
        if ((u8ButtonNumber != E_KEY_PWR) && (eSysState == E_STATE_CONTROLLING))
        {
			vSetLedState(E_LED_STATE_WINK);
        }

		/* Production Test Code to echo key presses to serial port */
		UART_vPTSChar('A'+u8ButtonNumber);
	}
	eLastTouchButtonEvent = eEvent; /* Remember last button event */
}

PRIVATE void vTouchChecker(void)
{
     static uint32 u32LedWink = 0;

     static teKeyStatusCode eKeyStatusCode = 0;

	sDevice.bTouchPosted = FALSE;

	if (eTouchProcess() == TOUCH_STATUS_OK)
	{
		u32LedWink ^= 0x01UL;
#if MK_CALIBRATION
    ((u32LedWink & u32EnableFilterLed) ^ u32GoodSelectKeyPress) ? DBG_LED_ON : DBG_LED_OFF;
#warning Calibration Mode Enabled

#endif
	  if (sDevice.bTouchPosted) /* was a key pressed ? */
	  {
		  bool_t bNormal = (eSysState == E_STATE_CONTROLLING) ? TRUE : FALSE;
		  sDevice.bTouchPosted = FALSE;
		  u32ActivityTimer = USER_INACTIVE_TIME_MS; /* reset the timer if we press a key */
		  eKeyStatusCode = eKeyPressTracker(sDevice.u8ButtonNumber,bNormal);

		  switch (eKeyStatusCode) /* process it */
		  {
		  case E_MODE_CMSNG_BULB_START:
			  vSetLedState(E_LED_STATE_COMMISSIONING);

			  i6LP_ResumeStack();
			  vCommissionInit(&sDevice,&sAuthorise);
			  sAuthorise.u16DeviceType = 0x00E1;
			  eSysState = E_STATE_COMMISSION_BULB;
			  break;

		  case E_MODE_CMSNG_BR_START:
			  vSetLedState(E_LED_STATE_COMMISSIONING);

			  i6LP_ResumeStack();
			  vCommissionInit(&sDevice,&sAuthorise);
			  sAuthorise.u16DeviceType = 0x0001;
			  eSysState = E_STATE_COMMISSION_BR;
			  break;

		  case E_MODE_CMSNG_REMOTE_START:
			  vSetLedState(E_LED_STATE_COMMISSIONING);
			  i6LP_ResumeStack();
			  vCommissionInit(&sDevice,&sAuthorise);
  			  sAuthorise.u16DeviceType = 0x00C5;
			  eSysState = E_STATE_COMMISSION_REMOTE;
			  break;

		  case E_MODE_CLONE_REMOTE_START:
			  vSetLedState(E_LED_STATE_CLONING);
			  i6LP_ResumeStack();
			  vCommissionInit(&sDevice,&sAuthorise);
			  sAuthorise.u16DeviceType = 0x00C5;
			  eSysState = E_STATE_CLONE_REMOTE;
			  break;

		  case E_MODE_DCMSNG_START:
			  i6LP_ResumeStack();
			  eSysState = E_STATE_DECOMMISSIONING;
			  vDecommissionMode(&eSysState,E_EVENT_DECMSNG_START);
			  break;

		  case E_MODE_RESET_TO_GATEWAY:
		  		/* Update PDM data */
		  		sRemotePdm.u8StackModeInit = 0;
		  		sRemotePdm.u8DeviceType    = E_6LP_ROUTER;
		  		sRemotePdm.u8UpMode        = UP_MODE_NONE;
		  		memset(&sRemotePdm.asSecurityKey[0], 0, sizeof(tsSecurityKey));
				/* Save PDM data */
//				DBG_vPrintf(TRACE_RM, "\nvAppSave() 5");
				bSave = TRUE;
				vAppSave();
				/* Wait for all buttons released */
				while (eLastTouchButtonEvent != TOUCH_BUTTON_EVENT_ALL_RELEASED)
				{
					DBG_vPrintf(TRACE_RM, ".");
					eTouchProcess();
				}
				/* Reset */
				vAHI_SwReset();
			  break;

		  case E_MODE_RESET_TO_STANDALONE:
		  		/* Update PDM data */
		  		sRemotePdm.u8StackModeInit = STACK_MODE_STANDALONE;
		  		sRemotePdm.u8DeviceType    = E_6LP_COORDINATOR;
		  		sRemotePdm.u8UpMode        = UP_MODE_NONE;
		  		memset(&sRemotePdm.asSecurityKey[0], 0, sizeof(tsSecurityKey));
				/* Save PDM data */
//				DBG_vPrintf(TRACE_RM, "\nvAppSave() 6");
				bSave = TRUE;
				vAppSave();
				/* Wait for all buttons released */
				while (eLastTouchButtonEvent != TOUCH_BUTTON_EVENT_ALL_RELEASED)
				{
					eTouchProcess();
				}
				/* Reset */
				vAHI_SwReset();
			  break;

		  case E_MODE_RESET:
				/* Save PDM data */
//				DBG_vPrintf(TRACE_RM, "\nvAppSave() 7");
				bSave = TRUE;
				vAppSave();
				/* Wait for all buttons released */
				while (eLastTouchButtonEvent != TOUCH_BUTTON_EVENT_ALL_RELEASED)
				{
					eTouchProcess();
				}
				/* Reset */
				vAHI_SwReset();
			  break;

		  case E_MODE_FACTORY_RESET:
				DBG_vPrintf(TRACE_RM, "\nFACTORY_RESET");
		  		/* Delete all data */
				//PDM_vDelete();
				Remote_vPdmErase();
				DBG_vPrintf(TRACE_RM, " PDM_DELETED");
				/* Wait for all buttons released */
				while (eLastTouchButtonEvent != TOUCH_BUTTON_EVENT_ALL_RELEASED)
				{
					eTouchProcess();
				}
				DBG_vPrintf(TRACE_RM, " BUTTON_RELEASED");
				/* Reset */
				vAHI_SwReset();
			  break;

		  case E_MODE_ADD_GROUP_START:
			    vSetLedState(E_LED_STATE_ADDDEL_GROUP);
		  		/* Transmit remove group to global address failed ? */
				eBcastGroupMibVar(0xF00F, MIB_ID_GROUPS, VAR_IX_GROUPS_ADD_GROUP, u8GetLastGroup());
			  break;

		  case E_MODE_DEL_GROUP_START:
			    vSetLedState(E_LED_STATE_ADDDEL_GROUP);
				eBcastGroupMibVar(0xF00F, MIB_ID_GROUPS, VAR_IX_GROUPS_REMOVE_GROUP, u8GetLastGroup());
			  break;

		 /* MOVED TO PWR BUTTON (IGNORE CAP TOUCH ABORTS)*/
		  case E_MODE_ABORT:
			  break;

		  default:
			  break;
		  }
	  }
	}

	/* check for PWR button press */
	if (bCheckPowerButton() != FALSE)
	{
		if ((eSysState & ANY_STATE_COMMISSIONING) || (eSysState == E_STATE_DECOMMISSIONING))
		{
			bStackSleep = TRUE;
		}
		   /* NOt waiting to join ? */
		if(eSysState != E_STATE_NO_NWK)
		{
			vSetLedState(E_LED_STATE_OFF);

			/* restore the TTL stack profile default for future operation
			* as we force it to zero for decommissioning
			*/
			if (eSysState==E_STATE_DECOMMISSIONING)
			{
				vDecommissionMode(&eSysState,E_EVENT_DECMSNG_FINISH);
			}

			sDevice.sTimers.u8SetVarTimeout = 0;
			/* Leave commissioning mode */
			vApi_SetStackMode(STACK_MODE_STANDALONE);
			DBG_vPrintf(TRACE_RM,"\nvApi_SetStackMode(STACK_MODE_STANDALONE)");
			/* Invalidate key 1 */
			vSecurityInvalidateKey(1);
			/* Clear down authorisation structure */
			//memset(&sAuthorise, 0, sizeof(tsAuthorise));
			/* Clear down stack authorisation data */
			//eApi_CommissionNode(NULL, NULL);
			eSysState = E_STATE_CONTROLLING;
		}
			/* Leave led showing waiting to join as we're still in NO_NWK state */
		else
		{
			vSetLedState(E_LED_STATE_JOINING);
		}

	}
}

/* Erase PDM data directly due to bug in PDM_vDelete() */
PRIVATE void Remote_vPdmErase(void)
{
	/* JN514x Chip family ? */
	#ifdef JENNIC_CHIP_FAMILY_JN514x
	{
		uint8 u8Sector;
		bool_t bErase;
		/* Loop through sectors */
		for (u8Sector = 0; u8Sector < PDM_NUM_SECTORS; u8Sector++)
		{
			DBG_vPrintf(TRACE_RM, "\nbAHI_FlashEraseSector(%d)", (PDM_START_SECTOR+u8Sector));
			bErase = bAHI_FlashEraseSector(PDM_START_SECTOR+u8Sector);
			DBG_vPrintf(TRACE_RM, "=%d", bErase);
		}
	}
	/* Other ? */
	#else
	{
		/* Delete PDM data */
		PDM_vDelete();
	}
	#endif
}

/****************************************************************************
 *
 * NAME: vJIP_Remote_SetResponse
 *
 * DESCRIPTION:
 * Callback to handle response to remote Set request.
 *
 * PARAMETERS: Name                     RW  Usage
 *             *psAddr
 *             u8Handle
 *             u8ModuleIndex
 *             u8VarIndex
 *             eStatus
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vJIP_Remote_SetResponse(ts6LP_SockAddr *psAddr,
                                    uint8 u8Handle,
                                    uint8 u8ModuleIndex,
                                    uint8 u8VarIndex,
                                    teJIP_Status eStatus)
{
//    DBG_vPrintf(DBG_CALLBACKS, "%s()\n", __FUNCTION__);
//    DBG_vPrintf(DBG_CALLBACKS, "%s(%d, %d)\n", __FUNCTION__, u8Handle, eStatus);



	if (sAuthorise.eAuthState == E_STATE_CMSNG_SENDGROUP_INPRG)
	{
		/* Commissioning a bulb ? */
		if (sAuthorise.u16DeviceType == 0x00E1)
		{
			/* Zero timer */
			sDevice.sTimers.u8SetVarTimeout = 0;
			/* Is this the last group ? */
			if (sAuthorise.u8Group == 1)
			{
				/* Send final commissioning command */
				sAuthorise.eAuthState = E_STATE_CMSNG_FINISH_START;
				DBG_vPrintf(TRACE_RM, "\nFINISH_START");
			}
			else
			{
				/* Increment group for next group */
				sAuthorise.u8Group++;
				/* Start sending next group */
				sAuthorise.eAuthState = E_STATE_CMSNG_SENDGROUP_START;
			}
		}
		/* Commissioning a remote ? */
		else if (sAuthorise.u16DeviceType == 0x00C5)
		{
			/* Zero timer */
			sDevice.sTimers.u8SetVarTimeout = 0;
			/* Is this the last group ? */
			if (sAuthorise.u8Group == 4)
			{
				/* Send final commissioning command */
				sAuthorise.eAuthState = E_STATE_CMSNG_FINISH_START;
				DBG_vPrintf(TRACE_RM, "\nFINISH_START");
			}
			else
			{
				/* Increment group for next group */
				sAuthorise.u8Group++;
				/* Start sending next group */
				sAuthorise.eAuthState = E_STATE_CMSNG_SENDGROUP_START;
			}
		}
	}
	/* Finishing commissioning */
	else if (sAuthorise.eAuthState == E_STATE_CMSNG_FINISH_INPRG)
	{
		/* Zero timer */
		sDevice.sTimers.u8SetVarTimeout = 0;
		/* End commissioning */
		sAuthorise.eAuthState = E_STATE_CMSNG_IDLE;
		DBG_vPrintf(TRACE_RM, "\nCMSNG_IDLE 2");
	   	/* Cancel commissioning timer */
	   	sDevice.sTimers.u8CmsngTimeout = 0;
		 // If we are running in non gateway mode break the link
		if(u16Api_GetStackMode() & STACK_MODE_STANDALONE)
		{
			vSetLedState(E_LED_STATE_ON);
		  /* CAUTION! After issuing this we can't unicast to device */
		   vNwk_DeleteChildVect(&sAuthorise.sAddr);
		   DBG_vPrintf(TRACE_RM,"\nvNwk_DeleteChildVect(%s)", __FUNCTION__);
		}
	}
}

/****************************************************************************
 *
 * NAME: Remote_vSetUserData
 *
 * DESCRIPTION:
 * Puts wanted network id into establish route requests and beacon responses
 *
 ****************************************************************************/
PUBLIC void Remote_vSetUserData(void)
{
	tsBeaconUserData sBeaconUserData;

	/* Debug */
	DBG_vPrintf(TRACE_RM, "\nRemote_vSetUserData()");

	/* Set up user data */
	sBeaconUserData.u32NetworkId  = CONFIG_NETWORK_ID;
	sBeaconUserData.u16DeviceType = MK_JIP_DEVICE_TYPE;

	/* Set beacon payload */
	vApi_SetUserBeaconBits((uint8 *) &sBeaconUserData);
	/* Debug */
	DBG_vPrintf(TRACE_RM, "\n\tvApi_SetUserBeaconBits(%x, %x)", sBeaconUserData.u32NetworkId, sBeaconUserData.u16DeviceType);
	/* Set up beacon response callback */
	vApi_RegBeaconNotifyCallback(Remote_bBeaconNotifyCallback);

	/* Set establish route payload */
	v6LP_SetUserData(sizeof(tsBeaconUserData), (uint8 *) &sBeaconUserData);
	/* Debug */
	DBG_vPrintf(TRACE_RM, "\n\tv6LP_SetUserData(%x, %x)", sBeaconUserData.u32NetworkId, sBeaconUserData.u16DeviceType);
	/* Set up establish route callback */
	v6LP_SetNwkCallback(Remote_bNwkCallback);

	/* Set device types */
	vJIP_SetDeviceTypes(1, &u16DeviceType);
}

/****************************************************************************
 *
 * NAME: Remote_bBeaconNotifyCallback
 *
 * DESCRIPTION:
 * Callback when processing establish route request
 * Rejects requests from nodes using a different network ID
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC bool_t Remote_bBeaconNotifyCallback(tsScanElement *psBeaconInfo,
                                    uint16 	   u16ProtocolVersion)
{
	bool_t bReturn 		= TRUE;
    uint32 u32NetworkId = 0;

	/* Not a coordinator (don't filter results for a coordinator) ? */
	if (sRemotePdm.u8DeviceType != E_6LP_COORDINATOR)
	{
	    /* Extract network id from request */
        memcpy((uint8 *)&u32NetworkId, psBeaconInfo->au8UserDefined, sizeof(uint32));
        /* Does it not match the network ID we are using ? */
        if (u32NetworkId != CONFIG_NETWORK_ID)
        {
			/* Discard the beacon */
            bReturn = FALSE;
        }
        /* Matching network id ? */
        else
        {
			/* Is this a response from a device in standalone commissioning mode ? */
			if ((psBeaconInfo->u16StackMode & (STACK_MODE_STANDALONE | STACK_MODE_COMMISSION)) == (STACK_MODE_STANDALONE | STACK_MODE_COMMISSION))
			{
				/* Are we currently in gateway mode ? */
				if ((u16Api_GetStackMode() & STACK_MODE_STANDALONE) == 0)
				{
					/* Flag that we need to complete swapping to standalone mode */
					bStandaloneBeacon = TRUE;
					/* Swap to standalone commissioning mode */
					vApi_SetStackMode(STACK_MODE_STANDALONE | STACK_MODE_COMMISSION);
				}
			}
		}
	}

	/* Debug */
	//DBG_vPrintf(TRACE_RM, "\nbBeaconNotifyCallback(%x)=%d", u32NetworkId, bReturn);

    return bReturn;
}

/****************************************************************************
 *
 * NAME: Remote_bNwkCallback
 *
 * DESCRIPTION:
 * Callback when processing establish route request
 * Rejects requests from nodes using a different network ID
 *
 ****************************************************************************/
PUBLIC bool_t Remote_bNwkCallback(MAC_ExtAddr_s *psAddr,
 			                                 uint8 		   u8DataLength,
                            				 uint8 		 *pu8Data)
{
	bool_t             bReturn		  = FALSE;
	tsEstablishRouteUserData *psEstablishRouteUserData;
	uint8 			  u8ExtraType;

	/* Debug */
	DBG_vPrintf(TRACE_RM, "\nRemote_bNwkCallback()");

	/* Are we in a commissioning mode ? */
    if (eSysState == E_STATE_COMMISSION_BULB ||
    	eSysState == E_STATE_COMMISSION_BR ||
    	eSysState == E_STATE_COMMISSION_REMOTE ||
    	eSysState == E_STATE_CLONE_REMOTE)
    {
		/* Take a user data pointer to the data */
		psEstablishRouteUserData = (tsEstablishRouteUserData *) pu8Data;
		/* Take a copy of the device ID from earlier in the message */
		memcpy((uint8 *)&sAuthorise.u32DeviceId,  pu8Data-30, sizeof(uint32));

		/* Debug */
		DBG_vPrintf(TRACE_RM, "\n\tu8DataLength = %d", u8DataLength);
		/* There is at least a uint32 in the data ? */
		if (u8DataLength >= sizeof(uint32))
		{
			/* Debug */
			DBG_vPrintf(TRACE_RM, "\n\tUserData.u32NetworkId = %x", psEstablishRouteUserData->u32NetworkId);
			/* Does it match the network ID we are using ? */
			if (psEstablishRouteUserData->u32NetworkId == CONFIG_NETWORK_ID)
			{
				/* Debug */
				DBG_vPrintf(TRACE_RM, "\n\tsAuthorise.u16DeviceType = %x", sAuthorise.u16DeviceType);
				/* Is this using a legacy device id ? */
				if ((sAuthorise.u32DeviceId & 0xFF000000) == 0x80000000)
				{
					/* Debug */
					DBG_vPrintf(TRACE_RM, "\n\tLegacy");
					/* Is the legacy device type (least significant byte) one we are allowed to commission ? */
					if (sAuthorise.u16DeviceType == (uint16)(sAuthorise.u32DeviceId & 0xFF))
					{
						/* Debug */
						DBG_vPrintf(TRACE_RM, " MATCHED");
						/* Allow commissioning */
						bReturn = TRUE;
					}
				}
				/* Is there enough data for a primary device type ? */
				else if (u8DataLength >= offsetof(tsEstablishRouteUserData, u8ExtraTypes))
				{
					/* Debug */
					DBG_vPrintf(TRACE_RM, "\n\tNew u16DeviceType=%x", psEstablishRouteUserData->u16DeviceType);
					/* Is the primary device type one we are allowed to commission ? */
					if (sAuthorise.u16DeviceType == psEstablishRouteUserData->u16DeviceType)
					{
						/* Debug */
						DBG_vPrintf(TRACE_RM, " MATCHED");
						/* Allow commissioning */
						bReturn = TRUE;
					}
					/* Is there enough data for secondary device types ? */
					else if (u8DataLength >= offsetof(tsEstablishRouteUserData, au16ExtraTypes))
					{
						/* Loop through any extra device types */
						for (u8ExtraType = 0; u8ExtraType < MIN(ESTABLISH_ROUTE_EXTRA_TYPES,psEstablishRouteUserData->u8ExtraTypes); u8ExtraType++)
						{
							/* Debug */
							DBG_vPrintf(TRACE_RM, "\n\tNew au16ExtraTypes[%d]=%x", u8ExtraType, psEstablishRouteUserData->au16ExtraTypes[u8ExtraType]);
							/* Is this extra type one we are allowed to commission ? */
							if (sAuthorise.u16DeviceType == psEstablishRouteUserData->au16ExtraTypes[u8ExtraType])
							{
								/* Debug */
								DBG_vPrintf(TRACE_RM, " MATCHED");
								/* Allow commissioning */
								bReturn = TRUE;
							}
						}
					}
				}
			}
		}
	}

    return bReturn;
}

PRIVATE bool_t bCheckPowerButton(void)
{
/* Production test code to de-bounce PWR button during active operation   */
/* (as opposed to default waking the remote up ) and echo to serial port  */

	static bool_t bWakeButtonPressed = FALSE;
	static uint16 u16ShiftReg = 0xffff;
	static volatile uint32 i;
	/* Make the LED an input so we can sample it */
	vAHI_DioSetDirection(DIO_WAKEMASK,0);
	vAHI_DioSetPullup(DIO_WAKEMASK,0);

	for(i=0;i<160;i++);  /* small DIO turn-around delay */

	/* shuffle in the sampled value (all 0's = press, all 1's = release) */
	u16ShiftReg <<= 1;
	u16ShiftReg |= (u32AHI_DioReadInput() & DIO_WAKEMASK);

	/* Set-Reset latch to track the press events and spoof a call-back that would */
	/* be generated if the power button was a capacitance touch pad               */

	if ((bWakeButtonPressed==FALSE) && (u16ShiftReg == 0))
	{
		bWakeButtonPressed = TRUE;
		u32EnableFilterLed ^= 0x01;
		vCbTouchEventButton(TOUCH_BUTTON_EVENT_PRESSED,E_KEY_PWR);
	}
	if ((bWakeButtonPressed == TRUE) && (u16ShiftReg == 0xffff))
	{
		vCbTouchEventButton(TOUCH_BUTTON_EVENT_RELEASED,E_KEY_PWR);
		bWakeButtonPressed = FALSE;
	}

	vAHI_DioSetDirection(0,DIO_WAKEMASK);  /* restore the led DIO as an output */
	vAHI_DioSetPullup(0,DIO_WAKEMASK);

    return (bWakeButtonPressed);
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
