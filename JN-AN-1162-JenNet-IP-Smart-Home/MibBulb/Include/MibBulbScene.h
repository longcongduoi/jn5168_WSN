/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         BulbScene MIB Interface
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
#ifndef  MIBBULBSCENE_H_INCLUDED
#define  MIBBULBSCENE_H_INCLUDED

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
#define MIB_BULB_SCENE_SCENES					8

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
/* Bulb configuration permament data */
typedef struct
{
	/* Config MIB variable data */
   	uint8   au8SceneMode[MIB_BULB_SCENE_SCENES];
   	uint8   au8SceneLumTarget[MIB_BULB_SCENE_SCENES];
   	uint16 au16SceneId[MIB_BULB_SCENE_SCENES];

}  tsMibBulbScenePerm;

/* Bulb configuration temporary data */
typedef struct
{
	/* Config MIB variable data */
	uint16 u16AddSceneId;
	uint16 u16DelSceneId;

} tsMibBulbSceneTemp;

/* Bulb configuration mib */
typedef struct
{
	/* MIB handle */
	thJIP_Mib 	hMib;

	/* Config Mib flash variable handles */
   	tsTable sSceneId;
   	tsTable sSceneMode;
   	tsTable sSceneLumTarget;

	/* PDM record descriptor */
	PDM_tsRecordDescriptor  sDesc;

	/* Data pointers */
	tsMibBulbScenePerm 	sPerm;
	tsMibBulbSceneTemp 	sTemp;

	/* Other MIB pointers */
	void *pvMibBulbControl;

	/* Other data */
	bool_t    bSaveRecord;

} tsMibBulbScene;

/****************************************************************************/
/***        Public Function Prototypes                                     ***/
/****************************************************************************/
PUBLIC void 			 MibBulbScene_vInit(thJIP_Mib         hMibBulbSceneInit,
							   				tsMibBulbScene   *psMibBulbSceneInit,
							   				void 			 *pvMibBulbControlInit);
PUBLIC void 			 MibBulbScene_vRegister(void);
PUBLIC void 			 MibBulbScene_vSecond(void);
PUBLIC uint8 			 MibBulbScene_u8FindSceneId(uint16 u16SceneId);
PUBLIC teJIP_Status	 MibBulbScene_eSetAddSceneId(uint16 u16Val, void *pvCbData);
PUBLIC teJIP_Status     MibBulbScene_eSetDelSceneId(uint16 u16Val, void *pvCbData);


#if defined __cplusplus
}
#endif

#endif  /* MIBBULBSCENE_H_INCLUDED */
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
