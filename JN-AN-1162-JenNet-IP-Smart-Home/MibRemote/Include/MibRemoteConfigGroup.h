/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         Remote Config Group MIB - Interface
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
#ifndef  MIBREMOTECONFIGGROUP_H_INCLUDED
#define  MIBREMOTECONFIGGROUP_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
/* SDK includes */
#include <jendefs.h>
/* JenOS includes */
#include <os.h>
#include <pdm.h>
/* Application common includes */
#include "Table.h"
#include "MibRemote.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
/* Remote configuration permament data */
typedef struct
{
	/* Config MIB variable data */
	uint8   	u8Count;
	in6_addr    asAddr[VAR_VAL_REMOTE_CONFIG_GROUP_COUNT];

}  tsMibRemoteConfigGroupPerm;

/* Remote configuration temporary data */
typedef struct
{
	/* Config MIB variable data */
	uint8   	u8Finish;

}  tsMibRemoteConfigGroupTemp;

/* Remote configuration mib */
typedef struct
{
	/* MIB handle */
	thJIP_Mib 				hMib;

	/* PDM record descriptor */
	PDM_tsRecordDescriptor  sDesc;

	/* Data pointers */
	tsMibRemoteConfigGroupPerm 	sPerm;
	tsMibRemoteConfigGroupTemp 	sTemp;

} tsMibRemoteConfigGroup;

/****************************************************************************/
/***        Public Function Prototypes                                     ***/
/****************************************************************************/
PUBLIC void 			 MibRemoteConfigGroup_vInit(thJIP_Mib          hMibRemoteConfigGroupInit,
											 tsMibRemoteConfigGroup   *psMibRemoteConfigGroupInit);
PUBLIC void 			 MibRemoteConfigGroup_vRegister(void);
PUBLIC void 			 MibRemoteConfigGroup_vBuildAddr(in6_addr *psAddr, MAC_ExtAddr_s *psMacAddr, uint16 u16Group);
PUBLIC teJIP_Status 	 MibRemoteConfigGroup_eSetAddr(const uint8 *pu8Val, uint8 u8Len, void *pvCbData);
PUBLIC void 			 MibRemoteConfigGroup_vGetAddr(thJIP_Packet hPacket, void *pvCbData);

#if defined __cplusplus
}
#endif

#endif  /* MIBREMOTECONFIGGROUP_H_INCLUDED */
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
