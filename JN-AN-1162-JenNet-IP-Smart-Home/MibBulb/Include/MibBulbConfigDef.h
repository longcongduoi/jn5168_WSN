/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         BulbConfig MIB Definition
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
#undef MIB_HEADER
#define MIB_HEADER "MibBulbConfigDef.h"

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
/* Stack includes */
#include <jip_define_mib.h>
/* Application common includes */
#include "MibBulb.h"

/****************************************************************************/
/***        MIB definition                                                ***/
/****************************************************************************/
START_DEFINE_MIB(MIB_ID_BULB_CONFIG, BulbConfigDef)
/*         ID 					   		 		 Type    Name  	   		 Disp  Flags Access 	             Cache Security */
DEFINE_VAR(VAR_IX_BULB_CONFIG_LUM_RATE		   , UINT8,  LumRate,		 NULL, 0,    (READ | WRITE | TRAP),  NONE, NONE)
DEFINE_VAR(VAR_IX_BULB_CONFIG_INIT_MODE		   , UINT8,  InitMode,		 NULL, 0,    (READ | WRITE | TRAP),  NONE, NONE)
DEFINE_VAR(VAR_IX_BULB_CONFIG_INIT_LUM_TARGET  , UINT8,  InitLumTarget,	 NULL, 0,    (READ | WRITE | TRAP),  NONE, NONE)
DEFINE_VAR(VAR_IX_BULB_CONFIG_DOWN_UP_CAD_FLAGS, UINT8,  DownUpCadFlags, NULL, 0,    (READ | WRITE | TRAP),  NONE, NONE)
DEFINE_VAR(VAR_IX_BULB_CONFIG_DOWN_CADENCE	   , UINT32, DownCadence,	 NULL, 0,    (READ | WRITE | TRAP),  NONE, NONE)
DEFINE_VAR(VAR_IX_BULB_CONFIG_DOWN_CAD_TIMER   , UINT16, DownCadTimer,	 NULL, 0,    (READ | WRITE | TRAP),  NONE, NONE)
DEFINE_VAR(VAR_IX_BULB_CONFIG_UP_CADENCE	   , UINT32, UpCadence,		 NULL, 0,    (READ | WRITE | TRAP),  NONE, NONE)
DEFINE_VAR(VAR_IX_BULB_CONFIG_UP_CAD_TIMER	   , UINT16, UpCadTimer,	 NULL, 0,    (READ | WRITE | TRAP),  NONE, NONE)
END_DEFINE_MIB(BulbConfigDef)

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
