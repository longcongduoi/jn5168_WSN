/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         BulbConfig MIB Declaration
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
#include "BulbDefault.h"
#include "MibBulb.h"
#include "MibBulbConfig.h"

/****************************************************************************/
/***        MIB structure                                                 ***/
/****************************************************************************/
PUBLIC tsMibBulbConfig sMibBulbConfig =
{
	/* Permament data defaults - may be overridden from flash during initialisation */
	.sPerm.u8LumRate   		= 2,   									 /* Full range in 1.28s */
	.sPerm.u8InitMode  		= 1,									 /* On */
	.sPerm.u8InitLumTarget 	= 255,									 /* Max */
	.sPerm.u8DownUpCadFlags = BULB_DEFAULT_CONFIG_DOWN_UP_CAD_FLAGS, /* Up after reset, Down after reset */
	.sPerm.u32UpCadence 	= 0x1000ff00,  							 /* Double switch between 100% and 0% */
	.sPerm.u16UpCadTimer 	= 0x40, 						 		 /* 160ms */
	.sPerm.u32DownCadence 	= 0x0002ff50,  					 		 /* Fade 100% and 30% */
	.sPerm.u16DownCadTimer 	= 30000 						 		 /* 5 mins */
};

/****************************************************************************/
/***        MIB declaration                                                ***/
/****************************************************************************/
/* Registering MIB ? */
#if MK_REG_MIB_BULB_CONFIG

#define DECLARE_MIB
#include "MibBulbConfigDef.h"

JIP_START_DECLARE_MIB(BulbConfigDef, BulbConfig)
JIP_CALLBACK(LumRate,		  MibBulbConfig_eSetUint8,	 vGetUint8, 	&sMibBulbConfig.sPerm.u8LumRate)
JIP_CALLBACK(InitMode,		  MibBulbConfig_eSetUint8,	 vGetUint8,  	&sMibBulbConfig.sPerm.u8InitMode)
JIP_CALLBACK(InitLumTarget,  MibBulbConfig_eSetUint8,	 vGetUint8,  	&sMibBulbConfig.sPerm.u8InitLumTarget)
JIP_CALLBACK(DownUpCadFlags, MibBulbConfig_eSetUint8,	 vGetUint8,  	&sMibBulbConfig.sPerm.u8DownUpCadFlags)
JIP_CALLBACK(DownCadence,	  MibBulbConfig_eSetUint32, vGetUint32, 	&sMibBulbConfig.sPerm.u32DownCadence)
JIP_CALLBACK(DownCadTimer,	  MibBulbConfig_eSetUint16, vGetUint16,		&sMibBulbConfig.sPerm.u16DownCadTimer)
JIP_CALLBACK(UpCadence,	  MibBulbConfig_eSetUint32, vGetUint32, 	&sMibBulbConfig.sPerm.u32UpCadence)
JIP_CALLBACK(UpCadTimer,	  MibBulbConfig_eSetUint16, vGetUint16, 	&sMibBulbConfig.sPerm.u16UpCadTimer)
JIP_END_DECLARE_MIB(BulbConfig, hBulbConfig)

/* Public MIB handle */
PUBLIC const thJIP_Mib hMibBulbConfig = &sBulbConfigMib.sMib;

#else

/* Public MIB handle */
PUBLIC const thJIP_Mib hMibBulbConfig = NULL;

#endif
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
