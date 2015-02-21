/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         BulbConfig MIB Implementation
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
/* JenOS includes */
#include <dbg.h>
#include <dbg_uart.h>
#include <os.h>
#include <pdm.h>
/* Application common includes */
#include "MibBulb.h"
#include "MibBulbDebug.h"
#include "Table.h"
/* Application device includes */
#include "MibBulbConfig.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

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
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE tsMibBulbConfig  *psMibBulbConfig;			/* Bulb Config Mib data */

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: MibBulbConfig_vInit
 *
 * DESCRIPTION:
 * Initialises data
 *
 ****************************************************************************/
PUBLIC void MibBulbConfig_vInit(thJIP_Mib          hMibBulbConfigInit,
								tsMibBulbConfig   *psMibBulbConfigInit)
{
	/* Valid data pointer ? */
	if (psMibBulbConfigInit != (tsMibBulbConfig *) NULL)
	{
		PDM_teStatus   ePdmStatus;

		/* Debug */
		DBG_vPrintf(CONFIG_DBG_MIB_BULB_CONFIG, "\nMibBulbConfig_vInit() {%d}", sizeof(tsMibBulbConfig));

		/* Take copy of pointer to data */
		psMibBulbConfig = psMibBulbConfigInit;

		/* Take a copy of the MIB handle */
		psMibBulbConfig->hMib = hMibBulbConfigInit;

		/* Load BulbStatus mib data */
		ePdmStatus = PDM_eLoadRecord(&psMibBulbConfig->sDesc,
#if defined(JENNIC_CHIP_FAMILY_JN514x)
									 "MibBulbConfig",
#else
									 (uint16)(MIB_ID_BULB_CONFIG & 0xFFFF),
#endif
									 (void *) &psMibBulbConfig->sPerm,
									 sizeof(psMibBulbConfig->sPerm),
									 FALSE);
	}
}

/****************************************************************************
 *
 * NAME: MibBulbConfig_vRegister
 *
 * DESCRIPTION:
 * Registers MIB
 *
 ****************************************************************************/
PUBLIC void MibBulbConfig_vRegister(void)
{
	teJIP_Status eStatus;

	/* Register MIB */
	eStatus = eJIP_RegisterMib(psMibBulbConfig->hMib);
	/* Debug */
	DBG_vPrintf(CONFIG_DBG_MIB_BULB_CONFIG, "\nMibBulbConfig_vRegister()");
	DBG_vPrintf(CONFIG_DBG_MIB_BULB_CONFIG, "\n\teJIP_RegisterMib(BulbConfig)=%d", eStatus);

	/* Make sure permament data is saved */
	psMibBulbConfig->bSaveRecord = TRUE;
}

/****************************************************************************
 *
 * NAME: MibBulbConfig_vSecond
 *
 * DESCRIPTION:
 * Timer function
 *
 ****************************************************************************/
PUBLIC void MibBulbConfig_vSecond(void)
{
	/* Need to save record ? */
	if (psMibBulbConfig->bSaveRecord)
	{
		/* Clear flag */
		psMibBulbConfig->bSaveRecord = FALSE;
		/* Make sure permament data is saved */
		PDM_vSaveRecord(&psMibBulbConfig->sDesc);
		/* Debug */
		DBG_vPrintf(CONFIG_DBG_MIB_BULB_CONFIG, "\nMibBulbConfig_vSecond()");
		DBG_vPrintf(CONFIG_DBG_MIB_BULB_CONFIG, "\n\tPDM_vSaveRecord(MibBulbConfig)");
	}
}

/****************************************************************************
 *
 * NAME: MibBulbConfig_eSetUint8
 *
 * DESCRIPTION:
 * Generic set data callback
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbConfig_eSetUint8(uint8 u8Val, void *pvCbData)
{
	teJIP_Status eReturn;

	/* Call standard function */
	eReturn = eSetUint8(u8Val, pvCbData);

	/* Make sure permament data is saved */
	psMibBulbConfig->bSaveRecord = TRUE;

	return eReturn;
}

/****************************************************************************
 *
 * NAME: MibBulbConfig_eSetUint16
 *
 * DESCRIPTION:
 * Generic set data callback
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbConfig_eSetUint16(uint16 u16Val, void *pvCbData)
{
	teJIP_Status eReturn;

	/* Call standard function */
	eReturn = eSetUint16(u16Val, pvCbData);

	/* Make sure permament data is saved */
	psMibBulbConfig->bSaveRecord = TRUE;

	return eReturn;
}

/****************************************************************************
 *
 * NAME: MibBulbConfig_eSetUint32
 *
 * DESCRIPTION:
 * Generic set data callback
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbConfig_eSetUint32(uint32 u32Val, void *pvCbData)
{
	teJIP_Status eReturn;

	/* Call standard function */
	eReturn = eSetUint32(u32Val, pvCbData);

	/* Make sure permament data is saved */
	psMibBulbConfig->bSaveRecord = TRUE;

	return eReturn;
}

/****************************************************************************
 *
 * NAME: MibBulbConfig_eSetLumRate
 *
 * DESCRIPTION:
 * LumRate set data callback
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbConfig_eSetLumRate(uint8   u8Val, void *pvCbData)
{
	teJIP_Status eReturn = E_JIP_ERROR_BAD_VALUE;

	/* Valid device type ? */
	if (u8Val > 0)
	{
		/* Call standard function */
		eReturn = eSetUint8(u8Val, pvCbData);

        /* Make sure permament data is saved */
		psMibBulbConfig->bSaveRecord = TRUE;
	}

	return eReturn;
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
