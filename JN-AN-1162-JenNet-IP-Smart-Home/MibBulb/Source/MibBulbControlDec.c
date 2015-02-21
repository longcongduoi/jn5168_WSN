/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         BulbControl MIB Declaration
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
#include "MibBulb.h"
#include "MibBulbControl.h"

/****************************************************************************/
/***        MIB structure                                                 ***/
/****************************************************************************/
PUBLIC tsMibBulbControl sMibBulbControl =
{
	/* Permament data defaults - may be overridden from flash during initialisation */
	.sPerm.u8Mode       = VAR_VAL_BULB_CONTROL_MODE_ON,
	.sPerm.u8LumTarget  = 0xff
};

/****************************************************************************/
/***        MIB declaration                                               ***/
/****************************************************************************/
/* Registering MIB ? */
#if MK_REG_MIB_BULB_CONTROL

#define DECLARE_MIB
#include "MibBulbControlDef.h"

JIP_START_DECLARE_MIB(BulbControlDef, BulbControl)
JIP_CALLBACK(Mode,		  	MibBulbControlPatch_eSetMode,			vGetUint8, 	&sMibBulbControl.sPerm.u8Mode)
JIP_CALLBACK(SceneId,	  	MibBulbControl_eSetSceneId,	  			vGetUint16, &sMibBulbControl.sPerm.u16SceneId)
JIP_CALLBACK(LumTarget,	MibBulbControl_eSetLumTarget,			vGetUint8,  &sMibBulbControl.sPerm.u8LumTarget)
JIP_CALLBACK(LumCurrent,  	MibBulbControl_eSetLumCurrent, 			vGetUint8,  &sMibBulbControl.sTemp.u8LumCurrent)
JIP_CALLBACK(LumChange,	MibBulbControl_eSetLumChange,			vGetInt16,  &sMibBulbControl.sTemp.i16LumChange)
JIP_CALLBACK(LumCadence,  	MibBulbControlPatch_eSetLumCadence, 	vGetUint32,	&sMibBulbControl.sTemp.u32LumCadence)
JIP_CALLBACK(LumCadTimer, 	MibBulbControlPatch_eSetLumCadTimer,	vGetUint16, &sMibBulbControl.sTemp.u16LumCadTimer)
JIP_END_DECLARE_MIB(BulbControl, hBulbControl)

#include "MibDeviceControlDef.h"
JIP_START_DECLARE_MIB(DeviceControlDef, DeviceControl)
JIP_CALLBACK(Mode,		  	MibDeviceControl_eSetMode,		vGetUint8, 	&sMibBulbControl.sPerm.u8Mode)
JIP_CALLBACK(SceneId,	  	MibDeviceControl_eSetSceneId,  	vGetUint16, &sMibBulbControl.sPerm.u16SceneId)
JIP_END_DECLARE_MIB(DeviceControl, hDeviceControl)

/* Public MIB handle */
PUBLIC const thJIP_Mib hMibBulbControl   = &sBulbControlMib.sMib;
PUBLIC const thJIP_Mib hMibDeviceControl = &sDeviceControlMib.sMib;

#else

/* Public MIB handle */
PUBLIC const thJIP_Mib hMibBulbControl   = NULL;
PUBLIC const thJIP_Mib hMibDeviceControl = NULL;

#endif

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
