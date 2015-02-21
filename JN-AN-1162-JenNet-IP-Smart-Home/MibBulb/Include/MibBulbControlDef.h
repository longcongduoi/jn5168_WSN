/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         BulbControl MIB Definition
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
#define MIB_HEADER "MibBulbControlDef.h"

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
START_DEFINE_MIB(MIB_ID_BULB_CONTROL, BulbControlDef)
/*         ID 					   		 	 Type    Name  	   	  Disp  Flags Access 	             Cache Security */
DEFINE_VAR(VAR_IX_BULB_CONTROL_MODE		   , UINT8,  Mode,		  NULL, 0,    (READ | WRITE | TRAP), NONE, NONE)
DEFINE_VAR(VAR_IX_BULB_CONTROL_SCENE_ID	   , UINT16, SceneId,	  NULL, 0,    (READ | WRITE | TRAP), NONE, NONE)
DEFINE_VAR(VAR_IX_BULB_CONTROL_LUM_TARGET  , UINT8,  LumTarget,	  NULL, 0,    (READ | WRITE | TRAP), NONE, NONE)
DEFINE_VAR(VAR_IX_BULB_CONTROL_LUM_CURRENT , UINT8,  LumCurrent,  NULL, 0,    (READ | WRITE | TRAP), NONE, NONE)
DEFINE_VAR(VAR_IX_BULB_CONTROL_LUM_CHANGE  , INT16,  LumChange,	  NULL, 0,    (READ | WRITE | TRAP), NONE, NONE)
DEFINE_VAR(VAR_IX_BULB_CONTROL_LUM_CADENCE , UINT32, LumCadence,  NULL, 0,    (READ | WRITE | TRAP), NONE, NONE)
DEFINE_VAR(VAR_IX_BULB_CONTROL_LUM_CADTIMER, UINT16, LumCadTimer, NULL, 0,    (READ | WRITE | TRAP), NONE, NONE)
END_DEFINE_MIB(BulbControlDef)

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
