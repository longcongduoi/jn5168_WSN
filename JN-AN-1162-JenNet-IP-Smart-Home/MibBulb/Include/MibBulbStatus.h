/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         BulbStatus MIB Interface
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
#ifndef  MIBBULBSTATUS_H_INCLUDED
#define  MIBBULBSTATUS_H_INCLUDED

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

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/
/* Bulb status permament data */
typedef struct
{
	uint16 u16OnCount;
	int16  i16ChipTemp;
	int16  i16BusVolts;
	uint32 u32OnTime;
	uint32 u32OffTime;
} tsMibBulbStatusPerm;

/* Network status mib */
typedef struct
{
	/* MIB handle */
	thJIP_Mib 	hMib;

	/* PDM record descriptor */
	PDM_tsRecordDescriptor   sDesc;

	/* Data pointers */
	tsMibBulbStatusPerm sPerm;

	/* Other data */
	bool_t bSaveRecord;
	uint8 u8AdcSrcBusVolts;

} tsMibBulbStatus;

/****************************************************************************/
/***        Public Function Prototypes                                     ***/
/****************************************************************************/
/* Patched functions call INSTEAD of library functions */
PUBLIC void MibBulbStatusPatch_vAnalogue(tsMibBulbStatus  *psMibBulbStatus, uint8 u8Adc);

/* Library functions */
PUBLIC void MibBulbStatus_vInit(thJIP_Mib        hMibBulbSceneInit,
								tsMibBulbStatus *psMibBulbStatusInit,
								uint8   		 u8AdcSrcBusVoltsInit);
PUBLIC void MibBulbStatus_vRegister(void);
PUBLIC void MibBulbStatus_vSecond(void);
PUBLIC void MibBulbStatus_vAnalogue(uint8 u8Adc);
PUBLIC void MibBulbStatus_vOn(void);
PUBLIC void MibBulbStatus_vOff(void);

#if defined __cplusplus
}
#endif

#endif  /* MIBBULBSTATUS_H_INCLUDED */
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
