/****************************************************************************
 *
 * MODULE:             JenNet-IP Border Router
 *
 * COMPONENT:          Module configuration
 *
 * VERSION:            $Name:  $
 *
 * REVISION:           $Revision: 1.14 $
 *
 * DATED:              $Date: 2010/01/13 10:26:10 $
 *
 * STATUS:             $State: Exp $
 *
 * AUTHOR:             Matt Redfearn
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

#include <jendefs.h>
#include <AppHardwareApi.h>
#include <AppApi.h>
#include <Api.h>

#include "ModuleConfig.h"
#include "log.h"

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

/** Module configuration */
tsModule_Config sConfig;


/* Activity LED is enabled on reception of an E_SL_MSG_ACTIVITY_LED message */
#ifdef ENABLE_ACTIVITY_LED
PUBLIC uint32 u32ActivityLEDMask = ACTIVITY_LED_DISABLED;
#endif /* ENABLE_ACTIVITY_LED */


/** Keep a map of the pins that are allocated.
 *  This is used to sanity check when we're asked
 *  to a pin for the activity LED for example.
 */
PUBLIC uint32 u32AllocatedPins =
    3 << 6              |           /* UART0 TX/RX */
#ifdef UART_DEBUG
    3 << 19             |           /* UART1 TX/RX */
#endif /* UART_DEBUG */
    MODULE_FEATURE_SELECT_PIN_HIGH_POWER |
    MODULE_FEATURE_SELECT_PIN_ANTENNA_DIVERSITY;


/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/


/****************************************************************************
 *
 * NAME: bModuleIsHighPower
 *
 * DESCRIPTION:
 * Checks if the high power module select jumper is fitted or not.
 *
 * RETURNS:
 * TRUE if high power jumper fitted.
 *
 ****************************************************************************/
PUBLIC bool bModuleIsHighPower(void)
{
    /* Invert due to internal pullup / external pulldown to enable */
    return (!(u32AHI_DioReadInput() & MODULE_FEATURE_SELECT_PIN_HIGH_POWER));
}


/****************************************************************************
 *
 * NAME: vModuleSetRadioFrontEnd
 *
 * DESCRIPTION:
 * Set up the module appropriately for a given radio front end
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vModuleSetRadioFrontEnd(teRadioFrontEnd eRadioFrontEnd)
{
    if ((u8AHI_PowerStatus() & (1 << 3)) == 0)
    {
        /* Power domain is inactive - writing to the register will cause a bus exception */
        return;
    }

    switch(eRadioFrontEnd)
    {
    case (E_FRONTEND_STANDARD_POWER):
        u32AllocatedPins &= ~(3 << 2); /* DIO2 & 3 Used for Tx/Rx on High power module - mark available  */
        vLog_Printf(TRACE_BR, LOG_INFO, "\nEnable Std Power");
#if defined JENNIC_CHIP_FAMILY_JN516x
        vAppApiSetHighPowerMode(APP_API_MODULE_STD, TRUE);
#else
        vAHI_HighPowerModuleEnable(FALSE, FALSE);
#endif /* JENNIC_CHIP_FAMILY_JN516x */
        break;

    case (E_FRONTEND_HIGH_POWER):
    case (E_FRONTEND_ETSI):
        /* Pin is pulled low - High power mode selected */
        u32AllocatedPins |= 3 << 2; /* DIO2 & 3 Used for Tx/Rx on High power module - mark allocated  */
        if (u32ActivityLEDMask != ACTIVITY_LED_DISABLED)
        {
            if (u32ActivityLEDMask & (3 << 2))
            {
                /* The activity LED has already been set up as pin 2/3, disable it! */
                u32ActivityLEDMask = ACTIVITY_LED_DISABLED;
            }
        }

        vLog_Printf(TRACE_BR, LOG_INFO, "\nEnable High Power");
#if defined JENNIC_CHIP_FAMILY_JN516x
#else
        vAHI_HighPowerModuleEnable(TRUE, TRUE);
#endif /* JENNIC_CHIP_FAMILY_JN516x */

        switch(sConfig.u8Region)
        {

        case E_REGION_EUROPE:
            break;

        case E_REGION_USA:
#if defined JENNIC_CHIP_FAMILY_JN516x
#else
        	bAHI_PhyRadioSetPower(5);
#endif /* JENNIC_CHIP_FAMILY_JN516x */
            break;

        case E_REGION_JAPAN:
            break;
        }

        if (eRadioFrontEnd == E_FRONTEND_HIGH_POWER)
        {
#if defined JENNIC_CHIP_FAMILY_JN516x
        	vAppApiSetHighPowerMode(APP_API_MODULE_HPM06, TRUE);
#else
        	vAppApiSetHighPowerMode(APP_API_HPM_MODULE_M06, APP_API_HPM_MODE_FULL);
#endif /* JENNIC_CHIP_FAMILY_JN516x */

        }
        else if (eRadioFrontEnd == E_FRONTEND_ETSI)
        {
            vLog_Printf(TRACE_BR, LOG_INFO, "\nEnable ETSI");

#if defined JENNIC_CHIP_FAMILY_JN516x
            vAppApiSetHighPowerMode(APP_API_MODULE_HPM05, TRUE);
#else
            vAppApiSetHighPowerMode(APP_API_HPM_MODULE_M06, APP_API_HPM_MODE_ETSI);
#endif /* JENNIC_CHIP_FAMILY_JN516x */
        }
        {
            uint32 u32TxPower;
            /* Read then write back TX power level
             * This forces the MAC to implement the previous change.
             */
            eAppApiPlmeGet(PHY_PIB_ATTR_TX_POWER,&u32TxPower);
            eAppApiPlmeSet(PHY_PIB_ATTR_TX_POWER, u32TxPower);
        }
        break;

    default:
        break;
    }
}


/****************************************************************************
 *
 * NAME: bModuleHasAntennaDiversity
 *
 * DESCRIPTION:
 * Checks if the antenna diversity select jumper is fitted or not.
 *
 * RETURNS:
 * TRUE if antenna diversity jumper fitted.
 *
 ****************************************************************************/
PUBLIC bool bModuleHasAntennaDiversity(void)
{
    /* Invert due to internal pullup / external pulldown to enable */
    return (!(u32AHI_DioReadInput() & MODULE_FEATURE_SELECT_PIN_ANTENNA_DIVERSITY));
}


/****************************************************************************
 *
 * NAME: vModuleEnableDiversity
 *
 * DESCRIPTION:
 * Enable antenna diversity.
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vModuleEnableDiversity(void)
{
    if ((u8AHI_PowerStatus() & (1 << 3)) == 0)
    {
        /* Power domain is inactive - writing to the register will cause a bus exception */
        return;
    }

    /* DIO12 & 13 Used for antenna diversity */
    u32AllocatedPins |= 3 << 12;

    if (u32ActivityLEDMask != ACTIVITY_LED_DISABLED)
    {
        if (u32ActivityLEDMask & (3 << 12))
        {
            /* The activity LED has already been set up as pin 12/13, disable it! */
            u32ActivityLEDMask = ACTIVITY_LED_DISABLED;
        }
    }
    vLog_Printf(TRACE_BR, LOG_INFO, "\nEnable Antenna Diversity");
    vAHI_AntennaDiversityOutputEnable(TRUE,  TRUE);
}


/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/


/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

