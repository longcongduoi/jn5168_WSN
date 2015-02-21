/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         Remote Config Group MIB - Implementation
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
#include "MibRemote.h"
#include "MibRemoteDebug.h"
#include "Table.h"
#include "MibBulb.h"
/* Application device includes */
#include "MibRemoteConfigGroup.h"

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
PRIVATE tsMibRemoteConfigGroup  *psMibRemoteConfigGroup;			/* Remote Config Mib data */

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: MibRemoteConfigGroup_vInit
 *
 * DESCRIPTION:
 * Initialises data
 *
 ****************************************************************************/
PUBLIC void MibRemoteConfigGroup_vInit(thJIP_Mib          hMibRemoteConfigGroupInit,
								tsMibRemoteConfigGroup   *psMibRemoteConfigGroupInit)
{
	/* Valid data pointer ? */
	if (psMibRemoteConfigGroupInit != (tsMibRemoteConfigGroup *) NULL)
	{
		PDM_teStatus   ePdmStatus;

		/* Debug */
		DBG_vPrintf(CONFIG_DBG_MIB_REMOTE_CONFIG_GROUP, "\nMibRemoteConfigGroup_vInit() {%d}", sizeof(tsMibRemoteConfigGroup));

		/* Take copy of pointer to data */
		psMibRemoteConfigGroup = psMibRemoteConfigGroupInit;

		/* Take a copy of the MIB handle */
		psMibRemoteConfigGroup->hMib = hMibRemoteConfigGroupInit;

		/* Load RemoteStatus mib data */
		ePdmStatus = PDM_eLoadRecord(&psMibRemoteConfigGroup->sDesc,
#if defined(JENNIC_CHIP_FAMILY_JN514x)
									 "MibRemoteConfigGroup",
#else
									 (uint16)(MIB_ID_REMOTE_CONFIG_GROUP & 0xFFFF),
#endif
									 (void *) &psMibRemoteConfigGroup->sPerm,
									 sizeof(psMibRemoteConfigGroup->sPerm),
									 FALSE);
	}
}

/****************************************************************************
 *
 * NAME: MibRemoteConfigGroup_vRegister
 *
 * DESCRIPTION:
 * Registers MIB
 *
 ****************************************************************************/
PUBLIC void MibRemoteConfigGroup_vRegister(void)
{
	teJIP_Status eStatus;
	uint16 		u16Index;

	/* Debug */
	DBG_vPrintf(CONFIG_DBG_MIB_REMOTE_CONFIG_GROUP, "\nMibRemoteConfigGroup_vRegister()");

	/* Allocate default groups derived from MAC address */
	for (u16Index = 0; u16Index < VAR_VAL_REMOTE_CONFIG_GROUP_COUNT; u16Index++)
	{
		/* Group address not set in flash ? */
		if (psMibRemoteConfigGroup->sPerm.asAddr[u16Index].s6_addr32[0] == 0 &&
			psMibRemoteConfigGroup->sPerm.asAddr[u16Index].s6_addr32[1] == 0 &&
			psMibRemoteConfigGroup->sPerm.asAddr[u16Index].s6_addr32[2] == 0 &&
			psMibRemoteConfigGroup->sPerm.asAddr[u16Index].s6_addr32[3] == 0)
		{
			/* All group address ? */
			if (u16Index == 0)
			{
				/* Set global bulb group address */
				MibRemoteConfigGroup_vBuildAddr(&psMibRemoteConfigGroup->sPerm.asAddr[u16Index],
												(MAC_ExtAddr_s *) NULL,
												(uint16)(MIB_ID_BULB_CONTROL & 0xffff));
			}
			/* Other addresses */
			else
			{
				/* Set address unique to this remote */
				MibRemoteConfigGroup_vBuildAddr(&psMibRemoteConfigGroup->sPerm.asAddr[u16Index],
												(MAC_ExtAddr_s *) pvAppApiGetMacAddrLocation(),
												u16Index);
			}
		}
		/* Debug */
		DBG_vPrintf(CONFIG_DBG_MIB_REMOTE_CONFIG_GROUP, "\n\tasAddr[%d]=%x:%x:%x:%x:%x:%x:%x:%x",
			u16Index,
		 	psMibRemoteConfigGroup->sPerm.asAddr[u16Index].s6_addr16[0],
			psMibRemoteConfigGroup->sPerm.asAddr[u16Index].s6_addr16[1],
			psMibRemoteConfigGroup->sPerm.asAddr[u16Index].s6_addr16[2],
			psMibRemoteConfigGroup->sPerm.asAddr[u16Index].s6_addr16[3],
		 	psMibRemoteConfigGroup->sPerm.asAddr[u16Index].s6_addr16[4],
			psMibRemoteConfigGroup->sPerm.asAddr[u16Index].s6_addr16[5],
			psMibRemoteConfigGroup->sPerm.asAddr[u16Index].s6_addr16[6],
			psMibRemoteConfigGroup->sPerm.asAddr[u16Index].s6_addr16[7]);
	}

	/* Register MIB */
	eStatus = eJIP_RegisterMib(psMibRemoteConfigGroup->hMib);
	/* Debug */
	DBG_vPrintf(CONFIG_DBG_MIB_REMOTE_CONFIG_GROUP, "\n\teJIP_RegisterMib(RemoteConfigGroup)=%d", eStatus);

	/* Make sure permament data is saved */
	PDM_vSaveRecord(&psMibRemoteConfigGroup->sDesc);
	/* Debug */
	DBG_vPrintf(CONFIG_DBG_MIB_REMOTE_CONFIG_GROUP, "\n\tPDM_vSaveRecord(MibRemoteConfigGroup)");
}

/****************************************************************************
 *
 * NAME: MibRemoteConfigGroup_vBuildAddr
 *
 * DESCRIPTION:
 * Timer function
 *
 ****************************************************************************/
PUBLIC void MibRemoteConfigGroup_vBuildAddr(in6_addr *psAddr, MAC_ExtAddr_s *psMacAddr, uint16 u16Group)
{
	/* Zero the address */
	memset(psAddr, 0, sizeof(in6_addr));

	/* Set multicast portion of address */
	psAddr->s6_addr[0] = 0xFF;
	psAddr->s6_addr[1] = (1 << 4) | (5);

	/* Set group part of group address */
	psAddr->s6_addr16[7] = u16Group;

	/* Got a MAC address ? */
	if (psMacAddr != (MAC_ExtAddr_s *) NULL)
	{
		/* Cast MAC address to uint16 pointer for easier manipulation */
		uint16 *pu16ExtAddr = (uint16 *) psMacAddr;

		/* Set MAC address portion of address */
		psAddr->s6_addr16[6] = pu16ExtAddr[3];
		psAddr->s6_addr16[5] = pu16ExtAddr[2];
		psAddr->s6_addr16[4] = pu16ExtAddr[1];
		psAddr->s6_addr16[3] = pu16ExtAddr[0];
	}
}

/****************************************************************************
 *
 * NAME: MibRemoteConfigGroup_eSetAddr
 *
 * DESCRIPTION:
 * Handle remote set of a group address
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibRemoteConfigGroup_eSetAddr(const uint8 *pu8Val, uint8 u8Len, void *pvCbData)

{
	bool_t bReturn = FALSE;

	/* Has the correct amount of data being passed in ? */
	if (u8Len == sizeof(in6_addr))
	{
		/* Copy data */
		memcpy(pvCbData, pu8Val, u8Len);
		/* Make sure permament data is saved */
		PDM_vSaveRecord(&psMibRemoteConfigGroup->sDesc);
		/* Debug */
		DBG_vPrintf(CONFIG_DBG_MIB_REMOTE_CONFIG_GROUP, "\n\tPDM_vSaveRecord(MibRemoteConfigGroup)");
		/* Success */
		bReturn = TRUE;
	}

	return bReturn;
}

/****************************************************************************
 *
 * NAME: MibNwkSecurity_vGetKey
 *
 * DESCRIPTION:
 * Handle remote get of a key
 *
 ****************************************************************************/
PUBLIC void MibRemoteConfigGroup_vGetAddr(thJIP_Packet hPacket, void *pvCbData)

{
	eJIP_PacketAddData(hPacket, pvCbData, sizeof(in6_addr), 0);
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
