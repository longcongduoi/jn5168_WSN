/****************************************************************************
 *
 * MODULE:             JenNet-IP Border Router
 *
 * COMPONENT:          Main functionality
 *
 * VERSION:            $Name:  $
 *
 * REVISION:           $Revision: 1.14 $
 *
 * DATED:              $Date: 2010/01/13 10:26:10 $
 *
 * STATUS:             $State: Exp $
 *
 * AUTHOR:             Lee Mitchell / Matt Redfearn
 *
 * DESCRIPTION:
 *
 * CHANGE HISTORY:
 *
 *
 ****************************************************************************
 *
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
 *
 ***************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <string.h>

// SDK include files
#include <jendefs.h>
#include <AppHardwareApi.h>
#include <UartBuffered.h>
#include <Api.h>
#include <AppApi.h>
#include <Exceptions.h>
#include <pdm.h>
#include <dbg_uart.h>
#include <6LP.h>
#include <jip.h>

#ifdef ENABLE_OND
#include <OverNetworkDownload.h>
extern PUBLIC uint8 u8OND_SrvMaxServers; // Access private data of OND
#endif /* ENABLE_OND */

// Project include files
#include "IPv6Header.h"
#include "Config.h"
#include "SerialLink.h"
#include "ModuleConfig.h"
#include "RADIUS.h"
#include "log.h"

#ifdef NWK_TEST_MIB
/* Register the network test MiB if enabled. */
#include "MibNwkTest.h"
#endif /* NWK_TEST_MIB */

// Access private function of JIP
extern PUBLIC void vJIP_StartDeviceIDPoll(uint32 u32PollPeriodMs, uint8 u8TimeoutTicks);

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

// Check that the number of routing table entries will fit in available RAM
#if (defined JENNIC_CHIP_JN5148) && (ROUTE_TABLE_ENTRIES >= 300)
#error This will overflow the heap!!
#endif


/** @{ Chip family PDM configuration */
#if (defined JENNIC_CHIP_FAMILY_JN514x)
#define PDM_START_SECTOR			5
#define PDM_NUM_SECTORS				2
#define PDM_SECTOR_SIZE				0x10000
#elif (defined JENNIC_CHIP_FAMILY_JN516x)
#define PDM_START_SECTOR            0
#define PDM_NUM_SECTORS             63
#define PDM_SECTOR_SIZE             64
#else
#error No PDM config for selected chip family
#endif /** @} Chip family PDM configuration */


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

#ifdef ENABLE_TEST_MODES
/** Enumeration of test modes */
typedef enum
{
    E_TESTMODE_CW,
} teTestMode;
#endif /* ENABLE_TEST_MODES */

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

PRIVATE void vInitPeripherals(void);
PRIVATE void vStartNetwork(void);

PRIVATE void vProcessDataFromGateway(void);
PRIVATE void vProcessDataFromLink(void);
PRIVATE void vNode_AuthoriseCallback(uint64 u64MacAddress, teRADIUS_Status eStatus, tsSecurityKey *psCommisioningKey);

PRIVATE bool_t vJenNetAuthoriseCallback(MAC_ExtAddr_s *psAddr, uint8 u8DataLength, uint8 *pu8Data);
PRIVATE bool_t vJenNetBeaconUserAuthoriseCallback(tsScanElement *psBeaconInfo,
                                       			 uint16 	   u16ProtocolVersion);

PUBLIC void Security_vBuildCommissioningKey(uint8 *psAddr,uint8 *psSecKey);

#ifdef ENABLE_TEST_MODES
PRIVATE void vEnterTestMode(teTestMode eTestMode);
#endif /* ENABLE_TEST_MODES */

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/** Access private 6LoWPAN configuration of maximum groupcast addresses */
extern uint8 u8_6LP_GCastAddrStoreEntries;

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/** Configured security information */
PRIVATE tsSecurityConfig sSecurityConfig;


/** State information for commissioning nodes */
PRIVATE volatile struct
{
	MAC_ExtAddr_s 			sLastCommissionedNode;
											/**< MAC Address of the node we last attempted to commission */
	tsSecurityKey 			sLastCommissioningKey;
											/**< Commissioning key associated with this node */

/** Number of ticks to keep the commissioning window open */
#define COMMISSIONING_WINDOW_TICKS (100 * 20) /* 20s */
	uint32					u32Ticks;		/**< Count of ticks left until commissioning window closes */

/** Maximum number of attempts a node will be given to authorise before closing the commissioning window */
#define NODE_AUTHORISE_ATTEMPTS 5
	uint32					u32Attempts;	/**< Count of attempts at commissioning this node we have made */

/** @{ Convenience macros to manipulate the commissioning window */
#define COMMISSIONING_WINDOW_OPEN       sCommissioningState.u32Ticks = COMMISSIONING_WINDOW_TICKS
#define COMMISSIONING_WINDOW_AVAILABLE (sCommissioningState.u32Ticks == 0)
#define COMMISSIONING_WINDOW_CLOSE      sCommissioningState.u32Ticks = 0
/** @} */
} sCommissioningState;


/** Version information */
PRIVATE const uint8 au8SoftwareVersion[3] = {VERSION_MAJOR,VERSION_MINOR,VERSION_BUILD};


/** Structure to hold runtime flags */
PRIVATE volatile struct
{
	unsigned bRunCoordinator	: 1;		/**< Flag to start network up as coordinator */
	unsigned bRunRouter			: 1;		/**< Flag to start network up as router */
	unsigned bRunCommissioning	: 1;		/**< Flag to run in commissioning mode */
	unsigned bSecurity 			: 1;		/**< Security is enabled */
	unsigned bNetworkUp			: 1;		/**< Network is up and running */
	unsigned bConfigured		: 1;		/**< Configuration has been received. */
    unsigned bAntennaDiversity  : 1;        /**< Enable Antenna Diversity */
#ifdef ENABLE_ACTIVITY_LED
	unsigned bActivityLEDState	: 1;		/**< Shadow copy of activity LED */
#endif /* ENABLE_ACTIVITY_LED */
} sFlags;


/** Variable holding configured radio frontend */
PRIVATE teRadioFrontEnd eRadioFrontEnd = E_FRONTEND_STANDARD_POWER;


/** JenNet Profile to use on network. Default to 0 = large network */
PRIVATE uint8 u8JenNetProfile = 0;


#ifdef ENABLE_GATEWAY_ANNOUNCEMENTS

/** Structure to control the sending of gateway announcements */
PRIVATE volatile struct
{
#ifndef GATEWAY_ANNOUNCE_INTERVAL
#define GATEWAY_ANNOUNCE_INTERVAL 			(1 * 60 * 100) 	/** 10ms ticks between announces */
#endif /* GATEWAY_ANNOUNCE_INTERVAL */

#define GATEWAY_ANNOUNCE_INITIAL_INTERVAL 	(1 * 100) 		/**< 10ms ticks between first announces */
	uint32   u32LastAnnounce;								/**< Ticks since last announcement */
	unsigned bEnabled 				: 1;					/**< Enable / disable announcements */
	unsigned bFirstAnnounce 		: 1;					/**< Flag the first announcement */
} sGatewayAnnouncement;

#endif /* ENABLE_GATEWAY_ANNOUNCEMENTS */


#ifdef ENABLE_PERSISTENT_SECURITY_INFO

/** Amount the frame counter must change by before a save
 *  is triggered. Also the amount to add when the frame counter is restored
 */
#define SECURITY_FRAME_COUNTER_DELTA  100

/** PDM Descriptor for persistent network information */
PDM_tsRecordDescriptor   sPDMDescPersistentSecurityInfo;

/** Structure to contain persistent security information
 * 	When the security key changes, all of the stored information must be flushed to
 *  defaults, so store it here as well as the information we need
 */
PRIVATE volatile struct {

	tsSecurityKey			sKey;			/**< The security key associated this data */

	uint32					u32FrameCounter;/**< The last known frame counter that we transmitted with */

} sPersistentSecurityInfo;

#endif /* ENABLE_PERSISTENT_SECURITY_INFO */


/** Transmit buffer to host */
PRIVATE uint8 au8HostTxBuffer[100];

/** Receive buffer from host */
PRIVATE uint8 au8HostRxBuffer[1500];


/** Transmit buffer into 6LP */
PRIVATE uint8 au8LinkTxBuffer[MAX_PACKET_SIZE];

/** Receive buffer from 6LP */
PRIVATE uint8 au8LinkRxBuffer[MAX_PACKET_SIZE];


/** work around on JN5148J01 */
extern uint32 g_u32MacAddrLocation;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
/** Entry point from bootloader - start up application */
PUBLIC void AppColdStart(void)
{
#ifdef WATCHDOG_DISABLED
    /* Stop watchdog timer */
    vAHI_WatchdogStop();
#endif

	/* Initialise peripherals */
	vInitPeripherals();

	/* Initialise debugging */
#ifdef UART_DEBUG
	/* Send debug output to DBG_UART */
    DBG_vUartInit(DBG_UART, DBG_BAUD_RATE);
#else
    /* Send debug output through SerialLink to host */
    vSL_LogInit();
#endif

    /* Initialise UART connection to host processor */
	vUartInit(HOST_UART, HOST_BAUD_RATE, au8HostTxBuffer, sizeof(au8HostTxBuffer), au8HostRxBuffer, sizeof(au8HostRxBuffer));

	/* Initialise exception handlers */
	vEXC_Register();

	/* Log startup */
	if (bAHI_WatchdogResetEvent())
	{
		vLog_Printf(TRACE_BR, LOG_CRIT, "\nReset by watchdog");
	}
    vLog_Printf(TRACE_BR, LOG_INFO, "\nInit %cP Module V" VERSION, bModuleIsHighPower() ? 'H' : 'L');
    vSL_LogSend();

    /* Initialise flags */
    sFlags.bRunCoordinator	= FALSE;
    sFlags.bRunRouter		= FALSE;
    sFlags.bSecurity 		= FALSE;
    sFlags.bNetworkUp		= FALSE;
    sFlags.bConfigured		= FALSE;
    sFlags.bAntennaDiversity= FALSE;

#ifdef JENNIC_CHIP_JN5148J01
    /** This is not ideal, but we have to fudge the mac address in order for the PDM to work. */
    g_u32MacAddrLocation = 0;
#endif /* JENNIC_CHIP_JN5148J01 */

    /* Always initialise the PDM */
	PDM_vInit(PDM_START_SECTOR,
			  PDM_NUM_SECTORS,
			  PDM_SECTOR_SIZE,
			  (OS_thMutex) 1,	/* Mutex */
			  NULL,
			  NULL,
			  NULL);

#ifdef ENABLE_PERSISTENT_SECURITY_INFO
	/* Initialise persistent security information first */
	memset((void*)&sPersistentSecurityInfo, 0, sizeof(sPersistentSecurityInfo));

	/* Attempt to load persistent security information */
	{
		PDM_teStatus   ePdmStatus;

		/* Load NodeStatus mib data */
		ePdmStatus = PDM_eLoadRecord(&sPDMDescPersistentSecurityInfo,
#if (defined JENNIC_CHIP_FAMILY_JN514x)
                                     "PersistentSecurityInfo",
#elif (defined JENNIC_CHIP_FAMILY_JN516x)
                                     (0xA5A5),
#else
#error No PDM record identifier for chip family
#endif /* Chip family PDM Config */
									 (void *)&sPersistentSecurityInfo,
									 sizeof(sPersistentSecurityInfo),
									 FALSE);
		if (ePdmStatus != PDM_E_STATUS_OK)
		{
			vLog_Printf(TRACE_BR, LOG_INFO, "\nError restoring security info (%d)", ePdmStatus);
		}
		else
		{
			vLog_Printf(TRACE_BR, LOG_INFO, "\nLoaded security info, FC: %d", sPersistentSecurityInfo.u32FrameCounter);
		}
	}
#endif /* ENABLE_PERSISTENT_SECURITY_INFO */

    /* Request a config packet in case 6LoWPANd is already running */
    vSL_WriteMessage(E_SL_MSG_CONFIG_REQUEST, 0, NULL);

	/* while we've not received the run command */
	while((!sFlags.bRunCoordinator) && (!sFlags.bRunRouter))
	{
		/* See if there is any data available on the serial link */
		vProcessDataFromLink();
	}

	/* Initialise network */
	vStartNetwork();

	/* Main application loop */
	while(1)
	{
#ifdef ENABLE_PERSISTENT_SECURITY_INFO
		if (sFlags.bNetworkUp)
		{
			/* Check the persistent security information */
			void *s_pvMac;
			MAC_Pib_s *s_psMacPib;
			uint32 u32CurrentFrameCounter;
			s_pvMac 	= pvAppApiGetMacHandle();          	// Get handle to MAC
			s_psMacPib 	= MAC_psPibGetHandle(s_pvMac);  	// Get handle to Pib

			/* Get current frame counter from MAC */
			u32CurrentFrameCounter = s_psMacPib->u32MacFrameCounter;

			if (u32CurrentFrameCounter >=
				(sPersistentSecurityInfo.u32FrameCounter + SECURITY_FRAME_COUNTER_DELTA))
			{
				/* Update our outgoing frame counter from the MAC */
				sPersistentSecurityInfo.u32FrameCounter = u32CurrentFrameCounter;

				PDM_vSaveRecord(&sPDMDescPersistentSecurityInfo);
			}
		}
#endif /* ENABLE_PERSISTENT_SECURITY_INFO */

	    /* Poke 6LP */
	    v6LP_Tick();

		/* See if there is any data available on the serial link */
		vProcessDataFromLink();

		/* See if there is any data to send on the serial link */
		vProcessDataFromGateway();

		/* Send any pending log messages */
		vSL_LogSend();

#ifndef WATCHDOG_DISABLED
		vAHI_WatchdogRestart();
#endif
	}

}


PUBLIC void AppWarmStart(void)
{
    AppColdStart();
}


/****************************************************************************
 *
 * NAME: vInitPeripherals
 *
 * DESCRIPTION:
 * Initialises peripherals
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vInitPeripherals(void)
{
    /* Initialise stack and hardware interfaces */
    v6LP_InitHardware();

    /* Configure DIO pins for High power selection */
    vAHI_DioSetDirection(MODULE_FEATURE_SELECT_PIN_HIGH_POWER, 0);
    vAHI_DioSetPullup(MODULE_FEATURE_SELECT_PIN_HIGH_POWER, 0);

    /* Configure DIO pins for antenna diversity selection */
    vAHI_DioSetDirection(MODULE_FEATURE_SELECT_PIN_ANTENNA_DIVERSITY, 0);
	vAHI_DioSetPullup(MODULE_FEATURE_SELECT_PIN_ANTENNA_DIVERSITY, 0);
}

#ifdef ENABLE_ACTIVITY_LED
/** Toggle activity LED */
static void vToggleActivityLED(void)
{
    if (u32ActivityLEDMask != ACTIVITY_LED_DISABLED)
    {
        sFlags.bActivityLEDState = !sFlags.bActivityLEDState;

        if (sFlags.bActivityLEDState)
        {
            vAHI_DioSetOutput(u32ActivityLEDMask, 0);
        }
        else
        {
            vAHI_DioSetOutput(0, u32ActivityLEDMask);
        }
    }
}
#endif /* ENABLE_ACTIVITY_LED */


/****************************************************************************
 *
 * NAME: vStartNetwork
 *
 * DESCRIPTION:
 * Starts the JenNet-IP stack using the configuration received from the host
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vStartNetwork(void)
{
    tsJIP_InitData sJIP_InitData;

#ifdef ENABLE_OND
    /* Set number of concurrent OND downloads */
    u8OND_SrvMaxServers = 8;
#endif /* ENABLE_OND */

    /* Set up stack init data */
    memset(&sJIP_InitData, 0, sizeof(sJIP_InitData));

    /* Set device type */
    if (sFlags.bRunCoordinator)
    {
    	sJIP_InitData.eDeviceType           = E_JIP_DEVICE_COORDINATOR;
    }
    else if (sFlags.bRunRouter)
    {
    	sJIP_InitData.eDeviceType			= E_JIP_DEVICE_ROUTER;
    }
    else
    {
    	/* BAD */
    }

    /* Set device ID and version string */
    sJIP_InitData.u32DeviceId              = GATEWAY_DEVICE_ID;
    sJIP_InitData.pcVersion                = "BR V" VERSION;

    /* Set network parameters */
    /* If configured channel is 0, pick the first clear channel */
    if(sConfig.u8Channel == 0)
    {
        sJIP_InitData.u32Channel           = GATEWAY_ALL_CHANNELS_BITMAP;
    }
    else
    {
        sJIP_InitData.u32Channel           = sConfig.u8Channel;
    }
    sJIP_InitData.u16PanId                 = sConfig.u16PanID;
    sJIP_InitData.u64AddressPrefix         = sConfig.u64NetworkPrefix;

    /* Set resource allocation */
    sJIP_InitData.u16NumPacketBuffers      = PACKET_BUFFERS;
    sJIP_InitData.u8UdpSockets             = 4;
    sJIP_InitData.u32RoutingTableEntries   = ROUTE_TABLE_ENTRIES;

    /* Set JIP parameters */
    sJIP_InitData.u16Port                  = JIP_DEFAULT_PORT;
    sJIP_InitData.u8UniqueWatchers         = 2;
    sJIP_InitData.u8MaxTraps               = 16;
    sJIP_InitData.u8MaxNameLength          = 16;
    sJIP_InitData.u8QueueLength            = 8;

    /* Start the stack */
    if (eJIP_Init(&sJIP_InitData) != E_JIP_OK)
    {
        vLog_Printf(TRACE_BR, LOG_ERR, "\nFailed to initialise JenNet-IP stack");
    }

    /* Set up our JIP Device Types */
    {
        PRIVATE const uint16 au16DeviceType[] = {0x0001};
        vJIP_SetDeviceTypes(sizeof(au16DeviceType)/sizeof(uint16),
                            (uint16 *)au16DeviceType);
    }
    /* Set our descriptive name */
    vJIP_SetNodeName("Border-Router");

#ifdef NWK_TEST_MIB
    /* Register the network test MiB if enabled. */
    MibNwkTest_vInit();
    MibNwkTest_vRegister();
#endif /* NWK_TEST_MIB */

    /* Set module output power level depending on region */
    if (bModuleIsHighPower())
    {
        vModuleSetRadioFrontEnd(E_FRONTEND_HIGH_POWER);
    }
    else
    {
        vModuleSetRadioFrontEnd(eRadioFrontEnd);
    }

    /* Set up antenna diversity if the module supports it */
    if (bModuleHasAntennaDiversity() && sFlags.bAntennaDiversity)
    {
    	vModuleEnableDiversity();
    }

    /* Set short packet fragmentation timeout */
    v6LP_SetPacketDefragTimeout(1);

    if (sFlags.bRunCoordinator)
    {
    	/* Coordinator needs to start OND and security services */

#ifdef ENABLE_OND
		/* Initialise OND */
		{
			u8_6LP_GCastAddrStoreEntries = 24;
			teOND_Result eOND_Result = eOND_SrvInit(1874);
			if (eOND_Result != E_OND_SUCCESS)
			{
				vLog_Printf(TRACE_BR, LOG_ERR, "\nFailed to initialise OND server %d", eOND_Result);
			}
		}
#endif /* ENABLE_OND */

		if (sFlags.bSecurity)
		{
			/* Initialise security service */
			switch (sSecurityConfig.eAuthScheme)
			{
			case (E_AUTH_SCHEME_RADIUS_PAP):
				/* Initialise RADIUS client */
				if (eRADIUS_Init(E_RADIUS_AUTH_PAP, &sSecurityConfig.uScheme.sRadiusPAP.sAuthServerIP, &sSecurityConfig.sNetworkKey, vNode_AuthoriseCallback) == E_RADIUS_OK)
				{
					/* Initialise the commissioning process */
					memset((uint8 *)&sCommissioningState, 0, sizeof(sCommissioningState));
					COMMISSIONING_WINDOW_CLOSE;
				}
				else
				{
					vLog_Printf(TRACE_BR, LOG_ERR, "\nFailed to initialise RADIUS Client\n");
				}
				break;
			default:
				break;
			}
		}
    }
}


/****************************************************************************
 *
 * NAME: v6LP_ConfigureNetwork
 *
 * DESCRIPTION:
 * Callback function from 6LoWPAN allowing us to configure some network
 * parameters.
 *
 * PARAMETERS  Name                    RW  Usage
 *             psNetworkConfigData     RW  Configuration data for us to manipulate
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void v6LP_ConfigureNetwork(tsNetworkConfigData *psNetworkConfigData)
{
	/* Select profile for network size */
	vLog_Printf(TRACE_BR, LOG_INFO, "\nConfiguring Network Profile %d\n", u8JenNetProfile & 0xFF);

	if ((bJnc_SetRunProfile  (u8JenNetProfile, NULL) != TRUE) ||
		(bJnc_SetJoinProfile (u8JenNetProfile, NULL) != TRUE))
	{
		vLog_Printf(TRACE_BR, LOG_INFO, "\nError configuring Network Profile %d\n", u8JenNetProfile & 0xFF);
	}

	if (sFlags.bRunCoordinator || sFlags.bRunRouter)
	{
		/* Set up network ID / User data stuff */
		if (sConfig.u32NetworkID != 0)
		{
			/* Set beacon filtering callback */
			vApi_RegBeaconNotifyCallback(vJenNetBeaconUserAuthoriseCallback);

			/* Set establish route payload */
			v6LP_SetUserData(sizeof(uint32), (uint8*)&sConfig.u32NetworkID);
		}
	}

	if (sFlags.bRunCommissioning)
	{
		/* Run commissioning - set stack into commissioning mode */
#define STACK_MODE_STANDALONE  0x0001
#define STACK_MODE_COMMISSION  0x0002
		/* We can only currently commission to a standalone
		 * remote that is is commissioning mode. */
		vApi_SetStackMode(STACK_MODE_STANDALONE | STACK_MODE_COMMISSION);
	}

	if (sFlags.bSecurity)
	{
		/* Enable security in the stack */
		v6LP_EnableSecurity();

		if (sFlags.bRunCommissioning)
        {
        	/* Set commissioning key */
        	tsSecurityKey sCommissioningKey;

        	Security_vBuildCommissioningKey((uint8 *) pvAppApiGetMacAddrLocation(),
											(uint8 *) &sCommissioningKey);
        	vApi_SetNwkKey(1, &sCommissioningKey);
        }
		else
		{
        	/* Set network key */
			vApi_SetNwkKey(0,&sSecurityConfig.sNetworkKey);
        }
	}
}


/****************************************************************************
 *
 * NAME: v6LP_DataEvent
 *
 * DESCRIPTION:
 * Deals with any incoming data events. When a packet arrives
 * (E_DATA_RECEIVED), it is placed in a queue for retrieval later
 * in the application idle loop.
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
        DBG_vPrintf(TRACE_BR_COMMS, "<Sent>");
        break;

    case E_DATA_SEND_FAILED:
        DBG_vPrintf(TRACE_BR_COMMS, "<Send Fail>");
        break;

    case E_DATA_RECEIVED:
        DBG_vPrintf(TRACE_BR_COMMS, "<RXD>");
    	/* Discard 6LP packets as only interested in JIP communication */
    	i6LP_RecvFrom(iSocket, NULL, 0, 0, NULL, NULL);
    	break;

    case E_6LP_ICMP_MESSAGE:
        DBG_vPrintf(TRACE_BR_COMMS, "<RXICMP>");
	    /* Discard 6LP packets as only interested in JIP communication */
        i6LP_RecvFrom(iSocket, NULL, 0, 0, NULL, NULL);
        break;

    case E_IP_DATA_RECEIVED:
        DBG_vPrintf(TRACE_BR_COMMS, "<GRX %d>", u16_6LP_GetNumberOfAvailableIPv6Buffers());
        break;

    default:
        DBG_vPrintf(TRACE_BR_COMMS, "\n? in v6LP_DataEvent");
    	break;
    }
}


/****************************************************************************
 *
 * NAME: v6LP_PeripheralEvent
 *
 * DESCRIPTION:
 * Processes any incoming peripheral events to see if the event is a tick
 * timer event, which we use to time events in the application
 *
 * PARAMETERS: Name          RW Usage
 *             u32Device     R  Device that caused peripheral event
 *             u32ItemBitmap R  Events within that peripheral
 *
 ****************************************************************************/
PUBLIC void v6LP_PeripheralEvent(uint32 u32Device, uint32 u32ItemBitmap)
{
    /*DBG_vPrintf(FALSE, "%s(%u, %x)\n", __FUNCTION__, u32Device, u32ItemBitmap);*/

	/* The Tick timer is used to time events */
	if (u32Device == E_AHI_DEVICE_TICK_TIMER)
	{
	    if (sFlags.bSecurity)
	    {
			switch (sSecurityConfig.eAuthScheme)
			{
			case (E_AUTH_SCHEME_RADIUS_PAP):
				/* Tick the RADIUS client every 10ms */
				vRADIUS_Tick();

				if (sCommissioningState.u32Ticks)
				{
					/* If commisioning window is open for a device, count down until it times out */
					if (--sCommissioningState.u32Ticks == 0)
					{
						DBG_vPrintf(TRACE_BR, "Authorisation timed out\n");
					}
				}
				break;

			default:
				break;
			}
	    }

#ifdef ENABLE_GATEWAY_ANNOUNCEMENTS
	    if (sGatewayAnnouncement.bEnabled)
	    {
	    	/* Gateway announcements are enabled */
	    	/* If time has expired since last announce, send a new one. */
	    	if (++sGatewayAnnouncement.u32LastAnnounce >= GATEWAY_ANNOUNCE_INTERVAL)
	    	{
	    		eApi_SendGatewayStarted();
	    		if (sGatewayAnnouncement.bFirstAnnounce == TRUE)
	    		{
	    			/* If this was the first announcement, announce again after initial interval */
	    			sGatewayAnnouncement.bFirstAnnounce = FALSE;
	    			sGatewayAnnouncement.u32LastAnnounce =
	    				GATEWAY_ANNOUNCE_INTERVAL - GATEWAY_ANNOUNCE_INITIAL_INTERVAL;
	    		}
	    		else
	    		{
	    			/* Otherwise, announce again after the interval */
	    			sGatewayAnnouncement.u32LastAnnounce = 0;
	    		}
	    	}
	    }
#endif /* ENABLE_GATEWAY_ANNOUNCEMENTS */

#ifdef ENABLE_ACTIVITY_LED
	    {
#define ACTIVITY_LED_RATE_RUNNING (100) 	/* Toggle every second when running */
	    	static uint8 u8ActivityLedCountDown = ACTIVITY_LED_RATE_RUNNING;

			if (--u8ActivityLedCountDown == 0)
			{
				vToggleActivityLED();
				/* Set next timeout dependent on what mode we are currently in */
				u8ActivityLedCountDown = ACTIVITY_LED_RATE_RUNNING;
			}
	    }
#endif /* ENABLE_ACTIVITY_LED */
	}
}


/****************************************************************************
 *
 * NAME: vJIP_StackEvent
 *
 * DESCRIPTION:
 * Processes any incoming stack events.
 *
 * PARAMETERS: Name          RW Usage
 *             eEvent        R  Stack event
 *             pu8Data       R  Additional information associated with event
 *             u8DataLen     R  Length of additional information
 *
 ****************************************************************************/
PUBLIC void vJIP_StackEvent(te6LP_StackEvent eEvent, uint8 *pu8Data, uint8 u8DataLen)
{
	/* DBG_vPrintf(DBG_CALLBACKS, "%s(%s)\n", __FUNCTION__, apcStackEvents[eEvent]);*/

    switch (eEvent)
    {
    case E_STACK_NODE_JOINED_NWK:
    case E_STACK_NODE_JOINED:
    	/* Node has joined the network */
    	if (eEvent == E_STACK_NODE_JOINED_NWK)
    	{
    		vLog_Printf(TRACE_BR, LOG_DEBUG, "NJ");
    	}
    	else
    	{
    		vLog_Printf(TRACE_BR, LOG_DEBUG, "J");
    	}
    	if (sFlags.bSecurity)
		{
    		/* Get the MAC address from the event data */
    		MAC_ExtAddr_s *psAddr = (MAC_ExtAddr_s *)pu8Data;
			switch (sSecurityConfig.eAuthScheme)
			{
			case (E_AUTH_SCHEME_RADIUS_PAP):
				if (memcmp(psAddr, (void*)&sCommissioningState.sLastCommissionedNode, sizeof(MAC_ExtAddr_s)) == 0)
				{
					/* The node that joined was the node we were just commissioning.
					 * We can now close the comissioning window and allow another node to join
					 */
					vLog_Printf(TRACE_BR, LOG_DEBUG, " AN ");
					COMMISSIONING_WINDOW_CLOSE;
					memset((void*)&sCommissioningState.sLastCommissionedNode, 0, sizeof(MAC_ExtAddr_s));
					memset((void*)&sCommissioningState.sLastCommissioningKey, 0, sizeof(tsSecurityKey));
#ifdef ENABLE_TEST_SINGLE_CHILD
					/* Disable associations so we only accept a single child node */
					vApi_SetAssociationState(FALSE);
#endif /* ENABLE_TEST_SINGLE_CHILD */
				}
				break;

			default:
				break;
			}
		}
    	else
    	{
#ifdef ENABLE_TEST_SINGLE_CHILD
    		/* Disable associations so we only accept a single child node */
    		vApi_SetAssociationState(FALSE);
#endif /* ENABLE_TEST_SINGLE_CHILD */
    	}
        break;

    case E_STACK_STARTED:
    case E_STACK_JOINED:
    {
    	/* Stack has started */
    	tsNwkInfo *psNwkInfo = (tsNwkInfo *)pu8Data;

        vLog_Printf(TRACE_BR, LOG_INFO,
					"\nNetwork Started. Channel: %d, PAN ID: 0x%x",
					psNwkInfo->u8Channel & 0xFF, psNwkInfo->u16PanID & 0xFFFF);

        /* Update network configuration information */
        sConfig.u8Channel	= psNwkInfo->u8Channel;
		sConfig.u16PanID	= psNwkInfo->u16PanID;

		if (sFlags.bRunCommissioning)
		{
			/* We have successfully joined a parent node */
			if (sFlags.bSecurity)
			{
				/* Save the network key that has been received */
				memcpy(&sSecurityConfig.sNetworkKey, psApi_GetNwkKey(), sizeof(tsSecurityKey));
			}
		}

		if (sFlags.bRunCoordinator || sFlags.bRunRouter)
		{
			if (sConfig.u32NetworkID != 0)
			{
				/* Create array containing network ID */
				uint8 au8NwkId[MAX_BEACON_USER_DATA];
				memset(au8NwkId, 0, MAX_BEACON_USER_DATA);
				memcpy(au8NwkId, (uint8 *)&sConfig.u32NetworkID, sizeof(uint32));

				/* Set beacon payload */
				vApi_SetUserBeaconBits(au8NwkId);

				/* Set up establish route callback */
				v6LP_SetNwkCallback(vJenNetAuthoriseCallback);
			}
		}

#ifdef ENABLE_PERSISTENT_SECURITY_INFO
		if (sFlags.bSecurity)
		{
			/* Restore the persistent security information */
			MAC_DeviceDescriptor_s *psDevDesc;
			psDevDesc = psSecurityGetDescriptor(0);   // Get pointer to our descriptor

			if (memcmp(	(void*)&sPersistentSecurityInfo.sKey,
						&sSecurityConfig.sNetworkKey,
						sizeof(tsSecurityKey)) == 0)
			{
				/* Same network key is in use */

				/* Set frame counter to what we last saved plus the delta */
				psDevDesc->u32FrameCounter =
					sPersistentSecurityInfo.u32FrameCounter + SECURITY_FRAME_COUNTER_DELTA;
				bSecuritySetDescriptor(0,psDevDesc);
				vLog_Printf(TRACE_BR, LOG_INFO,
						"\nFrame counter set to %d", psDevDesc->u32FrameCounter);
			}
			else
			{
				vLog_Printf(TRACE_BR, LOG_INFO,
							"\nPersisted network key does not match current key");
				/* Save new network key */
				memcpy( (void*)&sPersistentSecurityInfo.sKey,
						&sSecurityConfig.sNetworkKey,
						sizeof(tsSecurityKey));
			}
		}
#endif /* ENABLE_PERSISTENT_SECURITY_INFO */

#ifdef ENABLE_GATEWAY_ANNOUNCEMENTS
		if (sFlags.bRunCoordinator)
		{
			if (sFlags.bSecurity)
			{
			
			}
			/* Enable announcements, and set next announce immediately */
			sGatewayAnnouncement.bEnabled = TRUE;
			sGatewayAnnouncement.bFirstAnnounce = TRUE;
			sGatewayAnnouncement.u32LastAnnounce = GATEWAY_ANNOUNCE_INTERVAL;
#endif /* ENABLE_GATEWAY_ANNOUNCEMENTS */
		}

		if (sFlags.bRunCoordinator)
        {
		    /* Start polling device IDs */
		    vJIP_StartDeviceIDPoll(2000, 2);
        }

        /* Flag that network is up and running */
        sFlags.bNetworkUp = TRUE;
        break;
    }

    case E_STACK_RESET:
#ifdef ENABLE_GATEWAY_ANNOUNCEMENTS
    	/* Stack has reset - don't send any gateway announces until it comes back up */
    	sGatewayAnnouncement.bEnabled = FALSE;
#endif /* ENABLE_GATEWAY_ANNOUNCEMENTS */

    	/* Flag that network is down */
    	sFlags.bNetworkUp = FALSE;
    	break;

    case E_STACK_NODE_LEFT:
    	vLog_Printf(TRACE_BR, LOG_DEBUG, "L");

#ifdef ENABLE_TEST_SINGLE_CHILD
    		/* Re-enable association when our single child node leaves */
    		vApi_SetAssociationState(TRUE);
#endif /* ENABLE_TEST_SINGLE_CHILD */
		break;

    case E_STACK_NODE_LEFT_NWK:
    	vLog_Printf(TRACE_BR, LOG_DEBUG, "NL");

#ifdef ENABLE_TEST_SINGLE_CHILD
    		/* Re-enable association when our single child node leaves */
    		vApi_SetAssociationState(TRUE);
#endif /* ENABLE_TEST_SINGLE_CHILD */
		break;

    case E_STACK_TABLES_RESET:
    	break;

    case E_STACK_NODE_AUTHORISE:
    	/* JenNet request to authorise a node that is attempting to join */
    	if (sFlags.bSecurity)
		{
    		/* Get the MAC address from the event data */
			MAC_ExtAddr_s *psAddr = (MAC_ExtAddr_s *)pu8Data;
			uint64 u64Addr;

			/* Convert the MAC address into a 64bit integer */
			u64Addr = ( (((uint64)psAddr->u32H) << 32) | (((uint64)psAddr->u32L) << 0));

			vLog_Printf(TRACE_BR, LOG_DEBUG, "\nA 0x%016llx\n", u64Addr);

			switch (sSecurityConfig.eAuthScheme)
			{
			case (E_AUTH_SCHEME_RADIUS_PAP):
				if (COMMISSIONING_WINDOW_AVAILABLE)
				{
					/* Request RADIUS authentication of the node address */
					if (eRADIUS_Request_Authentication(u64Addr) == E_RADIUS_OK)
					{
						sCommissioningState.u32Attempts = 0;
						sCommissioningState.sLastCommissionedNode = *psAddr;
						/* Open window of time for the node to join */
						COMMISSIONING_WINDOW_OPEN;
					}
				}
				else
				{
					/* Check if this is another request from the last node to ask */
					if ((memcmp(psAddr, (void*)&sCommissioningState.sLastCommissionedNode, sizeof(MAC_ExtAddr_s)) == 0))
					{
						/* Same node trying again */
						if (++sCommissioningState.u32Attempts < NODE_AUTHORISE_ATTEMPTS)
						{
							/* Re-authorise the node and re-open the commissioning window. */
						    /* Though they're the right types, casts are needed to avoid qualifier warnings */
							eApi_CommissionNode((MAC_ExtAddr_s *)&sCommissioningState.sLastCommissionedNode, (tsSecurityKey *)&sCommissioningState.sLastCommissioningKey);
							/* Re-open window of time for the node to join */
							COMMISSIONING_WINDOW_OPEN;
						}
						else
						{
							/* The node has had NODE_AUTHORISE_ATTEMPTS at joining but
							 * not succeeded - close the window to allow a new node to try
							 */
							COMMISSIONING_WINDOW_CLOSE;
						}
					}
				}
				break;

			default:
				break;
			}
		}
    	break;

    default:
        break;
    }
}


/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

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
PRIVATE bool_t vJenNetAuthoriseCallback(MAC_ExtAddr_s *psAddr,
                            uint8 u8DataLength,
                            uint8 *pu8Data)
{
    if (sConfig.u32NetworkID == 0)
    {
    	/* No network ID is set - allow any node */
    	return TRUE;
    }

	/* Authorise a node based on it's establish route payload */
    if (u8DataLength >= sizeof(uint32))
    {
    	/* Compare first uint32 in establish route payload with our NetworkID
    	 * Allow the node to join if they match
    	 */
        if (memcmp((uint8 *)&sConfig.u32NetworkID, pu8Data, sizeof(uint32)) == 0)
        {
            return TRUE;
        }
        else
        {
        	return FALSE;
        }
    }

    return FALSE;
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
PRIVATE bool_t vJenNetBeaconUserAuthoriseCallback(tsScanElement *psBeaconInfo,
                                       			 uint16 	   u16ProtocolVersion)
{
    if ((sFlags.bRunRouter) && (sConfig.u32NetworkID != 0))
	{

    	/* Compare user data with our configured network ID
    	 * If it matches attempt to join it
    	 */
		if (memcmp((uint8 *)&sConfig.u32NetworkID, psBeaconInfo->au8UserDefined, sizeof(uint32)) == 0)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
    return TRUE;
}


/****************************************************************************
 *
 * NAME:       vProcessDataFromGateway
 *
 * DESCRIPTION: Send data from 6LoWPAN network up to host
 *
 * PARAMETERS:
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vProcessDataFromGateway(void)
{
	uint8 *pu8Ptr = au8LinkTxBuffer;
	tsIPv6_Datagram *psDatagram = (tsIPv6_Datagram*)pu8Ptr;

	int iBytesRcd = 0;

	/* Get buffer from 6LP */
	iBytesRcd = i6LP_IpRecvFrom(pu8Ptr, MAX_PACKET_SIZE);

	if(iBytesRcd > 0)
	{
		/* Send data to host */
//		vPrintf("\nTx %d %d P%d",E_LINK_MSG_IPV6, (uint32)psDatagram->u16PayloadLength + 40, u32Count++);
	    DBG_vPrintf(TRACE_BR_COMMS, "<LTX>");

#ifdef ENABLE_ACTIVITY_LED
	    vToggleActivityLED();
#endif /* ENABLE_ACTIVITY_LED */

	    /* Send packet with length IPv6 payload length + 40 bytes header */
		vSL_WriteMessage(E_SL_MSG_IPV6, (uint32)psDatagram->u16PayloadLength + 40, pu8Ptr);
	}
}


/****************************************************************************
 *
 * NAME:       vProcessDataFromLink
 *
 * DESCRIPTION: Handle packets from host
 *
 * PARAMETERS:
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vProcessDataFromLink(void)
{
	static uint8 u8Type = 0;
	static uint16 u16Length = 0;
	int iResult = -1;

	/* Read data from the serial link */
	if(bSL_ReadMessage(&u8Type, &u16Length, MAX_PACKET_SIZE, au8LinkRxBuffer))
	{
		/* Got a complete, valid packet */
	    DBG_vPrintf(TRACE_BR_COMMS, "<LRX>");

		switch(u8Type)
		{
		case E_SL_MSG_CONFIG:
			/* Basic network configuration information */
			memcpy(&sConfig, au8LinkRxBuffer, sizeof(tsModule_Config));
			vLog_Printf(TRACE_BR, LOG_INFO, "\nRegion %d Ch %d Pan 0x%04x Net 0x%08x Pref 0x%016llx", sConfig.u8Region, sConfig.u8Channel, sConfig.u16PanID, sConfig.u32NetworkID, sConfig.u64NetworkPrefix);
			/* Send version information back */
			vSL_WriteMessage(E_SL_MSG_CONFIG, sizeof(au8SoftwareVersion), (uint8*)au8SoftwareVersion);
			sFlags.bConfigured = TRUE;
			break;

		case E_SL_MSG_CONFIG_REQUEST:
			/* Request for configuration - send the configuration to the host if the network is up*/
			if (sFlags.bNetworkUp)
			{
				/* Send the security configuration first */
				if (sFlags.bSecurity)
				{
					vSL_WriteMessage(E_SL_MSG_SECURITY, sizeof(tsSecurityConfig), (uint8*)&sSecurityConfig);
				}

				/* Now the basic network configuration */
				vSL_WriteMessage(E_SL_MSG_CONFIG, sizeof(tsModule_Config), (uint8*)&sConfig);
			}
			break;

		case E_SL_MSG_VERSION_REQUEST:
			/* Version request - send version array */
			vSL_WriteMessage(E_SL_MSG_VERSION, sizeof(au8SoftwareVersion), (uint8*)au8SoftwareVersion);
			break;

		case E_SL_MSG_RUN_COORDINATOR:
			vLog_Printf(TRACE_BR, LOG_DEBUG, "\nRun Coordinator");
			if (sFlags.bConfigured)
			{
				sFlags.bRunCoordinator = TRUE;
				sFlags.bRunRouter = FALSE;
				sFlags.bRunCommissioning = FALSE;
			}
			else
			{
				/* We're been told to run without a config packet - request one */
				vSL_WriteMessage(E_SL_MSG_CONFIG_REQUEST, 0, NULL);
			}
			break;

		case E_SL_MSG_RUN_ROUTER:
			vLog_Printf(TRACE_BR, LOG_DEBUG, "\nRun Router");
			if (sFlags.bConfigured)
			{
				sFlags.bRunCoordinator = FALSE;
				sFlags.bRunRouter = TRUE;
				sFlags.bRunCommissioning = FALSE;
			}
			else
			{
				/* We're been told to run without a config packet - request one */
				vSL_WriteMessage(E_SL_MSG_CONFIG_REQUEST, 0, NULL);
			}
			break;

		case E_SL_MSG_RUN_COMMISSIONING:
			vLog_Printf(TRACE_BR, LOG_DEBUG, "\nRun Commissioning");
			if (sFlags.bConfigured)
			{
				sFlags.bRunCoordinator = FALSE;
				sFlags.bRunRouter = TRUE;
				sFlags.bRunCommissioning = TRUE;
			}
			else
			{
				/* We're been told to run without a config packet - request one */
				vSL_WriteMessage(E_SL_MSG_CONFIG_REQUEST, 0, NULL);
			}
			break;

		case E_SL_MSG_IPV6:
			/* IPv6 poacket from host - pass on to 6LP for routing */
			if (sFlags.bNetworkUp)
			{
				iResult = i6LP_IpSendTo(au8LinkRxBuffer);
				if (iResult == -1)
				{
					iResult = u32_6LP_GetErrNo();
				}
				DBG_vPrintf(TRACE_BR_COMMS, "<GTX %d %04x>", u16_6LP_GetNumberOfAvailableIPv6Buffers(), iResult);
			}
			break;

		case E_SL_MSG_ADDR:
			/* Request for our address - pass it back */
			if (sFlags.bNetworkUp)
			{
				ts6LP_SockAddr sAddr;

				i6LP_GetOwnDeviceAddress(&sAddr, TRUE);

				vSL_WriteMessage(E_SL_MSG_ADDR, sizeof(sAddr.sin6_addr), sAddr.sin6_addr.s6_addr);
			}
			break;

		case E_SL_MSG_RESET:
			/* Software reset */
			vAHI_SwReset();
			break;

		case E_SL_MSG_SECURITY:
			/* Security configuration - save settings and enable security */
			memcpy(&sSecurityConfig, au8LinkRxBuffer, sizeof(sSecurityConfig));

			vLog_Printf(TRACE_BR, LOG_INFO, "\nInit Security Scheme %d, Network Key: 0x%016llx%016llx", sSecurityConfig.eAuthScheme, sSecurityConfig.u64NetworkKeyH, sSecurityConfig.u64NetworkKeyL);

			/* Set flag to enable JenNet security during network start */
			sFlags.bSecurity = TRUE;
			break;

		case E_SL_MSG_PING:
			/* Ping back to host */
			vSL_WriteMessage(E_SL_MSG_PING, 0, NULL);

#ifdef ENABLE_TEST_MODES
             vEnterTestMode(TEST_MODE);
#endif /* ENABLE_TEST_MODES */
			break;

		case E_SL_MSG_PROFILE:
			/* Set JenNet Profile */
			u8JenNetProfile = au8LinkRxBuffer[0];

			vLog_Printf(TRACE_BR, LOG_INFO, "\nNetwork Profile %d selected\n", u8JenNetProfile & 0xFF);
			break;

		case E_SL_MSG_ACTIVITY_LED:
		{
			/* Set up an activity LED */
		    uint8 u8ActivityLED = au8LinkRxBuffer[0];

		    /* Sanity check that the LED we've been told to toggle doesn't overlap anything important. */

		    if (((1 << u8ActivityLED) & u32AllocatedPins) ||
		        (u8ActivityLED > 20))
		    {
		        /* Requested pin is already allocated or out of range - ignore the request */
		        vLog_Printf(TRACE_BR, LOG_WARNING, "Activity LED Already used: 0x%08x / 0x%08x\n", (1 << u8ActivityLED), u32AllocatedPins);
		    }
		    else
		    {
		        /* Pin is OK to use. */
		        u32ActivityLEDMask = 1 << u8ActivityLED;

		        /* Configure DIO pin */
		        vAHI_DioSetDirection(0, u32ActivityLEDMask);

		        u32AllocatedPins |= u32ActivityLEDMask;
		    }
		    break;
		}

		case E_SL_MSG_SET_RADIO_FRONTEND:
			/* Set radio power settings / module configuration */
		    eRadioFrontEnd = (teRadioFrontEnd)au8LinkRxBuffer[0];
            vModuleSetRadioFrontEnd(eRadioFrontEnd);
		    break;

		case E_SL_MSG_ENABLE_DIVERSITY:
			/* Turn on antenna diversity */
		    sFlags.bAntennaDiversity = TRUE;
		    vModuleEnableDiversity();
            break;
		}
	}
}


/****************************************************************************
 *
 * NAME:       vNode_AuthoriseCallback
 *
 * DESCRIPTION:
 * Callback function called when a node has been authorised / not authorised to join the network.
 *
 * PARAMETERS: Name          RW  Usage
 *         u64MacAddress     R   uint64 containing mac address that is being reported.
 *         eStatus           R   E_RADIUS_OK if node authorised.
 *         psCommisioningKey R   pointer to location of commisioning key.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vNode_AuthoriseCallback(uint64 u64MacAddress, teRADIUS_Status eStatus, tsSecurityKey *psCommissioningKey)
{
	MAC_ExtAddr_s sAddr;

	if (sFlags.bSecurity)
	{
		switch (sSecurityConfig.eAuthScheme)
		{
		case (E_AUTH_SCHEME_RADIUS_PAP):
			/* Handle RADIUS authentication response */
			switch (eStatus)
			{
			case(E_RADIUS_OK):
				/* Node accepted */
				vLog_Printf(TRACE_BR, LOG_DEBUG, "\nNode Authorised\n");
				sAddr.u32H = (uint32)((u64MacAddress >> 32) & 0xFFFFFFFF);
				sAddr.u32L = (uint32)((u64MacAddress >>  0) & 0xFFFFFFFF);
				eApi_CommissionNode(&sAddr, psCommissioningKey);

				/* Hold on to the key in case the node asks again */
				if (memcmp((uint8 *)&sAddr, (uint8 *)&sCommissioningState.sLastCommissionedNode, sizeof(MAC_ExtAddr_s)) == 0)
				{
					sCommissioningState.sLastCommissioningKey = *psCommissioningKey;
				}
				break;
			default:
				/* Timeout or authorisation denied response */
				vLog_Printf(TRACE_BR, LOG_DEBUG, "\nNode NOT Authorised\n");
				memset((uint8 *)&sCommissioningState.sLastCommissionedNode, 0, sizeof(MAC_ExtAddr_s));
				/* Close commissioning window to allow another node to attempt to join */
				COMMISSIONING_WINDOW_CLOSE;
				break;
			}
			break;

		default:
			break;
		}
	}

	return;
}


/****************************************************************************
 *
 * NAME: Security_vBuildCommissioningKey
 *
 * DESCRIPTION:
 *
 * Shared key generation algorithm for non-gateway operation
 *
 * PARAMETERS: Name          RW  Usage
 *             psAddr        R   Input data into key generation algorithm
 *             psSecKey      W   Location to write commissioning key
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void Security_vBuildCommissioningKey(uint8 *psAddr,uint8 *psSecKey)
{
    uint8 i;

    for (i = 0; i < 8; i++)
    {
    	*(psSecKey+(i*2)) = 0;
    	*(psSecKey+(i*2)+1) = *(psAddr+7-i);
    }
}


/****************************************************************************
 *
 * NAME: vEnterTestMode
 *
 * DESCRIPTION:
 *
 * RF Test mode - entered upon first ping from host - this sets the
 * radio into RF test mode for compliance testing.
 *
 * PARAMETERS: Name          RW  Usage
 *             eTestMode     R   Which test mode to set up
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
#ifdef ENABLE_TEST_MODES
PRIVATE void vEnterTestMode(teTestMode eTestMode)
{
    static int iTestMode = 0;
    if (iTestMode) return;
    iTestMode = 1;

#warning Test mode enabled - will not function as a normal border router!
#define REG_SET(reg, value) (*(uint32 *)reg = (uint32)value)
#ifdef JENNIC_CHIP_FAMILY_JN514x
#define REG_PHY_CHAN        (0x02001e08)
#define REG_PHY_MCTRL       (0x02001e30)
#define REG_PHY_MTX_CTRL    (0x02001e40)
#else
#error Test modes not supported for this chip family
#endif /* JENNIC_CHIP_FAMILY */

    /* Disable watchdog */
    vAHI_WatchdogStop();

    vLog_Printf(TRUE, LOG_WARNING, "\nEnter test mode %d channel %d\n", eTestMode, sConfig.u8Channel);
    switch (eTestMode)
    {
    case E_TESTMODE_CW:
        /* Go to required channel */
        REG_SET(REG_PHY_CHAN, sConfig.u8Channel);
        /* Enable modem override manual transmit mode */
        REG_SET(REG_PHY_MCTRL, 0xe);
        /* Enable modem transmit mode */
        REG_SET(REG_PHY_MTX_CTRL, 1);
        break;

    default:
        return;
    }
    /* Flush the log */
    vSL_LogFlush();

    vLog_Printf(TRACE_BR, LOG_INFO, "\nVCO_TXO: 0x%08x", *(uint32*)(0x02001E00 + (4*10)));
    vLog_Printf(TRACE_BR, LOG_INFO, "\nTST_SIG: 0x%08x", *(uint32*)(0x02001E00 + (4*26)));
    vLog_Printf(TRACE_BR, LOG_INFO, "\nPA_CTRL: 0x%08x", *(uint32*)(0x02001E00 + (4*3)));
    vLog_Printf(TRACE_BR, LOG_INFO, "\nVCO_FTO: 0x%08x", *(uint32*)(0x02001E00 + (4*9)));
    vLog_Printf(TRACE_BR, LOG_INFO, "\nTST0   : 0x%08x", *(uint32*)(0x02001E00 + (4*28)));

    {
        uint32 *pu32Reg = 0x02001E00;
        while (pu32Reg <= 0x02001EFC)
        {
            vLog_Printf(TRACE_BR, LOG_INFO, "\n0x%08x : 0x%08x", pu32Reg, *pu32Reg);
            vSL_LogFlush();
            pu32Reg++;
        }
    }

    /* FLush the log */
    vSL_LogFlush();
    /* Now sit in a loop */
    while (1);
}
#endif /* ENABLE_TEST_MODES */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

