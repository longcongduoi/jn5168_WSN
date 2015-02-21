/****************************************************************************
 *
 * MODULE:             JenNet-IP Border Router
 *
 * COMPONENT:          Module Configuration message definition
 *
 * VERSION:            $Name:  $
 *
 * REVISION:           $Revision: 1.2 $
 *
 * DATED:              $Date: 2009/01/21 15:49:32 $
 *
 * STATUS:             $State: Exp $
 *
 * AUTHOR:             Lee Mitchell
 *
 * DESCRIPTION:
 *
 * LAST MODIFIED BY:   $Author: lmitch $
 *                     $Modtime: $
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

#ifndef  MODULECONFIG_H_INCLUDED
#define  MODULECONFIG_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <jendefs.h>
#include <Api.h>
#include <6LP.h>


/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/** Define GPIO pin used to select high power module */
#define MODULE_FEATURE_SELECT_PIN_HIGH_POWER            (1 << 18)


/** Define GPIO pin used to select antenna diversity */
#define MODULE_FEATURE_SELECT_PIN_ANTENNA_DIVERSITY     (1 << 17)

/* Activity LED is enabled on reception of an E_SL_MSG_ACTIVITY_LED message */
#ifdef ENABLE_ACTIVITY_LED
#define ACTIVITY_LED_DISABLED (0xFFFFFFFF)
#endif /* ENABLE_ACTIVITY_LED */

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/** Basic configuration of the IEEE802.15.4 & 6LoWPAN parameters */
typedef struct
{
	uint8	u8Region;            /**< Radio Region */
	uint8	u8Channel;           /**< IEEE802.15.4 Channel */
	uint16 	u16PanID;            /**< IEEE802.15.4 PAN ID */
	uint32	u32NetworkID;        /**< 32bit Network identifier */
	uint64 	u64NetworkPrefix;    /**< IPv6 Prefix */
}tsModule_Config;


/** Enumerated type of regions for radio regulation standards */
typedef enum
{
	E_REGION_EUROPE,
	E_REGION_USA,
	E_REGION_JAPAN
}teRegion;


/** Structure defining the message passed for setting JenNet-IP network security options */
typedef struct
{
    union {
        struct {
            uint64          u64NetworkKeyH;
            uint64          u64NetworkKeyL;
        };
        tsSecurityKey       sNetworkKey;    /**< Configured security key */
    };

    enum
    {
        E_AUTH_SCHEME_NONE,                 /**< No authentication scheme - note no nodes without the network key will be able to join */
        E_AUTH_SCHEME_RADIUS_PAP,           /**< Ask a RADIUS server to authorise joining nodes using PAP */
    } eAuthScheme;

    union
    {
        struct
        {
            in6_addr        sAuthServerIP;  /**< IPv6 Address of RADIUS server */
        } sRadiusPAP;
    } uScheme;

} __attribute__((__packed__))  __attribute__((__aligned__)) tsSecurityConfig;


/** Enumerated type of supported radio front ends */
typedef enum
{
    E_FRONTEND_STANDARD_POWER,          /**< No frontend - just a standard power device */
    E_FRONTEND_HIGH_POWER,              /**< High power module - enable PA and LNA */
    E_FRONTEND_ETSI,                    /**< Enable ETSI compliant mode */
} __attribute__((__packed__)) teRadioFrontEnd;


/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/** Module configuration */
extern tsModule_Config sConfig;


/* Activity LED is enabled on reception of an E_SL_MSG_ACTIVITY_LED message */
#ifdef ENABLE_ACTIVITY_LED
extern uint32 u32ActivityLEDMask;
#endif /* ENABLE_ACTIVITY_LED */


/** Keep a map of the pins that are allocated.
 *  This is used to sanity check when we're asked
 *  to a pin for the activity LED for example.
 */
extern uint32 u32AllocatedPins;


/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

PUBLIC bool bModuleIsHighPower(void);
PUBLIC void vModuleSetRadioFrontEnd(teRadioFrontEnd eRadioFrontEnd);
PUBLIC bool bModuleHasAntennaDiversity(void);
PUBLIC void vModuleEnableDiversity(void);

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* MODULECONFIG_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

