/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         DeviceBulb - Main Bulb Source File
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
/* Standard includes */
#include <string.h>
/* SDK includes */
#include <jendefs.h>
/* Hardware includes */
#include <AppHardwareApi.h>
#include <PeripheralRegs.h>
/* Stack includes */
#include <Api.h>
#include <AppApi.h>
#include <JIP.h>
#include <6LP.h>
#include <AccessFunctions.h>
#include <OverNetworkDownload.h>
#include <sec2006.h>
/* JenOS includes */
#include <dbg.h>
#include <dbg_uart.h>
#include <os.h>
#include <pdm.h>
/* Application includes */
#include "Config.h"
#include "Exception.h"
#include "Security.h"
#include "MibCommon.h"
#include "MibBulb.h"
#include "MibNode.h"
#include "MibGroup.h"
#include "MibAdcStatus.h"
#include "MibNodeStatus.h"
#include "MibNodeControl.h"
#include "MibNwkStatus.h"
#include "MibNwkConfig.h"
#include "MibNwkProfile.h"
#include "MibNwkSecurity.h"
#include "MibNwkTest.h"
#include "MibBulbStatus.h"
#include "MibBulbConfig.h"
#include "MibBulbScene.h"
#include "MibBulbControl.h"
#include "DriverBulb.h"
#include "BulbDefault.h"
#ifdef MK_BLD_THERMAL_CONTROL_LOOP
#include "ThermalControl.h"
#endif
/* Optional Application Includes */
#ifdef  JENNIC_CHIP_FAMILY_JN516x
#include "AHI_EEPROM.h" /* Enables EEPROM factory reset detection, for 6x use only */
#endif
//#include "Uart.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
/* Check for unsupported chip type */
#ifdef  JENNIC_CHIP_FAMILY_JN514x
#ifndef JENNIC_CHIP_JN5142J01
#ifndef JENNIC_CHIP_JN5148J01
#ifndef JENNIC_CHIP_JN5148
#error Unsupported chip!
#endif
#endif
#endif
#endif
#ifdef  JENNIC_CHIP_FAMILY_JN516x
#ifndef JENNIC_CHIP_JN5164
#ifndef JENNIC_CHIP_JN5168
#error Unsupported chip!
#endif
#endif
#endif

/* Apply patches definitions */
#define MIB_NWK_SECURITY_PATCH 		   TRUE	/* When true patches in automatic gateway to standalone mode switchover */
#define MIB_BULB_CONTROL_PATCH_FAILED FALSE /* When true patches in failed bulb lockout */

/* JenOS debug config */
#define DBG_UART					DBG_E_UART_0
#define DBG_BAUD_RATE				DBG_E_UART_BAUD_RATE_115200

/* Factory reset magic number */
#define BULB_FACTORY_RESET_MAGIC 	0xFA5E13CB

/* ADC control */
/* Eval kit build ? */
#if (MK_JIP_DEVICE_ID == 0x08011175) || (MK_JIP_DEVICE_ID == 0x0801175F)
	#define BULB_ADC_MASK (MIB_ADC_MASK_SRC_VOLT|MIB_ADC_MASK_SRC_TEMP) /* ADCs to be read */
	#define BULB_ADC_SRC_BUS_VOLTS E_AHI_ADC_SRC_VOLT
/* Bulb builds ? */
#else
	/* 48 or 48J01 ? */
	#if (JENNIC_CHIP == JN5148) || (JENNIC_CHIP == JN5148J01)
		#define BULB_ADC_MASK (MIB_ADC_MASK_SRC_4|MIB_ADC_MASK_SRC_TEMP) /* ADCs to be read */
		#define BULB_ADC_SRC_BUS_VOLTS E_AHI_ADC_SRC_ADC_4
	/* Other chips ? */
	#else
		#define BULB_ADC_MASK (MIB_ADC_MASK_SRC_1|MIB_ADC_MASK_SRC_TEMP) /* ADCs to be read */
		#define BULB_ADC_SRC_BUS_VOLTS E_AHI_ADC_SRC_ADC_1
	#endif
#endif
#define BULB_ADC_PERIOD			   25	/* ADC sample period 0-100 in 10ms intervals */

/* DIO pins */
#define BULB_DIO_OSC_PULL			16	/* Oscillator push/pull pin */

/* Stack modes */
#define STACK_MODE_STANDALONE  0x0001
#define STACK_MODE_COMMISSION  0x0002

/* Factory reset storage */
#ifdef  JENNIC_CHIP_FAMILY_JN514x
#define FACTORY_RESET_FLASH 	    1	/* 0 - use wake timers, else use flash */
#else
#define FACTORY_RESET_FLASH 	    0	/* 0 - use wake timers, else use flash */
#endif
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/
/* MIB structures */
#if MK_BLD_MIB_NODE
extern tsMibNode		 sMibNode;
#endif

#if MK_BLD_MIB_GROUP
extern tsMibGroup		 sMibGroup;
#endif

#if MK_BLD_MIB_NODE_STATUS
extern tsMibNodeStatus	 sMibNodeStatus;
extern thJIP_Mib		 hMibNodeStatus;
#endif

#if MK_BLD_MIB_NODE_CONTROL
extern tsMibNodeControl sMibNodeControl;
extern thJIP_Mib		hMibNodeControl;
#endif

#if MK_BLD_MIB_ADC_STATUS
extern tsMibAdcStatus	 sMibAdcStatus;
extern thJIP_Mib		 hMibAdcStatus;
#endif

#if MK_BLD_MIB_NWK_CONFIG
extern tsMibNwkConfig	 sMibNwkConfig;
extern thJIP_Mib		 hMibNwkConfig;
#endif

#if MK_BLD_MIB_NWK_PROFILE
extern tsMibNwkProfile	 sMibNwkProfile;
extern thJIP_Mib		 hMibNwkProfile;
#endif

#if MK_BLD_MIB_NWK_STATUS
extern tsMibNwkStatus	 sMibNwkStatus;
extern thJIP_Mib		 hMibNwkStatus;
#endif

#if MK_BLD_MIB_NWK_SECURITY
extern tsMibNwkSecurity	 sMibNwkSecurity;
extern thJIP_Mib		 hMibNwkSecurity;
#endif

#if MK_BLD_MIB_NWK_TEST
extern tsMibNwkTest	 	 sMibNwkTest;
extern thJIP_Mib		 hMibNwkTest;
#endif

#if MK_BLD_MIB_BULB_CONFIG
extern tsMibBulbConfig sMibBulbConfig;
extern thJIP_Mib 	   hMibBulbConfig;
#endif

#if MK_BLD_MIB_BULB_STATUS
extern tsMibBulbStatus	 sMibBulbStatus;
extern thJIP_Mib 	     hMibBulbStatus;
#endif

#if MK_BLD_MIB_BULB_SCENE
extern tsMibBulbScene   sMibBulbScene;
extern thJIP_Mib 	    hMibBulbScene;
#endif

#if MK_BLD_MIB_BULB_CONTROL
extern tsMibBulbControl sMibBulbControl;
extern thJIP_Mib 	    hMibBulbControl;
extern thJIP_Mib 	    hMibDeviceControl;
#endif

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
/* MAC pointers */
PRIVATE void      *pvMac;
PRIVATE MAC_Pib_s *psPib;

/* Jip initialisation structure */
PRIVATE	tsJIP_InitData sJipInitData;

/* Counters */
PRIVATE uint8   u8TickQueue;	/* Tick timer queue */
PRIVATE uint8   u8Tick;	   		/* Tick counter */
PUBLIC  uint32 u32Second;  		/* Second counter */
/* Other data */
PRIVATE bool_t   bInitialised;
PRIVATE uint8    u8NwkSecurityReset;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PUBLIC void 			Device_vInit(bool_t bWarmStart);
PUBLIC bool_t 			Device_bFactoryReset(uint8 u8RstSector, uint32 u32SectorSize);
PUBLIC void 			Device_vMibInit(void);
PUBLIC void 			Device_vReset(bool_t bFactoryReset);
PUBLIC int 				Device_i6lpInit(void);
PUBLIC teJIP_Status 	Device_eJipInit(void);
PUBLIC void 			Device_vMibRegister(void);
PUBLIC void 			Device_vTick(void);

/****************************************************************************
 *
 * NAME: AppColdStart
 *
 * DESCRIPTION:
 * Entry point for application from boot loader.
 *
 * RETURNS:
 * void, never returns
 *
 ****************************************************************************/
PUBLIC void AppColdStart(void)
{
	/* Initialise device */
	Device_vInit(FALSE);
}

/****************************************************************************
 *
 * NAME: AppWarmStart
 *
 * DESCRIPTION:
 * Entry point for application from boot loader.
 *
 * RETURNS:
 * Never returns.
 *
 ****************************************************************************/
PUBLIC void AppWarmStart(void)
{
	/* Initialise device */
	Device_vInit(TRUE);
}

/****************************************************************************
 *
 * NAME: Device_vInit
 *
 * DESCRIPTION:
 * Entry point for application
 *
 * RETURNS:
 * void, never returns
 *
 ****************************************************************************/
PUBLIC void Device_vInit(bool_t bWarmStart)
{
	bool_t   				  bFactoryReset;
	uint8					 u8Sectors 			= 0;
	uint32 					u32SectorSizeKb 	= 0;
	uint8 					 u8PdmStartSector	= 0xff;
	uint8					 u8PdmNumSectors	= 0;
	uint32 					u32PdmSectorSize 	= 0;
	uint8					 u8RstSector		= 0xff;

   	/* Initialise exception handler */
   	Exception_vInit();

	/* Initialise stack and hardware interfaces */
	v6LP_InitHardware();

	/* JN514x Chip family ? */
	#ifdef JENNIC_CHIP_FAMILY_JN516x
	{
		/* Enable for high temperature use */
		vAHI_ExtendedTemperatureOperation(TRUE);
	}
	#endif

	/* Initialise all DIO as outputs and drive low */
	vAHI_DioSetDirection(0, 0xFFFFFFFE);
	vAHI_DioSetOutput(0, 0xFFFFFFFE);

	/* UART ? */
	#ifdef UART_H_INCLUDED
	{
		/* Initialise */
		UART_vInit();
		/* Debug */
		UART_vNumber(u32Second, 10);
		UART_vString(" INIT\r\n");
	}
	#endif

	/* JN514x Chip family ? */
	#ifdef JENNIC_CHIP_FAMILY_JN514x
	{
		extern tSPIflashFncTable *pSPIflashFncTable;

		/* Initialise flash */
		bAHI_FlashInit(E_FL_CHIP_AUTO, NULL);
		/* What is the device type ? */
		switch (pSPIflashFncTable->u16FlashId)
		{
			/* ST M25P40A 8*64Kb */
			case CONFIG_FLASH_ID_STM25P40A:
			{
				/* JN5142J01 ? */
				#ifdef JENNIC_CHIP_JN5142J01
				{
					/* JN5142J01 / M25P40A (8*64Kb) */
					/* Sector 0 - OND Application 1 (Exception dumps) */
					/* Sector 1 - OND Application 2 */
					/* Sector 2 - PDM */
					/* Sector 3 - PDM */
					/* Sector 4 - PDM */
					/* Sector 5 - PDM */
					/* Sector 6 - PDM */
					/* Sector 7 - Factory Reset */

					/* Initialise sector information */
					u8Sectors 			   = 8;
					u32SectorSizeKb 	   = 64;
					#ifdef OND_H_INCLUDED
					/* OND configuration */
					u8OND_SectorsAvailable = 2;
					u8OND_SectorSize 	   = u32SectorSizeKb;
					#endif
					/* PDM configuration */
					u8PdmStartSector	   = 2;
					u8PdmNumSectors        = 5;
					u32PdmSectorSize		   = u32SectorSizeKb * 1024;
					/* Reset configuration */
					u8RstSector            = 7;
				}
				#endif

				/* JN5148J01 ? */
				#ifdef JENNIC_CHIP_JN5148J01
				{
					/* JN5148J01 / M25P40A (8*64Kb) */
					/* Sector 0 - OND Application 1 */
					/* Sector 1 - OND Application 1 (Exception dumps) */
					/* Sector 2 - OND Application 2 */
					/* Sector 3 - OND Application 2 */
					/* Sector 4 - PDM */
					/* Sector 5 - PDM */
					/* Sector 6 - PDM */
					/* Sector 7 - Factory Reset */

					/* Initialise sector information */
					u8Sectors 			   = 8;
					u32SectorSizeKb 	   = 64;
					/* OND configuration - assumed on JN5148 devices */
					//u8OND_SectorsAvailable = 4;
					//u8OND_SectorSize 	   = u32SectorSizeKb;
					/* PDM configuration */
					u8PdmStartSector	   = 4;
					u8PdmNumSectors        = 3;
					u32PdmSectorSize		   = u32SectorSizeKb * 1024;
					/* Reset configuration */
					u8RstSector            = 7;
				}
				#endif

				/* JN5148 ? */
				#ifdef JENNIC_CHIP_JN5148
				{
					/* JN5148 / M25P40A (8*64Kb) */
					/* Sector 0 - Bootloader (Exception dumps) */
					/* Sector 1 - OND Application 1 */
					/* Sector 2 - OND Application 1 */
					/* Sector 3 - OND Application 2 */
					/* Sector 4 - OND Application 2 */
					/* Sector 5 - PDM */
					/* Sector 6 - PDM */
					/* Sector 7 - Factory Reset */

					/* Initialise sector information */
					u8Sectors 			   = 8;
					u32SectorSizeKb 	   = 64;
					/* OND configuration - assumed on JN5148 devices */
					//u8OND_SectorsAvailable = 4;
					//u8OND_SectorSize 	   = u32SectorSizeKb;
					/* PDM configuration */
					u8PdmStartSector	   = 5;
					u8PdmNumSectors        = 2;
					u32PdmSectorSize		   = u32SectorSizeKb * 1024;
					/* Reset configuration */
					u8RstSector            = 7;
				}
				#endif
			}
			break;

			/* ST M25P10A 4*32Kb */
			case CONFIG_FLASH_ID_STM25P10A:
			{
				/* JN5142J01 ? */
				#ifdef JENNIC_CHIP_JN5142J01
				{
					/* JN5142J01 / M25P10A (4*32Kb) */
					/* Sector 0 - OND Application 1 (Exception dumps) */
					/* Sector 1 - OND Application 2 */
					/* Sector 2 - PDM */
					/* Sector 3 - Factory Reset */

					/* Initialise sector information */
					u8Sectors 			   = 4;
					u32SectorSizeKb 	   = 32;
					#ifdef OND_H_INCLUDED
					/* OND configuration */
					u8OND_SectorsAvailable = 2;
					u8OND_SectorSize 	   = u32SectorSizeKb;
					#endif
					/* PDM configuration */
					u8PdmStartSector	   = 2;
					u8PdmNumSectors        = 1;
					u32PdmSectorSize		   = u32SectorSizeKb *1024;
					/* Reset configuration */
					u8RstSector            = 3;
				}
				#endif
			}
			break;

			default:
			{
				/* Do nothing */
				;
			}
			break;
		}
	}
	/* Other chip families ? */
	#else
	{
		/* Assume 8*32Kb sectors */
		/* Sector 0 - OND Application 1 */
		/* Sector 1 - OND Application 1 */
		/* Sector 2 - OND Application 1 */
		/* Sector 3 - OND Application 1 */
		/* Sector 0 - OND Application 2 */
		/* Sector 1 - OND Application 2 */
		/* Sector 2 - OND Application 2 */
		/* Sector 3 - OND Application 2 (Exception dumps) */

		/* Initialise sectors available */
		u8Sectors       = 8;
		u32SectorSizeKb = 32;

		/* PDM configuration Assume 64*64byte sectors */
		u8PdmStartSector = 0;
		u32PdmSectorSize = 64;
		/* Using EEPROM for factory reset ? */
		#ifdef AHI_EEPROM_H_INCLUDED
		{
			/* Use 62 sectors for PDM */
			u8PdmNumSectors  = 62;
			/* Reset sector is 62 */
			u8RstSector      = 62;
		}
		#else
		{
			/* Use 63 sectors for PDM */
			u8PdmNumSectors  = 63;
		}
		#endif
	}
	#endif

	/* Early call to Bulb initialisation for optimising startup */
	DriverBulb_vInit();

	/* Initialise debugging */
	DBG_vUartInit(DBG_UART, DBG_BAUD_RATE);
	/* Disable the debug port flow control lines to turn off LED2 */
	vAHI_UartSetRTSCTS(DBG_UART, FALSE);
	/* Debug */
	DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\n\nDEVICE BULB");
	DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\nDevice_vInit(%d)", bWarmStart);
	DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\n\tHeapMax=%x StackMin=%x", u32ExceptionHeapMax, u32ExceptionStackMin);

	/* Factory reset detection */
	bFactoryReset = Device_bFactoryReset(u8RstSector, u32SectorSizeKb*1024);

	/* Initialise PDM */
	PDM_vInit(u8PdmStartSector,
			  u8PdmNumSectors,
			  u32PdmSectorSize,
			  (OS_thMutex) 1,	/* Mutex */
			  NULL,
			  NULL,
			  NULL);

	/* Initialise mib data */
	Device_vMibInit();

	/* Apply factory reset if required */
	if (bFactoryReset) Device_vReset(TRUE);

	/* Initialise JIP */
	(void) Device_eJipInit();

	/* Register mibs and variables */
	Device_vMibRegister();

	/*late call to bulb init to allow any post setup processing to occur */
	DriverBulb_vInit();

	/* Now initialised */
	bInitialised = TRUE;

	/* Main loop */
	while(1)
	{
	   	/* Main processing for network config changing mode ? */
	   	if (MibNwkConfigPatch_bMain())
	   	{
			#if MK_BLD_MIB_NWK_PROFILE
				/* Ensure the configured profile is applied */
				MibNwkProfile_vApply();
			#endif
		}
		/* Restart watchdog */
	   	vAHI_WatchdogRestart();
	   	/* Deal with device tick timer events ? */
	   	Device_vTick();
		/* Doze */
		vAHI_CpuDoze();
	}
}

/****************************************************************************
 *
 * NAME: Device_bFactoryReset
 *
 * DESCRIPTION:
 * Factory reset detection
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
#ifdef AHI_EEPROM_H_INCLUDED
PUBLIC bool_t Device_bFactoryReset(uint8 u8RstSector, uint32 u32SectorSize)
{
	bool_t	 bReturn = FALSE;
	uint32 au32Eeprom[3];

	/* Read wake timer registers */
	bAHI_ReadEEPROM(u8RstSector, 0, sizeof(au32Eeprom), (uint8 *) au32Eeprom);
	/* Debug */
	DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_VARS, "\n\tEeprom = %x:%x", au32Eeprom[0], au32Eeprom[1]);
	/* No history or different device ? */
	if (BULB_FACTORY_RESET_MAGIC != au32Eeprom[1] ||
	    MK_JIP_DEVICE_ID         != au32Eeprom[2])
	{
		uint8  u8Sector;
		uint8 au8Junk[64];
		/* Zero junk data */
		memset(au8Junk, 0, sizeof(au8Junk));
		/* Loop through EEPROM sectors */
		for (u8Sector = 0; u8Sector < 63; u8Sector++)
		{
			/* Write junk into the sector to invalidate it */
			bAHI_WriteEEPROM(u8Sector, 0, 64, au8Junk);
		}
		/* Initialise history */
		au32Eeprom[0] = 0xfffffffe;
		au32Eeprom[1] = BULB_FACTORY_RESET_MAGIC;
		au32Eeprom[2] = MK_JIP_DEVICE_ID;
		/* Write back to EEPROM */
		bAHI_WriteEEPROM(u8RstSector, 0, sizeof(au32Eeprom), (uint8 *) au32Eeprom);
	}
	/* Have some history ? */
	else
	{
		/* Left shift history and write */
		au32Eeprom[0] <<= 1;
		bAHI_WriteEEPROM(u8RstSector, 0, sizeof(au32Eeprom), (uint8 *) au32Eeprom);
	}
	/* Start tick timer for 1 second */
	vAHI_TickTimerConfigure(E_AHI_TICK_TIMER_DISABLE);
	vAHI_TickTimerWrite(0);
	vAHI_TickTimerInterval(32000000);
	vAHI_TickTimerConfigure(E_AHI_TICK_TIMER_STOP);
	vAHI_TickTimerIntEnable(TRUE);
	/* Doze until tick timer fires */
	while (u32AHI_TickTimerRead() < 32000000)
	{
		/* Doze until tick timer fires */
		vAHI_CpuDoze();
	}
	/* Update history to flag we've run for a second */
	au32Eeprom[0] |= 0x1;
	bAHI_WriteEEPROM(u8RstSector, 0, sizeof(au32Eeprom), (uint8 *) au32Eeprom);
	/* Disable tick timer */
	vAHI_TickTimerConfigure(E_AHI_TICK_TIMER_DISABLE);
	/* Reset the tick queue */
	u8TickQueue = 0;

	/* Has the device been on with the following history long, short, short, short, long ? */
	if ((au32Eeprom[0] & 0x1F) == 0x11) bReturn = TRUE;

	return bReturn;
}
#elif FACTORY_RESET_FLASH
PUBLIC bool_t Device_bFactoryReset(uint8 u8RstSector, uint32 u32SectorSize)
{
	bool_t	 bReturn = FALSE;
	uint8   u8Flash;
	uint8   u8Mask = 0;
	uint8   u8Loop;

	/* Read flags from flash */
	bAHI_FullFlashRead((u8RstSector * u32SectorSize), 1, &u8Flash);
	/* Debug */
	DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_VARS, "\n\tu8Flash = %x", u8Flash);
	/* Loop through reset bits */
	for (u8Loop = 0, u8Mask = 0; u8Loop < 8 && u8Mask == 0; u8Loop++)
	{
		/* Bit is set ? */
		if (u8Flash & (1<<u8Loop))
		{
			/* Set mask */
			u8Mask = (1<<u8Loop);
			/* Clear bit */
			U8_CLR_BITS(&u8Flash, u8Mask);
		}
	}
	/* Didn't find a bit set (shouldn't happen) - zero the whole byte */
	if (u8Mask == 0) u8Flash = 0;
	/* Write updated data to flash */
	bAHI_FullFlashProgram((u8RstSector * u32SectorSize), 1, &u8Flash);

	/* Start tick timer for 2 seconds */
	vAHI_TickTimerConfigure(E_AHI_TICK_TIMER_DISABLE);
	vAHI_TickTimerWrite(0);
	vAHI_TickTimerInterval(32000000);
	vAHI_TickTimerConfigure(E_AHI_TICK_TIMER_STOP);
	vAHI_TickTimerIntEnable(TRUE);
	/* Doze until tick timer fires */
	while (u32AHI_TickTimerRead() < 32000000)
	{
		/* Doze until tick timer fires */
		vAHI_CpuDoze();
	}
	/* Has the device been on with the following history long, short, short, short, long ? */
	if (u8Flash == 0xF0) bReturn = TRUE;
	/* Disable tick timer */
	vAHI_TickTimerConfigure(E_AHI_TICK_TIMER_DISABLE);
	/* Write updated data to flash */
	bAHI_FlashEraseSector(u8RstSector);

	return bReturn;
}
#else
PUBLIC bool_t Device_bFactoryReset(uint8 u8RstSector, uint32 u32SectorSize)
{
	bool_t	 bReturn = FALSE;
	uint32 u32WakeTimer0;
	uint32 u32WakeTimer1;

	/* Read wake timer registers */
	u32WakeTimer0 = u32REG_SysRead(REG_SYS_WK_T0);
	u32WakeTimer1 = u32REG_SysRead(REG_SYS_WK_T1);
	/* Debug */
	DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_VARS, "\n\tWakeTimers = %x:%x", u32WakeTimer0, u32WakeTimer1);
	/* No history ? */
	if (BULB_FACTORY_RESET_MAGIC != u32WakeTimer1)
	{
		/* Initialise history and write */
		u32WakeTimer0 = 0xfffffffe;
		vREG_SysWrite(REG_SYS_WK_T0, u32WakeTimer0);
		/* Initialise signature */
		vREG_SysWrite(REG_SYS_WK_T1, BULB_FACTORY_RESET_MAGIC);
	}
	/* Have some history ? */
	else
	{
		/* Left shift history and write */
		u32WakeTimer0 <<= 1;
		vREG_SysWrite(REG_SYS_WK_T0, u32WakeTimer0);
	}
	/* Start tick timer for 1 second */
	vAHI_TickTimerConfigure(E_AHI_TICK_TIMER_DISABLE);
	vAHI_TickTimerWrite(0);
	vAHI_TickTimerInterval(32000000);
	vAHI_TickTimerConfigure(E_AHI_TICK_TIMER_STOP);
	vAHI_TickTimerIntEnable(TRUE);
	/* Doze until tick timer fires */
	while (u32AHI_TickTimerRead() < 32000000)
	{
		/* Doze until tick timer fires */
		vAHI_CpuDoze();
	}
	/* Update history to flag we've run for a second */
	u32WakeTimer0 |= 0x1;
	vREG_SysWrite(REG_SYS_WK_T0, u32WakeTimer0);
	/* Disable tick timer */
	vAHI_TickTimerConfigure(E_AHI_TICK_TIMER_DISABLE);
	/* Reset the tick queue */
	u8TickQueue = 0;

	/* Has the device been on with the following history long, short, short, short, long ? */
	if ((u32WakeTimer0 & 0x1F) == 0x11) bReturn = TRUE;

	return bReturn;
}
#endif

/****************************************************************************
 *
 * NAME: Device_vMibInit
 *
 * DESCRIPTION:
 * Initialise MIB data
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void Device_vMibInit(void)
{
	PDM_teStatus   ePdmStatus;

	#if 0
	CONFIG_DBG_DEVICE_BULB_PDM
	{
		#include <pdm_private.h>

		tsFlashHeader 					sFlashHeader;
		tsFlashRecordDescriptionHeader 	sFlashRecordDescriptionHeader;
		tsFlashChangesetHeader 			sFlashChangesetHeader;
		tsFlashRecordChangeHeader 		sFlashRecordChangeHeader;

		/* Debug */
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n\ntypedef struct {");
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16 u16MasterCookie;              // %03d @ %03d", sizeof(sFlashHeader.u16MasterCookie), offsetof(tsFlashHeader, u16MasterCookie));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16 u16NumRecords;                // %03d @ %03d", sizeof(sFlashHeader.u16NumRecords),   offsetof(tsFlashHeader, u16NumRecords));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n} tsFlashHeader;                         // %03d",        sizeof(tsFlashHeader));

		/* Debug */
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n\ntypedef struct {");
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    char acName[%02d];                     // %03d @ %03d", PDM_NAME_SIZE, sizeof(sFlashRecordDescriptionHeader.acName),   offsetof(tsFlashRecordDescriptionHeader, acName));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16 u16Size;                      // %03d @ %03d", sizeof(sFlashRecordDescriptionHeader.u16Size),  offsetof(tsFlashRecordDescriptionHeader, u16Size));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16 u16Flags;                     // %03d @ %03d", sizeof(sFlashRecordDescriptionHeader.u16Flags), offsetof(tsFlashRecordDescriptionHeader, u16Flags));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n} tsFlashRecordDescriptionHeader         // %03d",        sizeof(tsFlashRecordDescriptionHeader));

		/* Debug */
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n\ntypedef struct {");
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16 u16Cookie;                    // %03d @ %03d", sizeof(sFlashChangesetHeader.u16Cookie),   	    offsetof(tsFlashChangesetHeader, u16Cookie   	 ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16 u16NumRecords;                // %03d @ %03d", sizeof(sFlashChangesetHeader.u16NumRecords),      offsetof(tsFlashChangesetHeader, u16NumRecords   ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16 u16PrevChangeset;             // %03d @ %03d", sizeof(sFlashChangesetHeader.u16PrevChangeset),   offsetof(tsFlashChangesetHeader, u16PrevChangeset));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16 u16NextChangeset;             // %03d @ %03d", sizeof(sFlashChangesetHeader.u16NextChangeset),   offsetof(tsFlashChangesetHeader, u16NextChangeset));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n} tsFlashChangesetHeader;                // %03d",        sizeof(tsFlashChangesetHeader));

		/* Debug */
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n\ntypedef struct {");
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16 u16Id;                        // %03d @ %03d", sizeof(sFlashRecordChangeHeader.u16Id            ),   offsetof(tsFlashRecordChangeHeader, u16Id            ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16 u16NextRecord;                // %03d @ %03d", sizeof(sFlashRecordChangeHeader.u16NextRecord    ),   offsetof(tsFlashRecordChangeHeader, u16NextRecord    ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint32 u32WaterMark;                 // %03d @ %03d", sizeof(sFlashRecordChangeHeader.u32WaterMark     ),   offsetof(tsFlashRecordChangeHeader, u32WaterMark     ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint32 au32ChangedBlocks[%02d]         // %03d @ %03d", (PDM_NUM_BLOCKS/32), sizeof(sFlashRecordChangeHeader.au32ChangedBlocks), offsetof(tsFlashRecordChangeHeader, au32ChangedBlocks));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n} tsFlashRecordChangeHeader;             // %03d",        sizeof(tsFlashRecordChangeHeader));
	}
	#endif

	/* Initialise mibs (which reads PDM data) */
	#if MK_BLD_MIB_NODE
		MibNode_vInit       (&sMibNode);
		/* Debug */
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n\ntypedef struct {");
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    char acName[16];                     // %03d @ %03d", sizeof(sMibNode.sPerm.acName), offsetof(tsMibNodePerm, acName));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n} tsMibNodePerm;                         // %03d",        sizeof(sMibNode.sPerm));
	#endif
	#if MK_BLD_MIB_GROUP
		MibGroup_vInit      (&sMibGroup);
		/* Debug */
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n\ntypedef struct {");
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    in6_addr asGroupAddr[%02d];            // %03d @ %03d", MIB_GROUP_MAX, sizeof(sMibGroup.sPerm.asGroupAddr), offsetof(tsMibGroupPerm, asGroupAddr));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n} tsMibGroupPerm;                        // %03d",        sizeof(sMibGroup.sPerm));
	#endif
	#if MK_BLD_MIB_NODE_STATUS
		MibNodeStatus_vInit (hMibNodeStatus, &sMibNodeStatus);
		/* Debug */
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n\ntypedef struct {");
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16  u16SystemStatus;             // %03d @ %03d", sizeof(sMibNodeStatus.sPerm.u16SystemStatus),   offsetof(tsMibNodeStatusPerm, u16SystemStatus));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16  u16ColdStartCount;           // %03d @ %03d", sizeof(sMibNodeStatus.sPerm.u16ColdStartCount), offsetof(tsMibNodeStatusPerm, u16ColdStartCount));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16  u16ResetCount;               // %03d @ %03d", sizeof(sMibNodeStatus.sPerm.u16ResetCount),     offsetof(tsMibNodeStatusPerm, u16ResetCount));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16  u16WatchdogCount;            // %03d @ %03d", sizeof(sMibNodeStatus.sPerm.u16WatchdogCount),  offsetof(tsMibNodeStatusPerm, u16WatchdogCount));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16  u16BrownoutCount;            // %03d @ %03d", sizeof(sMibNodeStatus.sPerm.u16BrownoutCount),  offsetof(tsMibNodeStatusPerm, u16BrownoutCount));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n} tsMibNodeStatusPerm;                   // %03d",		 sizeof(sMibNodeStatus.sPerm));
	#endif
	#if MK_BLD_MIB_NODE_CONTROL
		MibNodeControl_vInit(hMibNodeControl, &sMibNodeControl);
		/* Debug */
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n\ntypedef struct {");
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n} tsMibNodeControlPerm;                  // 000");
	#endif
	#if MK_BLD_MIB_ADC_STATUS
		MibAdcStatusPatch_vInit  (hMibAdcStatus, &sMibAdcStatus, BULB_ADC_MASK, BULB_DIO_OSC_PULL, BULB_ADC_PERIOD);
		/* Debug */
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n\ntypedef struct {");
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n} tsMibAdcStatus;                        // 000");
	#endif
	#if MK_BLD_MIB_NWK_CONFIG
		MibNwkConfigPatch_vInit  (hMibNwkConfig, &sMibNwkConfig, (void *) &sMibNwkStatus);
		/* Debug */
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n\ntypedef struct {");
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint8    u8DeviceType;               // %03d @ %03d", sizeof(sMibNwkConfig.sPerm.u8DeviceType               ),   offsetof(tsMibNwkConfigPerm, u8DeviceType               ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint8    u8StackModeInit;            // %03d @ %03d", sizeof(sMibNwkConfig.sPerm.u8StackModeInit            ),   offsetof(tsMibNwkConfigPerm, u8StackModeInit            ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16	u16PanId;                   // %03d @ %03d", sizeof(sMibNwkConfig.sPerm.u16PanId                   ),   offsetof(tsMibNwkConfigPerm, u16PanId                   ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16  u16FrameCounterDelta;        // %03d @ %03d", sizeof(sMibNwkConfig.sPerm.u16FrameCounterDelta       ),   offsetof(tsMibNwkConfigPerm, u16FrameCounterDelta       ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint32  u32NetworkId;                // %03d @ %03d", sizeof(sMibNwkConfig.sPerm.u32NetworkId               ),   offsetof(tsMibNwkConfigPerm, u32NetworkId               ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint32  u32ScanChannels;             // %03d @ %03d", sizeof(sMibNwkConfig.sPerm.u32ScanChannels            ),   offsetof(tsMibNwkConfigPerm, u32ScanChannels            ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint32  u32EndDeviceActivityTimeout; // %03d @ %03d", sizeof(sMibNwkConfig.sPerm.u32EndDeviceActivityTimeout),   offsetof(tsMibNwkConfigPerm, u32EndDeviceActivityTimeout));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n} tsMibNwkConfigPerm;                    // %03d",        sizeof(sMibNwkConfig.sPerm));

	#endif
	#if MK_BLD_MIB_NWK_PROFILE
		MibNwkProfile_vInit  (hMibNwkProfile, &sMibNwkProfile);
		/* Debug */
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n\ntypedef struct {");
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint8    u8MaxFailedPackets;         // %03d @ %03d", sizeof(sMibNwkProfile.sPerm.u8MaxFailedPackets       ), offsetof(tsMibNwkProfilePerm, u8MaxFailedPackets ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint8    u8MinBeaconLqi;             // %03d @ %03d", sizeof(sMibNwkProfile.sPerm.u8MinBeaconLqi           ), offsetof(tsMibNwkProfilePerm, u8MinBeaconLqi     ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint8    u8PrfBeaconLqi;             // %03d @ %03d", sizeof(sMibNwkProfile.sPerm.u8PrfBeaconLqi           ), offsetof(tsMibNwkProfilePerm, u8PrfBeaconLqi     ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16  u16RouterPingPeriod;         // %03d @ %03d", sizeof(sMibNwkProfile.sPerm.u16RouterPingPeriod      ), offsetof(tsMibNwkProfilePerm, u16RouterPingPeriod));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n} tsMibNwkProfilePerm;                   // %03d",        sizeof(sMibNwkProfile.sPerm));

	#endif
	#if MK_BLD_MIB_NWK_STATUS
		MibNwkStatus_vInit  (hMibNwkStatus, &sMibNwkStatus, MK_SECURITY, sMibNwkConfig.sPerm.u16FrameCounterDelta);
		/* Debug */
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n\ntypedef struct {");
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint8   u8UpMode;                    // %03d @ %03d", sizeof(sMibNwkStatus.sPerm.u8UpMode       ), offsetof(tsMibNwkStatusPerm, u8UpMode       ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16 u16UpCount;                   // %03d @ %03d", sizeof(sMibNwkStatus.sPerm.u16UpCount     ), offsetof(tsMibNwkStatusPerm, u16UpCount     ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint32 u32UpTime;                    // %03d @ %03d", sizeof(sMibNwkStatus.sPerm.u32UpTime      ), offsetof(tsMibNwkStatusPerm, u32UpTime      ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint32 u32DownTime;                  // %03d @ %03d", sizeof(sMibNwkStatus.sPerm.u32DownTime    ), offsetof(tsMibNwkStatusPerm, u32DownTime    ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint32 u32FrameCounter;              // %03d @ %03d", sizeof(sMibNwkStatus.sPerm.u32FrameCounter), offsetof(tsMibNwkStatusPerm, u32FrameCounter));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n} tsMibNwkStatusPerm;                    // %03d",        sizeof(sMibNwkStatus.sPerm));
	#endif
	#if MK_BLD_MIB_NWK_SECURITY
		#if MIB_NWK_SECURITY_PATCH
		{
			/* Call patched init function */
			MibNwkSecurityPatch_vInit (hMibNwkSecurity, &sMibNwkSecurity, MK_SECURITY);
		}
		#else
		{
			/* Call unpatched init function */
			MibNwkSecurity_vInit (hMibNwkSecurity, &sMibNwkSecurity, MK_SECURITY);
		}
		#endif
		/* Debug */
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n\ntypedef struct {");
    	DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint8          u8Channel;            // %03d @ %03d", sizeof(sMibNwkSecurity.sPerm.u8Channel),     offsetof(tsMibNwkSecurityPerm, u8Channel));
    	DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16        u16PanId;              // %03d @ %03d", sizeof(sMibNwkSecurity.sPerm.u16PanId),      offsetof(tsMibNwkSecurityPerm, u16PanId));
    	DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    tsSecurityKey asSecurityKey[3];      // %03d @ %03d", sizeof(sMibNwkSecurity.sPerm.asSecurityKey), offsetof(tsMibNwkSecurityPerm, asSecurityKey));
    	DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    MAC_ExtAddr_s asSecureAddr[%02d];      // %03d @ %03d", MIB_NWK_SECURITY_SECURE_ADDR_COUNT, sizeof(sMibNwkSecurity.sPerm.asSecureAddr),  offsetof(tsMibNwkSecurityPerm, asSecureAddr));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n} tsMibNwkSecurityPerm;                  // %03d",        sizeof(sMibNwkSecurity.sPerm));
	#endif
	#if MK_BLD_MIB_NWK_TEST
		MibNwkTest_vInit(hMibNwkTest, &sMibNwkTest);
		/* Debug */
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n\ntypedef struct {");
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n} tsMibNwkTestPerm;                      // 000");
	#endif
	#if MK_BLD_MIB_BULB_CONFIG
		MibBulbConfig_vInit (hMibBulbConfig, &sMibBulbConfig);
		/* Debug */
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n\ntypedef struct {");
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint8   u8LumRate;                   // %03d @ %03d", sizeof(sMibBulbConfig.sPerm.u8LumRate       ), offsetof(tsMibBulbConfigPerm, u8LumRate       ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint8   u8InitMode;                  // %03d @ %03d", sizeof(sMibBulbConfig.sPerm.u8InitMode      ), offsetof(tsMibBulbConfigPerm, u8InitMode      ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint8   u8InitLumTarget;             // %03d @ %03d", sizeof(sMibBulbConfig.sPerm.u8InitLumTarget ), offsetof(tsMibBulbConfigPerm, u8InitLumTarget ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint8   u8DownUpCadFlags;            // %03d @ %03d", sizeof(sMibBulbConfig.sPerm.u8DownUpCadFlags), offsetof(tsMibBulbConfigPerm, u8DownUpCadFlags));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16 u16DownCadTimer;              // %03d @ %03d", sizeof(sMibBulbConfig.sPerm.u16DownCadTimer ), offsetof(tsMibBulbConfigPerm, u16DownCadTimer ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16 u16UpCadTimer;                // %03d @ %03d", sizeof(sMibBulbConfig.sPerm.u16UpCadTimer   ), offsetof(tsMibBulbConfigPerm, u16UpCadTimer   ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint32 u32DownCadence;               // %03d @ %03d", sizeof(sMibBulbConfig.sPerm.u32DownCadence  ), offsetof(tsMibBulbConfigPerm, u32DownCadence  ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint32 u32UpCadence;                 // %03d @ %03d", sizeof(sMibBulbConfig.sPerm.u32UpCadence    ), offsetof(tsMibBulbConfigPerm, u32UpCadence    ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n} tsMibBulbConfigPerm;                   // %03d",        sizeof(sMibBulbConfig.sPerm));
	#endif
	#if MK_BLD_MIB_BULB_STATUS
		MibBulbStatus_vInit (hMibBulbStatus, &sMibBulbStatus, BULB_ADC_SRC_BUS_VOLTS);
		/* Debug */
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n\ntypedef struct {");
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16 u16OnCount;                   // %03d @ %03d", sizeof(sMibBulbStatus.sPerm.u16OnCount),  offsetof(tsMibBulbStatusPerm, u16OnCount));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    int16  i16ChipTemp;                  // %03d @ %03d", sizeof(sMibBulbStatus.sPerm.i16ChipTemp), offsetof(tsMibBulbStatusPerm, i16ChipTemp));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    int16  i16BusVolts;                  // %03d @ %03d", sizeof(sMibBulbStatus.sPerm.i16BusVolts), offsetof(tsMibBulbStatusPerm, i16BusVolts));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint32 u32OnTime;                    // %03d @ %03d", sizeof(sMibBulbStatus.sPerm.u32OnTime),   offsetof(tsMibBulbStatusPerm, u32OnTime));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint32 u32OffTime;                   // %03d @ %03d", sizeof(sMibBulbStatus.sPerm.u32OffTime),  offsetof(tsMibBulbStatusPerm, u32OffTime));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n} tsMibBulbStatusPerm;                   // %03d",        sizeof(sMibBulbStatus.sPerm));
	#endif
	#if MK_BLD_MIB_BULB_SCENE
		MibBulbScene_vInit  (hMibBulbScene, &sMibBulbScene, (void *) &sMibBulbControl);
		/* Debug */
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n\ntypedef struct {");
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint8  au8SceneMode[%02d];             // %03d @ %03d", MIB_BULB_SCENE_SCENES, sizeof(sMibBulbScene.sPerm.au8SceneMode     ),  offsetof(tsMibBulbScenePerm, au8SceneMode     ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint8  au8SceneLumTarget[%02d];        // %03d @ %03d", MIB_BULB_SCENE_SCENES, sizeof(sMibBulbScene.sPerm.au8SceneLumTarget),  offsetof(tsMibBulbScenePerm, au8SceneLumTarget));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16 au16SceneId[%02d];              // %03d @ %03d", MIB_BULB_SCENE_SCENES, sizeof(sMibBulbScene.sPerm.au16SceneId      ),  offsetof(tsMibBulbScenePerm, au16SceneId      ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n}  tsMibBulbScenePerm;                     // %03d", 	   sizeof(sMibBulbScene.sPerm));
	#endif
	#if MK_BLD_MIB_BULB_CONTROL
		MibBulbControl_vInit(hMibBulbControl, hMibDeviceControl, &sMibBulbControl, (void *) &sMibBulbStatus, (void *) &sMibBulbConfig, (void *) &sMibBulbScene);
		MibBulbControlPatch_vInit(&sMibBulbControl);
		/* Debug */
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n\ntypedef struct {");
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint8   u8NwkState;                    // %03d @ %03d", sizeof(sMibBulbControl.sPerm.u8NwkState ),  offsetof(tsMibBulbControlPerm, u8NwkState ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint8   u8Mode;                        // %03d @ %03d", sizeof(sMibBulbControl.sPerm.u8Mode     ),  offsetof(tsMibBulbControlPerm, u8Mode     ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint8   u8LumTarget;                   // %03d @ %03d", sizeof(sMibBulbControl.sPerm.u8LumTarget),  offsetof(tsMibBulbControlPerm, u8LumTarget));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n    uint16 u16SceneId;                     // %03d @ %03d", sizeof(sMibBulbControl.sPerm.u16SceneId  ),  offsetof(tsMibBulbControlPerm, u16SceneId  ));
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_PDM, "\n} tsMibBulbControlPerm;                    // %03d",        sizeof(sMibBulbControl.sPerm));
	#endif

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
}

/****************************************************************************
 *
 * NAME: Device_vReset
 *
 * DESCRIPTION:
 * Reset device
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void Device_vReset(bool_t bFactoryReset)
{
	/* Increment reset counter in node status mib */
	#if MK_BLD_MIB_NODE_STATUS
		MibNodeStatus_vIncrementResetCount();
	#endif

	/* FactoryReset ? */
	if (bFactoryReset)
	{
		/* Save or delete records as appropriate for a factory reset */
		#if MK_BLD_MIB_NODE
			PDM_vDeleteRecord(&sMibNode.sDesc);
		#endif
		#if MK_BLD_MIB_GROUP
			PDM_vDeleteRecord(&sMibGroup.sDesc);
		#endif
		#if MK_BLD_MIB_NODE_STATUS
			PDM_vSaveRecord  (&sMibNodeStatus.sDesc);
		#endif
		#if MK_BLD_MIB_NWK_CONFIG
			PDM_vDeleteRecord(&sMibNwkConfig.sDesc);
		#endif
		#if MK_BLD_MIB_NWK_PROFILE
			PDM_vDeleteRecord(&sMibNwkProfile.sDesc);
		#endif
		#if MK_BLD_MIB_NWK_STATUS
			PDM_vSaveRecord  (&sMibNwkStatus.sDesc);
		#endif
		#if MK_BLD_MIB_NWK_SECURITY
			PDM_vDeleteRecord(&sMibNwkSecurity.sDesc);
		#endif
		#if MK_BLD_MIB_BULB_CONFIG
			PDM_vDeleteRecord(&sMibBulbConfig.sDesc);
		#endif
		#if MK_BLD_MIB_BULB_STATUS
			PDM_vSaveRecord  (&sMibBulbStatus.sDesc);
		#endif
		#if MK_BLD_MIB_BULB_SCENE
			PDM_vDeleteRecord(&sMibBulbScene.sDesc);
		#endif
		#if MK_BLD_MIB_BULB_CONTROL
			PDM_vDeleteRecord(&sMibBulbControl.sDesc);
		#endif
	}
	/* Normal reset */
	else
	{
		/* Save all records to PDM */
		PDM_vSave();
	}

	/* Debug */
	DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\nvAHI_SwReset(%d)!                ", bFactoryReset);
	/* Reset */
	vAHI_SwReset();
}

/****************************************************************************
 *
 * NAME: Device_eJipInit
 *
 * DESCRIPTION:
 * Initialise JIP layer stack
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC teJIP_Status Device_eJipInit(void)
{
	teJIP_Status     eStatus;

	/* Debug */
	DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\nDevice_eJipInit()");
	/* Configure jip */
	sJipInitData.u64AddressPrefix       = CONFIG_ADDRESS_PREFIX; 		/* IPv6 address prefix (C only) */
	sJipInitData.u32Channel				= CONFIG_SCAN_CHANNELS;     	/* Channel 'bitmap' */
	sJipInitData.u16PanId				= 0xffff; /* PAN ID to use or look for (0xffff to search/generate) */
	sJipInitData.u16MaxIpPacketSize		= 0; /*CONFIG_PACKET_BUFFER_LEN-216;*/ /* Max IP packet size, 0 defaults to 1280 */
	sJipInitData.u16NumPacketBuffers	= 2;    						/* Number of IP packet buffers */
	sJipInitData.u8UdpSockets			= 2;           					/* Number of UDP sockets supported */
	sJipInitData.eDeviceType			= E_6LP_ROUTER;            		/* Device type (C, R, or ED) */
	sJipInitData.u32RoutingTableEntries	= CONFIG_ROUTING_TABLE_ENTRIES; /* Routing table size (not ED) */
	sJipInitData.u32DeviceId			= MK_JIP_DEVICE_ID;
	sJipInitData.u8UniqueWatchers		= CONFIG_UNIQUE_WATCHERS;
	sJipInitData.u8MaxTraps				= CONFIG_MAX_TRAPS;
	sJipInitData.u8QueueLength 			= CONFIG_QUEUE_LENGTH;
	sJipInitData.u8MaxNameLength		= CONFIG_MAX_NAME_LEN;
	sJipInitData.u16Port				= JIP_DEFAULT_PORT;
	sJipInitData.pcVersion 				= MK_VERSION;

	/* Initialise data from network config mib */
	#if MK_BLD_MIB_NWK_CONFIG
		MibNwkConfig_vJipInitData(&sJipInitData);
	#endif

	/* Initialise data from network security mib */
	#if MK_BLD_MIB_NWK_SECURITY
		MibNwkSecurity_vJipInitData(&sJipInitData);
	#endif
	/* Initialise */
	eStatus = eJIP_Init(&sJipInitData);
	/* Debug */
	DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\neJIP_Init()=%d", eStatus);

	/* set 1 second defrag timeout */
	v6LP_SetPacketDefragTimeout(1);

	/* Debug */
	DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\n\tHeapMax=%x StackMin=%x", u32ExceptionHeapMax, u32ExceptionStackMin);

	return eStatus;
}

/****************************************************************************
 *
 * NAME: v6LP_ConfigureNetwork
 *
 * DESCRIPTION:
 * Configures network
 *
 ****************************************************************************/
PUBLIC void v6LP_ConfigureNetwork(tsNetworkConfigData *psNetworkConfigData)
{
	uint8   u8UpMode 	    = 0;
	uint32 u32FrameCounter  = 0;
	uint8   u8StackModeInit = 0;

	/* Debug */
	DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\nv6LP_ConfigureNetwork()");

	/* Get MAC and PIB pointers */
	pvMac = pvAppApiGetMacHandle();
	psPib = MAC_psPibGetHandle(pvMac);

	/* Using Node MIB ? */
	#if MK_BLD_MIB_NODE
	{
		/* Did we not load a name from flash ? */
		if (sMibNode.sPerm.acName[0] == '\0')
		{
			uint8 		  u8NameLen;
			char          acHex[17]="0123456789ABCDEF";
			uint64      *pu64MacAddr;

			/* Get pointer to MAC address */
			pu64MacAddr = (uint64 *) pvAppApiGetMacAddrLocation();
			#ifdef MK_DEVICE_NAME
			/* Copy base device name */
			strncpy(sMibNode.sPerm.acName, MK_DEVICE_NAME, 8);
			#else
			/* Copy base device name */
			strcpy(sMibNode.sPerm.acName, "Bulb");
			#endif
			/* Note length */
			u8NameLen = strlen(sMibNode.sPerm.acName);
			/* Append least significant 6 digits of mac address */
			sMibNode.sPerm.acName[u8NameLen    ] = ' ';
			sMibNode.sPerm.acName[u8NameLen + 1] = acHex[((*pu64MacAddr >> 20) & 0xf)];
			sMibNode.sPerm.acName[u8NameLen + 2] = acHex[((*pu64MacAddr >> 16) & 0xf)];
			sMibNode.sPerm.acName[u8NameLen + 3] = acHex[((*pu64MacAddr >> 12) & 0xf)];
			sMibNode.sPerm.acName[u8NameLen + 4] = acHex[((*pu64MacAddr >>  8) & 0xf)];
			sMibNode.sPerm.acName[u8NameLen + 5] = acHex[((*pu64MacAddr >>  4) & 0xf)];
			sMibNode.sPerm.acName[u8NameLen + 6] = acHex[((*pu64MacAddr      ) & 0xf)];
			/* Debug */
			DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_VARS, "\n\tsMibNode.sPerm.acName=%s", sMibNode.sPerm.acName);
		}

	}
	#endif

	/* Using network control MIB ? */
	#if MK_BLD_MIB_NWK_SECURITY
	{
		uint8 u8Key;
		/* Loop through commissioning keys */
		for (u8Key = 1; u8Key < 3; u8Key++)
		{
			/* Did we not load a valid commissioning key from flash ? */
			if (sMibNwkSecurity.sPerm.asSecurityKey[u8Key].u32KeyVal_1 == 0 &&
				sMibNwkSecurity.sPerm.asSecurityKey[u8Key].u32KeyVal_2 == 0 &&
				sMibNwkSecurity.sPerm.asSecurityKey[u8Key].u32KeyVal_3 == 0 &&
				sMibNwkSecurity.sPerm.asSecurityKey[u8Key].u32KeyVal_4 == 0)
			{
				/* Generate commissioning key from MAC address */
				Security_vBuildCommissioningKey((uint8 *) pvAppApiGetMacAddrLocation(),
												(uint8 *) &sMibNwkSecurity.sPerm.asSecurityKey[u8Key]);

				/* Save to flash */
				sMibNwkSecurity.bSaveRecord = TRUE;
				/* Debug */
				DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_VARS, "\n\tasSecurityKey[%d]   = %x:%x:%x:%x",
					u8Key,
					sMibNwkSecurity.sPerm.asSecurityKey[u8Key].u32KeyVal_1,
					sMibNwkSecurity.sPerm.asSecurityKey[u8Key].u32KeyVal_2,
					sMibNwkSecurity.sPerm.asSecurityKey[u8Key].u32KeyVal_3,
					sMibNwkSecurity.sPerm.asSecurityKey[u8Key].u32KeyVal_4);
			}
		}
	}
	#endif

	#if MK_BLD_MIB_NWK_CONFIG
		/* Allow network config mib to set its settings and register callbacks */
		MibNwkConfig_vNetworkConfigData(psNetworkConfigData);
		/* Update user data */
		MibNwkConfigPatch_vSetUserData();
	#endif

	/* NwkStatus MIB included ? */
	#if MK_BLD_MIB_NWK_STATUS
	{
		/* Set UpMode and FrameCounter from netwrok status mib */
		u8UpMode = sMibNwkStatus.sPerm.u8UpMode;
		u32FrameCounter = sMibNwkStatus.sPerm.u32FrameCounter;
	}
	#endif

	/* Network config MIB included ? */
	#if MK_BLD_MIB_NWK_CONFIG
	{
		/* Set UpMode and FrameCounter from netwrok status mib */
		u8StackModeInit = sMibNwkConfig.sPerm.u8StackModeInit;
	}
	#endif

	/* Running patched network security */
	#if (MIB_NWK_SECURITY_PATCH)
	{
		/* Up mode is gateway - start in standalone mode until the announce picks us up */
		if (u8UpMode == MIB_NWK_STATUS_UP_MODE_GATEWAY)
		{
			u8UpMode = MIB_NWK_STATUS_UP_MODE_STANDALONE;
			/* Debug */
			//DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_VARS, "\n\tOVERRIDE u8UpMode=%d", sMibNwkStatus.sPerm.u8UpMode);
		}
	}
	#endif

	/* Network contorl MIB included ? */
	#if MK_BLD_MIB_NWK_SECURITY
	{
		/* Get network security set up correctly */
		u8UpMode = MibNwkSecurity_u8NetworkConfigData(psNetworkConfigData, u8UpMode, u32FrameCounter, u8StackModeInit);
	}
	#endif

	/* NwkStatus MIB included ? */
	#if MK_BLD_MIB_NWK_STATUS
	{
		/* Copy the final up mode value */
		sMibNwkStatus.sPerm.u8UpMode = u8UpMode;
		/* Debug */
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_VARS, "\n\tsMibNwkStatus.sPerm.u8UpMode=%d", sMibNwkStatus.sPerm.u8UpMode);
	}
	#endif

	#if MK_BLD_MIB_NWK_PROFILE
	{
		/* Override scan sort handler with patched beacon response handler */
		vApi_RegScanSortCallback(MibNwkProfile_bScanSortCallback);
		/* Ensure the configured profile is applied */
		MibNwkProfile_vApply();
	}
	#else
	{
		/* Set to default join profile */
		(void) bJnc_SetJoinProfile(CONFIG_JOIN_PROFILE, NULL);
	}
	#endif

	#if CONFIG_DBG_DEVICE_BULB_VARS
	{
		tsNwkProfile sNwkProfile;

		/* Read back the settings */
		vJnc_GetNwkProfile(&sNwkProfile);

		/* Debug */
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_VARS, "\nCONFIG NwkProfile St=%d Chd=%d Slp=%d Fld=%d Ttl=%d Png=%d Lqi=%d Scn=%d Scx=%d Esn=%d Esx=%d",
			u8Api_GetStackState(),
			sNwkProfile.u8MaxChildren,
			sNwkProfile.u8MaxSleepingChildren,
			sNwkProfile.u8MaxFailedPkts,
			sNwkProfile.u8MaxBcastTTL,
			sNwkProfile.u16RouterPingPeriod,
			sNwkProfile.u8MinBeaconLQI,
			sNwkProfile.u16ScanBackOffMin,
			sNwkProfile.u16ScanBackOffMax,
			sNwkProfile.u16EstRtBackOffMin,
			sNwkProfile.u16EstRtBackOffMax);
	}
	#endif
}

/****************************************************************************
 *
 * NAME: Device_vMibRegister
 *
 * DESCRIPTION:
 * Register MIbs with jip
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void Device_vMibRegister(void)
{
	/* Register mib variables (don't bother to register ADC mib) */
	#if MK_BLD_MIB_NODE
		MibNode_vRegister();
	#endif
	#if MK_BLD_MIB_GROUP
		MibGroup_vRegister();
	#endif
	#if MK_REG_MIB_NODE_STATUS
		MibNodeStatus_vRegister();
	#endif
	#if MK_REG_MIB_NODE_CONTROL
		MibNodeControl_vRegister();
	#endif
	#if MK_REG_MIB_NWK_CONFIG
		MibNwkConfig_vRegister();
	#endif
	#if MK_REG_MIB_NWK_PROFILE
		MibNwkProfile_vRegister();
	#endif
	#if MK_REG_MIB_NWK_STATUS
		MibNwkStatus_vRegister();
	#endif
	#if MK_REG_MIB_NWK_SECURITY
		MibNwkSecurity_vRegister();
	#endif
	#if MK_REG_MIB_NWK_TEST
		MibNwkTest_vRegister();
	#endif
	#if MK_REG_MIB_ADC_STATUS
		MibAdcStatus_vRegister();
	#endif
	#if MK_REG_MIB_BULB_CONFIG
		MibBulbConfig_vRegister();
	#endif
	#if MK_REG_MIB_BULB_STATUS
		MibBulbStatus_vRegister();
	#endif
	#if MK_REG_MIB_BULB_SCENE
		MibBulbScene_vRegister();
	#endif
	#if MK_REG_MIB_BULB_CONTROL
		MibBulbControl_vRegister();
		MibBulbControl_vDeviceControlRegister();
	#endif
}

/****************************************************************************
 *
 * NAME: v6LP_DataEvent
 *
 * DESCRIPTION:
 * As this app uses JIP for all communication we are not interested in the
 * 6LP data events. For any receive events we simply discard the packet to
 * free the packet buffer.
 *
 * PARAMETERS: Name        RW  Usage
 *             iSocket     R   Socket on which packet received
 *             eEvent      R   Data event
 *             psAddr      R   Source address (for RX) or destination (for TX)
 *             u8AddrLen   R   Length of address
 *
 ****************************************************************************/
PUBLIC void v6LP_DataEvent(int iSocket, te6LP_DataEvent eEvent,
                           ts6LP_SockAddr *psAddr, uint8 u8AddrLen)
{
	/* Which event ? */
	switch (eEvent)
	{
		/* Data received ? */
		/* IP data received ? */
		/* 6LP ICMP message ? */
		case E_DATA_RECEIVED:
		case E_IP_DATA_RECEIVED:
		case E_6LP_ICMP_MESSAGE:
		{
			/* Debug */
			DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\nv6LP_DataEvent(UNEXPECTED)");
			/* Discard 6LP packets as only interested in JIP communication  */
			i6LP_RecvFrom(iSocket, NULL, 0, 0, NULL, NULL);
		}
		break;

		/* Others ? */
		default:
		{
			/* Do nothing */
			;
		}
		break;
	}
}

/****************************************************************************
 *
 * NAME: vJIP_StackEvent
 *
 * DESCRIPTION:
 * Processes any incoming stack events.
 * Once a join indication has been received, we initialise JIP and register
 * the various MIBs.
 *
 * PARAMETERS: Name          RW Usage
 *             eEvent        R  Stack event
 *             pu8Data       R  Additional information associated with event
 *             u8DataLen     R  Length of additional information
 *
 ****************************************************************************/
PUBLIC void vJIP_StackEvent(te6LP_StackEvent eEvent, uint8 *pu8Data, uint8 u8DataLen)
{
	bool_t bSuppress = FALSE;

	/* Which event ? */
    switch (eEvent)
    {
    	/* Joined network ? */
		case E_STACK_JOINED:
		{
			/* Debug */
			DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\n\nvJIP_StackEvent(E_STACK_JOINED %s) %d", ((u16Api_GetStackMode() & STACK_MODE_STANDALONE) ? "STANDALONE" : "GATEWAY"),  bSuppress);
			/* Debug */
			DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\n\tHeapMax=%x StackMin=%x", u32ExceptionHeapMax, u32ExceptionStackMin);

			/* UART ? */
			#ifdef UART_H_INCLUDED
			{
				/* Debug */
				UART_vNumber(u32Second, 10);
				UART_vString(" JOIN\r\n");
			}
			#endif

			/* OND ? */
			#ifdef OND_H_INCLUDED
				/* Initialise OND */
	    		eOND_DevInit();
		    #else
		    	#warning OND IS DISABLED!!!
		    #endif

		    /* Commissioning timeout set ? */
		    #if (BULB_DEFAULT_DEVICE_COMMISSION_TIMEOUT > 0)
			#if MK_BLD_MIB_NODE_CONTROL && MK_BLD_MIB_NWK_STATUS
		    {
				/* Have we joined in standalone mode (rather than resumed) ? */
				if ((u16Api_GetStackMode() & STACK_MODE_STANDALONE) != 0 &&
					sMibNwkStatus.sPerm.u8UpMode == MIB_NWK_STATUS_UP_MODE_NONE)
				{
					DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\n\tFactoryResetTimeout SET");
					/* Start the factory reset timer */
					sMibNodeControl.sTemp.u16FactoryReset = BULB_DEFAULT_DEVICE_COMMISSION_TIMEOUT;
				}
			}
			#endif
			#endif

			/* MibNwkConfig included ? */
			#if MK_BLD_MIB_NWK_CONFIG
			{
				/* Call set user data function */
				MibNwkConfigPatch_vSetUserData();
			}
			#endif
		}
		break;

		/* Lost network ? */
		case E_STACK_RESET:
		{
			/* Debug */
			DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\n\nvJIP_StackEvent(E_STACK_RESET %s) %d", ((u16Api_GetStackMode() & STACK_MODE_STANDALONE) ? "STANDALONE" : "GATEWAY"), bSuppress);

			/* UART ? */
			#ifdef UART_H_INCLUDED
			{
				/* Debug */
				UART_vNumber(u32Second, 10);
				UART_vString(" RSET\r\n");
			}
			#endif

			/* Were we deliberately resetting the stack to gateway mode ? */
			if (u8NwkSecurityReset == MIB_NWK_SECURITY_RESET_STACK_TO_GATEWAY)
			{
				DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\n\tTO_GATEWAY");
				/* Suppress passing this event to other modules */
				bSuppress = TRUE;
				/* Stack should now be idle -  clear reset state */
				u8NwkSecurityReset = MIB_NWK_SECURITY_RESET_NONE;
				/* Set up stack to resume running in gateway mode (IGNORE RETURNED UP MODE) */
				(void) MibNwkSecurity_u8ResumeGateway(sMibNwkSecurity.psNetworkConfigData,
												 	  sMibNwkStatus.sPerm.u8UpMode,
												 	  psPib->u32MacFrameCounter);
				/* Debug */
				DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\n\tvApi_SetStackMode(0)");
				/* Ensure we are in gateway mode */
				vApi_SetStackMode(0);
				/* Ensure we are using correct profile for gateway system */
				MibNwkSecurity_vSetProfile(FALSE);
				/* Debug */
				DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\n\tvApi_EnableJoin");
				/* Don't skip joining procedure */
				vApi_EnableJoin();
				/* Debug */
				DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\n\tvApi_ReStartCR(FALSE)");
				/* Restart the stack */
				vApi_ReStartCR(FALSE);
			}
			/* Are we deliberately resetting the stack to standalone mode ? */
			else if (u8NwkSecurityReset == MIB_NWK_SECURITY_RESET_STACK_TO_STANDALONE)
			{
				DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\n\tTO_STANDALONE");
				/* Suppress passing this event to other modules */
				bSuppress = TRUE;
				/* Stack should now be idle -  clear reset state */
				u8NwkSecurityReset = MIB_NWK_SECURITY_RESET_NONE;
				/* Update network status */
				sMibNwkStatus.sPerm.u8UpMode = MIB_NWK_STATUS_UP_MODE_STANDALONE;
				sMibNwkStatus.bSaveRecord    = TRUE;
				/* Set up stack to resume running in standalone mode */
				(void)	MibNwkSecurity_u8ResumeStandalone(sMibNwkSecurity.psNetworkConfigData,
				 		 							  	  sMibNwkStatus.sPerm.u8UpMode,
												 		  psPib->u32MacFrameCounter);
				/* Debug */
				DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\n\tvApi_ReStartCR(FALSE)");
				/* Restart the stack */
				vApi_ReStartCR(FALSE);
			}
			/* Never joined a network ? */
			else if (sMibNwkStatus.sPerm.u8UpMode == MIB_NWK_STATUS_UP_MODE_NONE)
			{
				/* Re-apply default join profile */
				(void) bJnc_SetJoinProfile(CONFIG_JOIN_PROFILE, NULL);
			}
		}
		break;

		/* Gateway present ? */
		case E_STACK_GATEWAY_STARTED:
		{
			/* Debug */
			DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\n\nvJIP_StackEvent(E_STACK_GATEWAY_STARTED) %d", bSuppress);

			/* UART ? */
			#ifdef UART_H_INCLUDED
			{
				/* Debug */
				UART_vNumber(u32Second, 10);
				UART_vString(" GWAY\r\n");
			}
			#endif
		}
		break;

		/* Others ? */
	    default:
		{
			/* Debug */
			DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\n\nvJIP_StackEvent(%d)", eEvent);
			/* Do nothing */
			;
		}
        break;
    }

	/* Pass on to mibs */
	#if MK_BLD_MIB_GROUP
		MibGroup_vStackEvent(eEvent);
	#endif
	#if MK_BLD_MIB_NWK_STATUS
		/* Not stack reset ? */
		if (eEvent != E_STACK_RESET)
		{
			/* Pass on to MIB */
			MibNwkStatus_vStackEvent(eEvent);
		}
		/* Stack reset (override standard handling to avoid up mode being changed) ? */
		else
		{
			/* Was network up previously ? */
			if (TRUE == sMibNwkStatus.bUp)
			{
				/* Debug */
				DBG_vPrintf(CONFIG_DBG_MIB_NWK_STATUS, "\nMibNwkStatus_vStackEvent(STACK_RESET)");
				/* Clear up flag */
				sMibNwkStatus.bUp = FALSE;
			}
		}
	#endif
	#if MK_BLD_MIB_NWK_SECURITY
	{
		/* Patch network security MIB ? */
		#if MIB_NWK_SECURITY_PATCH
		{
			/* Not suppressing ? */
			if (FALSE == bSuppress)
			{
				/* Pass to patched NwkSecurity MIB  */
				u8NwkSecurityReset = MibNwkSecurityPatch_u8StackEvent(eEvent, pu8Data, u8DataLen);
				/* Need to reset ? */
				switch (u8NwkSecurityReset)
				{
					/* Factory reset ? */
					case MIB_NWK_SECURITY_RESET_FACTORY:
					{
						/* Clear reset type */
						u8NwkSecurityReset = MIB_NWK_SECURITY_RESET_NONE;
						/* Perform a factory reset */
						Device_vReset(TRUE);
					}
					break;
					/* Chip reset ? */
					case MIB_NWK_SECURITY_RESET_CHIP:
					{
						/* Clear reset type */
						u8NwkSecurityReset = MIB_NWK_SECURITY_RESET_NONE;
						/* Perform a factory reset */
						Device_vReset(FALSE);
					}
					break;
					/* Reset stack ? */
					case MIB_NWK_SECURITY_RESET_STACK_TO_GATEWAY:
					case MIB_NWK_SECURITY_RESET_STACK_TO_STANDALONE:
					{
						/* Debug */
						DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\n\tbApi_ResetStack(FALSE, 0x00)");
						/* Reset the stack to idle mode - which will cause a re-entrant call with another stack reset (handled above) */
						while(FALSE == bApi_ResetStack(FALSE, 0x00));
						DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\n\tbApi_ResetStack() DONE");
					}
					break;
				}
			}
		}
		/* Unpatched network security MIB ? */
		#else
		{
			/* Pass to NwkSecurity MIB - need to reset ? */
			if (MibNwkSecurity_bStackEvent(eEvent, pu8Data, u8DataLen) == TRUE)
			{
				/* Perform a reset */
				Device_vReset(FALSE);
			}
		}
		#endif
	}
	#endif
	#if MK_BLD_MIB_NWK_TEST
		MibNwkTest_vStackEvent(eEvent, pu8Data, u8DataLen);
	#endif
	#if MK_BLD_MIB_BULB_CONTROL
		MibBulbControlPatch_vStackEvent(eEvent);
	#endif
	#if MK_BLD_MIB_NWK_PROFILE
		/* Ensure the configured profile is applied */
		MibNwkProfile_vApply();
	#endif

	#if CONFIG_DBG_DEVICE_BULB_VARS
	{
		tsNwkProfile sNwkProfile;

		/* Read back the settings */
		vJnc_GetNwkProfile(&sNwkProfile);

		/* Debug */
		DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_VARS, "\nEVENT NwkProfile St=%d Chd=%d Slp=%d Fld=%d Ttl=%d Png=%d Lqi=%d Scn=%d Scx=%d Esn=%d Esx=%d",
			u8Api_GetStackState(),
			sNwkProfile.u8MaxChildren,
			sNwkProfile.u8MaxSleepingChildren,
			sNwkProfile.u8MaxFailedPkts,
			sNwkProfile.u8MaxBcastTTL,
			sNwkProfile.u16RouterPingPeriod,
			sNwkProfile.u8MinBeaconLQI,
			sNwkProfile.u16ScanBackOffMin,
			sNwkProfile.u16ScanBackOffMax,
			sNwkProfile.u16EstRtBackOffMin,
			sNwkProfile.u16EstRtBackOffMax);
	}
	#endif

	DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_FUNC, "\n\tvJIP_StackEvent() EXIT");
}

/****************************************************************************
 *
 * NAME: v6LP_PeripheralEvent
 *
 * DESCRIPTION:
 * Processes any incoming peripheral events. The end device is completely
 * event driven (the idle loop does nothing) and events here are simply used
 * to update the appropriate JIP module.
 *
 * PARAMETERS: Name          RW Usage
 *             u32Device     R  Device that caused peripheral event
 *             u32ItemBitmap R  Events within that peripheral
 *
 ****************************************************************************/
PUBLIC void v6LP_PeripheralEvent(uint32 u32Device, uint32 u32ItemBitmap)
{
	/* Tick Timer is run by stack at 10ms intervals */
	if (u32Device == E_AHI_DEVICE_TICK_TIMER)
	{
		/* Initialised ? */
		if (bInitialised == TRUE)
		{
			/* Increment pending ticks */
			if (u8TickQueue < 100) u8TickQueue++;
		}
	}

	/* ADC reading ? */
	if (u32Device == E_AHI_DEVICE_ANALOGUE)
	{
		uint8 u8Adc;

		/* Pass on to mib and thermal control loop*/
		#if MK_BLD_MIB_ADC_STATUS
			u8Adc = MibAdcStatusPatch_u8Analogue();
           #ifdef MK_BLD_THERMAL_CONTROL_LOOP
				if (u8Adc ==  E_AHI_ADC_SRC_TEMP)
				{
					vDecimator(MibAdcStatusPatch_i16DeciCentigrade(u8Adc)/10);
				}
           #endif
		#endif

		#if MK_BLD_MIB_BULB_STATUS
			MibBulbStatusPatch_vAnalogue(&sMibBulbStatus, u8Adc);
		#endif
	}
}

/****************************************************************************
 *
 * NAME: Device_vTick
 *
 * DESCRIPTION:
 * Reset device
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void Device_vTick(void)
{
	/* Tick timer fired ? */
	while (u8TickQueue > 0)
	{
		/* Increment tick counter */
		u8Tick++;

		/* A second has passed ? */
		if (u8Tick >= 100)
		{
			/* Zero tick counter */
			u8Tick = 0;
			/* Increment second counter */
			u32Second++;

			/* UART ? */
			#ifdef UART_H_INCLUDED
			{
				/* One minute ? */
				if (u32Second % 60 == 0)
				{
					/* Debug */
					UART_vNumber(u32Second, 10);
					UART_vString(" 1MIN\r\n");
				}
			}
			#endif

			#if CONFIG_DBG_DEVICE_BULB_VARS
			if (u32Second % 60 == 0)
			{
				tsNwkProfile sNwkProfile;

				/* Read back the settings */
				vJnc_GetNwkProfile(&sNwkProfile);

				/* Debug */
				DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_VARS, "\nTICK   NwkProfile St=%d Chd=%d Slp=%d Fld=%d Ttl=%d Png=%d Lqi=%d Scn=%d Scx=%d Esn=%d Esx=%d",
					u8Api_GetStackState(),
					sNwkProfile.u8MaxChildren,
					sNwkProfile.u8MaxSleepingChildren,
					sNwkProfile.u8MaxFailedPkts,
					sNwkProfile.u8MaxBcastTTL,
					sNwkProfile.u16RouterPingPeriod,
					sNwkProfile.u8MinBeaconLQI,
					sNwkProfile.u16ScanBackOffMin,
					sNwkProfile.u16ScanBackOffMax,
					sNwkProfile.u16EstRtBackOffMin,
					sNwkProfile.u16EstRtBackOffMax);
			}
			#endif

			/* Join timeout configured ? */
			#if BULB_DEFAULT_DEVICE_JOIN_TIMEOUT
			{
				/* Should we stop trying to join now ? */
				if (u32Second == BULB_DEFAULT_DEVICE_JOIN_TIMEOUT &&
				    sMibNwkStatus.sPerm.u8UpMode == MIB_NWK_STATUS_UP_MODE_NONE)
				{
					/* Reset to idle */
					while(FALSE == bApi_ResetStack(FALSE, 0x00));
				}
			}
			#endif
			/* Update stack min address */
			Exception_vUpdateStackMin();
		}

		/* Pass seconds onto mibs (spread across a second) */
		#if MK_BLD_MIB_NODE
			if (u8Tick == 0 ) MibNode_vSecond();
		#endif
		#if MK_BLD_MIB_GROUP
			if (u8Tick == 8 ) MibGroup_vSecond();
		#endif
		#if MK_BLD_MIB_NODE_STATUS
			if (u8Tick == 16) MibNodeStatus_vSecond();
		#endif
		#if MK_BLD_MIB_NODE_STATUS
			if (u8Tick == 24) MibNodeControl_vSecond();
		#endif
		#if MK_BLD_MIB_NWK_CONFIG
			if (u8Tick == 32) MibNwkConfig_vSecond();
		#endif
		#if MK_BLD_MIB_NWK_PROFILE
			if (u8Tick == 36) MibNwkProfile_vSecond();
		#endif
		#if MK_BLD_MIB_NWK_STATUS
			if (u8Tick == 40) MibNwkStatus_vSecond();
		#endif
		#if MK_BLD_MIB_NWK_SECURITY
			if (u8Tick == 48) MibNwkSecurityPatch_vSecond();
		#endif
		#if MK_BLD_MIB_BULB_CONFIG
			if (u8Tick == 56) MibBulbConfig_vSecond();
		#endif
		#if MK_BLD_MIB_BULB_STATUS
			if (u8Tick == 64) MibBulbStatus_vSecond();
		#endif
		#if MK_BLD_MIB_BULB_SCENE
			if (u8Tick == 80) MibBulbScene_vSecond();
		#endif
		#if MK_BLD_MIB_BULB_CONTROL
			if (u8Tick == 88)
			{
				#if MIB_BULB_CONTROL_PATCH_FAILED
					MibBulbControlPatch_vSecond();
				#endif
				MibBulbControl_vSecond();
			}
		#endif

		/* Pass tick on to mibs */
		#if MK_BLD_MIB_ADC_STATUS
			MibAdcStatus_vTick();
		#endif
		#if MK_BLD_MIB_NWK_TEST
			MibNwkTest_vTick();
		#endif
		#if MK_BLD_MIB_BULB_CONTROL
			MibBulbControl_vTick();
		#endif

		/* Call 6LP tick for last tick in queue */
		if (u8TickQueue == 1)
		{
			/* Call 6LP tick for last entry in queue */
			v6LP_Tick();
			/* Debug out parent lqi */
			#if CONFIG_DBG_DEVICE_BULB_VARS
			{
				static uint8 u8LastParentLqi;
				uint8        u8ThisParentLqi;
				u8ThisParentLqi = MibBulbControl_u8ParentLqi();
				if (u8ThisParentLqi != u8LastParentLqi)
				{
					/* Debug */
					DBG_vPrintf(CONFIG_DBG_DEVICE_BULB_VARS, "\n\tParentLqi = %d", u8ThisParentLqi);
					u8LastParentLqi = u8ThisParentLqi;
				}
			}
			#endif
		}
		/* Decrement pending ticks */
		if (u8TickQueue > 0) u8TickQueue--;
	}
}

/****************************************************************************
 *
 * NAME: Device_vException
 *
 * DESCRIPTION:
 *
 ****************************************************************************/
PUBLIC WEAK void Device_vException(uint32 u32HeapAddr, uint32 u32Vector, uint32 u32Code)
{
	/* Production build ? */
	#if MK_PRODUCTION
	{
		/* Reset */
		vAHI_SwReset();
	}
	#else
	{
		uint32 i = 0;
		volatile uint32 n;

		DriverBulb_vOn();
		DriverBulb_vSetLevel(0);

		while (1)
		{

			for(n = 0; n < 5000000; n++);

			for(i = 0; i < u32Code; i++)
			{
				for(n = 0; n < 2000000; n++);
				DriverBulb_vSetLevel(255);
				for(n = 0; n < 2000000; n++);
				DriverBulb_vSetLevel(0);
			}
		}
	}
	#endif
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
