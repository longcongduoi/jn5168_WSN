/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         BulbConfig MIB Interface
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
#ifndef  MIBBULBCONFIG_H_INCLUDED
#define  MIBBULBCONFIG_H_INCLUDED

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
#define MIB_BULB_CONFIG_LUM_RATE_DEFAULT		  2
#define MIB_BULB_CONFIG_INIT_MODE_DEFAULT		  1 /* On */
#define MIB_BULB_CONFIG_INIT_LUM_TARGET_DEFAULT	255	/* Max */
#define MIB_BULB_CONFIG_CHR_RATE_DEFAULT	  	512

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
/* Bulb configuration permament data */
typedef struct
{
	/* Config MIB variable data */
	uint8   u8LumRate;
	uint8   u8InitMode;
	uint8   u8InitLumTarget;
	uint8   u8DownUpCadFlags;
	uint16 u16DownCadTimer;
	uint16 u16UpCadTimer;
	uint32 u32DownCadence;
	uint32 u32UpCadence;
}  tsMibBulbConfigPerm;

/* Bulb configuration mib */
typedef struct
{
	/* MIB handle */
	thJIP_Mib 				hMib;

	/* PDM record descriptor */
	PDM_tsRecordDescriptor  sDesc;

	/* Data pointers */
	tsMibBulbConfigPerm 	sPerm;

	/* Other data */
	bool_t    				bSaveRecord;

} tsMibBulbConfig;

/****************************************************************************/
/***        Public Function Prototypes                                     ***/
/****************************************************************************/
PUBLIC void 			 MibBulbConfig_vInit(thJIP_Mib          hMibBulbConfigInit,
											 tsMibBulbConfig   *psMibBulbConfigInit);
PUBLIC void 			 MibBulbConfig_vRegister(void);
PUBLIC void 			 MibBulbConfig_vSecond(void);
PUBLIC teJIP_Status 	 MibBulbConfig_eSetUint8(  uint8   u8Val, void *pvCbData);
PUBLIC teJIP_Status 	 MibBulbConfig_eSetUint16( uint16 u16Val, void *pvCbData);
PUBLIC teJIP_Status 	 MibBulbConfig_eSetUint32( uint32 u32Val, void *pvCbData);
PUBLIC teJIP_Status     MibBulbConfig_eSetLumRate(uint8   u8Val, void *pvCbData);

#if defined __cplusplus
}
#endif

#endif  /* MIBBULBCONFIG_H_INCLUDED */
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
