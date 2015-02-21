/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         Thermal Control Loop
 *
 * Three term PID controller to provide thermal stability to Dde temperature
 * under harsh luminaire environments
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

/* SDK includes */
#include <jendefs.h>
#include <dbg.h>
#include <dbg_uart.h>

/* Application includes */
#include "Config.h"
#include "ThermalControl.h"
#include "DriverBulb.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#ifdef JENNIC_CHIP_FAMILY_JN514x
#ifdef JENNIC_CHIP_JN5142J01
#define GAIN_DIFF         1024
#else
#define GAIN_DIFF           64
#endif
#else
#define GAIN_DIFF          256
#endif

#define GAIN_INT             8
#define SETPOINT           120

#define CLIP_HI          65535
#define CLIP_LO          16384

#define PWM_LIM            128
#define FILTER_SIZE          8


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/* symbolic access to the (1/z) elements in controller transfer function    */
typedef enum
{
	E_Z_DIFF,
	E_Z_INTG,
	E_Z_FILT
} teUnitDelayIndex;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

PRIVATE void vTransferFunction(int16 i16ChipTemp);
PRIVATE void vPwmCorrection(uint8 u8PwmIn, int32 i32LpFilterOut);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/


/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

PRIVATE int32 gi32LoopFilter = 65535;    /* Output from PID controller      */
PRIVATE uint8 gu8UserLevel;              /* Copy of last user light request */




/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: vDecimator
 *
 * DESCRIPTION:
 *
 * Accepts the 4Hz ADC temperature samples and implements a decimation
 * function by undersampling by a factor of 20 to give a 5 second sample
 * period of the chip temperature.  This is the passed to the various
 * helper functions that implement the controller transfer function
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/


PUBLIC void vDecimator(int16 i16ChipTemp)
{
	static uint8 u8SampleCount = 1;
	static bool_t bFirstFunctionCall = TRUE;

	static uint8 u8Index = 0;
	static int16 ai16Samples[FILTER_SIZE] = {0};

	static int16 i16FilteredChipTemp;

	/* The first temperature reading after cold-start initialises the filter array */

	if (bFirstFunctionCall == TRUE)
	{
		uint32 i;
		for (i=0;i<FILTER_SIZE;i++)
		{
			ai16Samples[i] = i16ChipTemp;
		}
		i16FilteredChipTemp = i16ChipTemp << 3;
		bFirstFunctionCall = FALSE;
	}

	/* Moving average filter Called every 0.25 seconds */

	i16FilteredChipTemp = i16FilteredChipTemp - ai16Samples[u8Index] + i16ChipTemp;
	ai16Samples[u8Index] = i16ChipTemp;
	u8Index = (u8Index+1) & 7;
	if (--u8SampleCount ==0)
	{
		u8SampleCount = DECIMATE_FACTOR;
		vTransferFunction((i16FilteredChipTemp >>3));
	}
}

/****************************************************************************
 *
 * NAME:  vSetUserLevel
 *
 * DESCRIPTION:
 *
 * setter function to allow the driver to push any new user light level commands
 * through the control loop. Note it must not execute the algorithm as the
 * this is under control of the decimation filter.  The existing correction
 * is simply applied before updating the driver
 *
 * RETURNS:
 *
 ****************************************************************************/
PUBLIC void vSetUserLevel(uint8 u8Level)
{
	/* take a copy for next time the transfer function is executed */
	gu8UserLevel = u8Level;

	/* apply the exiting correction factor from the transfer function */
	(void)vPwmCorrection(u8Level,gi32LoopFilter);

}


/****************************************************************************
 *
 * NAME: vTransferFunction
 *
 * DESCRIPTION:
 *
 * Implements the PID controller to limit chip die temperature under
 * enclosed luminaire conditions
 *
 * RETURNS:
 * Loop filter output for application to PWM corrector
 *
 ****************************************************************************/

PRIVATE void vTransferFunction(int16 i16ChipTemp)
{
    /* 1/z unit delay elements and initial values */
	static int32 ai32UnitDelays[3] = {0, 65535,65535};

    /* temporary output variables for each stage */
	int16 i16Error       = 0;
	int16 i16DiffOut     = 0;
	int32 i32IntLoopOut  = 0;
	int32 i32LpFilterOut = 0;

	/*start of transfer function */

	i16Error = (int16)(SETPOINT - i16ChipTemp);

	/* Differentiate Error and clip */

	if (i16Error > ai32UnitDelays[E_Z_DIFF]) i16DiffOut = GAIN_DIFF;
	if (i16Error < ai32UnitDelays[E_Z_DIFF]) i16DiffOut = -GAIN_DIFF;

	ai32UnitDelays[E_Z_DIFF] = i16Error;

	/* Integrating loop  */

	i32IntLoopOut = ai32UnitDelays[E_Z_INTG];

	if (i32IntLoopOut > CLIP_HI) i32IntLoopOut = CLIP_HI;
	if (i32IntLoopOut < CLIP_LO) i32IntLoopOut = CLIP_LO;

	ai32UnitDelays[E_Z_INTG] = (int32)(i16Error * GAIN_INT) + i32IntLoopOut + (int32)i16DiffOut;

   /* Low pass Ripple Filter  */

	i32LpFilterOut = ai32UnitDelays[E_Z_FILT];

	ai32UnitDelays[E_Z_FILT] = ((i32LpFilterOut*7) + i32IntLoopOut) >> 3;

    /* algorithm done so update monitor structure used by TH and apply to current user demand */

    gi32LoopFilter = i32LpFilterOut;

#ifdef DEBUG_TCL
	sTclVarMon.u8PwmIn        = gu8UserLevel;
	sTclVarMon.i8ChipTemp     = (int8)i16ChipTemp;
	sTclVarMon.i16Error       = i16Error;
	sTclVarMon.i16DiffOut     = i16DiffOut;
	sTclVarMon.i32IntLoopOut  = i32IntLoopOut;
	sTclVarMon.i32LpFilterOut = i32LpFilterOut;
#endif

	(void)vPwmCorrection(gu8UserLevel,i32LpFilterOut);

}

/****************************************************************************
 *
 * NAME: vPwmCorrection
 *
 * DESCRIPTION:
 *
 * Takes the user demand for light level and applies any correction necessary
 * based on the the chip temperature and the thermal control loop demand
 * this needs to be decoupled from the sampled transfer function to allow
 * asynchronous user events to be corrected without affecting the loop performance
 *
 ****************************************************************************/
PRIVATE void vPwmCorrection(uint8 u8PwmIn, int32 i32LpFilterOut)
{
	int32 i32PwmAdjOut   = 0;
	uint8 au8PwmTemp[2]  = {0};

    /* PWM correction */
	au8PwmTemp[0] = (u8PwmIn >=PWM_LIM) ? PWM_LIM : u8PwmIn;
	au8PwmTemp[1] = (u8PwmIn >=PWM_LIM) ? (u8PwmIn-PWM_LIM) : 0;

	i32PwmAdjOut = (i32LpFilterOut >> 8) * (int32)au8PwmTemp[1];
	i32PwmAdjOut = (i32PwmAdjOut   >> 8) + (int32)au8PwmTemp[0];

    /* call-back into driver to pass the thermal loop adjusted value to the PWM duty cycle */
	/* this is only enabled if we have a valid  (non-zero) set-point temperature           */


#ifdef DEBUG_TCL
	sTclVarMon.u8PwmOut = (uint8)i32PwmAdjOut;
#endif

	DriverBulb_vCbSetLevel((uint8)i32PwmAdjOut);

}


/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
