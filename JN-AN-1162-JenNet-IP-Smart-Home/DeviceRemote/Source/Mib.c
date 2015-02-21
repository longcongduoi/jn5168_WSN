/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         MIB Variable Setting Handler
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
#include <Api.h>
#include <JIP.h>
#include <6LP.h>
#include <string.h>

#include "key.h"
#include "Mib.h"
#include "MibCommon.h"
#include "MibBulb.h"

#include "dbg.h"
#include "dbg_uart.h"
#include "AppHardwareApi.h"
#include "AppApi.h"

#include "MibRemote.h"
#include "MibRemoteConfigGroup.h"

#if (MK_JIP_DEVICE_ID == 0x0801035C) || (MK_JIP_DEVICE_ID == 0x0801035E)	/* NXP RD6035 Colour ? */
#include "MibColour.h"
#endif

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#ifdef DBG_ENABLE
#define TRACE_MIB TRUE
#else
#define TRACE_MIB FALSE
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
/***        Global Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
#ifdef MIBREMOTECONFIGGROUP_H_INCLUDED
extern tsMibRemoteConfigGroup	 sMibRemoteConfigGroup;
#endif

PRIVATE volatile bool_t bSafeToSleep;
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

PUBLIC void vSetModeMibVar(teTouchKeys eTouchKeys)
{

    teJIP_Status eStatus;
	ts6LP_SockAddr s6LP_SockAddr;
	uint8 u8NewMode = 255;
	uint32 u32ErrCode;
	uint32 u32MibId;
	uint32 u32VarIdx;

	static uint32 u32Toggle = 0;

    s6LP_SockAddr.sin6_family = E_6LP_PF_INET6;
	s6LP_SockAddr.sin6_flowinfo =0;
	s6LP_SockAddr.sin6_port = JIP_DEFAULT_PORT;
	s6LP_SockAddr.sin6_scope_id =0;
	memcpy(&s6LP_SockAddr.sin6_addr, &sMibRemoteConfigGroup.sPerm.asAddr[u8GetLastGroup()], sizeof(in6_addr));

	/* Colour control ? */
	#if (MK_JIP_DEVICE_ID == 0x0801035C) || (MK_JIP_DEVICE_ID == 0x0801035E)	/* NXP RD6035 Colour ? */
	{
		/* Use BulbControl MIB ID */
		u32MibId = MIB_ID_COLOUR_CONTROL;
		/* Use mode variable index */
		u32VarIdx = VAR_IX_COLOUR_CONTROL_MODE;
		/* Which key was pressed ? */
		switch (eTouchKeys)
		{
		case E_KEY_OFF  : u8NewMode = VAR_VAL_COLOUR_CONTROL_MODE_OFF;  			break;
		case E_KEY_ON   : u8NewMode = VAR_VAL_COLOUR_CONTROL_MODE_ON;   			break;
		case E_KEY_SEL  : u8NewMode = VAR_VAL_COLOUR_CONTROL_MODE_CYCLE;   			break;
		case E_KEY_UP   : u8NewMode = VAR_VAL_COLOUR_CONTROL_MODE_UP_RED;   	    break;
		case E_KEY_DOWN : u8NewMode = VAR_VAL_COLOUR_CONTROL_MODE_DOWN_RED; 	    break;
		case E_KEY_1    : u8NewMode = VAR_VAL_COLOUR_CONTROL_MODE_UP_GREEN;   	    break;
		case E_KEY_3    : u8NewMode = VAR_VAL_COLOUR_CONTROL_MODE_DOWN_GREEN; 	    break;
		case E_KEY_2    : u8NewMode = VAR_VAL_COLOUR_CONTROL_MODE_UP_BLUE;   	    break;
		case E_KEY_4    : u8NewMode = VAR_VAL_COLOUR_CONTROL_MODE_DOWN_BLUE; 	    break;
		case E_KEY_NONE : u8NewMode = VAR_VAL_COLOUR_CONTROL_MODE_ON_IF_UP_DOWN;	break;
		default:	                                                  			    break;
		}
	}
	/* Bulb control */
	#else
	{
		/* Use BulbControl MIB ID */
		u32MibId = MIB_ID_BULB_CONTROL;
		/* Use mode variable index */
		u32VarIdx = VAR_IX_BULB_CONTROL_MODE;
		/* Which key was pressed ? */
		switch (eTouchKeys)
		{
		case E_KEY_OFF  : u8NewMode = VAR_VAL_BULB_CONTROL_MODE_OFF;  			break;
		case E_KEY_ON   : u8NewMode = VAR_VAL_BULB_CONTROL_MODE_ON;   			break;
		case E_KEY_UP   : u8NewMode = VAR_VAL_BULB_CONTROL_MODE_UP;   			break;
		case E_KEY_DOWN : u8NewMode = VAR_VAL_BULB_CONTROL_MODE_DOWN_IF_ON; 	break;
		case E_KEY_NONE : u8NewMode = VAR_VAL_BULB_CONTROL_MODE_ON_IF_DOWN_UP;	break;

		case E_KEY_SEL  : u8NewMode = (u32Toggle ^= 0x01UL) ? VAR_VAL_BULB_CONTROL_MODE_ON : VAR_VAL_BULB_CONTROL_MODE_OFF; break;

		default:	                                                  			break;
		}
	}
	#endif

	if (u8NewMode != 255)
	{
		bSafeToSleep=FALSE;
		i6LP_ResumeStack();
		v6LP_Tick();
		eStatus = eJIP_Remote_Mib_Set(&s6LP_SockAddr, 0, u32MibId, u32VarIdx, E_JIP_VAR_TYPE_UINT8, &u8NewMode, 1);

		if (eStatus != E_JIP_OK)
		{
			u32ErrCode = u32_6LP_GetErrNo();
			DBG_vPrintf(TRACE_MIB,  "\nError: %d\ncode: %d\ninfo: %d",eStatus, (u32ErrCode & 0xff),((u32ErrCode >>8) & 0xff));
			/* Run out of buffers (setting blobs seems to leak buffers) ? */
			if ((u32ErrCode & 0xffff) == 0x0d09)
			{
				/* Debug */
				DBG_vPrintf(TRACE_MIB, "\nvAHI_SwReset()        ");
				/* Reset to get them back */
				vAHI_SwReset();
			}
		}
		else
		{
			DBG_vPrintf(TRACE_MIB,  "\nRemote Set OK addr=%x:%x:%x:%x:%x:%x:%x:%x mib=%x var=%d cmd=%d fc=%d",
					    s6LP_SockAddr.sin6_addr.s6_addr16[0],
					    s6LP_SockAddr.sin6_addr.s6_addr16[1],
					    s6LP_SockAddr.sin6_addr.s6_addr16[2],
					    s6LP_SockAddr.sin6_addr.s6_addr16[3],
					    s6LP_SockAddr.sin6_addr.s6_addr16[4],
					    s6LP_SockAddr.sin6_addr.s6_addr16[5],
					    s6LP_SockAddr.sin6_addr.s6_addr16[6],
					    s6LP_SockAddr.sin6_addr.s6_addr16[7],
					    MIB_ID_BULB_CONTROL,
					    VAR_IX_BULB_CONTROL_MODE,
					    u8NewMode,
					    MAC_psPibGetHandle(pvAppApiGetMacHandle())->u32MacFrameCounter);

			while(!bSafeToSleep)
			{
                vAHI_CpuDoze();
				v6LP_Tick();
			}
		}
	}
}

PUBLIC void vSetGroupMibVar(MAC_ExtAddr_s *psMacAddr, uint32 u32MibId, uint8 u8VarIdx, uint8 u8Group)
{
	teJIP_Status eStatus;
	ts6LP_SockAddr s6LP_SockAddr;
	uint32 u32ErrCode;
	EUI64_s  sIntAddr;
	uint8    i;

    memset(&s6LP_SockAddr, 0, sizeof(ts6LP_SockAddr));

	i6LP_CreateInterfaceIdFrom64(&sIntAddr, (EUI64_s *) psMacAddr);
	i6LP_CreateLinkLocalAddress (&s6LP_SockAddr.sin6_addr, &sIntAddr);
	/* Complete full socket address */
	s6LP_SockAddr.sin6_family = E_6LP_PF_INET6;
	s6LP_SockAddr.sin6_flowinfo =0;
	s6LP_SockAddr.sin6_port = JIP_DEFAULT_PORT;
	s6LP_SockAddr.sin6_scope_id =0;

	eStatus = eJIP_Remote_Mib_Set(&s6LP_SockAddr,0,
			                       u32MibId ,
			                       u8VarIdx,
			                       E_JIP_VAR_TYPE_BLOB,
			                       &sMibRemoteConfigGroup.sPerm.asAddr[u8Group],
			                       sizeof(in6_addr));
	if (eStatus != E_JIP_OK)
	{
		u32ErrCode = u32_6LP_GetErrNo();
		DBG_vPrintf(TRACE_MIB,  "\nError: %d\ncode: %d\ninfo: %d",eStatus, (u32ErrCode & 0xff),((u32ErrCode >>8) & 0xff));

		/* Run out of buffers (setting blobs seems to leak buffers) ? */
		if ((u32ErrCode & 0xffff) == 0x0d09)
		{
			/* Debug */
			DBG_vPrintf(TRACE_MIB, "\nvAHI_SwReset()        ");
			/* Reset to get them back */
			vAHI_SwReset();
		}
	}
	else
	{
		DBG_vPrintf(TRACE_MIB,  "\nRemote Set OK addr=%x:%x:%x:%x mib=%x var=%d blob=",
					s6LP_SockAddr.sin6_addr.s6_addr32[0],
					s6LP_SockAddr.sin6_addr.s6_addr32[1],
					s6LP_SockAddr.sin6_addr.s6_addr32[2],
					s6LP_SockAddr.sin6_addr.s6_addr32[3],
					u32MibId,
					u8VarIdx);
		for(i=0;i<8;i++)
		{
			DBG_vPrintf(TRACE_MIB, "%04x:",sMibRemoteConfigGroup.sPerm.asAddr[u8Group].s6_addr16[i]);
		}
	}

}

PUBLIC teJIP_Status eBcastGroupMibVar(uint16 u16GroupAddr, uint32 u32MibId, uint8 u8VarIdx, uint8 u8Group)
{
	teJIP_Status eStatus;
	ts6LP_SockAddr s6LP_SockAddr;
	uint32 u32ErrCode;
	uint8    i;

    memset(&s6LP_SockAddr, 0, sizeof(ts6LP_SockAddr));

	/* Initialise address */
    s6LP_SockAddr.sin6_family = E_6LP_PF_INET6;
	s6LP_SockAddr.sin6_flowinfo =0;
	s6LP_SockAddr.sin6_port = JIP_DEFAULT_PORT;
	s6LP_SockAddr.sin6_scope_id =0;

    /* Build all group address */
	MibRemoteConfigGroup_vBuildAddr(&s6LP_SockAddr.sin6_addr,
									NULL,
									u16GroupAddr);

	/* Start running stack */
	bSafeToSleep=FALSE;
	i6LP_ResumeStack();
	v6LP_Tick();
	/* Lower TTL */
	vTtlOverride(0);
	/* Lower radio power */
	eAppApiPlmeSet(PHY_PIB_ATTR_TX_POWER, 0x2C); /* Level 1 */
	/* Send command */
	eStatus = eJIP_Remote_Mib_Set(&s6LP_SockAddr,0,
			                       u32MibId ,
			                       u8VarIdx,
			                       E_JIP_VAR_TYPE_BLOB,
			                       &sMibRemoteConfigGroup.sPerm.asAddr[u8Group],
			                       sizeof(in6_addr));
	if (eStatus != E_JIP_OK)
	{
		u32ErrCode = u32_6LP_GetErrNo();
		DBG_vPrintf(TRACE_MIB,  "\nError: %d\ncode: %d\ninfo: %d",eStatus, (u32ErrCode & 0xff),((u32ErrCode >>8) & 0xff));

		/* Run out of buffers (setting blobs seems to leak buffers) ? */
		if ((u32ErrCode & 0xffff) == 0x0d09)
		{
			/* Restore TTL */
			vTtlRestore();
			/* Return to default radio power */
			eAppApiPlmeSet(PHY_PIB_ATTR_TX_POWER, PHY_PIB_TX_POWER_DEF); /* Default */
			/* Debug */
			DBG_vPrintf(TRACE_MIB, "\nvAHI_SwReset()        ");
			/* Reset to get them back */
			vAHI_SwReset();
		}
	}
	else
	{
		DBG_vPrintf(TRACE_MIB,  "\nRemote Set OK addr=%x:%x:%x:%x mib=%x var=%d blob=",
					s6LP_SockAddr.sin6_addr.s6_addr32[0],
					s6LP_SockAddr.sin6_addr.s6_addr32[1],
					s6LP_SockAddr.sin6_addr.s6_addr32[2],
					s6LP_SockAddr.sin6_addr.s6_addr32[3],
					u32MibId,
					u8VarIdx);
		for(i=0;i<8;i++)
		{
			DBG_vPrintf(TRACE_MIB, "%04x:",sMibRemoteConfigGroup.sPerm.asAddr[u8Group].s6_addr16[i]);
		}

		while(!bSafeToSleep)
		{
            vAHI_CpuDoze();
			v6LP_Tick();
		}
		/* Restore TTL */
		vTtlRestore();
		/* Return to default radio power */
		eAppApiPlmeSet(PHY_PIB_ATTR_TX_POWER, PHY_PIB_TX_POWER_DEF); /* Default */
	}

	return eStatus;
}

/****************************************************************************
 *
 * NAME: vSetNodeControlMibVar
 *
 * DESCRIPTION:
 *
 * Performs a group cast to commissioned bulbs Node Control MIB to initiate
 * a factory reset count down. This function is called several times with the
 * counts down decremented each time to increase reliability of message
 * propoagation
 *
 *
 * PARAMETERS:      Name            RW  Usage
 *                  u8CountDown     R   Number of seconds count down to reset
 *
 ****************************************************************************/

PUBLIC void vSetNodeControlMibVar(uint8 u8CountDown)
{
    teJIP_Status eStatus;
	ts6LP_SockAddr s6LP_SockAddr;
	uint32 u32ErrCode;
    uint16 u16FactoryReset;

    s6LP_SockAddr.sin6_family = E_6LP_PF_INET6;
	s6LP_SockAddr.sin6_flowinfo =0;
	s6LP_SockAddr.sin6_port = JIP_DEFAULT_PORT;
	s6LP_SockAddr.sin6_scope_id =0;
	memcpy(&s6LP_SockAddr.sin6_addr, &sMibRemoteConfigGroup.sPerm.asAddr[u8GetLastGroup()], sizeof(in6_addr));

	u16FactoryReset = (uint16)u8CountDown;

	eStatus = eJIP_Remote_Mib_Set(&s6LP_SockAddr,0,MIB_ID_NODE_CONTROL, VAR_IX_NODE_CONTROL_FACTORY_RESET , E_JIP_VAR_TYPE_UINT16, &u16FactoryReset, 1);

	if (eStatus != E_JIP_OK)
	{
		u32ErrCode = u32_6LP_GetErrNo();
		DBG_vPrintf(TRACE_MIB,  "\nError: %d\ncode: %d\ninfo: %d",eStatus, (u32ErrCode & 0xff),((u32ErrCode >>8) & 0xff));
		/* Run out of buffers (setting blobs seems to leak buffers) ? */
		if ((u32ErrCode & 0xffff) == 0x0d09)
		{
			/* Debug */
			DBG_vPrintf(TRACE_MIB, "\nvAHI_SwReset()        ");
			/* Reset to get them back */
			vAHI_SwReset();
		}
	}
	else
	{
		DBG_vPrintf(TRACE_MIB,  "\nRemote Set OK addr=%x:%x:%x:%x mib=%d var=%d cmd=%d fc=%d",
					s6LP_SockAddr.sin6_addr.s6_addr32[0],
					s6LP_SockAddr.sin6_addr.s6_addr32[1],
					s6LP_SockAddr.sin6_addr.s6_addr32[2],
					s6LP_SockAddr.sin6_addr.s6_addr32[3],
					MIB_ID_NODE_CONTROL,
					VAR_IX_NODE_CONTROL_FACTORY_RESET,
					u16FactoryReset,
					MAC_psPibGetHandle(pvAppApiGetMacHandle())->u32MacFrameCounter);
	}

}

PUBLIC void vSetMibVarUint16(MAC_ExtAddr_s *psMacAddr, uint32 u32MibId, uint8 u8VarIdx, uint16 u16Val)
{
	teJIP_Status eStatus;
	ts6LP_SockAddr s6LP_SockAddr;
	uint32 u32ErrCode;
	EUI64_s  sIntAddr;

	/* Build socket address */
    memset(&s6LP_SockAddr, 0, sizeof(ts6LP_SockAddr));
	i6LP_CreateInterfaceIdFrom64(&sIntAddr, (EUI64_s *) psMacAddr);
	i6LP_CreateLinkLocalAddress (&s6LP_SockAddr.sin6_addr, &sIntAddr);
	s6LP_SockAddr.sin6_family = E_6LP_PF_INET6;
	s6LP_SockAddr.sin6_flowinfo =0;
	s6LP_SockAddr.sin6_port = JIP_DEFAULT_PORT;
	s6LP_SockAddr.sin6_scope_id =0;

	/* Issue remote set */
	eStatus = eJIP_Remote_Mib_Set(&s6LP_SockAddr,
								   0,
			                       u32MibId ,
			                       u8VarIdx,
			                       E_JIP_VAR_TYPE_UINT16,
			                       &u16Val,
			                       sizeof(uint16));
	if (eStatus != E_JIP_OK)
	{
		DBG_vPrintf(TRACE_MIB,  "\nError: %d",eStatus);
		u32ErrCode = u32_6LP_GetErrNo();
		DBG_vPrintf(TRACE_MIB,  "\ncode: %d",(u32ErrCode & 0xff));
		DBG_vPrintf(TRACE_MIB,  "\ninfo: %d",((u32ErrCode >>8) & 0xff));
		/* Run out of buffers (setting blobs seems to leak buffers) ? */
		if ((u32ErrCode & 0xffff) == 0x0d09)
		{
			/* Debug */
			DBG_vPrintf(TRACE_MIB, "\nvAHI_SwReset()        ");
			/* Reset to get them back */
			vAHI_SwReset();
		}
	}
	else
	{
		DBG_vPrintf(TRACE_MIB,  "\nRemote Set Uint16 OK addr=%x:%x:%x:%x mib=%x var=%d val=%d",
					s6LP_SockAddr.sin6_addr.s6_addr32[0],
					s6LP_SockAddr.sin6_addr.s6_addr32[1],
					s6LP_SockAddr.sin6_addr.s6_addr32[2],
					s6LP_SockAddr.sin6_addr.s6_addr32[3],
					u32MibId,
					u8VarIdx,
					u16Val);
	}
}

PUBLIC void vSetMibVarUint8(MAC_ExtAddr_s *psMacAddr, uint32 u32MibId, uint8 u8VarIdx, uint8 u8Val)
{
	teJIP_Status eStatus;
	ts6LP_SockAddr s6LP_SockAddr;
	uint32 u32ErrCode;
	EUI64_s  sIntAddr;

	/* Build socket address */
    memset(&s6LP_SockAddr, 0, sizeof(ts6LP_SockAddr));
	i6LP_CreateInterfaceIdFrom64(&sIntAddr, (EUI64_s *) psMacAddr);
	i6LP_CreateLinkLocalAddress (&s6LP_SockAddr.sin6_addr, &sIntAddr);
	s6LP_SockAddr.sin6_family = E_6LP_PF_INET6;
	s6LP_SockAddr.sin6_flowinfo =0;
	s6LP_SockAddr.sin6_port = JIP_DEFAULT_PORT;
	s6LP_SockAddr.sin6_scope_id =0;

	/* Issue remote set */
	eStatus = eJIP_Remote_Mib_Set(&s6LP_SockAddr,
								   0,
			                       u32MibId ,
			                       u8VarIdx,
			                       E_JIP_VAR_TYPE_UINT8,
			                       &u8Val,
			                       sizeof(uint8));
	if (eStatus != E_JIP_OK)
	{
		DBG_vPrintf(TRACE_MIB,  "\nError: %d",eStatus);
		u32ErrCode = u32_6LP_GetErrNo();
		DBG_vPrintf(TRACE_MIB,  "\ncode: %d",(u32ErrCode & 0xff));
		DBG_vPrintf(TRACE_MIB,  "\ninfo: %d",((u32ErrCode >>8) & 0xff));
		/* Run out of buffers (setting blobs seems to leak buffers) ? */
		if ((u32ErrCode & 0xffff) == 0x0d09)
		{
			/* Debug */
			DBG_vPrintf(TRACE_MIB, "\nvAHI_SwReset()        ");
			/* Reset to get them back */
			vAHI_SwReset();
		}
	}
	else
	{
		DBG_vPrintf(TRACE_MIB,  "\nRemote Set Uint8 OK addr=%x:%x:%x:%x mib=%x var=%d val=%d",
					s6LP_SockAddr.sin6_addr.s6_addr32[0],
					s6LP_SockAddr.sin6_addr.s6_addr32[1],
					s6LP_SockAddr.sin6_addr.s6_addr32[2],
					s6LP_SockAddr.sin6_addr.s6_addr32[3],
					u32MibId,
					u8VarIdx,
					u8Val);
	}
}

/****************************************************************************
 *
 * NAME: vSetSafetoSleep
 *
 * DESCRIPTION:
 *
 * Access function to allow JIP call-backs to break  local v6LP_Tick loops
 * within this module and allow the execution thread to terminate via sleep
 *
 ****************************************************************************/

PUBLIC void vSetSafetoSleep(void)
{
	bSafeToSleep = TRUE;
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
