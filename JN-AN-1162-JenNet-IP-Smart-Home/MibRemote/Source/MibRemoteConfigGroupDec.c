/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         Remote Config Group MIB - Declaration
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
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
/* SDK includes */
#include <jendefs.h>
/* Stack includes */
#include <JIP.h>
#include <AccessFunctions.h>
/* Application common includes */
#include "MibRemote.h"
#include "MibRemoteConfigGroup.h"

/****************************************************************************/
/***        MIB structure                                                 ***/
/****************************************************************************/
PUBLIC tsMibRemoteConfigGroup sMibRemoteConfigGroup =
{
	/* Permament data defaults - may be overridden from flash during initialisation */
	.sPerm.u8Count = VAR_VAL_REMOTE_CONFIG_GROUP_COUNT
};

/****************************************************************************/
/***        MIB declaration                                                ***/
/****************************************************************************/
#define DECLARE_MIB
#include "MibRemoteConfigGroupDef.h"
JIP_START_DECLARE_MIB(RemoteConfigGroupDef, RemoteConfigGroup)
JIP_CALLBACK(Count,		  eSetUint8,	 vGetUint8, 	&sMibRemoteConfigGroup.sPerm.u8Count)
JIP_CALLBACK(Finish,		  eSetUint8,	 vGetUint8, 	&sMibRemoteConfigGroup.sTemp.u8Finish)
#if VAR_VAL_REMOTE_CONFIG_GROUP_COUNT > 0
JIP_CALLBACK(Addr0, MibRemoteConfigGroup_eSetAddr, MibRemoteConfigGroup_vGetAddr, &sMibRemoteConfigGroup.sPerm.asAddr[0])
#if VAR_VAL_REMOTE_CONFIG_GROUP_COUNT > 1
JIP_CALLBACK(Addr1, MibRemoteConfigGroup_eSetAddr, MibRemoteConfigGroup_vGetAddr, &sMibRemoteConfigGroup.sPerm.asAddr[1])
#if VAR_VAL_REMOTE_CONFIG_GROUP_COUNT > 2
JIP_CALLBACK(Addr2, MibRemoteConfigGroup_eSetAddr, MibRemoteConfigGroup_vGetAddr, &sMibRemoteConfigGroup.sPerm.asAddr[2])
#if VAR_VAL_REMOTE_CONFIG_GROUP_COUNT > 3
JIP_CALLBACK(Addr3, MibRemoteConfigGroup_eSetAddr, MibRemoteConfigGroup_vGetAddr, &sMibRemoteConfigGroup.sPerm.asAddr[3])
#if VAR_VAL_REMOTE_CONFIG_GROUP_COUNT > 4
JIP_CALLBACK(Addr4, MibRemoteConfigGroup_eSetAddr, MibRemoteConfigGroup_vGetAddr, &sMibRemoteConfigGroup.sPerm.asAddr[4])
#if VAR_VAL_REMOTE_CONFIG_GROUP_COUNT > 5
JIP_CALLBACK(Addr5, MibRemoteConfigGroup_eSetAddr, MibRemoteConfigGroup_vGetAddr, &sMibRemoteConfigGroup.sPerm.asAddr[5])
#if VAR_VAL_REMOTE_CONFIG_GROUP_COUNT > 6
JIP_CALLBACK(Addr6, MibRemoteConfigGroup_eSetAddr, MibRemoteConfigGroup_vGetAddr, &sMibRemoteConfigGroup.sPerm.asAddr[6])
#if VAR_VAL_REMOTE_CONFIG_GROUP_COUNT > 7
JIP_CALLBACK(Addr7, MibRemoteConfigGroup_eSetAddr, MibRemoteConfigGroup_vGetAddr, &sMibRemoteConfigGroup.sPerm.asAddr[7])
#if VAR_VAL_REMOTE_CONFIG_GROUP_COUNT > 8
#error Only a maximum of 8 remote groups can be configured by simply updating VAR_VAL_REMOTE_CONFIG_GROUP_COUNT
#endif /* >8 */
#endif /* >7 */
#endif /* >6 */
#endif /* >5 */
#endif /* >4 */
#endif /* >3 */
#endif /* >2 */
#endif /* >1 */
#endif /* >0 */
JIP_END_DECLARE_MIB(RemoteConfigGroup, hRemoteConfigGroup)

/* Public MIB handle */
PUBLIC const thJIP_Mib hMibRemoteConfigGroup = &sRemoteConfigGroupMib.sMib;

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
