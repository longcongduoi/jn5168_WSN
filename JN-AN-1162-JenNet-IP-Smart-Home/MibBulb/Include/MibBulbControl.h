/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         BulbControl MIB Interface
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
#ifndef  MIBBULBCONTROL_H_INCLUDED
#define  MIBBULBCONTROL_H_INCLUDED

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

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define MIB_BULB_CONTROL_NWK_STATE_DOWN_FACTORY 0
#define MIB_BULB_CONTROL_NWK_STATE_UP_FACTORY   1
#define MIB_BULB_CONTROL_NWK_STATE_DOWN_RESET   2
#define MIB_BULB_CONTROL_NWK_STATE_UP_RESET     3
#define MIB_BULB_CONTROL_NWK_STATE_DOWN_RUNNING 4
#define MIB_BULB_CONTROL_NWK_STATE_UP_RUNNING   5

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
/* Bulb status permament data */
typedef struct
{
	uint8    u8NwkState;
	uint8    u8Mode;
	uint8    u8LumTarget;
	uint16  u16SceneId;
} tsMibBulbControlPerm;


/* Bulb control temporary data */
typedef struct
{
	uint8    u8LumCurrent;
	int16   i16LumChange;
	uint16  u16LumCadTimer;
	uint32  u32LumCadence;
} tsMibBulbControlTemp;

/* Network control mib */
typedef struct
{
	/* MIB handles */
	thJIP_Mib 	hMib;
	thJIP_Mib  hDeviceControlMib;

	/* PDM record descriptor */
	PDM_tsRecordDescriptor  sDesc;

	/* Data pointers */
	tsMibBulbControlPerm sPerm;
	tsMibBulbControlTemp sTemp;

	/* Other MIB pointers */
	void *pvMibBulbStatus;
	void *pvMibBulbConfig;
	void *pvMibBulbScene;

	/* Other data */
	bool_t   bSaveRecord;
	bool_t   bUp;
	bool_t   bJoined;
	bool_t	 bDriverReady;
	bool_t   bDownCadence;
	bool_t	 bUpCadence;
	uint8   u8LumCadMin;
	uint8   u8LumCadMax;
	uint8   u8LumCadFade;
	uint8   u8LumCadSwitch;
	uint8   u8LumCadTarget;
	uint8   u8LumTmpTarget;
	uint32  u32Tick;
	uint32  u32LumCadTick;

} tsMibBulbControl;

/****************************************************************************/
/***        Public Data                                     			  ***/
/****************************************************************************/
//extern tsMibBulbControl 	*psMibBulbControl;

/****************************************************************************/
/***        Public Function Prototypes                                    ***/
/****************************************************************************/
/* Patched functions in MibBulbControlPatch.c */
/* MibBulbControlPatch_vInit() call AFTER MibBulbControl_vInit() */
PUBLIC void 				MibBulbControlPatch_vInit(tsMibBulbControl *psMibBulbControlInit);
/* MibBulbControlPatch_vSecond() call BEFORE MibBulbControl_vSecond() ONLY if failed bulbs should be locked */
PUBLIC void 				MibBulbControlPatch_vSecond(void);
PUBLIC void 				MibBulbControlPatch_vStackEvent(te6LP_StackEvent eEvent);
PUBLIC teJIP_Status 		MibBulbControlPatch_eSetMode(uint8 u8Val, void *pvCbData);
PUBLIC teJIP_Status 		MibBulbControlPatch_eSetLumCadence(uint32 u32Val, void *pvCbData);
PUBLIC teJIP_Status 		MibBulbControlPatch_eSetLumCadTimer(uint16 u16Val, void *pvCbData);

/* Unpatched functions in MibBulbControl.c */
PUBLIC void 				MibBulbControl_vInit(thJIP_Mib         hMibBulbControlInit,
												 thJIP_Mib         hMibDeviceControlInit,
								 				 tsMibBulbControl *psMibBulbControlInit,
								 				 void  *psMibBulbStatusInit,
								 				 void  *psMibBulbConfigInit,
								 				 void  *psMibBulbSceneInit);
PUBLIC void 				MibBulbControl_vRegister(void);
PUBLIC void 				MibBulbControl_vDeviceControlRegister(void);
PUBLIC void 				MibBulbControl_vSecond(void);
PUBLIC void 				MibBulbControl_vTick(void);
PUBLIC void 				MibBulbControl_vTickDriverReady(void);
PUBLIC bool_t 				MibBulbControl_bTickLumCadence(void);
PUBLIC void 				MibBulbControl_vTickModeTest(void);
PUBLIC void 				MibBulbControl_vTickModeOff(void);
PUBLIC void 				MibBulbControl_vTickModeOn(void);
PUBLIC void 				MibBulbControl_vTickModeDownUp(void);
PUBLIC bool_t 				MibBulbControl_bFadeLumCurrent(uint8 u8LumTarget, uint8 u8LumRate);
/* PATCHED PUBLIC void 				MibBulbControl_vStackEvent(te6LP_StackEvent eEvent); */
PUBLIC uint8  				MibBulbControl_u8ParentLqi(void);
PUBLIC void 				MibBulbControl_vLumCadence(uint32 u32LumCadence, uint16 u16LumCadTimer);
PUBLIC void 				MibBulbControl_vLumCadenceStop(void);
PUBLIC void 				MibBulbControl_vLumCadenceStackEvent(te6LP_StackEvent eEvent);
PUBLIC bool_t 				MibBulbControl_bLumCadence(void);
PUBLIC teJIP_Status		MibBulbControl_eSetMode(uint8 u8Val, void *pvCbData);
PUBLIC teJIP_Status 		MibDeviceControl_eSetMode(uint8 u8Val, void *pvCbData);
PUBLIC teJIP_Status 		MibBulbControl_eSetSceneId(uint16 u16Val, void *pvCbData);
PUBLIC teJIP_Status 		MibDeviceControl_eSetSceneId(uint16 u16Val, void *pvCbData);
PUBLIC teJIP_Status 		MibBulbDeviceControl_eSetSceneId(uint16 u16Val, void *pvCbData, bool_t bBulbControl);
PUBLIC teJIP_Status 		MibBulbControl_eSetLumTarget(uint8 u8Val, void *pvCbData);
PUBLIC teJIP_Status 		MibBulbControl_eSetLumCurrent(uint8 u8Val, void *pvCbData);
PUBLIC teJIP_Status 		MibBulbControl_eSetLumChange(int16 i16Val, void *pvCbData);
/* PATCHED PUBLIC teJIP_Status 		MibBulbControl_eSetLumCadence(uint32 u32Val, void *pvCbData);*/
/* PATCHED PUBLIC teJIP_Status 		MibBulbControl_eSetLumCadTimer(uint16 u16Val, void *pvCbData);*/
PUBLIC teJIP_Status 		MibBulbControl_eSetModeOff(uint8 *pu8Mode);
PUBLIC teJIP_Status 		MibBulbControl_eSetModeOn(uint8 *pu8Mode);
PUBLIC teJIP_Status 		MibBulbControl_eSetModeDownUp(uint8 *pu8Mode);
PUBLIC teJIP_Status 		MibBulbControl_eSetModeToggle(uint8 *pu8Mode);
PUBLIC teJIP_Status 		MibBulbControl_eSetModeTest(uint8 *pu8Mode);
PUBLIC uint8  				MibBulbControl_u8FindSceneId(uint16 u16SceneId);

#if defined __cplusplus
}
#endif

#endif  /* MIBBULBCONTROL_H_INCLUDED */
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
