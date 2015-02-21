/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         LED Driver
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
#ifndef LED_DRIVER
#define LED_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/******************************************************************************
**  INCLUDE FILES
*******************************************************************************/

/******************************************************************************
**  MACROS
*******************************************************************************/


#if MK_KEYMAP == 2
#define LED1_DIO  (0)
#define LED2_DIO  (1)
#else
#define LED1_DIO  (2)
#define LED2_DIO  (3)
#endif


#define LED_DIO_MASK ((1<<LED1_DIO) | (1<<LED2_DIO))

#define LED_OFF  vAHI_DioSetOutput( (1<<LED1_DIO),0)
#define LED_ON   vAHI_DioSetOutput(0, (1<<LED1_DIO))

/* 2nd LED on PCB */
#define DBG_LED_OFF vAHI_DioSetOutput((1<<LED2_DIO),0)
#define DBG_LED_ON 	vAHI_DioSetOutput(0,(1<<LED2_DIO))

/******************************************************************************
**  CONSTANTS
*******************************************************************************/

/******************************************************************************
**  TYPEDEFS
*******************************************************************************/

typedef enum
{
	E_LED_STATE_OFF,
	E_LED_STATE_ON,
	E_LED_STATE_WINK,
	E_LED_STATE_JOINING,
	E_LED_STATE_JOINED,
	E_LED_STATE_COMMISSIONING,
	E_LED_STATE_DECOMMISSIONING,
	E_LED_STATE_CLONING,
	E_LED_STATE_LEARNING,
	E_LED_STATE_ADDDEL_GROUP
} teLedStates;

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

PUBLIC void vLedInit(void);

PUBLIC void vSetLedState(teLedStates eLedState);

PUBLIC void vLedTick(void);

#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  // LED_DRIVER_H

/*****************************************************************************************
**  END OF FILE
*****************************************************************************************/

