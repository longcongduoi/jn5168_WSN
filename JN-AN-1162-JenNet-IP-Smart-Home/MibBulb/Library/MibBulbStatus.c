/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         BulbStatus MIB Implementation
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
/* JenOS includes */
#include <dbg.h>
#include <dbg_uart.h>
#include <os.h>
#include <pdm.h>
/* Application common includes */
#include "MibBulbDebug.h"
#include "MibBulb.h"
#include "MibAdcStatus.h"
/* Application device includes */
#include "DriverBulb.h"
#include "MibBulbControl.h"
#include "MibBulbStatus.h"

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
PRIVATE tsMibBulbStatus  *psMibBulbStatus;			/* Bulb Status Mib data */

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: MibBulbStatus_vInit
 *
 * DESCRIPTION:
 * Initialises data
 *
 ****************************************************************************/
PUBLIC void MibBulbStatus_vInit(thJIP_Mib        hMibBulbStatusInit,
								tsMibBulbStatus *psMibBulbStatusInit,
								uint8  			  u8AdcSrcBusVoltsInit)

{
	/* Valid data pointer ? */
	if (psMibBulbStatusInit != (tsMibBulbStatus *) NULL)
	{
		PDM_teStatus   ePdmStatus;

		/* Debug */
		DBG_vPrintf(CONFIG_DBG_MIB_BULB_STATUS,   "\nMibBulbStatus_vInit() {%d}", sizeof(tsMibBulbStatus));

		/* Take copy of pointer to data */
		psMibBulbStatus = psMibBulbStatusInit;

		/* Take a copy of the MIB handle */
		psMibBulbStatus->hMib = hMibBulbStatusInit;

		/* Load BulbStatus mib data */
		ePdmStatus = PDM_eLoadRecord(&psMibBulbStatus->sDesc,
#if defined(JENNIC_CHIP_FAMILY_JN514x)
									 "MibBulbStatus",
#else
									 (uint16)(MIB_ID_BULB_STATUS & 0xFFFF),
#endif
									 (void *) &psMibBulbStatus->sPerm,
									 sizeof(psMibBulbStatus->sPerm),
									 FALSE);

		/* Note ADC source for bus voltage */
		psMibBulbStatus->u8AdcSrcBusVolts = u8AdcSrcBusVoltsInit;
	}
}

/****************************************************************************
 *
 * NAME: MibBulbStatus_vRegister
 *
 * DESCRIPTION:
 * Registers MIB
 *
 ****************************************************************************/
PUBLIC void MibBulbStatus_vRegister(void)
{
	teJIP_Status eStatus;

	/* Register MIB */
	eStatus = eJIP_RegisterMib(psMibBulbStatus->hMib);
	/* Debug */
	DBG_vPrintf(CONFIG_DBG_MIB_BULB_STATUS, "\nMibBulbStatus_vRegister()");
	DBG_vPrintf(CONFIG_DBG_MIB_BULB_STATUS, "\n\teJIP_RegisterMib(BulbStatus)=%d", eStatus);

	/* Make sure permament data is saved */
	psMibBulbStatus->bSaveRecord = TRUE;
}

/****************************************************************************
 *
 * NAME: MibBulbStatus_vSecond
 *
 * DESCRIPTION:
 * Timer function
 *
 ****************************************************************************/
PUBLIC void MibBulbStatus_vSecond(void)
{
	/* Bulb is on ? */
	if (DriverBulb_bOn())
	{
		/* Increment on timer */
		psMibBulbStatus->sPerm.u32OnTime++;
		/* Been on an hour ? */
		if ((psMibBulbStatus->sPerm.u32OnTime % 3600) == 0)
		{
			/* Save data to flash */
			psMibBulbStatus->bSaveRecord = TRUE;
			/* Need to notify for the on time variable */
			vJIP_NotifyChanged(psMibBulbStatus->hMib, VAR_IX_BULB_STATUS_ON_TIME);
		}
	}
	/* Bulb is off */
	else
	{
		/* Increment down timer */
		psMibBulbStatus->sPerm.u32OffTime++;
		/* Been down an hour ? */
		if ((psMibBulbStatus->sPerm.u32OffTime % 3600) == 0)
		{
			/* Save data to flash */
			psMibBulbStatus->bSaveRecord = TRUE;
			/* Need to notify for the off time variable */
			vJIP_NotifyChanged(psMibBulbStatus->hMib, VAR_IX_BULB_STATUS_OFF_TIME);
		}
	}

	/* Need to save record ? */
	if (psMibBulbStatus->bSaveRecord)
	{
		/* Clear flag */
		psMibBulbStatus->bSaveRecord = FALSE;
		/* Make sure permament data is saved */
		PDM_vSaveRecord(&psMibBulbStatus->sDesc);
		/* Debug */
		DBG_vPrintf(CONFIG_DBG_MIB_BULB_STATUS, "\nMibBulbStatus_vSecond()");
		DBG_vPrintf(CONFIG_DBG_MIB_BULB_STATUS, "\n\tPDM_vSaveRecord(MibBulbStatus)");
	}
}

/****************************************************************************
 *
 * NAME: MibBulbStatus_vAnalogue
 *
 * DESCRIPTION:
 * Called when analogue readings have completed
 *
 ****************************************************************************/
PUBLIC void MibBulbStatus_vAnalogue(uint8 u8Adc)
{
	/* ADC 4 - bus voltage ? */
	if (u8Adc == psMibBulbStatus->u8AdcSrcBusVolts)
	{
		/* Pass on to bulb driver and note the returned bus voltage */
		psMibBulbStatus->sPerm.i16BusVolts = DriverBulb_i16Analogue(u8Adc, MibAdcStatus_u16Read(u8Adc));
	}
	/* ADC temperature - chip temperature ? */
	else if (u8Adc == E_AHI_ADC_SRC_TEMP)
	{
		/* Scale reading to deci-centigrade */
		psMibBulbStatus->sPerm.i16ChipTemp = (int16) MibAdcStatus_i16DeciCentigrade(u8Adc);
	}
}

/****************************************************************************
 *
 * NAME: MibBulbStatus_vOn
 *
 * DESCRIPTION:
 * Called when bulb has been turned on for monitoring purposes
 *
 ****************************************************************************/
PUBLIC void MibBulbStatus_vOn(void)
{
	/* Increment on count */
	psMibBulbStatus->sPerm.u16OnCount++;
	/* Save data to flash */
	psMibBulbStatus->bSaveRecord = TRUE;
	/* Need to notify for the on count variable */
	vJIP_NotifyChanged(psMibBulbStatus->hMib, VAR_IX_BULB_STATUS_ON_COUNT);
	/* Need to notify for the off time variable */
	vJIP_NotifyChanged(psMibBulbStatus->hMib, VAR_IX_BULB_STATUS_OFF_TIME);
}

/****************************************************************************
 *
 * NAME: MibBulbStatus_vOff
 *
 * DESCRIPTION:
 * Called when bulb has been turned on for monitoring purposes
 *
 ****************************************************************************/
PUBLIC void MibBulbStatus_vOff(void)
{
	/* Save data to flash */
	psMibBulbStatus->bSaveRecord = TRUE;
	/* Need to notify for the on time variable */
	vJIP_NotifyChanged(psMibBulbStatus->hMib, VAR_IX_BULB_STATUS_ON_TIME);
}
/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
