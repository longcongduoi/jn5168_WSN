/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         Device Commissioning Controller
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
#include <Api.h>
#include <6LP.h>
#include <String.h>

#include "dbg.h"
#include "dbg_uart.h"
#include "Uart.h"
#include "AppHardwareApi.h"
#include "ModeCommission.h"
#include "DriverLed.h"
#include "Security.h"
#include "Mib.h"
#include "Api.h"
#include "RemoteDefault.h"
#include "MibCommon.h"
#include "MibRemote.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#ifdef DBG_ENABLE
#define TRACE_RM    TRUE
#else
#define TRACE_RM    FALSE
#endif

#define DCMSNG_DURATION_HUNDREDTHS 3000
#define DCMSNG_INTER_PACKET_DELAY   500
#define DCMSNG_RETRY_PACKET_DELAY   100

#define TWO_SECOND_COUNTDOWN          2
#define ONE_SECOND_COUNTDOWN          1

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

typedef struct
{
	uint32 u32H;
	uint32 u32L;
}tsWsMacAddr;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Global Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE uint8 u8StoredTTL;
PRIVATE uint8 u8StoredProfile;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/


PUBLIC void vCommissionInit(tsDevice *psDevice ,tsAuthorise *psAuthorise)
{
   UART_vChar('C');
   DBG_vPrintf(TRACE_RM, "\nUser Initiated Commissioning");
   vApi_SetStackMode(NONE_GATEWAY_MODE | CMSN_MODE);
   DBG_vPrintf(TRACE_RM,"\nvApi_SetStackMode(NONE_GATEWAY_MODE | CMSN_MODE)");
   psAuthorise->eAuthState = E_STATE_CMSNG_IDLE;          // Node commissioning -moved on by stack events            // User commissioning -moved on by timeout or user request
   DBG_vPrintf(TRACE_RM, "\nCMSNG_IDLE 3");
   /* Cancel commissioning timer */
   psDevice->sTimers.u8CmsngTimeout = 0;
   psDevice->sTimers.u16CmsngDuration = CMSNG_DURATION_S;
}

PUBLIC void vCommissionMode(tsDevice *psDevice ,tsAuthorise *psAuthorise, teSysState *peSysState)
{
	uint8 i;
	tsWsMacAddr sWsMacAddr;

	UART_vChar('c');
	UART_vChar('0'+(psAuthorise->eAuthState));

	if(psAuthorise->eAuthState == E_STATE_CMSNG_START)	// Node to commission (after correct stack event occurs just authorised a node)
	{
		DBG_vPrintf(TRACE_RM, "\nCommissioning Node %x",psAuthorise->sAddr.u32L);
		void *p;

		/* swap MAC words around to preserve maintain word order */

		sWsMacAddr.u32H = psAuthorise->sAddr.u32H;
		sWsMacAddr.u32L = psAuthorise->sAddr.u32L;

		Security_vBuildCommissioningKey((uint8 *)&sWsMacAddr,(uint8 *)&psAuthorise->sSecKey);

		p=&psAuthorise->sSecKey;

		for(i=0;i<16;i++)
		{
			DBG_vPrintf(TRACE_RM, "%02x", *(uint8 *)p++);
		}

		eApi_CommissionNode(&psAuthorise->sAddr,&psAuthorise->sSecKey);
		psAuthorise->eAuthState = E_STATE_CMSNG_INPRG;
	    DBG_vPrintf(TRACE_RM, "\nCMSNG_INPRG");
		psDevice->sTimers.u8CmsngTimeout = AUTH_TIMEOUT_S;
	}

	/* if node has joined then we can safely attempt to uni-cast */
	/* the remote set of the add group MIB   */
	if(psAuthorise->eAuthState == E_STATE_CMSNG_SENDGROUP_START)
	{
		/* Commissioning a bulb ? */
		if (psAuthorise->u16DeviceType == 0x00E1)
		{
			/* Commissioning all group ? */
			if (psAuthorise->u8Group == 0)
			{
				/* Put bulb into all group */
				vSetGroupMibVar(&psAuthorise->sAddr, MIB_ID_GROUPS, VAR_IX_GROUPS_ADD_GROUP, 0);
				psAuthorise->eAuthState =  E_STATE_CMSNG_SENDGROUP_INPRG;
				psDevice->sTimers.u8SetVarTimeout = SETVAR_TIMEOUT_TICKS;
				DBG_vPrintf(TRACE_RM, "\nCMSNG_SENDGROUP_INPRG 0");
			}
			/* Commissioning selected group (doesn't matter if it is the all group again) ? */
			else if (psAuthorise->u8Group == 1)
			{
				/* Put bulb into selected group */
				vSetGroupMibVar(&psAuthorise->sAddr, MIB_ID_GROUPS, VAR_IX_GROUPS_ADD_GROUP, u8GetLastGroup());
				psAuthorise->eAuthState =  E_STATE_CMSNG_SENDGROUP_INPRG;
				psDevice->sTimers.u8SetVarTimeout = SETVAR_TIMEOUT_TICKS;
				DBG_vPrintf(TRACE_RM, "\nCMSNG_SENDGROUP_INPRG %d", u8GetLastGroup());
			}
		}
		/* Commissioning a remote ? */
		else if (psAuthorise->u16DeviceType == 0x00C5)
		{
			/* Put bulb into all group */
			vSetGroupMibVar(&psAuthorise->sAddr, MIB_ID_REMOTE_CONFIG_GROUP, VAR_IX_REMOTE_CONFIG_GROUP_ADDR_0+psAuthorise->u8Group, psAuthorise->u8Group);
			psAuthorise->eAuthState =  E_STATE_CMSNG_SENDGROUP_INPRG;
			psDevice->sTimers.u8SetVarTimeout = SETVAR_TIMEOUT_TICKS;
			DBG_vPrintf(TRACE_RM, "\nCMSNG_SENDGROUP_INPRG %d", psAuthorise->u8Group);
		}
	}
	/* if node has been commissioned then we can safely attempt to uni-cast */
	/* the finish command to the node control MIB   */
	else if(psAuthorise->eAuthState == E_STATE_CMSNG_FINISH_START)
	{
		/* Commissioning a bulb ? */
		if (psAuthorise->u16DeviceType == 0x00E1)
		{
			/* Cancel factory reset */
			vSetMibVarUint16(&psAuthorise->sAddr, MIB_ID_NODE_CONTROL, VAR_IX_NODE_CONTROL_FACTORY_RESET, 0);
			psAuthorise->eAuthState =  E_STATE_CMSNG_FINISH_INPRG;
			psDevice->sTimers.u8SetVarTimeout = SETVAR_TIMEOUT_TICKS;
			DBG_vPrintf(TRACE_RM, "\nCMSNG_FINISH_INPRG");
		}
		/* Commissioning a remote ? */
		else if (psAuthorise->u16DeviceType == 0x00C5)
		{
			/* Finish commissioning */
			vSetMibVarUint8(&psAuthorise->sAddr, MIB_ID_REMOTE_CONFIG_GROUP, VAR_IX_REMOTE_CONFIG_GROUP_FINISH, 1);
			psAuthorise->eAuthState =  E_STATE_CMSNG_FINISH_INPRG;
			psDevice->sTimers.u8SetVarTimeout = SETVAR_TIMEOUT_TICKS;
			DBG_vPrintf(TRACE_RM, "\nCMSNG_FINISH_INPRG");
		}
	}

	/* If we've timed out move the system state back to controlling    */
	/* this will allow the the remote to sleep during normal operation */

	if (psDevice->sTimers.u16CmsngDuration == 0)
	{
		DBG_vPrintf(TRACE_RM,"\nCommissioning Timeout");
		*peSysState = E_STATE_CONTROLLING;
		vSetLedState(E_LED_STATE_OFF);
		psAuthorise->eAuthState = E_STATE_CMSNG_IDLE;
		/* Leave commissioning mode */
		vApi_SetStackMode(NONE_GATEWAY_MODE);
		DBG_vPrintf(TRACE_RM,"\nvApi_SetStackMode(NONE_GATEWAY_MODE)");
		/* Invalidate key 1 */
		vSecurityInvalidateKey(1);
	}
}


PUBLIC void vDecommissionMode(teSysState *peSysState,teDecommssionEvent eDecommissionEvent)
{
	static uint32 u32DeCmsngTimer = 0;

    if (*peSysState == E_STATE_DECOMMISSIONING)
    {
		switch (eDecommissionEvent)
		{
			case E_EVENT_DECMSNG_START:
			    vSetLedState(E_LED_STATE_DECOMMISSIONING);
				u32DeCmsngTimer = DCMSNG_DURATION_HUNDREDTHS;
#ifdef REMOTE_DEFAULT_DECOMMISION_TTL_ZERO
				/* Override ttl */
				vTtlOverride(0);
#endif
				break;

			/* Broadcast de-commission count-downs (factory reset in N seconds) */
			case E_EVENT_DECMSNG_TICK:
				if (u32DeCmsngTimer >0)
				{
					if ( (u32DeCmsngTimer % DCMSNG_INTER_PACKET_DELAY) == 0 )
					{
						vSetNodeControlMibVar(TWO_SECOND_COUNTDOWN);
					}

					if ( ( (u32DeCmsngTimer+DCMSNG_RETRY_PACKET_DELAY ) % DCMSNG_INTER_PACKET_DELAY ) == 0 )
					{
						vSetNodeControlMibVar(ONE_SECOND_COUNTDOWN);
					}
					u32DeCmsngTimer--;
					break;
				}

			case E_EVENT_DECMSNG_FINISH:
				*peSysState = E_STATE_CONTROLLING;
				vSetLedState(E_LED_STATE_OFF);
#ifdef REMOTE_DEFAULT_DECOMMISION_TTL_ZERO
				vTtlRestore();
#endif
			default:
				break;
		}
    }
}

PUBLIC void vTtlOverride(uint8 u8MaxBcastTtl)
{
	tsNwkProfile sMyProfile;

	/* check the current stack run profile and force TTL to specified value */
	u8StoredProfile = u8GetCurRunProfile();
	vJnc_GetNwkProfile(&sMyProfile);
	u8StoredTTL = sMyProfile.u8MaxBcastTTL;
	sMyProfile.u8MaxBcastTTL = u8MaxBcastTtl;
	(void)bJnc_SetRunProfile(PROFILE_USER,&sMyProfile);
}

PUBLIC void vTtlRestore(void)
{
	tsNwkProfile sMyProfile;

	/* Was running user profile ?*/
	if (u8StoredProfile == PROFILE_USER)
	{
	    /* Assumes sMyProfile was not retained earlier, so we have to fetch it again */
	    vJnc_GetNwkProfile(&sMyProfile);
	    /* Set stored TTL */
	    sMyProfile.u8MaxBcastTTL = u8StoredTTL;
	}
	/* Restore previous profile */
	(void)bJnc_SetRunProfile(u8StoredProfile, &sMyProfile);
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/



/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
