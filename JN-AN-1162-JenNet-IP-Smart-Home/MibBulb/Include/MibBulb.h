/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         Bulb MIB IDsm indicies and values
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
#ifndef  MIBBULB_H_INCLUDED
#define  MIBBULB_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include <jendefs.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
/* Bulb MIBs ********************************************/
/* BulbStatus MIB */
#define MIB_ID_BULB_STATUS						0xfffffe00
#define VAR_IX_BULB_STATUS_ON_COUNT						 0
#define VAR_IX_BULB_STATUS_ON_TIME						 1
#define VAR_IX_BULB_STATUS_OFF_TIME						 2
#define VAR_IX_BULB_STATUS_CHIP_TEMP					 3
#define VAR_IX_BULB_STATUS_BUS_VOLTS					 4

/* BulbConfig MIB */
#define MIB_ID_BULB_CONFIG						0xfffffe01
#define VAR_IX_BULB_CONFIG_LUM_RATE						 0
#define VAR_IX_BULB_CONFIG_INIT_MODE					 1
#define VAR_IX_BULB_CONFIG_INIT_LUM_TARGET				 2
#define VAR_IX_BULB_CONFIG_DOWN_UP_CAD_FLAGS			 3
#define VAR_IX_BULB_CONFIG_DOWN_CADENCE					 4
#define VAR_IX_BULB_CONFIG_DOWN_CAD_TIMER				 5
#define VAR_IX_BULB_CONFIG_UP_CADENCE					 6
#define VAR_IX_BULB_CONFIG_UP_CAD_TIMER					 7

/* BulbGroup MIB */
#define MIB_ID_BULB_GROUP						0xfffffe02

/* BulbScene MIB */
#define MIB_ID_BULB_SCENE						0xfffffe03
#define VAR_IX_BULB_SCENE_ADD_SCENE_ID					 0
#define VAR_IX_BULB_SCENE_DEL_SCENE_ID					 1
#define VAR_IX_BULB_SCENE_SCENE_ID						 2
#define VAR_IX_BULB_SCENE_SCENE_MODE					 3
#define VAR_IX_BULB_SCENE_SCENE_LUM_TARGET				 4

/* BulbControl MIB */
#define MIB_ID_BULB_CONTROL						0xfffffe04
#define VAR_IX_BULB_CONTROL_MODE						 0
#define VAR_IX_BULB_CONTROL_SCENE_ID				   	 1
#define VAR_IX_BULB_CONTROL_LUM_TARGET				 	 2
#define VAR_IX_BULB_CONTROL_LUM_CURRENT					 3
#define VAR_IX_BULB_CONTROL_LUM_CHANGE					 4
#define VAR_IX_BULB_CONTROL_LUM_CADENCE					 5
#define VAR_IX_BULB_CONTROL_LUM_CADTIMER				 6

#define VAR_VAL_BULB_CONTROL_MODE_OFF			  		 0	/* Bulb off mode */
#define VAR_VAL_BULB_CONTROL_MODE_ON		  		 	 1	/* Bulb on mode */
#define VAR_VAL_BULB_CONTROL_MODE_TOGGLE  		  	 	 2	/* Bulb toggle between on and off */
#define VAR_VAL_BULB_CONTROL_MODE_TEST		  			 3	/* Bulb test mode */
#define VAR_VAL_BULB_CONTROL_MODE_DOWN		  			 4	/* Bulb fade down mode */
#define VAR_VAL_BULB_CONTROL_MODE_UP		  			 5	/* Bulb fade up mode */
#define VAR_VAL_BULB_CONTROL_MODE_DOWN_IF_ON  			 6	/* Bulb fade down if on mode */
#define VAR_VAL_BULB_CONTROL_MODE_UP_IF_ON	  			 7	/* Bulb fade up if on mode */
#define VAR_VAL_BULB_CONTROL_MODE_ON_IF_DOWN_UP			 8	/* Bulb on if fading down or up */
#define VAR_VAL_BULB_CONTROL_MODE_FAILED				 9  /* Bulb has failed */

#define VAR_VAL_BULB_CONTROL_MODE_COUNT	  		    	 8	/* Number of modes (for ROM library use) */
#define VAR_VAL_BULB_CONTROL_PATCH_MODE_COUNT	    	10	/* Number of modes (for patched RAM code) */

#if defined __cplusplus
}
#endif

#endif  /* MIBBULB_H_INCLUDED */
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
