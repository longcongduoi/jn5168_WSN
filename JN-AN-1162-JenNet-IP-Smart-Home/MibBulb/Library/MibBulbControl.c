/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         BulbControl MIB Implementation
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
#include "MibCommon.h"
#include "MibBulbDebug.h"
#include "MibBulb.h"
#include "MibNwkStatus.h"
/* Application device includes */
#include "DriverBulb.h"
#include "MibBulbStatus.h"
#include "MibBulbConfig.h"
#include "MibBulbScene.h"
#include "MibBulbControl.h"
#include "PATCH.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define PS_MIB_BULB_STATUS ((tsMibBulbStatus *) psMibBulbControl->pvMibBulbStatus)
#define PS_MIB_BULB_CONFIG ((tsMibBulbConfig *) psMibBulbControl->pvMibBulbConfig)
#define PS_MIB_BULB_SCENE  ((tsMibBulbScene  *) psMibBulbControl->pvMibBulbScene)

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
PRIVATE tsMibBulbControl *psMibBulbControl;  /* Bulb Control Mib data */

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: MibBulbControl_vInit
 *
 * DESCRIPTION:
 * Initialises data
 *
 ****************************************************************************/
PUBLIC void MibBulbControl_vInit(thJIP_Mib         hMibBulbControlInit,
                                 thJIP_Mib         hMibDeviceControlInit,
                                 tsMibBulbControl *psMibBulbControlInit,
                                 void             *pvMibBulbStatusInit,
                                 void             *pvMibBulbConfigInit,
                                 void             *pvMibBulbSceneInit)
{
    /* Debug */
    DBG_vPrintf(CONFIG_DBG_MIB_BULB_CONTROL, "\nMibBulbControl_vInit() {%d}", sizeof(tsMibBulbControl));

    /* Valid data pointer ? */
    if (psMibBulbControlInit != (tsMibBulbControl *) NULL)
    {
        PDM_teStatus   ePdmStatus;

        /* Take copy of pointer to data */
        psMibBulbControl = psMibBulbControlInit;

        /* Take a copy of the MIB handles */
        psMibBulbControl->hMib             = hMibBulbControlInit;
        psMibBulbControl->hDeviceControlMib = hMibDeviceControlInit;

        /* Take a copy of the pother mib pointers */
        psMibBulbControl->pvMibBulbStatus = pvMibBulbStatusInit;
        psMibBulbControl->pvMibBulbConfig = pvMibBulbConfigInit;
        psMibBulbControl->pvMibBulbScene  = pvMibBulbSceneInit;

        /* Load BulbControl mib data */
        ePdmStatus = PDM_eLoadRecord(&psMibBulbControl->sDesc,
#if defined(JENNIC_CHIP_FAMILY_JN514x)
									 "MibBulbControl",
#else
									 (uint16)(MIB_ID_BULB_CONTROL & 0xFFFF),
#endif
                                     (void *) &psMibBulbControl->sPerm,
                                     sizeof(psMibBulbControl->sPerm),
                                     FALSE);

        /* Not in down factory or down reset states? */
        if (psMibBulbControl->sPerm.u8NwkState != MIB_BULB_CONTROL_NWK_STATE_DOWN_FACTORY &&
            psMibBulbControl->sPerm.u8NwkState != MIB_BULB_CONTROL_NWK_STATE_DOWN_RESET)
        {
            /* Go to down reset state */
            psMibBulbControl->sPerm.u8NwkState = MIB_BULB_CONTROL_NWK_STATE_DOWN_RESET;
        }

        /* Bulb config data is available ? */
        if (PS_MIB_BULB_CONFIG != NULL)
        {
            /* Is an initialisation mode specified (values from flash retained otherwise) ? */
            if (PS_MIB_BULB_CONFIG->sPerm.u8InitMode < VAR_VAL_BULB_CONTROL_MODE_COUNT)
            {
                /* Apply initial mode and target level from config */
                psMibBulbControl->sPerm.u8Mode       = PS_MIB_BULB_CONFIG->sPerm.u8InitMode;
                psMibBulbControl->sPerm.u8LumTarget  = PS_MIB_BULB_CONFIG->sPerm.u8InitLumTarget;
                psMibBulbControl->sPerm.u16SceneId   = 0;
            }
        }

        /* Initialising to DOWN, UP or TOGGLE modes ? */
        if (psMibBulbControl->sPerm.u8Mode       == VAR_VAL_BULB_CONTROL_MODE_DOWN       ||
            psMibBulbControl->sPerm.u8Mode       == VAR_VAL_BULB_CONTROL_MODE_UP         ||
            psMibBulbControl->sPerm.u8Mode       == VAR_VAL_BULB_CONTROL_MODE_DOWN_IF_ON ||
            psMibBulbControl->sPerm.u8Mode       == VAR_VAL_BULB_CONTROL_MODE_UP_IF_ON   ||
            psMibBulbControl->sPerm.u8Mode       == VAR_VAL_BULB_CONTROL_MODE_TOGGLE)
        {
            /* Not allowed initialise to ON mode instead */
            psMibBulbControl->sPerm.u8Mode       = VAR_VAL_BULB_CONTROL_MODE_ON;
        }

        /* Default temporary control data */
        psMibBulbControl->sTemp.u8LumCurrent   = psMibBulbControl->sPerm.u8LumTarget;

        /* Initialise the bulb driver */
        DriverBulb_vInit();

        /* Apply down cadence if required */
        MibBulbControl_vLumCadenceStackEvent(E_STACK_RESET);
    }
}

/****************************************************************************
 *
 * NAME: MibBulbControl_vRegister
 *
 * DESCRIPTION:
 * Registers MIB
 *
 ****************************************************************************/
PUBLIC void MibBulbControl_vRegister(void)
{
    teJIP_Status eStatus;

    /* Register MIB */
    eStatus = eJIP_RegisterMib(psMibBulbControl->hMib);
    /* Debug */
    DBG_vPrintf(CONFIG_DBG_MIB_BULB_CONTROL, "\nMibBulbControl_vRegister()");
    DBG_vPrintf(CONFIG_DBG_MIB_BULB_CONTROL, "\n\teJIP_RegisterMib(BulbControl)=%d", eStatus);

    /* Make sure permament data is saved */
    psMibBulbControl->bSaveRecord = TRUE;
}

/****************************************************************************
 *
 * NAME: MibBulbControl_vDeviceControlRegister
 *
 * DESCRIPTION:
 * Registers device control MIB
 *
 ****************************************************************************/
PUBLIC void MibBulbControl_vDeviceControlRegister(void)
{
    teJIP_Status eStatus;

    /* Register MIB */
    eStatus = eJIP_RegisterMib(psMibBulbControl->hDeviceControlMib);
    /* Debug */
    DBG_vPrintf(CONFIG_DBG_MIB_BULB_CONTROL, "\nMibBulbControl_vDeviceControlRegister()");
    DBG_vPrintf(CONFIG_DBG_MIB_BULB_CONTROL, "\n\teJIP_RegisterMib(DeviceControl)=%d", eStatus);
}

/****************************************************************************
 *
 * NAME: MibBulbControl_vSecond
 *
 * DESCRIPTION:
 * Called once per second
 *
 ****************************************************************************/
PUBLIC void MibBulbControl_vSecond(void)
{
    /* Need to save record ? */
    if (psMibBulbControl->bSaveRecord)
    {
        /* Clear flag */
        psMibBulbControl->bSaveRecord = FALSE;
        /* Make sure permament data is saved */
        PDM_vSaveRecord(&psMibBulbControl->sDesc);
        /* Debug */
        DBG_vPrintf(CONFIG_DBG_MIB_BULB_CONTROL, "\nMibBulbControl_vSecond()");
        DBG_vPrintf(CONFIG_DBG_MIB_BULB_CONTROL, "\n\tPDM_vSaveRecord(MibBulbControl)");
    }
}

/****************************************************************************
 *
 * NAME: MibBulbControl_vTick
 *
 * DESCRIPTION:
 * Timer function
 *
 ****************************************************************************/
PUBLIC void MibBulbControl_vTick(void)
{
    /* Increment tick counter */
    psMibBulbControl->u32Tick++;

    /* Call bulb driver function */
    DriverBulb_vTick();

    /* Handle ready status of bulb driver */
    MibBulbControl_vTickDriverReady();

    /* Handle cadence - no cadence being applied ? */
    if (MibBulbControl_bTickLumCadence() == FALSE)
    {
        /* Handle tick for current bulb mode */
        switch (psMibBulbControl->sPerm.u8Mode)
        {
            case VAR_VAL_BULB_CONTROL_MODE_OFF:         MibBulbControl_vTickModeOff();      break;
            case VAR_VAL_BULB_CONTROL_MODE_ON:          MibBulbControl_vTickModeOn();       break;
            case VAR_VAL_BULB_CONTROL_MODE_DOWN:        MibBulbControl_vTickModeDownUp();   break;
            case VAR_VAL_BULB_CONTROL_MODE_UP:          MibBulbControl_vTickModeDownUp();   break;
            case VAR_VAL_BULB_CONTROL_MODE_DOWN_IF_ON:  MibBulbControl_vTickModeDownUp();   break;
            case VAR_VAL_BULB_CONTROL_MODE_UP_IF_ON:    MibBulbControl_vTickModeDownUp();   break;
            case VAR_VAL_BULB_CONTROL_MODE_TEST:        MibBulbControl_vTickModeTest();     break;
            default:                                                                        break;
        }
    }
}

/****************************************************************************
 *
 * NAME: MibBulbControl_vTickDriverReady
 *
 * DESCRIPTION:
 * Tick handling of bulb driver ready state
 *
 ****************************************************************************/
PUBLIC void MibBulbControl_vTickDriverReady(void)
{
    /* Is lamp ready ? */
    if (DriverBulb_bReady())
    {
        /* Was lamp not ready before ? */
        if (psMibBulbControl->bDriverReady == FALSE)
        {
            /* Note lamp is now ready */
            psMibBulbControl->bDriverReady = TRUE;
            /* Lamp should be on ? */
            if (psMibBulbControl->sPerm.u8Mode != VAR_VAL_BULB_CONTROL_MODE_OFF ||
               (psMibBulbControl->sTemp.u32LumCadence >= 0x00010000 && psMibBulbControl->sTemp.u16LumCadTimer > 0))
            {
                /* Make sure lamp is on */
                DriverBulb_vOn();
                /* Increment on count */
                if (PS_MIB_BULB_STATUS != NULL) MibBulbStatus_vOn();
                /* Set level to current */
                DriverBulb_vSetLevel(psMibBulbControl->sTemp.u8LumCurrent);
            }
            /* Lamp should be off ? */
            else if (psMibBulbControl->sPerm.u8Mode == VAR_VAL_BULB_CONTROL_MODE_OFF)
            {
                /* Make sure lamp is off */
                DriverBulb_vOff();
                /* Increment off count */
                if (PS_MIB_BULB_STATUS != NULL) MibBulbStatus_vOff();
            }
        }
    }
    /* Lamp is not ready ? */
    else
    {
        /* Note lamp is not ready */
        psMibBulbControl->bDriverReady = FALSE;
    }
}

/****************************************************************************
 *
 * NAME: MibBulbControl_bTickLumCadence
 *
 * DESCRIPTION:
 * Tick timer handler for displaying cadences
 *
 ****************************************************************************/
PUBLIC bool_t MibBulbControl_bTickLumCadence(void)
{
    bool_t bReturn = FALSE;

    /* Is cadence timer running ? */
    if (psMibBulbControl->sTemp.u16LumCadTimer > 0)
    {
        /* Timer is not maximum - decrement it */
        if (psMibBulbControl->sTemp.u16LumCadTimer != 0xffff) psMibBulbControl->sTemp.u16LumCadTimer--;

        /* Timer has expired ? */
        if (psMibBulbControl->sTemp.u16LumCadTimer == 0)
        {
            /* Stop it */
            MibBulbControl_vLumCadenceStop();
            /* Clear down and up cadence flags */
            psMibBulbControl->bDownCadence = FALSE;
            psMibBulbControl->bUpCadence = FALSE;
        }
        /* Timer is still running ? */
        else
        {
            /* Cadence is set ? */
            if (psMibBulbControl->sTemp.u32LumCadence >= 0x00010000)
            {
                /* Going to apply cadence so return true */
                bReturn = TRUE;
                /* Is a fade rate set ? */
                if (psMibBulbControl->u8LumCadFade != 0)
                {
                    /* Update current luminance at specified rate - reached target ? */
                    if (MibBulbControl_bFadeLumCurrent(psMibBulbControl->u8LumCadTarget, psMibBulbControl->u8LumCadFade))
                    {
                        /* Not min ? */
                        if (psMibBulbControl->sTemp.u8LumCurrent != psMibBulbControl->u8LumCadMin)
                        {
                            /* Start fading to min */
                            psMibBulbControl->u8LumCadTarget = psMibBulbControl->u8LumCadMin;
                        }
                        else
                        {
                            /* Start fading to max */
                            psMibBulbControl->u8LumCadTarget = psMibBulbControl->u8LumCadMax;
                        }
                    }
                }
                /* Is a switch rate set ? */
                else if (psMibBulbControl->u8LumCadSwitch != 0)
                {
                    /* Time to switch ? */
                    if ((psMibBulbControl->u32LumCadTick % psMibBulbControl->u8LumCadSwitch) == 0 &&
                         psMibBulbControl->u32LumCadTick                                     >  0)
                    {
                        /* Not min ? */
                        if (psMibBulbControl->sTemp.u8LumCurrent != psMibBulbControl->u8LumCadMin)
                        {
                            /* Switch to min */
                            psMibBulbControl->sTemp.u8LumCurrent = psMibBulbControl->u8LumCadMin;
                        }
                        else
                        {
                            /* Switch to max */
                            psMibBulbControl->sTemp.u8LumCurrent = psMibBulbControl->u8LumCadMax;
                        }
                        /* Set the light level */
                        DriverBulb_vSetLevel(psMibBulbControl->sTemp.u8LumCurrent);
                    }
                    /* Increment tick */
                    psMibBulbControl->u32LumCadTick++;
                }
            }
        }
    }

    return bReturn;
}

/****************************************************************************
 *
 * NAME: MibBulbControl_vTickModeTest
 *
 * DESCRIPTION:
 * Tick handler for test mode
 *
 ****************************************************************************/
PUBLIC void MibBulbControl_vTickModeTest(void)
{
    /* Is the network up (and will have just come up) ? */
    if (psMibBulbControl->bUp)
    {
        /* Update current level */
        (void) MibBulbControl_bFadeLumCurrent(MibBulbControl_u8ParentLqi(), MIB_BULB_CONFIG_LUM_RATE_DEFAULT);
    }
    /* Network is down */
    else
    {
        /* Update current luminance at specified rate - reached target ? */
        if (MibBulbControl_bFadeLumCurrent(psMibBulbControl->u8LumTmpTarget, MIB_BULB_CONFIG_LUM_RATE_DEFAULT))
        {
            /* Not min ? */
            if (psMibBulbControl->sTemp.u8LumCurrent != 0)
            {
                /* Start fading to min */
                psMibBulbControl->u8LumTmpTarget = 0;
            }
            else
            {
                /* Start fading to max */
                psMibBulbControl->u8LumTmpTarget = 255;
            }
        }
    }
}

/****************************************************************************
 *
 * NAME: MibBulbControl_vTickModeOff
 *
 * DESCRIPTION:
 * Tick handler for off mode
 *
 ****************************************************************************/
PUBLIC void MibBulbControl_vTickModeOff(void)
{
    /* Do nothing */
    ;
}

/****************************************************************************
 *
 * NAME: MibBulbControl_vTickModeOn
 *
 * DESCRIPTION:
 * Tick handler for on mode
 *
 ****************************************************************************/
PUBLIC void MibBulbControl_vTickModeOn(void)
{
    bool_t bHitTarget;

    /* Not at target level ? */
    if (psMibBulbControl->sTemp.u8LumCurrent != psMibBulbControl->sPerm.u8LumTarget)
    {
        /* Got access to bulb config mib data ? */
        if (PS_MIB_BULB_CONFIG != NULL)
        {
            /* Update current luminance at configured rate */
            bHitTarget = MibBulbControl_bFadeLumCurrent(psMibBulbControl->sPerm.u8LumTarget, PS_MIB_BULB_CONFIG->sPerm.u8LumRate);
        }
        else
        {
            /* Update current luminance at default */
            bHitTarget = MibBulbControl_bFadeLumCurrent(psMibBulbControl->sPerm.u8LumTarget, MIB_BULB_CONFIG_LUM_RATE_DEFAULT);
        }
        /* Now at target level ? */
        if (bHitTarget)
        {
            /* Need to notify for the current level variable */
            vJIP_NotifyChanged(psMibBulbControl->hMib, VAR_IX_BULB_CONTROL_LUM_CURRENT);
        }
    }
}

/****************************************************************************
 *
 * NAME: MibBulbControl_vTickModeDownUp
 *
 * DESCRIPTION:
 * Tick handler for fade down and up modes
 *
 ****************************************************************************/
PUBLIC void MibBulbControl_vTickModeDownUp(void)
{
    bool_t bHitTarget;

    /* Not at target ? */
    if (psMibBulbControl->sTemp.u8LumCurrent != psMibBulbControl->u8LumTmpTarget)
    {
        /* Got access to bulb config mib data ? */
        if (PS_MIB_BULB_CONFIG != NULL)
        {
            /* Update current luminance at configured rate */
            bHitTarget = MibBulbControl_bFadeLumCurrent(psMibBulbControl->u8LumTmpTarget, PS_MIB_BULB_CONFIG->sPerm.u8LumRate);
        }
        else
        {
            /* Update current luminance at default */
            bHitTarget = MibBulbControl_bFadeLumCurrent(psMibBulbControl->u8LumTmpTarget, MIB_BULB_CONFIG_LUM_RATE_DEFAULT);
        }
        /* Set as target level */
        psMibBulbControl->sPerm.u8LumTarget = psMibBulbControl->sTemp.u8LumCurrent;
        /* Now at target level ? */
        if (bHitTarget)
        {
            /* Revert to on mode */
            psMibBulbControl->sPerm.u8Mode = VAR_VAL_BULB_CONTROL_MODE_ON;
            /* Make sure permament data is saved */
            psMibBulbControl->bSaveRecord = TRUE;
            /* Need to notify for the mode, target and current level variables */
            vJIP_NotifyChanged(psMibBulbControl->hMib, VAR_IX_BULB_CONTROL_MODE);
            vJIP_NotifyChanged(psMibBulbControl->hMib, VAR_IX_BULB_CONTROL_LUM_TARGET);
            vJIP_NotifyChanged(psMibBulbControl->hMib, VAR_IX_BULB_CONTROL_LUM_CURRENT);
        }
    }
}

/****************************************************************************
 *
 * NAME: MibBulbControl_bFadeLumCurrent
 *
 * DESCRIPTION:
 * Move current level towards target level
 *
 ****************************************************************************/
PATCH_POINT_PUBLIC(bool_t,MibBulbControl_bFadeLumCurrent)(
                                        uint8 u8LumTarget, uint8 u8LumRate)
{
    bool_t bReturn = FALSE;

    /* Is the current level below the target level ? */
    if (psMibBulbControl->sTemp.u8LumCurrent < u8LumTarget)
    {
        /* Incrementing past target ? */
        if (u8LumTarget - psMibBulbControl->sTemp.u8LumCurrent < u8LumRate)
        {
            /* Set to target */
            psMibBulbControl->sTemp.u8LumCurrent = u8LumTarget;
        }
        /* Increment normally ? */
        else
        {
            /* Increment normally */
            psMibBulbControl->sTemp.u8LumCurrent += u8LumRate;
        }
        /* Set the light level */
        DriverBulb_vSetLevel(psMibBulbControl->sTemp.u8LumCurrent);
    }
    /* Is the current level above the target level ? */
    else if (psMibBulbControl->sTemp.u8LumCurrent > u8LumTarget)
    {
        /* Decrementing past target ? */
        if (psMibBulbControl->sTemp.u8LumCurrent - u8LumTarget < u8LumRate)
        {
            /* Set to target */
            psMibBulbControl->sTemp.u8LumCurrent = u8LumTarget;
        }
        /* Decrement normally ? */
        else
        {
            /* Decrement normally */
            psMibBulbControl->sTemp.u8LumCurrent -= u8LumRate;
        }
        /* Set the light level */
        DriverBulb_vSetLevel(psMibBulbControl->sTemp.u8LumCurrent);
    }

    /* Does the current rate match the target ? */
    if (psMibBulbControl->sTemp.u8LumCurrent == u8LumTarget)
    {
        /* Return true */
        bReturn = TRUE;
    }

    return bReturn;
}

/****************************************************************************
 *
 * NAME: MibBulbControl_vStackEvent
 *
 * DESCRIPTION:
 * Called when stack events take place
 *
 ****************************************************************************/
PUBLIC void MibBulbControl_vStackEvent(te6LP_StackEvent eEvent)
{
    /* Network is up ? */
    if (E_STACK_JOINED  == eEvent ||
        E_STACK_STARTED == eEvent)
    {
        /* First time joined ? */
        if (psMibBulbControl->bJoined == FALSE)
        {
            /* Place device into global and global bulb groups */
            (void) bJIP_JoinGroup(0xf00f);
            (void) bJIP_JoinGroup((uint16)(MIB_ID_BULB_CONTROL & 0xffff));
            /* Note we've now joined */
            psMibBulbControl->bJoined = TRUE;
        }

        /* Were we down before ? */
        if (FALSE == psMibBulbControl->bUp)
        {
            /* Update state */
            if      (psMibBulbControl->sPerm.u8NwkState == MIB_BULB_CONTROL_NWK_STATE_DOWN_FACTORY) psMibBulbControl->sPerm.u8NwkState = MIB_BULB_CONTROL_NWK_STATE_UP_FACTORY;
            else if (psMibBulbControl->sPerm.u8NwkState == MIB_BULB_CONTROL_NWK_STATE_DOWN_RESET)   psMibBulbControl->sPerm.u8NwkState = MIB_BULB_CONTROL_NWK_STATE_UP_RESET;
            else                                                                                    psMibBulbControl->sPerm.u8NwkState = MIB_BULB_CONTROL_NWK_STATE_UP_RUNNING;
            /* Save state */
            psMibBulbControl->bSaveRecord = TRUE;
        }

        /* Set up flag */
        psMibBulbControl->bUp = TRUE;

        /* Apply up cadence if required */
        MibBulbControl_vLumCadenceStackEvent(eEvent);
    }
    /* Network is down ? */
    else if (E_STACK_RESET == eEvent)
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

/****************************************************************************
 *
 * NAME: MibBulbControl_u8ParentLqi
 *
 * DESCRIPTION:
 * Return LQI from parent
 *
 ****************************************************************************/
PUBLIC uint8 MibBulbControl_u8ParentLqi(void)
{
    tsNeighbourEntry sNeighbourEntry;
    uint8 u8Return = 0;

    /* Network is up ? */
    if (psMibBulbControl->bUp)
    {
        /* Can we get neighbour table entry for our parent ? */
        if (bApi_GetNeighbourTableEntry(0, &sNeighbourEntry))
        {
            /* Turn lamp on to LQI level */
            u8Return = sNeighbourEntry.u8LinkQuality;
        }
    }

    return u8Return;
}

/****************************************************************************
 *
 * NAME: MibBulbControl_vLumCadence
 *
 * DESCRIPTION:
 * Configures cadence display
 *
 ****************************************************************************/
PATCH_POINT_PUBLIC(void,MibBulbControl_vLumCadence)(uint32 u32LumCadence, uint16 u16LumCadTimer)
{
    /* Turning off cadence ? */
    if (u32LumCadence < 0x00010000 || u16LumCadTimer == 0)
    {
        /* Cadence is still running ? */
        if (psMibBulbControl->sTemp.u32LumCadence  >= 0x00010000 && psMibBulbControl->sTemp.u16LumCadTimer > 0)
        {
            /* Stop it */
            MibBulbControl_vLumCadenceStop();
            /* Clear down and up cadence flags */
            psMibBulbControl->bDownCadence = FALSE;
            psMibBulbControl->bUpCadence = FALSE;
        }
        /* Copy cadence value */
        psMibBulbControl->sTemp.u32LumCadence = u32LumCadence;
        /* Copy timer value */
        psMibBulbControl->sTemp.u16LumCadTimer = u16LumCadTimer;
    }
    /* Turning on cadence ? */
    else
    {
        /* Extract various parts */
        psMibBulbControl->u8LumCadMin    = (uint8)( u32LumCadence        & 0xff);
        psMibBulbControl->u8LumCadMax    = (uint8)((u32LumCadence >>  8) & 0xff);
        psMibBulbControl->u8LumCadFade   = (uint8)((u32LumCadence >> 16) & 0xff);
        if (psMibBulbControl->u8LumCadFade == 0)
        {
            psMibBulbControl->u8LumCadSwitch = (uint8)((u32LumCadence >> 24) & 0xff);
        }
        else
        {
            psMibBulbControl->u8LumCadSwitch = 0;
        }

        /* Cadence settings have changed or timer is currently not running ? */
        if (psMibBulbControl->sTemp.u32LumCadence != u32LumCadence ||
            psMibBulbControl->sTemp.u16LumCadTimer == 0)
        {
            /* Clear down and up cadence flags */
            psMibBulbControl->bDownCadence = FALSE;
            psMibBulbControl->bUpCadence = FALSE;
            /* Set starting cadence target to min value */
            psMibBulbControl->u8LumCadTarget = psMibBulbControl->u8LumCadMin;
            /* Switching ? */
            if (psMibBulbControl->u8LumCadSwitch != 0)
            {
                /* Set current luminance target value to current */
                psMibBulbControl->sTemp.u8LumCurrent = psMibBulbControl->u8LumCadTarget;
                /* Set level to current */
                DriverBulb_vSetLevel(psMibBulbControl->sTemp.u8LumCurrent);
                /* Zero tick */
                psMibBulbControl->u32LumCadTick = 0;
            }

            /* Is bulb currently off ? */
            if (DriverBulb_bOn() == FALSE)
            {
                /* Turn on using lamp driver */
                DriverBulb_vOn();
                /* Increment on count */
                if (PS_MIB_BULB_STATUS != NULL) MibBulbStatus_vOn();
            }
        }

        /* Copy cadence value */
        psMibBulbControl->sTemp.u32LumCadence = u32LumCadence;
        /* Copy timer value */
        psMibBulbControl->sTemp.u16LumCadTimer = u16LumCadTimer;
    }
}

/****************************************************************************
 *
 * NAME: MibBulbControl_vLumCadenceStop
 *
 * DESCRIPTION:
 * Stop displaying cadence
 *
 ****************************************************************************/
PATCH_POINT_PUBLIC(void,MibBulbControl_vLumCadenceStop)(void)
{
    /* Bulb is currently on but needs to be off ? */
    if (DriverBulb_bOn() == TRUE && psMibBulbControl->sPerm.u8Mode == VAR_VAL_BULB_CONTROL_MODE_OFF)
    {
        /* Turn off using lamp driver */
        DriverBulb_vOff();
        /* Increment off count */
        if (PS_MIB_BULB_STATUS != NULL) MibBulbStatus_vOff();
    }

    /* Were we switching ? */
    if (psMibBulbControl->u8LumCadSwitch != 0)
    {
        /* Need to set current level to old target level */
        psMibBulbControl->sTemp.u8LumCurrent = psMibBulbControl->sPerm.u8LumTarget;
        /* Bulb was on - set level to new current */
        if (DriverBulb_bOn())
        {
            DriverBulb_vSetLevel(psMibBulbControl->sTemp.u8LumCurrent);
        }
    }
}

/****************************************************************************
 *
 * NAME: MibBulbConfig_bLumCadence
 *
 * DESCRIPTION:
 * Returns if a luminace cadence is currently in effect
 *
 ****************************************************************************/
PUBLIC bool_t MibBulbControl_bLumCadence(void)
{
    bool_t bReturn = FALSE;

    /* Is cadence timer running ? */
    if (psMibBulbControl->sTemp.u16LumCadTimer > 0 && psMibBulbControl->sTemp.u32LumCadence >= 0x00010000)
    {
        /* Return true */
        bReturn = TRUE;
    }

    return bReturn;
}

/****************************************************************************
 *
 * NAME: MibBulbControl_vLumCadenceStackEvent()
 *
 * DESCRIPTION:
 * Sets cadence effect for a stack event
 *
 ****************************************************************************/
PATCH_POINT_PUBLIC(void,MibBulbControl_vLumCadenceStackEvent)(te6LP_StackEvent eEvent)
{
    bool_t bCadence = FALSE;

    /* Bulb config and network status data is available ? */
    if (PS_MIB_BULB_CONFIG != NULL)
    {
        /* Network is up ? */
        if (E_STACK_JOINED  == eEvent ||
            E_STACK_STARTED == eEvent)
        {
            /* Is a network up cadence specified ? */
            if (PS_MIB_BULB_CONFIG->sPerm.u32UpCadence             >= 0x00010000 &&
                PS_MIB_BULB_CONFIG->sPerm.u16UpCadTimer            != 0)
            {
                /* Are we up following a factory reset and cadence should be applied ? */
                if (psMibBulbControl->sPerm.u8NwkState == MIB_BULB_CONTROL_NWK_STATE_UP_FACTORY &&
                   (PS_MIB_BULB_CONFIG->sPerm.u8DownUpCadFlags & 0x70) != 0)
                {
                    /* Apply cadence */
                    bCadence = TRUE;
                }
                /* Are we up following a reset and cadence should be applied ? */
                else if (psMibBulbControl->sPerm.u8NwkState == MIB_BULB_CONTROL_NWK_STATE_UP_RESET &&
                        (PS_MIB_BULB_CONFIG->sPerm.u8DownUpCadFlags & 0x30) != 0)
                {
                    /* Apply cadence */
                    bCadence = TRUE;
                }
                /* We should always apply cadence ? */
                else if ((PS_MIB_BULB_CONFIG->sPerm.u8DownUpCadFlags & 0x10) != 0)
                {
                    /* Apply cadence */
                    bCadence = TRUE;
                }

                /* Apply cadence ? */
                if (bCadence)
                {
                    /* Apply cadence */
                    MibBulbControl_vLumCadence(PS_MIB_BULB_CONFIG->sPerm.u32UpCadence,
                                               PS_MIB_BULB_CONFIG->sPerm.u16UpCadTimer);
                    /* Note up cadence is applied */
                    psMibBulbControl->bUpCadence = TRUE;
                    /* Note down cadence is not applied */
                    psMibBulbControl->bDownCadence = FALSE;
                }
            }
            /* Down cadence is still running ? */
            if (psMibBulbControl->bDownCadence == TRUE)
            {
                /* Cancel cadence */
                MibBulbControl_vLumCadence(0, 0);
                /* Note down cadence is not applied */
                psMibBulbControl->bDownCadence = FALSE;
            }
        }
        /* Network is down ? */
        else if (E_STACK_RESET == eEvent)
        {
            /* Is a network down cadence specified ? */
            if (PS_MIB_BULB_CONFIG->sPerm.u32DownCadence           >= 0x00010000 &&
                PS_MIB_BULB_CONFIG->sPerm.u16DownCadTimer          != 0)
            {
                /* Are we down following a factory reset and cadence should be applied ? */
                if (psMibBulbControl->sPerm.u8NwkState == MIB_BULB_CONTROL_NWK_STATE_DOWN_FACTORY &&
                   (PS_MIB_BULB_CONFIG->sPerm.u8DownUpCadFlags & 0x07) != 0)
                {
                    /* Apply cadence */
                    bCadence = TRUE;
                }
                /* Are we down following a reset and cadence should be applied ? */
                else if (psMibBulbControl->sPerm.u8NwkState == MIB_BULB_CONTROL_NWK_STATE_DOWN_RESET &&
                        (PS_MIB_BULB_CONFIG->sPerm.u8DownUpCadFlags & 0x03) != 0)
                {
                    /* Apply cadence */
                    bCadence = TRUE;
                }
                /* We should always apply cadence ? */
                else if ((PS_MIB_BULB_CONFIG->sPerm.u8DownUpCadFlags & 0x01) != 0)
                {
                    /* Apply cadence */
                    bCadence = TRUE;
                }

                /* Apply cadence ? */
                if (bCadence)
                {
                    /* Apply cadence */
                    MibBulbControl_vLumCadence(PS_MIB_BULB_CONFIG->sPerm.u32DownCadence,
                                               PS_MIB_BULB_CONFIG->sPerm.u16DownCadTimer);
                    /* Note down cadence is applied */
                    psMibBulbControl->bDownCadence = TRUE;
                    /* Note up cadence is not applied */
                    psMibBulbControl->bUpCadence = FALSE;
                }
            }
            /* Up cadence is still running ? */
            if (psMibBulbControl->bUpCadence == TRUE)
            {
                /* Cancel cadence */
                MibBulbControl_vLumCadence(0, 0);
                /* Note up cadence is not applied */
                psMibBulbControl->bUpCadence = FALSE;
            }
        }
    }
}

/****************************************************************************
 *
 * NAME: MibBulbControl_eSetMode
 *
 * DESCRIPTION:
 * Mode set data callback
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbControl_eSetMode(uint8 u8Val, void *pvCbData)
{
    teJIP_Status eReturn = E_JIP_ERROR_BAD_VALUE;
    bool_t        bSave   = FALSE;

    /* Debug */
    DBG_vPrintf(CONFIG_DBG_MIB_BULB_CONTROL, "\nMibBulbControl_eSetMode(%d)", u8Val);

    /* Valid mode ? */
    if (u8Val < VAR_VAL_BULB_CONTROL_MODE_COUNT)
    {
        /* Assume value will be set */
        eReturn = E_JIP_OK;

        /* New value is different to old one ? */
        if (psMibBulbControl->sPerm.u8Mode != u8Val)
        {
            /* Handle mode change */
            switch (u8Val)
            {
                case VAR_VAL_BULB_CONTROL_MODE_OFF:         {eReturn = MibBulbControl_eSetModeOff(&u8Val);      bSave = TRUE;}  break;
                case VAR_VAL_BULB_CONTROL_MODE_ON:          {eReturn = MibBulbControl_eSetModeOn(&u8Val);       bSave = TRUE;}  break;
                case VAR_VAL_BULB_CONTROL_MODE_TOGGLE:      {eReturn = MibBulbControl_eSetModeToggle(&u8Val);   bSave = TRUE;}  break;
                case VAR_VAL_BULB_CONTROL_MODE_DOWN:        {eReturn = MibBulbControl_eSetModeDownUp(&u8Val);                }  break;
                case VAR_VAL_BULB_CONTROL_MODE_UP:          {eReturn = MibBulbControl_eSetModeDownUp(&u8Val);                }  break;
                case VAR_VAL_BULB_CONTROL_MODE_DOWN_IF_ON:  {eReturn = MibBulbControl_eSetModeDownUp(&u8Val);                }  break;
                case VAR_VAL_BULB_CONTROL_MODE_UP_IF_ON:    {eReturn = MibBulbControl_eSetModeDownUp(&u8Val);                }  break;
                case VAR_VAL_BULB_CONTROL_MODE_TEST:        {eReturn = MibBulbControl_eSetModeTest(&u8Val);     bSave = TRUE;}  break;
                default:                                    {eReturn = E_JIP_ERROR_BAD_VALUE;                               }  break;
            }
        }

        /* Mode change was made ? */
        if (eReturn == E_JIP_OK && psMibBulbControl->sPerm.u8Mode != u8Val)
        {
            /* Call standard function */
            eReturn = eSetUint8(u8Val, pvCbData);
            /* Issue notify for device control mib variable */
            vJIP_NotifyChanged(psMibBulbControl->hDeviceControlMib, VAR_IX_DEVICE_CONTROL_MODE);
            /* Need to save permament data - set flag */
            psMibBulbControl->bSaveRecord = bSave;
        }
    }

    return eReturn;
}

/****************************************************************************
 *
 * NAME: MibDeviceControl_eSetMode
 *
 * DESCRIPTION:
 * Device Control Mode set data callback
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibDeviceControl_eSetMode(uint8 u8Val, void *pvCbData)
{
    teJIP_Status eReturn = E_JIP_ERROR_BAD_VALUE;
    bool_t        bSave   = FALSE;

    /* Valid mode ? */
    if (u8Val < VAR_VAL_DEVICE_CONTROL_MODE_COUNT)
    {
        /* Assume value will be set */
        eReturn = E_JIP_OK;

        /* New value is different to old one ? */
        if (psMibBulbControl->sPerm.u8Mode != u8Val)
        {
            /* Handle mode change */
            switch (u8Val)
            {
                case VAR_VAL_BULB_CONTROL_MODE_OFF:         {eReturn = MibBulbControl_eSetModeOff(&u8Val);      bSave = TRUE;}  break;
                case VAR_VAL_BULB_CONTROL_MODE_ON:          {eReturn = MibBulbControl_eSetModeOn(&u8Val);       bSave = TRUE;}  break;
                case VAR_VAL_BULB_CONTROL_MODE_TOGGLE:      {eReturn = MibBulbControl_eSetModeToggle(&u8Val);   bSave = TRUE;}  break;
                default:                                    {eReturn = E_JIP_ERROR_BAD_VALUE;                               }  break;
            }
        }

        /* Mode change was made ? */
        if (eReturn == E_JIP_OK && psMibBulbControl->sPerm.u8Mode != u8Val)
        {
            /* Call standard function */
            eReturn = eSetUint8(u8Val, pvCbData);
            /* Issue notify for bulb control mib variable */
            vJIP_NotifyChanged(psMibBulbControl->hMib, VAR_IX_BULB_CONTROL_MODE);
            /* Need to save permament data - set flag */
            psMibBulbControl->bSaveRecord = bSave;
        }
    }

    return eReturn;
}

/****************************************************************************
 *
 * NAME: MibBulbControl_eSetSceneId
 *
 * DESCRIPTION:
 * Set SceneId callback
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbControl_eSetSceneId(uint16 u16Val, void *pvCbData)
{
    /* Call helper function */
    return MibBulbDeviceControl_eSetSceneId(u16Val, pvCbData, TRUE);
}

/****************************************************************************
 *
 * NAME: MibDeviceControl_eSetSceneId
 *
 * DESCRIPTION:
 * Device control sets scene id
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibDeviceControl_eSetSceneId(uint16 u16Val, void *pvCbData)
{
    /* Call helper function */
    return MibBulbDeviceControl_eSetSceneId(u16Val, pvCbData, FALSE);
}

/****************************************************************************
 *
 * NAME: MibBulbDeviceControl_eSetSceneId
 *
 * DESCRIPTION:
 * Sets scene id
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbDeviceControl_eSetSceneId(uint16 u16Val, void *pvCbData, bool_t bBulbControl)
{
    teJIP_Status eReturn = E_JIP_ERROR_BAD_VALUE;
    uint8        u8Scene;

    /* Valid scene ? */
    if (u16Val > 0)
    {
        /* Assume scenes are disabled */
        eReturn = E_JIP_ERROR_DISABLED;
        /* Scene MIB available ? */
        if (PS_MIB_BULB_SCENE != NULL)
        {
            /* Try to find scene */
            u8Scene = MibBulbControl_u8FindSceneId(u16Val);
            /* Found valid scene ? */
            if (u8Scene < MIB_BULB_SCENE_SCENES)
            {
                /* Call standard function */
                eReturn = eSetUint16(u16Val, pvCbData);
                /* Bulb should be off for this scene ? */
                if (PS_MIB_BULB_SCENE->sPerm.au8SceneMode[u8Scene] == VAR_VAL_BULB_CONTROL_MODE_OFF)
                {
                    /* Is bulb not already off ? */
                    if (psMibBulbControl->sPerm.u8Mode != VAR_VAL_BULB_CONTROL_MODE_OFF)
                    {
                        /* Turn off using lamp driver */
                        DriverBulb_vOff();
                        /* Increment off count */
                        if (PS_MIB_BULB_STATUS != NULL) MibBulbStatus_vOff();
                        /* Note new value */
                        psMibBulbControl->sPerm.u8Mode = VAR_VAL_BULB_CONTROL_MODE_OFF;
                        /* Need to notify for the mode variable */
                        vJIP_NotifyChanged(psMibBulbControl->hMib, VAR_IX_BULB_CONTROL_MODE);
                        vJIP_NotifyChanged(psMibBulbControl->hDeviceControlMib, VAR_IX_DEVICE_CONTROL_MODE);
                        /* Make sure permament data is saved */
                        psMibBulbControl->bSaveRecord = TRUE;
                    }
                }
                /* Bulb should be on for this scene ? */
                else
                {
                    /* Is bulb not currently on ? */
                    if (psMibBulbControl->sPerm.u8Mode != VAR_VAL_BULB_CONTROL_MODE_ON)
                    {
                        /* Lamp is currently off ? */
                        if (psMibBulbControl->sPerm.u8Mode == VAR_VAL_BULB_CONTROL_MODE_OFF)
                        {
                            /* Turn on lamp using driver */
                            DriverBulb_vOn();
                            /* Increment on count */
                            if (PS_MIB_BULB_STATUS != NULL) MibBulbStatus_vOn();
                        }
                        /* Note new value */
                        psMibBulbControl->sPerm.u8Mode = VAR_VAL_BULB_CONTROL_MODE_ON;
                        /* Need to notify for the mode variable */
                        vJIP_NotifyChanged(psMibBulbControl->hMib, VAR_IX_BULB_CONTROL_MODE);
                        vJIP_NotifyChanged(psMibBulbControl->hDeviceControlMib, VAR_IX_DEVICE_CONTROL_MODE);
                        /* Make sure permament data is saved */
                        psMibBulbControl->bSaveRecord = TRUE;
                    }
                    /* Is the scene target different to the current target ? */
                    if (PS_MIB_BULB_SCENE->sPerm.au8SceneLumTarget[u8Scene] != psMibBulbControl->sPerm.u8LumTarget)
                    {
                        /* Note new value */
                        psMibBulbControl->sPerm.u8LumTarget = PS_MIB_BULB_SCENE->sPerm.au8SceneLumTarget[u8Scene];
                        /* Need to notify for the mode variable */
                        vJIP_NotifyChanged(psMibBulbControl->hMib, VAR_IX_BULB_CONTROL_LUM_TARGET);
                        /* Make sure permament data is saved */
                        psMibBulbControl->bSaveRecord = TRUE;
                    }
                }
                /* Issued to bulb control mib ? */
                if (bBulbControl)
                {
                    /* Issue notify for device control mib variable */
                    vJIP_NotifyChanged(psMibBulbControl->hDeviceControlMib, VAR_IX_DEVICE_CONTROL_SCENE_ID);
                }
                /* Issued to device control mib ? */
                else
                {
                    /* Issue notify for bulb control mib variable */
                    vJIP_NotifyChanged(psMibBulbControl->hMib, VAR_IX_BULB_CONTROL_SCENE_ID);
                }
            }
            /* Couldn't find scene ? */
            else
            {
                /* Return bad value */
                eReturn = E_JIP_ERROR_BAD_VALUE;
            }
        }
    }

    return eReturn;
}

/****************************************************************************
 *
 * NAME: MibBulbControl_eSetLumTarget
 *
 * DESCRIPTION:
 * Sets luminance target
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbControl_eSetLumTarget(uint8 u8Val, void *pvCbData)
{
    teJIP_Status eReturn = E_JIP_ERROR_DISABLED;

    /* In a mode where the user is in control of the target level ? */
    if (psMibBulbControl->sPerm.u8Mode == VAR_VAL_BULB_CONTROL_MODE_OFF || psMibBulbControl->sPerm.u8Mode == VAR_VAL_BULB_CONTROL_MODE_ON)
    {
        /* Call standard function */
        eReturn = eSetUint8(u8Val, pvCbData);

        /* Make sure permament data is saved */
        psMibBulbControl->bSaveRecord = TRUE;
    }

    return eReturn;
}

/****************************************************************************
 *
 * NAME: MibBulbControl_eSetLumCurrent
 *
 * DESCRIPTION:
 * Sets LumCurrent variable
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbControl_eSetLumCurrent(uint8 u8Val, void *pvCbData)
{
    teJIP_Status eReturn = E_JIP_ERROR_DISABLED;

    /* In a mode where the user is in control of the target level ? */
    if (psMibBulbControl->sPerm.u8Mode == VAR_VAL_BULB_CONTROL_MODE_OFF || psMibBulbControl->sPerm.u8Mode == VAR_VAL_BULB_CONTROL_MODE_ON)
    {
        /* Call standard function */
        eReturn = eSetUint8(u8Val, pvCbData);

        /* Set lamp level straight to current level using driver */
        DriverBulb_vSetLevel(psMibBulbControl->sTemp.u8LumCurrent);

        /* Target level is different to current level ? */
        if (psMibBulbControl->sPerm.u8LumTarget != psMibBulbControl->sTemp.u8LumCurrent)
        {
            /* Update target level */
            psMibBulbControl->sPerm.u8LumTarget = psMibBulbControl->sTemp.u8LumCurrent;
            /* Need to notify for the target level variable */
            vJIP_NotifyChanged(psMibBulbControl->hMib, VAR_IX_BULB_CONTROL_LUM_TARGET);
            /* Make sure permament data is saved */
            psMibBulbControl->bSaveRecord = TRUE;
        }
    }

    return eReturn;
}

/****************************************************************************
 *
 * NAME: MibBulbControl_eSetLumChange
 *
 * DESCRIPTION:
 * Sets LumChange variable
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbControl_eSetLumChange(int16 i16Val, void *pvCbData)
{
    teJIP_Status eReturn = E_JIP_ERROR_DISABLED;

    /* In a mode where the user is in control of the target level ? */
    if (psMibBulbControl->sPerm.u8Mode == VAR_VAL_BULB_CONTROL_MODE_OFF || psMibBulbControl->sPerm.u8Mode == VAR_VAL_BULB_CONTROL_MODE_ON)
    {
        int16  i16LumTarget;

        /* Call standard function */
        eReturn = eSetUint16(i16Val, pvCbData);

        /* Calculate changed target value */
        i16LumTarget = (int16) psMibBulbControl->sPerm.u8LumTarget + psMibBulbControl->sTemp.i16LumChange;
        /* Make sure it is valid */
        if      (i16LumTarget > 255) i16LumTarget = 255;
        else if (i16LumTarget <   0) i16LumTarget = 0;

        /* New value is different ? */
        if (psMibBulbControl->sPerm.u8LumTarget != (uint8) i16LumTarget)
        {
            /* Note new value */
            psMibBulbControl->sPerm.u8LumTarget = (uint8) i16LumTarget;
            /* Need to notify for the target level variable */
            vJIP_NotifyChanged(psMibBulbControl->hMib, VAR_IX_BULB_CONTROL_LUM_TARGET);
            /* Make sure permament data is saved */
            psMibBulbControl->bSaveRecord = TRUE;
        }
    }

    return eReturn;
}

/****************************************************************************
 *
 * NAME: MibBulbControl_eSetLumCadence
 *
 * DESCRIPTION:
 * Sets LumCadence variable
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbControl_eSetLumCadence(uint32 u32Val, void *pvCbData)
{
    teJIP_Status eReturn;

    /* Call standard function */
    eReturn = eSetUint32(u32Val, pvCbData);

    /* Update cadence */
    MibBulbControl_vLumCadence(psMibBulbControl->sTemp.u32LumCadence,
                               psMibBulbControl->sTemp.u16LumCadTimer);

    return eReturn;
}

/****************************************************************************
 *
 * NAME: MibBulbControl_eSetLumCadTimer
 *
 * DESCRIPTION:
 * Sets LumCadTimer variable
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbControl_eSetLumCadTimer(uint16 u16Val, void *pvCbData)
{
    teJIP_Status eReturn;

    /* Call standard function */
    eReturn = eSetUint16(u16Val, pvCbData);

    /* Update cadence */
    MibBulbControl_vLumCadence(psMibBulbControl->sTemp.u32LumCadence,
                               psMibBulbControl->sTemp.u16LumCadTimer);

    return eReturn;
}

/****************************************************************************
 *
 * NAME: MibBulbControl_eSetModeOff
 *
 * DESCRIPTION:
 * Mode off set data callback
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbControl_eSetModeOff(uint8 *pu8Mode)
{
    teJIP_Status eReturn = E_JIP_OK;

    /* Cadence not in effect ? */
    if (FALSE == MibBulbControl_bLumCadence())
    {
        /* Is the lamp currently not off ? */
        if (psMibBulbControl->sPerm.u8Mode != VAR_VAL_BULB_CONTROL_MODE_OFF)
        {
            /* Turn off using lamp driver */
            DriverBulb_vOff();
            /* Increment off count */
            if (PS_MIB_BULB_STATUS != NULL) MibBulbStatus_vOff();
        }
    }

    return eReturn;
}

/****************************************************************************
 *
 * NAME: MibBulbControl_eSetModeOn
 *
 * DESCRIPTION:
 * Mode On set data callback
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbControl_eSetModeOn(uint8 *pu8Mode)
{
    teJIP_Status eReturn = E_JIP_OK;

    /* Cadence not in effect ? */
    if (FALSE == MibBulbControl_bLumCadence())
    {
        /* Is the lamp currently off ? */
        if (psMibBulbControl->sPerm.u8Mode == VAR_VAL_BULB_CONTROL_MODE_OFF)
        {
            /* Turn on lamp using driver */
            DriverBulb_vOn();
            /* Increment on count */
            if (PS_MIB_BULB_STATUS != NULL) MibBulbStatus_vOn();
            /* Current level is different to target level ? */
            if (psMibBulbControl->sTemp.u8LumCurrent != psMibBulbControl->sPerm.u8LumTarget)
            {
                /* Update current level */
                psMibBulbControl->sTemp.u8LumCurrent = psMibBulbControl->sPerm.u8LumTarget;
                /* Notify that we've updated the target level */
                vJIP_NotifyChanged(psMibBulbControl->hMib, VAR_IX_BULB_CONTROL_LUM_CURRENT);
            }
            /* Set lamp level straight to target level using driver */
            DriverBulb_vSetLevel(psMibBulbControl->sPerm.u8LumTarget);
        }
        /* Is the lamp currently on in a special mode ? */
        else if (psMibBulbControl->sPerm.u8Mode != VAR_VAL_BULB_CONTROL_MODE_ON)
        {
            /* Need to notify that we've been changing the target and current levels */
            vJIP_NotifyChanged(psMibBulbControl->hMib, VAR_IX_BULB_CONTROL_LUM_TARGET);
            vJIP_NotifyChanged(psMibBulbControl->hMib, VAR_IX_BULB_CONTROL_LUM_CURRENT);
        }
    }

    return eReturn;
}

/****************************************************************************
 *
 * NAME: MibBulbControl_eSetModeDownUp
 *
 * DESCRIPTION:
 * Mode up down set data callback
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbControl_eSetModeDownUp(uint8 *pu8Mode)
{
    teJIP_Status eReturn = E_JIP_OK;

    /* Should the mode only be applied if the bulb is on ? */
    if (*pu8Mode == VAR_VAL_BULB_CONTROL_MODE_DOWN_IF_ON ||
        *pu8Mode == VAR_VAL_BULB_CONTROL_MODE_UP_IF_ON)
    {
        /* Is the bulb not on ? */
        if (psMibBulbControl->sPerm.u8Mode != VAR_VAL_BULB_CONTROL_MODE_ON)
        {
            /* Fail return false */
            eReturn = E_JIP_ERROR_DISABLED;
        }
    }

    /* Still OK to attempt mode change ? */
    if (eReturn == E_JIP_OK)
    {
        /* Cadence not in effect ? */
        if (FALSE == MibBulbControl_bLumCadence())
        {
            /* Is the lamp currently off ? */
            if (psMibBulbControl->sPerm.u8Mode == VAR_VAL_BULB_CONTROL_MODE_OFF)
            {
                /* Turn on lamp using driver */
                DriverBulb_vOn();
                /* Increment on count */
                if (PS_MIB_BULB_STATUS != NULL) MibBulbStatus_vOn();
                /* Set lamp level straight to target level using driver */
                DriverBulb_vSetLevel(psMibBulbControl->sPerm.u8LumTarget);
                /* Current level is different to target level ? */
                if (psMibBulbControl->sTemp.u8LumCurrent != psMibBulbControl->sPerm.u8LumTarget)
                {
                    /* Update current level */
                    psMibBulbControl->sTemp.u8LumCurrent = psMibBulbControl->sPerm.u8LumTarget;
                    /* Need to notify for the current level variable */
                    vJIP_NotifyChanged(psMibBulbControl->hMib, VAR_IX_BULB_CONTROL_LUM_CURRENT);
                }
            }
        }

        /* Set temporary target appropriate for mode ? */
        if (*pu8Mode == VAR_VAL_BULB_CONTROL_MODE_DOWN ||
            *pu8Mode == VAR_VAL_BULB_CONTROL_MODE_DOWN_IF_ON)
        {
            /* Use min for target */
            psMibBulbControl->u8LumTmpTarget = 0;
        }
        else if (*pu8Mode == VAR_VAL_BULB_CONTROL_MODE_UP ||
                 *pu8Mode == VAR_VAL_BULB_CONTROL_MODE_UP_IF_ON)
        {
            /* Use max for target */
            psMibBulbControl->u8LumTmpTarget = 0xff;
        }
    }

    return eReturn;
}

/****************************************************************************
 *
 * NAME: MibBulbControl_eSetModeToggle
 *
 * DESCRIPTION:
 * Mode toggle set data callback
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbControl_eSetModeToggle(uint8 *pu8Mode)
{
    teJIP_Status eReturn = E_JIP_ERROR_DISABLED;

    /* Is the lamp currently off ? */
    if (psMibBulbControl->sPerm.u8Mode == VAR_VAL_BULB_CONTROL_MODE_OFF)
    {
        /* Note we are going to on mode */
        *pu8Mode = VAR_VAL_BULB_CONTROL_MODE_ON;
        /* Turn it on */
        eReturn = MibBulbControl_eSetModeOn(pu8Mode);
    }
    /* Is the lamp currently on ? */
    else if (psMibBulbControl->sPerm.u8Mode == VAR_VAL_BULB_CONTROL_MODE_ON)
    {
        /* Note we are going to off mode */
        *pu8Mode = VAR_VAL_BULB_CONTROL_MODE_OFF;
        /* Turn it off */
        eReturn = MibBulbControl_eSetModeOff(pu8Mode);
    }

    return eReturn;
}

/****************************************************************************
 *
 * NAME: MibBulbControl_eSetModeTest
 *
 * DESCRIPTION:
 * Mode test set data callback
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbControl_eSetModeTest(uint8 *pu8Mode)
{
    teJIP_Status eReturn = E_JIP_OK;

    /* Cadence not in effect ? */
    if (FALSE == MibBulbControl_bLumCadence())
    {
        /* Is the lamp currently off ? */
        if (psMibBulbControl->sPerm.u8Mode == VAR_VAL_BULB_CONTROL_MODE_OFF)
        {
            /* Turn on lamp using driver */
            DriverBulb_vOn();
            /* Increment on count */
            if (PS_MIB_BULB_STATUS != NULL) MibBulbStatus_vOn();
            /* Set lamp level straight to current level using driver */
            DriverBulb_vSetLevel(psMibBulbControl->sTemp.u8LumCurrent);
        }
    }

    return eReturn;
}

/****************************************************************************
 *
 * NAME: MibBulbControl_u8FindSceneId
 *
 * DESCRIPTION:
 * Searches for a scene id
 *
 ****************************************************************************/
PUBLIC uint8 MibBulbControl_u8FindSceneId(uint16 u16SceneId)
{
    uint8 u8Scene;
    uint8 u8Found = 0xff;

    /* Got a pointer to scene data ? */
    if (PS_MIB_BULB_SCENE != NULL)
    {
        /* Look for scene */
        for (u8Scene = 0; u8Scene < MIB_BULB_SCENE_SCENES && u8Found == 0xff; u8Scene++)
        {
            /* Is this the scene we're looking for ? */
            if (PS_MIB_BULB_SCENE->sPerm.au16SceneId[u8Scene] == u16SceneId)
            {
                /* Note its index */
                u8Found = u8Scene;
            }
        }
    }

    return u8Found;
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
