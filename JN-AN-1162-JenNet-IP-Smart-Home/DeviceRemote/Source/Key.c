/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         Key Press Handler
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
#include <JIP.h>
#include "string.h"


#include "dbg.h"
#include "dbg_uart.h"
#include "Uart.h"
#include "AppHardwareApi.h"

#include "MibCommon.h"
#include "MibBulb.h"
#include "DriverCapTouch.h"
#include "Key.h"
#include "Mib.h"
#include "ModeCommission.h"
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifdef DBG_ENABLE
#define TRACE_KEYS  TRUE
#else
#define TRACE_KEYS  FALSE
#endif

#define MODE_CHANGE_KEY_PRESS_TIMEOUT 500U

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/


typedef enum
{
	E_KEY_TYPE_NONE,
	E_KEY_TYPE_GROUP,
	E_KEY_TYPE_COMMAND
} teKeyType;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

PRIVATE teKeyType eGetKeyType(teTouchKeys eTouchKeys);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Global Variables                                               ***/
/****************************************************************************/

static const uint8 au8StartNwk[4]     	 = {E_KEY_PROG, E_KEY_DOWN, E_KEY_UP,   E_KEY_DOWN};
static const uint8 au8JoinNwk[4]  		 = {E_KEY_PROG, E_KEY_UP,   E_KEY_DOWN, E_KEY_UP};
static const uint8 au8Reset[4]  	     = {E_KEY_PROG, E_KEY_ON,   E_KEY_UP,   E_KEY_ON};
static const uint8 au8FactoryReset[4]  	 = {E_KEY_PROG, E_KEY_OFF,  E_KEY_DOWN, E_KEY_OFF};
static const uint8 au8Commission[4]  	 = {E_KEY_PROG, E_KEY_ON,   E_KEY_OFF,  E_KEY_ON};
static const uint8 au8Decommission[4] 	 = {E_KEY_PROG, E_KEY_OFF,  E_KEY_ON,   E_KEY_OFF};
static const uint8 au8AddGroup[3]     	 = {E_KEY_PROG, E_KEY_UP,   E_KEY_ON};
static const uint8 au8DelGroup[3]     	 = {E_KEY_PROG, E_KEY_DOWN, E_KEY_OFF};

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE uint8 u8Group = 0;

PRIVATE struct sKeyInfo_Tag
{
	uint8 au8History[6];
	uint16 u16ModeChangeTimer;
	bool_t bModeChange;
	bool_t bModeNormal;
} sKeyInfo = {{E_KEY_NONE,E_KEY_NONE,E_KEY_NONE,E_KEY_NONE,E_KEY_NONE,E_KEY_NONE},0,FALSE,TRUE};

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: vKeyPressTracker
 *
 * DESCRIPTION:
 *
 * track user key press sequences on remote for various operating conditions
 * training and erasing behaviour
 *
 * PARAMETERS:      Name            RW  Usage
 *                  eTouckKeys      R   The key pressed on the remote
 *
 * RETURNS:         None
 *
 ****************************************************************************/
PUBLIC teKeyStatusCode eKeyPressTracker(teTouchKeys eTouchKeys, bool_t bNormal)
{

    uint8 u8Index;
    teKeyStatusCode eKeyStatusCode = E_MODE_NORMAL;

    if (eTouchKeys != E_KEY_NONE)
    {
		for (u8Index = 0;u8Index<5;u8Index++)
		{
			 sKeyInfo.au8History[u8Index] =  sKeyInfo.au8History[u8Index+1];
		}
		sKeyInfo.au8History[5] = eTouchKeys;


		/* S-R latch to track when user is entering a mode change */

		if (sKeyInfo.au8History[5] == E_KEY_PROG)
		{

			sKeyInfo.bModeChange = TRUE;
			sKeyInfo.u16ModeChangeTimer = MODE_CHANGE_KEY_PRESS_TIMEOUT;
		}

	}
    else
    {
    	/* No key pressed (=last key released so we check if we were dimming up or    */
    	/* down. If so then we need to tell the lamp to stop changing the light level */
    	if (((sKeyInfo.au8History[5] == E_KEY_UP) || (sKeyInfo.au8History[5] == E_KEY_DOWN)) && !sKeyInfo.bModeChange)
    	{
    		if (bNormal) vSetModeMibVar(eTouchKeys);
    	}
    }


	while (eTouchKeys != E_KEY_NONE)   /* Only process key presses */
	{


		if (memcmp(au8Decommission,&sKeyInfo.au8History[1],4)==0)
		{
			if ((eGetKeyType(eTouchKeys) == E_KEY_TYPE_GROUP) && (u16Api_GetStackMode() & NONE_GATEWAY_MODE) && bNormal)
			{
				DBG_vPrintf(TRACE_KEYS,"\nDecommission GRP");
				eKeyStatusCode = E_MODE_DCMSNG_START;
                break;
			}
		}

		if (memcmp(au8Commission,&sKeyInfo.au8History[1],4)==0)
		{
			/* Are we in standalone mode ? */
			if ((u16Api_GetStackMode() & NONE_GATEWAY_MODE) && bNormal)
			{
				/* Ended with a group ? */
				if (eGetKeyType(eTouchKeys) == E_KEY_TYPE_GROUP)
				{
					/* Commission bulb */
					eKeyStatusCode = E_MODE_CMSNG_BULB_START;
					break;
				}
				/* Ended with down ? */
				else if (eTouchKeys == E_KEY_DOWN)
				{
					/* Commission remote */
					eKeyStatusCode = E_MODE_CMSNG_REMOTE_START;
					break;
				}
				/* Ended with up ? */
				else if (eTouchKeys == E_KEY_UP)
				{
					/* Clone remote */
					eKeyStatusCode = E_MODE_CLONE_REMOTE_START;
					break;
				}
				/* Ended with off ? */
				else if (eTouchKeys == E_KEY_OFF)
				{
					/* Clone border router */
					eKeyStatusCode = E_MODE_CMSNG_BR_START;
					break;
				}
			}
		}
		if (memcmp(au8StartNwk,&sKeyInfo.au8History[2],4)==0)
		{
			DBG_vPrintf(TRACE_KEYS,"\nStandalone combo");
			DBG_vPrintf(TRACE_KEYS,"\nStandalone mode");
			/* Reset into standalone mode */
			eKeyStatusCode = E_MODE_RESET_TO_STANDALONE;
			break;
		}
		if (memcmp(au8JoinNwk,&sKeyInfo.au8History[2],4)==0)
		{
			DBG_vPrintf(TRACE_KEYS,"\nGateway combo");
			DBG_vPrintf(TRACE_KEYS,"\nGateway mode");
			/* Reset into gateway mode */
			eKeyStatusCode = E_MODE_RESET_TO_GATEWAY;
			break;
		}
		if (memcmp(au8Reset,&sKeyInfo.au8History[2],4)==0)
		{
			DBG_vPrintf(TRACE_KEYS,"\nReset combo");
			/* Reset into gateway mode */
			eKeyStatusCode = E_MODE_RESET;
			break;
		}
		if (memcmp(au8FactoryReset,&sKeyInfo.au8History[2],4)==0)
		{
			DBG_vPrintf(TRACE_KEYS,"\nFactory reset combo");
			/* Reset into gateway mode */
			eKeyStatusCode = E_MODE_FACTORY_RESET;
			break;
		}
		if (memcmp(au8AddGroup,&sKeyInfo.au8History[2],3)==0)
		{
			/* Ended with a group ? */
			if (eGetKeyType(eTouchKeys) == E_KEY_TYPE_GROUP && bNormal)
			{
				/* Add group */
				eKeyStatusCode = E_MODE_ADD_GROUP_START;
				break;
			}
		}
		if (memcmp(au8DelGroup,&sKeyInfo.au8History[2],3)==0)
		{
			/* Ended with a group ? */
			if (eGetKeyType(eTouchKeys) == E_KEY_TYPE_GROUP && bNormal)
			{
				/* Del group */
				eKeyStatusCode = E_MODE_DEL_GROUP_START;
				break;
			}
		}

		break;
	} /* while (eTouchKeys != E_KEY_NONE)  */


	if (!sKeyInfo.bModeChange)
	{
		if (bNormal)
		{
			if (eGetKeyType(eTouchKeys) == E_KEY_TYPE_COMMAND)
			{
				vSetModeMibVar(eTouchKeys);
			}
		}
	}
	else /* Successfully changed mode within timeout allowed so cancel timer */
	{
		if ((sKeyInfo.u16ModeChangeTimer >0) && (eKeyStatusCode != E_MODE_NORMAL))
		{
			sKeyInfo.u16ModeChangeTimer = 0;
			sKeyInfo.bModeChange = FALSE;

		}
	}

	return (eKeyStatusCode);
}

/****************************************************************************
 *
 * NAME: u16GetGroup
 *
 * DESCRIPTION:
 * Access function to get the group address from the last group key pressed
 *
 * RETURNS:   Last Group Selected (current)
 *
 ****************************************************************************/
PUBLIC uint8 u8GetLastGroup(void)
{
	return (u8Group);
}

/****************************************************************************
 *
 * NAME: vKeyTick
 *
 * DESCRIPTION:
 * 10ms ticks from main module to handle key timeouts
 * Currently ensures we can drop out of attempting to change the
 * remote mode after five seconds via a key press, returning to controlling
 *
 ****************************************************************************/

PUBLIC void vKeyTick()
{
	if (sKeyInfo.u16ModeChangeTimer>0)
	{
		sKeyInfo.u16ModeChangeTimer--;
		if ((sKeyInfo.u16ModeChangeTimer==0) && (sKeyInfo.bModeChange == TRUE))
		{
			/* Flush history as we've aborted the mode change. User will */
			/* need to key the mode change sequence again from scratch   */

			sKeyInfo.bModeChange   = FALSE;
			sKeyInfo.au8History[0] = E_KEY_NONE;
			sKeyInfo.au8History[1] = E_KEY_NONE;
			sKeyInfo.au8History[2] = E_KEY_NONE;
			sKeyInfo.au8History[3] = E_KEY_NONE;
			sKeyInfo.au8History[4] = E_KEY_NONE;
			sKeyInfo.au8History[5] = E_KEY_NONE;
		}
	}
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: e16GetKeyType
 *
 * DESCRIPTION:
 *
 * Determines the generic function of the remote keys and updates the
 * current group address if a group button was pressed
 *
 * PARAMETERS:      Name            RW  Usage
 *                  eTouckKeys      R   The key pressed on the remote
 *
 * RETURNS:         Type of Key (None, command or Group)
 *
 ****************************************************************************/

PRIVATE teKeyType eGetKeyType(teTouchKeys eTouchKeys)
{
	switch (eTouchKeys)
	{
		case E_KEY_ALL: u8Group=0; return (E_KEY_TYPE_GROUP); break;
		case E_KEY_A  : u8Group=1; return (E_KEY_TYPE_GROUP); break;
		case E_KEY_B  : u8Group=2; return (E_KEY_TYPE_GROUP); break;
		case E_KEY_C  : u8Group=3; return (E_KEY_TYPE_GROUP); break;
		case E_KEY_D  : u8Group=4; return (E_KEY_TYPE_GROUP); break;

		case E_KEY_OFF:
		case E_KEY_ON:
		case E_KEY_UP:
		case E_KEY_DOWN:
		case E_KEY_SEL:
#if (MK_JIP_DEVICE_ID == 0x0801035C) || (MK_JIP_DEVICE_ID == 0x0801035E)	/* NXP RD6035 Colour ? */
		case E_KEY_1:
		case E_KEY_2:
		case E_KEY_3:
		case E_KEY_4:
#endif
			return (E_KEY_TYPE_COMMAND);
			break;
		default:
		break;
	}
	return (E_KEY_TYPE_NONE);
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
