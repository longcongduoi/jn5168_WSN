/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         Key Press Handler
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
#ifndef KEY_OPS_H
#define KEY_OPS_H

#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/******************************************************************************
**  INCLUDE FILES
*******************************************************************************/
#include "ModeCommission.h"
/******************************************************************************
**  MACROS
*******************************************************************************/

/******************************************************************************
**  CONSTANTS
*******************************************************************************/

/******************************************************************************
**  TYPEDEFS
*******************************************************************************/
/* RD6035 / DR1159 5V1 key mapping  */
#if MK_KEYMAP ==2
typedef enum
{
	    E_KEY_PROG,        /* DIO2  Program         */
	    E_KEY_D,           /* DIO3  Group D         */
	    E_KEY_ON,          /* DI04  Lamp On         */
	    E_KEY_OFF,         /* DIO5  Lamp Off        */
	    E_KEY_4,           /* DIO8  Scene 4 UNUSED  */
	    E_KEY_2,           /* DIO9  Scene 2 UNUSED  */
	    E_KEY_C,           /* DIO10 Group C         */
	    E_KEY_B,           /* DIO11 Group B         */
	    E_KEY_1,           /* DIO12 Scene 1 UNUSED  */
	    E_KEY_3,           /* DIO13 Scene 3 UNUSED  */
	    E_KEY_DOWN,        /* DIO14 Lamp Dimmer     */
	    E_KEY_UP,          /* DIO15 Lamp Brighter   */
	    E_KEY_A,           /* DIO16 Group A         */
	    E_KEY_ALL,         /* DIO17 Group ALL       */
	    E_KEY_DSCVR,       /* DIO18 Discover UNUSED */
	    E_KEY_SEL,         /* DIO19 Select   UNUSED */
	    E_KEY_PWR,         /* DIO0 WAKE BUTTON      */
        E_KEY_NONE=254,
} teTouchKeys;

/* RD6031 / DR1159 4V2 key mapping ? */
#elif MK_KEYMAP == 1
typedef enum
{
        E_KEY_D,        	/* DIO10 */
        E_KEY_ON,       	/* DIO11 */
        E_KEY_C,        	/* DIO12 */
        E_KEY_DOWN,     	/* DIO13 */
        E_KEY_PROG,     	/* DIO14 */
        E_KEY_ALL,      	/* DIO15 */
        E_KEY_UP,       	/* DIO16 */
        E_KEY_OFF,      	/* DIO17 */
        E_KEY_B,        	/* DIO18 */
        E_KEY_A,        	/* DIO19 */
	    E_KEY_DSCVR = 200, 	/* Discover UNUSED */
	    E_KEY_SEL,         	/* Select   UNUSED */
	    E_KEY_1,           	/* Scene 1  UNUSED  */
	    E_KEY_2,           	/* Scene 2  UNUSED  */
	    E_KEY_3,           	/* Scene 3  UNUSED  */
	    E_KEY_4,           	/* Scene 4  UNUSED  */
        E_KEY_NONE=254,
} teTouchKeys;
/* Default RD6030 / DR1159 4V0 mapping ? */
#else
typedef enum
{
	  E_KEY_OFF,
	  E_KEY_ON,
	  E_KEY_C,
	  E_KEY_D,
	  E_KEY_PROG,
	  E_KEY_ALL,
	  E_KEY_B,
	  E_KEY_A,
	  E_KEY_DOWN,
	  E_KEY_UP,
      E_KEY_DSCVR = 200, 	/* Discover UNUSED */
	  E_KEY_SEL,         	/* Select   UNUSED */
	  E_KEY_1,           	/* Scene 1  UNUSED */
	  E_KEY_2,           	/* Scene 2  UNUSED */
	  E_KEY_3,           	/* Scene 3  UNUSED */
	  E_KEY_4,           	/* Scene 4  UNUSED */
	  E_KEY_PWR,
	  E_KEY_NONE=254,
} teTouchKeys;
#endif

typedef enum
{
	E_MODE_NORMAL,
	E_MODE_CMSNG_BULB_START,
	E_MODE_CMSNG_REMOTE_START,
	E_MODE_CMSNG_BR_START,
	E_MODE_CLONE_REMOTE_START,
	E_MODE_DCMSNG_START,
	E_MODE_RESET_TO_GATEWAY,
	E_MODE_RESET_TO_STANDALONE,
	E_MODE_RESET,
	E_MODE_FACTORY_RESET,
	E_MODE_ADD_GROUP_START,
	E_MODE_DEL_GROUP_START,
    E_MODE_ABORT

} teKeyStatusCode;

/******************************************************************************
**  EXTERNAL VARIABLES
*******************************************************************************/

/******************************************************************************
**  GLOBAL VARIABLES
*******************************************************************************/

/******************************************************************************
**  LOCAL VARIABLES
*******************************************************************************/

/******************************************************************************
**  EXPORTED FUNCTIONS
*******************************************************************************/

PUBLIC teKeyStatusCode eKeyPressTracker(teTouchKeys eTouchKeys, bool_t bNormal);

PUBLIC uint8 u8GetLastGroup(void);

PUBLIC void vKeyTick(void);

#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  // KEY_OPS_H

/*****************************************************************************************
**  END OF FILE
*****************************************************************************************/

