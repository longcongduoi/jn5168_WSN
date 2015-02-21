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

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <jendefs.h>
#include <JIP.h>

#include "DriverLed.h"

#include "dbg.h"
#include "dbg_uart.h"
#include "AppHardwareApi.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#define MARK  0
#define SPACE 1
#define TICK_TIME_MS 10

#ifdef DBG_ENABLE
#define TRACE_LED  TRUE
#else
#define TRACE_LED  FALSE
#endif

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
/***        Global Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE struct
{
	uint16 u16LedTimer;
	uint16 u16Mark;
	uint16 u16Space;

} sLedRegs;

PRIVATE uint16 au16LedParams[10][2] = {{   0,   0},
		                              {9990,  10},
		                              {  50,   0},
		                              { 250, 250},
		                              {5000,   0},
		                              {9990,  10},
		                              {3000,1000},
		                              {2000,2000},
		                              {1000,1000},
                                      {2000,   0}};

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: vLedInit
 *
 * DESCRIPTION:
 *
 * Initialises the DIO Led Drive
 *
 * PARAMETERS:      Name            RW  Usage
 *
 * RETURNS:         None
 *
 ****************************************************************************/
PUBLIC void vLedInit(void)
{
    /* Initialise LED DIO */
    vAHI_DioSetDirection(0,LED_DIO_MASK);
    vAHI_DioSetOutput(LED_DIO_MASK,0);
}

/****************************************************************************
 *
 * NAME: vSetLedState
 *
 * DESCRIPTION:     Loads Led Mode indication into timer
 *
 * PARAMETERS:      Name            RW  Usage
 *
 * RETURNS:         None
 *
 ****************************************************************************/
PUBLIC void vSetLedState(teLedStates eLedState)
{
	sLedRegs.u16Mark     = au16LedParams[eLedState][MARK];
	sLedRegs.u16Space    = au16LedParams[eLedState][SPACE];
	sLedRegs.u16LedTimer = sLedRegs.u16Space+sLedRegs.u16Mark;

	if(sLedRegs.u16Mark > 0)
	{
		LED_ON;
	}
}

/****************************************************************************
 *
 * NAME: vLedTick
 *
 * DESCRIPTION:		Provides timing of led mode indications
 *
 * PARAMETERS:      Name            RW  Usage
 *
 * RETURNS:         None
 *
 ****************************************************************************/

PUBLIC void vLedTick()
{
	if (sLedRegs.u16LedTimer >0)
	{
		sLedRegs.u16LedTimer -=TICK_TIME_MS;
	}
	else if ((sLedRegs.u16Mark >0) && (sLedRegs.u16Space>0)  )
	{
		sLedRegs.u16LedTimer = sLedRegs.u16Mark + sLedRegs.u16Space;
	}

	if (sLedRegs.u16Mark == 0)
	{
		LED_OFF;
	}
	else if ((sLedRegs.u16LedTimer == 0) && (sLedRegs.u16Space == 0 ))
	{
		LED_OFF;
	}
	else
	{
        (sLedRegs.u16LedTimer > sLedRegs.u16Space) ? LED_ON : LED_OFF ;
	}
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
