/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         Device Commissioning Controller
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
#ifndef COMMISSION_H
#define COMMISSION_H

#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/******************************************************************************
**  INCLUDE FILES
*******************************************************************************/
#include <Api.h>
/******************************************************************************
**  MACROS
*******************************************************************************/

#define CMSNG_DURATION_S	        300
#define AUTH_TIMEOUT_S              15
#define CMSNG_TIMEOUT_S             10
#define	SETVAR_TIMEOUT_TICKS		25


/* bit2 (4) set for any commissioning mode (sentinal bit) */
#define ANY_STATE_COMMISSIONING 0x04UL
/******************************************************************************
**  CONSTANTS
*******************************************************************************/

/******************************************************************************
**  TYPEDEFS
*******************************************************************************/

typedef enum
{
	E_STATE_CMSNG_IDLE,
	E_STATE_CMSNG_START,
	E_STATE_CMSNG_INPRG,
	E_STATE_CMSNG_SENDGROUP_START,
	E_STATE_CMSNG_SENDGROUP_INPRG,
	E_STATE_CMSNG_FINISH_START,
	E_STATE_CMSNG_FINISH_INPRG,
}teAuthState;

typedef enum
{
	E_EVENT_DECMSNG_START,
	E_EVENT_DECMSNG_TICK,
	E_EVENT_DECMSNG_FINISH
} teDecommssionEvent;

typedef enum
{
	E_STATE_NO_NWK,
	E_STATE_CONTROLLING,
	E_STATE_COMMISSION_BULB = 4,
	E_STATE_COMMISSION_BR,
	E_STATE_COMMISSION_REMOTE,
	E_STATE_CLONE_REMOTE,
	E_STATE_DECOMMISSIONING,
	E_STATE_CLONING,
	E_STATE_LEARNING
}teSysState;

typedef struct
{
	volatile uint16 u16CmsngDuration;
	volatile uint8   u8CmsngTimeout;
	volatile uint8   u8Tenths;
	volatile uint8   u8Ticks;
	volatile uint8   u8SetVarTimeout; /* In ticks */
	volatile uint16 u16LearningTimeout; /* In ticks */
} tsTimers;

typedef struct
{
    tsTimers sTimers;
	uint8 u8ButtonNumber;
	uint8 u8Retries;
	bool_t bTouchPosted;
	bool_t bWasOnOff;
	bool_t bGuardNeeded;
}tsDevice;


typedef struct
{
	  uint8          u8Group;
	  teAuthState     eAuthState;
      MAC_ExtAddr_s   sAddr;
      tsSecurityKey   sSecKey;
      uint32 		u32DeviceId; 	/* Device id of joined device */
      uint16 		u16DeviceType;	/* Device type of joined device */
} tsAuthorise;


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
PUBLIC void vCommissionInit(tsDevice *psDevice ,tsAuthorise *psAuthorise);

PUBLIC void vCommissionMode(tsDevice *psDevice ,tsAuthorise *psAuthorise, teSysState *peSysState);

PUBLIC void vDecommissionMode(teSysState *peSysState,teDecommssionEvent eDecommissionEvent);

PUBLIC void vTtlOverride(uint8 u8MaxBcastTtl);
PUBLIC void vTtlRestore(void);

#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  // KEY_OPS_H

/*****************************************************************************************
**  END OF FILE
*****************************************************************************************/

