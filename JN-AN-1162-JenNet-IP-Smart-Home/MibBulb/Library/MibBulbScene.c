/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         BulbScene MIB Implementation
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
#include "MibBulbDebug.h"
#include "MibBulb.h"
#include "Table.h"
/* Application device includes */
#include "MibBulb.h"
#include "MibBulbControl.h"
#include "MibBulbScene.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define PS_MIB_BULB_CONTROL ((tsMibBulbControl *) psMibBulbScene->pvMibBulbControl)

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
PRIVATE tsMibBulbScene  *psMibBulbScene;			/* Bulb Config Mib data */

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: MibBulbScene_vInit
 *
 * DESCRIPTION:
 * Initialises data
 *
 ****************************************************************************/
PUBLIC void MibBulbScene_vInit(thJIP_Mib         hMibBulbSceneInit,
							   tsMibBulbScene   *psMibBulbSceneInit,
							   void				*pvMibBulbControlInit)
{
	/* Debug */
	DBG_vPrintf(CONFIG_DBG_MIB_BULB_SCENE,  "\nMibBulbScene_vInit() {%d}", sizeof(tsMibBulbScene));

	/* Valid data pointer ? */
	if (psMibBulbSceneInit != (tsMibBulbScene *) NULL)
	{
		PDM_teStatus   ePdmStatus;

		/* Take copy of pointer to data */
		psMibBulbScene = psMibBulbSceneInit;

		/* Take a copy of the MIB handle */
		psMibBulbScene->hMib = hMibBulbSceneInit;

		/* Take a copy of the bulb control MIB pointer */
		psMibBulbScene->pvMibBulbControl = pvMibBulbControlInit;

		/* Load BulbStatus mib data */
		ePdmStatus = PDM_eLoadRecord(&psMibBulbScene->sDesc,
#if defined(JENNIC_CHIP_FAMILY_JN514x)
									 "MibBulbScene",
#else
									 (uint16)(MIB_ID_BULB_SCENE & 0xFFFF),
#endif
									 (void *) &psMibBulbScene->sPerm,
									 sizeof(psMibBulbScene->sPerm),
									 FALSE);
	}
}

/****************************************************************************
 *
 * NAME: MibBulbScene_vRegister
 *
 * DESCRIPTION:
 * Registers MIB
 *
 ****************************************************************************/
PUBLIC void MibBulbScene_vRegister(void)
{
	teJIP_Status eStatus;

	/* Register MIB */
	eStatus = eJIP_RegisterMib(psMibBulbScene->hMib);
	/* Debug */
	DBG_vPrintf(CONFIG_DBG_MIB_BULB_SCENE, "\nMibBulbScene_vRegister()");
	DBG_vPrintf(CONFIG_DBG_MIB_BULB_SCENE, "\n\teJIP_RegisterMib(BulbScene)=%d", eStatus);

	/* Configure table structures */
	psMibBulbScene->sSceneId.pvData				= (void *) psMibBulbScene->sPerm.au16SceneId;
	psMibBulbScene->sSceneId.u32Size			= sizeof(uint16);
	psMibBulbScene->sSceneId.u16Entries			= MIB_BULB_SCENE_SCENES;

	psMibBulbScene->sSceneMode.pvData			= (void *) psMibBulbScene->sPerm.au8SceneMode;
	psMibBulbScene->sSceneMode.u32Size			= sizeof(uint8);
	psMibBulbScene->sSceneMode.u16Entries		= MIB_BULB_SCENE_SCENES;

	psMibBulbScene->sSceneLumTarget.pvData		= (void *) psMibBulbScene->sPerm.au8SceneLumTarget;
	psMibBulbScene->sSceneLumTarget.u32Size		= sizeof(uint8);
	psMibBulbScene->sSceneLumTarget.u16Entries	= MIB_BULB_SCENE_SCENES;
}

/****************************************************************************
 *
 * NAME: MibBulbScene_vSecond
 *
 * DESCRIPTION:
 * Timer function
 *
 ****************************************************************************/
PUBLIC void MibBulbScene_vSecond(void)
{
	/* Need to save record ? */
	if (psMibBulbScene->bSaveRecord)
	{
		/* Clear flag */
		psMibBulbScene->bSaveRecord = FALSE;
		/* Make sure permament data is saved */
		PDM_vSaveRecord(&psMibBulbScene->sDesc);
		/* Debug */
		DBG_vPrintf(CONFIG_DBG_MIB_BULB_SCENE, "\nMibBulbScene_vSecond()");
		DBG_vPrintf(CONFIG_DBG_MIB_BULB_SCENE, "\n\tPDM_vSaveRecord(MibBulbScene)");
	}
}

/****************************************************************************
 *
 * NAME: MibBulbScene_u8FindSceneId
 *
 * DESCRIPTION:
 * Finds a scene id in the table
 *
 ****************************************************************************/
PUBLIC uint8 MibBulbScene_u8FindSceneId(uint16 u16SceneId)
{
	uint8 u8Scene;
	uint8 u8Found = 0xff;

	/* Look for scene */
	for (u8Scene = 0; u8Scene < MIB_BULB_SCENE_SCENES && u8Found == 0xff; u8Scene++)
	{
		/* Is this the scene we're looking for ? */
		if (psMibBulbScene->sPerm.au16SceneId[u8Scene] == u16SceneId)
		{
			/* Note its index */
			u8Found = u8Scene;
		}
	}

	return u8Found;
}

/****************************************************************************
 *
 * NAME: MibBulbScene_eSetAddSceneId
 *
 * DESCRIPTION:
 * AddSceneId set data callback
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbScene_eSetAddSceneId(uint16 u16Val, void *pvCbData)
{
	teJIP_Status eReturn = E_JIP_ERROR_FAILED;
	uint8   	 u8Scene;

	/* Valid scene and Bulb control Mib data is available ? */
	if (u16Val != 0 && PS_MIB_BULB_CONTROL != NULL)
	{
		/* Look for this scene */
		u8Scene = MibBulbScene_u8FindSceneId(u16Val);
		/* Didn't find scene ? */
		if (u8Scene >= MIB_BULB_SCENE_SCENES)
		{
			/* Look for unused scene */
			u8Scene = MibBulbScene_u8FindSceneId(0);
		}
		/* Do we have a scene index to add the scene ? */
		if (u8Scene < MIB_BULB_SCENE_SCENES)
		{
			/* Is the bulb currently on or off ? */
			if (VAR_VAL_BULB_CONTROL_MODE_ON  == PS_MIB_BULB_CONTROL->sPerm.u8Mode ||
				VAR_VAL_BULB_CONTROL_MODE_OFF == PS_MIB_BULB_CONTROL->sPerm.u8Mode)
			{
				/* Store the scene */
				psMibBulbScene->sPerm.au16SceneId[u8Scene]       = u16Val;
				psMibBulbScene->sPerm.au8SceneMode[u8Scene] 	 = PS_MIB_BULB_CONTROL->sPerm.u8Mode;
				psMibBulbScene->sPerm.au8SceneLumTarget[u8Scene] = PS_MIB_BULB_CONTROL->sPerm.u8LumTarget;
				/* Success */
				eReturn = eSetUint16(u16Val, pvCbData);
			}
		}
	}

	/* Success ? */
	if (eReturn == E_JIP_OK)
	{
		/* Increment table hashes */
		psMibBulbScene->sSceneId.u16Hash++;
		psMibBulbScene->sSceneMode.u16Hash++;
		psMibBulbScene->sSceneLumTarget.u16Hash++;
		/* Make sure permament data is saved */
		psMibBulbScene->bSaveRecord = TRUE;
		/* Notify traps */
		vJIP_NotifyChanged(psMibBulbScene->hMib, VAR_IX_BULB_SCENE_SCENE_ID);
		vJIP_NotifyChanged(psMibBulbScene->hMib, VAR_IX_BULB_SCENE_SCENE_MODE);
		vJIP_NotifyChanged(psMibBulbScene->hMib, VAR_IX_BULB_SCENE_SCENE_LUM_TARGET);
	}

	return eReturn;
}

/****************************************************************************
 *
 * NAME: MibBulbScene_eSetDelSceneId
 *
 * DESCRIPTION:
 * DelSceneId set data callback
 *
 ****************************************************************************/
PUBLIC teJIP_Status MibBulbScene_eSetDelSceneId(uint16 u16Val, void *pvCbData)
{
	teJIP_Status eReturn = E_JIP_ERROR_FAILED;
	uint8   	 u8Scene;

	/* Valid scene and Bulb control Mib data is available ? */
	if (u16Val != 0)
	{
		/* Look for this scene */
		u8Scene = MibBulbScene_u8FindSceneId(u16Val);
		/* Do we have a scene index to delete the scene ? */
		if (u8Scene < MIB_BULB_SCENE_SCENES)
		{
			/* Delete the scene */
			psMibBulbScene->sPerm.au16SceneId[u8Scene]        = 0;
			psMibBulbScene->sPerm.au8SceneMode[u8Scene] 	  = 0;
			psMibBulbScene->sPerm.au8SceneLumTarget[u8Scene]  = 0;
			/* Success */
			eReturn = eSetUint16(u16Val, pvCbData);
			/* Increment table hashes */
			psMibBulbScene->sSceneId.u16Hash++;
			psMibBulbScene->sSceneMode.u16Hash++;
			psMibBulbScene->sSceneLumTarget.u16Hash++;
			/* Make sure permament data is saved */
			psMibBulbScene->bSaveRecord = TRUE;
			/* Notify traps */
			vJIP_NotifyChanged(psMibBulbScene->hMib, VAR_IX_BULB_SCENE_SCENE_ID);
			vJIP_NotifyChanged(psMibBulbScene->hMib, VAR_IX_BULB_SCENE_SCENE_MODE);
			vJIP_NotifyChanged(psMibBulbScene->hMib, VAR_IX_BULB_SCENE_SCENE_LUM_TARGET);
		}
	}

	return eReturn;
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
