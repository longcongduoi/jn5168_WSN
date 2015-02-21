/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         BulbControl MIB Implementation Patches
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
//#include "MibBulbDebug.h"
#include "MibBulb.h"
/* Application device includes */
#include "MibBulbControl.h"
#include "MibBulbConfig.h"
#include "DriverBulb.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define PS_MIB_BULB_CONFIG ((tsMibBulbConfig *) psMibBulbControl->pvMibBulbConfig)

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE tsMibBulbControl *psMibBulbControl;  /* Bulb Control Mib data */

/****************************************************************************
 *
 * NAME: MibBulbControlPatch_vInit
 *
 * DESCRIPTION:
 * Initialises data
 *
 ****************************************************************************/
PUBLIC void MibBulbControlPatch_vInit(tsMibBulbControl *psMibBulbControlInit)
{
	/* Take a copy of the data pointer */
	psMibBulbControl = psMibBulbControlInit;

	/* Have a bulb config pointer ? */
	if (PS_MIB_BULB_CONFIG != NULL)
	{
		/* Does the configuration data indicate the bulb has failed ? */
		if (PS_MIB_BULB_CONFIG->sPerm.u8InitMode == VAR_VAL_BULB_CONTROL_MODE_FAILED)
		{
			/* Make sure control data indicates the same */
			psMibBulbControl->sPerm.u8Mode       = VAR_VAL_BULB_CONTROL_MODE_FAILED;
			/* Is the bulb on - turn it off ? */
			if (DriverBulb_bOn()) DriverBulb_vOff();
		}
		/* Config data indicates the bulb is ok ? */
		else
		{
			/* Is the control data flagged up as failed ? */
			if (psMibBulbControl->sPerm.u8Mode == VAR_VAL_BULB_CONTROL_MODE_FAILED)
			{
				/* Override with on mode */
				psMibBulbControl->sPerm.u8Mode = VAR_VAL_BULB_CONTROL_MODE_ON;
				/* Bulb is off - turn it on */
				if (DriverBulb_bOn() == FALSE) DriverBulb_vOn();
			}
		}
	}
}

/****************************************************************************
 *
 * NAME: MibBulbControl_vSecond
 *
 * DESCRIPTION:
 * Called once per second
 *
 ****************************************************************************/
PUBLIC void MibBulbControlPatch_vSecond(void)
{
    /* Has the bulb failed at the driver level and we've not trapped it yet ? */
    if (DriverBulb_bFailed() &&
        psMibBulbControl->sPerm.u8Mode != VAR_VAL_BULB_CONTROL_MODE_FAILED)
    {
    	/* Update mode to failed */
    	psMibBulbControl->sPerm.u8Mode = VAR_VAL_BULB_CONTROL_MODE_FAILED;
    	/* Issue trap notification */
        vJIP_NotifyChanged(psMibBulbControl->hMib, VAR_IX_BULB_CONTROL_MODE);
        /* Save record */
        psMibBulbControl->bSaveRecord = TRUE;

		/* Have a bulb config pointer ? */
		if (PS_MIB_BULB_CONFIG != NULL)
		{
			/* Update mode to failed */
			PS_MIB_BULB_CONFIG->sPerm.u8InitMode = VAR_VAL_BULB_CONTROL_MODE_FAILED;
			/* Issue trap notification */
			vJIP_NotifyChanged(PS_MIB_BULB_CONFIG->hMib, VAR_IX_BULB_CONFIG_INIT_MODE);
			/* Save record */
			PS_MIB_BULB_CONFIG->bSaveRecord = TRUE;
		}
    }
}

/****************************************************************************
 *
 * NAME: MibBulbControlPatch_vStackEvent
 *
 * DESCRIPTION:
 * Called when stack events take place
 *
 ****************************************************************************/
PUBLIC void MibBulbControlPatch_vStackEvent(te6LP_StackEvent eEvent)
{
	/* Network is up ? */
	if (E_STACK_JOINED  == eEvent ||
		E_STACK_STARTED == eEvent)
	{
		/* First time joined ? */
		if (psMibBulbControl->bJoined == FALSE)
		{
			/* Note we've now joined */
			psMibBulbControl->bJoined = TRUE;
		}

		/* Were we down before ? */
		if (FALSE == psMibBulbControl->bUp)
		{
			/* Down factory state ? */
			if (psMibBulbControl->sPerm.u8NwkState == MIB_BULB_CONTROL_NWK_STATE_DOWN_FACTORY)
			{
				/* Go to up factory state */
				psMibBulbControl->sPerm.u8NwkState = MIB_BULB_CONTROL_NWK_STATE_UP_FACTORY;
				/* Place device into global and global bulb groups */
				(void) bJIP_JoinGroup(0xf00f);
				(void) bJIP_JoinGroup((uint16)(MIB_ID_BULB_CONTROL & 0xffff));
			}
			/* Down reset state ? */
			else if (psMibBulbControl->sPerm.u8NwkState == MIB_BULB_CONTROL_NWK_STATE_DOWN_RESET)
			{
				/* Go to up reset state */
				psMibBulbControl->sPerm.u8NwkState = MIB_BULB_CONTROL_NWK_STATE_UP_RESET;
			}
			/* Other states ? */
			else
			{
				/* go to up running state */
				psMibBulbControl->sPerm.u8NwkState = MIB_BULB_CONTROL_NWK_STATE_UP_RUNNING;
			}

			/* Save state */
			psMibBulbControl->bSaveRecord = TRUE;

			/* Set up flag */
			psMibBulbControl->bUp = TRUE;

			/* Apply up cadence if required */
			MibBulbControl_vLumCadenceStackEvent(eEvent);
		}
	}
	/* Network is down ? */
	else if (E_STACK_RESET == eEvent)
	{
		/* Were we up before ? */
		if (TRUE == psMibBulbControl->bUp)
		{
			/* Go to down running state */
			psMibBulbControl->sPerm.u8NwkState = MIB_BULB_CONTROL_NWK_STATE_DOWN_RUNNING;
			/* Save state */
			psMibBulbControl->bSaveRecord = TRUE;

			/* Clear up flag */
			psMibBulbControl->bUp = FALSE;

			/* Apply down cadence if required */
			MibBulbControl_vLumCadenceStackEvent(eEvent);
		}
	}
}

/****************************************************************************
 *
 * NAME: MibBulbControlPatch_eSetMode
 *
 * DESCRIPTION:
 * Mode set data callback
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbControlPatch_eSetMode(uint8 u8Val, void *pvCbData)
{
    teJIP_Status eReturn = E_JIP_ERROR_BAD_VALUE;

    /* Has the bulb failed ? */
    if (psMibBulbControl->sPerm.u8Mode == VAR_VAL_BULB_CONTROL_MODE_FAILED)
    {
   		/* Return this option is currently disabled */
   		eReturn = E_JIP_ERROR_DISABLED;
    }
    /* Is mode on if fading up or down ? */
    else if (u8Val == VAR_VAL_BULB_CONTROL_MODE_ON_IF_DOWN_UP)
    {
    	/* Are we currently fading ? */
    	if (psMibBulbControl->sPerm.u8Mode == VAR_VAL_BULB_CONTROL_MODE_DOWN		||
    		psMibBulbControl->sPerm.u8Mode == VAR_VAL_BULB_CONTROL_MODE_UP			||
    		psMibBulbControl->sPerm.u8Mode == VAR_VAL_BULB_CONTROL_MODE_DOWN_IF_ON	||
    		psMibBulbControl->sPerm.u8Mode == VAR_VAL_BULB_CONTROL_MODE_UP_IF_ON)
    	{
    		/* Replace with on mode */
    		u8Val = VAR_VAL_BULB_CONTROL_MODE_ON;
	        /* Call library function */
    	    eReturn = MibBulbControl_eSetMode(u8Val, pvCbData);
    	}
    	else
    	{
    		/* Return this option is currently disabled */
    		eReturn = E_JIP_ERROR_DISABLED;
    	}
    }
    /* Other valid mode the library can handle ? */
    else if (u8Val < VAR_VAL_BULB_CONTROL_MODE_COUNT)
    {
        /* Call library function */
        eReturn = MibBulbControl_eSetMode(u8Val, pvCbData);
    }

    return eReturn;
}

/****************************************************************************
 *
 * NAME: MibBulbControlPatch_eSetLumCadence
 *
 * DESCRIPTION:
 * Sets LumCadence variable
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbControlPatch_eSetLumCadence(uint32 u32Val, void *pvCbData)
{
    teJIP_Status eReturn = E_JIP_OK;

    /* Update cadence */
    MibBulbControl_vLumCadence(u32Val, psMibBulbControl->sTemp.u16LumCadTimer);

    return eReturn;
}

/****************************************************************************
 *
 * NAME: MibBulbControlPatch_eSetLumCadTimer
 *
 * DESCRIPTION:
 * Sets LumCadTimer variable
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbControlPatch_eSetLumCadTimer(uint16 u16Val, void *pvCbData)
{
    teJIP_Status eReturn = E_JIP_OK;

    /* Update cadence */
    MibBulbControl_vLumCadence(psMibBulbControl->sTemp.u32LumCadence, u16Val);

    return eReturn;
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
