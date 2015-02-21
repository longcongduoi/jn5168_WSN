/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         UBA2027 Bulb Driver
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
/* Standard includes */
#include <string.h>
/* SDK includes */
#include <jendefs.h>
/* Hardware includes */
#include <AppHardwareApi.h>
#include <PeripheralRegs.h>
/* Application includes */
#include "Config.h"
/* Device includes */
#include "DriverBulb.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/* DIO for the NXP CFC/LED driver boards */

#define LAMP_PWM_PIN            10              /* Timer 0 output pin      */

/* Lamp On/Off control pins (13/14/15) REF: ticket lpap87                  */

#define LAMP_ON_OFF_MASK        0xe000UL

#define LAMP_LEVEL_MAX          255             /* Max value for level     */
#define LAMP_LEVEL_MIN_ON       20
#define LAMP_TIMER_FREQUENCY    1000000         /* Timer clock frequency   */
#define LAMP_TIMER_PRESCALE     4               /* Prescale value to use   */
#define LAMP_TIMER              E_AHI_TIMER_0   /* Which timer to use      */

#define CFL_PWM_FREQUENCY       2000
#define CFL_TURN_ON_10MS_TICKS  50

#define PWM_COUNT               (LAMP_TIMER_FREQUENCY/CFL_PWM_FREQUENCY)

/* Dimming values: Ref Table 6 HSI Spec                                     */
#define PWM_MIN                 59              /* Min Deep Dim             */
#define PWM_MIN_START           76              /* Min Ignitiion            */
#define PWM_MAX                 255             /* Max ON                   */

#define VBUS_MAXIMUM   		    480
#define ADC_BITS            	12

#if (JENNIC_CHIP == JN5148) || (JENNIC_CHIP == JN5148J01)
#define ADC_USED		E_AHI_ADC_SRC_ADC_4 /* ADC to be read */
#else
#define ADC_USED		E_AHI_ADC_SRC_ADC_1 /* ADC to be read */
#endif

/* Set LED pin for mimicking lamp PAM on LED output (set to > 20 to disable) */

#define LAMP_LED_PIN		22


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
#if (LAMP_LED_PIN < 21)
PRIVATE void vCbTimer0(uint32 u32DeviceId, uint32 u32ItemBitmap);
#endif

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Global Variables                                               ***/
/****************************************************************************/


/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

PRIVATE bool_t bIsOn = FALSE;

PRIVATE uint8 gu8LastLightLevel = 0;

PRIVATE uint8 gu8TurnOnTimer = 0;


/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME:       		LAMP_vInit
 *
 * DESCRIPTION:		Initialises the lamp drive system
 *
 * PARAMETERS:      Name     RW  Usage
 *
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void DriverBulb_vInit(void)
{
	bool_t bLampTimerInt = FALSE;

	static bool_t bInit = FALSE;

	if (bInit == FALSE)
	{
		/* Configure DIO pins */
		vAHI_DioSetDirection(0, LAMP_ON_OFF_MASK);

		/* Mimic PWM to led ? */
		#if (LAMP_LED_PIN < 21)
		{
			/* Configure as output */
			vAHI_DioSetDirection(0, (1 << LAMP_LED_PIN));
			/* Turn CFL lamp off */
			vAHI_DioSetOutput(0, (1 << LAMP_LED_PIN)/*, 0*/);
			/* Register interrupt callback */
			vAHI_Timer0RegisterCallback(vCbTimer0);
			/* Note we want to generate interrupts */
			bLampTimerInt = TRUE;
		}
		#endif

		/* Configure timer 0 to generate a PWM output on its output pin */
		vAHI_TimerEnable(LAMP_TIMER, LAMP_TIMER_PRESCALE, bLampTimerInt, bLampTimerInt, TRUE);
		vAHI_TimerConfigureOutputs(LAMP_TIMER, FALSE, TRUE);
		vAHI_TimerClockSelect(LAMP_TIMER, FALSE, TRUE);

		/* Register the thermal-loop max temperature set-point and Turn CFL lamp on          */
		/* via the Thermal Control Loop call-forward function (which sets up the TCL)        */

		vAHI_DioSetOutput(LAMP_ON_OFF_MASK,0);
		bIsOn = TRUE;
		vAHI_TimerStartRepeat(LAMP_TIMER, 0, PWM_COUNT );
		//DriverBulb_vSetLevel(LAMP_LEVEL_MAX);

        bInit = TRUE;
	}

}

/****************************************************************************
 *
 * NAME:       		LAMP_bReady
 *
 * DESCRIPTION:		Returns if lamp is ready to be operated
 *
 * PARAMETERS:      Name     RW  Usage
 *
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC bool_t DriverBulb_bReady(void)
{
	return (TRUE);
}

/****************************************************************************
 *
 * NAME:       		LAMP_vSetLevel
 *
 * DESCRIPTION:		Updates the PWM via the thermal control loop
 *
 *
 * PARAMETERS:      Name     RW  Usage
 *         	        u8Level  R   Light level 0-LAMP_LEVEL_MAX
 *
 ****************************************************************************/
PUBLIC void DriverBulb_vSetLevel(uint8 u8Level)
{
	uint32 u32DimmerPWM;

	/* if we're over-driving due to low last level or we have a high enough */
	/* level then update the PWM output accordingly                        */
	if ((gu8TurnOnTimer == 0) || (u8Level >= LAMP_LEVEL_MIN_ON))
	{
		/* scale to get 128 steps between min & max */
		u32DimmerPWM = PWM_MIN + ((PWM_MAX-PWM_MIN)*(u8Level|1)/PWM_MAX);
		u32DimmerPWM = (u32DimmerPWM * PWM_COUNT)/PWM_MAX;
		vAHI_TimerStartRepeat(LAMP_TIMER, (PWM_COUNT-u32DimmerPWM), PWM_COUNT );
		gu8LastLightLevel = u8Level;


	}
}

/****************************************************************************
 *
 * NAME:            LAMP_vOn
 *
 * DESCRIPTION:     Turns the lamp on, over-driving if user deep-dimmed
 *                  before turning off otherwise ignition failures occur
 *
 ****************************************************************************/
PUBLIC void DriverBulb_vOn(void)
{
	uint32 u32DimmerPWM;

	vAHI_DioSetOutput(LAMP_ON_OFF_MASK, 0);
	gu8TurnOnTimer = 0;

	if (gu8LastLightLevel <  PWM_MIN_START)
	{
		u32DimmerPWM = (PWM_COUNT*PWM_MIN_START) / LAMP_LEVEL_MAX;
	    gu8TurnOnTimer = CFL_TURN_ON_10MS_TICKS; 	/* schedule future restore point         */
	}
	else
	{
		 u32DimmerPWM = (PWM_COUNT*gu8LastLightLevel) / LAMP_LEVEL_MAX;
         bIsOn = TRUE;
	}

	vAHI_TimerStartRepeat(LAMP_TIMER, (PWM_COUNT-u32DimmerPWM), PWM_COUNT );

}


/****************************************************************************
 *
 * NAME:            LAMP_vOff
 *
 * DESCRIPTION:     Turns the lamp off
 *
 *
 * PARAMETERS:      Name     RW  Usage
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void DriverBulb_vOff(void)
{
    /* Turn lamp off */
    vAHI_DioSetOutput(0, LAMP_ON_OFF_MASK);
    vAHI_TimerStartRepeat(LAMP_TIMER, PWM_COUNT, PWM_COUNT );
    bIsOn = FALSE;
}

/****************************************************************************
 *
 * NAMES:           DriverBulb_bOn, u16ReadBusVoltage, u16ReadChipTemperature
 *
 * DESCRIPTION:		Access functions for Monitored Lamp Parameters
 *
 *
 * PARAMETERS:      Name     RW  Usage
 *
 * RETURNS:
 * Lamp state, Bus Voltage (Volts), Chip Temperature (Degrees C)
 *
 ****************************************************************************/
PUBLIC bool_t DriverBulb_bOn(void)
{
	return (bIsOn);
}

/****************************************************************************
 *
 * NAMES:           DriverBulb_vTick
 *
 * DESCRIPTION:		Hook for 10 ms Ticks from higher layer for timing
 *                  This currently restores very low dim levels after
 *                  we've started up with sufficient DCI level to ensure
 *                  ignition
 *
 ****************************************************************************/

PUBLIC void DriverBulb_vTick(void)
{
	if (gu8TurnOnTimer>0)
	{
		gu8TurnOnTimer--;
		if (gu8TurnOnTimer==0)
		{
			bIsOn = TRUE;
			DriverBulb_vSetLevel(gu8LastLightLevel);
		}
	}
}

/****************************************************************************
 *
 * NAME:			DriverBulb_i16Analogue
 *
 * DESCRIPTION:     converts a raw ADC reading from Analogue MIB layer
 *                  into a bus voltage.  Originally used for compatibility
 *                  with wall dimmers (auto-shut off lamp if vbus to low)
 *                  but this functionality depracated and now just returns
 *                  the bus voltage (2.4V ADC = 480V VBUS)
 *
 * PARAMETERS:      Name	     RW      Usage
 *                  u8Adc        R       ADC Channel
 *                  u16AdcRead   R       Raw ADC Value
 *
 * RETURNS:         Bus voltage
 *
 ****************************************************************************/
PUBLIC int16 DriverBulb_i16Analogue(uint8 u8Adc, uint16 u16AdcRead)
{
	uint32 u32AdcSample = 0;

	if (u8Adc == ADC_USED)
    {
        u32AdcSample = ((uint32)u16AdcRead*VBUS_MAXIMUM) >> ADC_BITS;
	}
    return (int16) u32AdcSample;
}


/****************************************************************************
 *
 * NAME:			DriverBulb_bFailed
 *
 * DESCRIPTION:     Access function for Failed bulb state
 *
 *
 * RETURNS:         bulb state
 *
 ****************************************************************************/

PUBLIC bool_t DriverBulb_bFailed(void)
{
	return (FALSE);
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/* Mimic PWM to led ? */
#if (LAMP_LED_PIN < 21)
PRIVATE void vCbTimer0(uint32 u32DeviceId, uint32 u32ItemBitmap)
{
	/* Lamp off or period interrupt ? */
	if (bIsOn == FALSE)
	{
		/* Set output high, turning off LED */
		vAHI_DioSetOutput((1 << LAMP_LED_PIN), 0);
	}
	else
	{
		/* Rise interrupt ? */
		if ((u32ItemBitmap & E_AHI_TIMER_RISE_MASK) != 0)
		{
			/* Set output low, turning on LED */
			vAHI_DioSetOutput(0, (1 << LAMP_LED_PIN));
		}
		/* Fall interrupt */
		else
		{
			/* Set output high, turning off LED */
			vAHI_DioSetOutput((1 << LAMP_LED_PIN), 0);
		}
	}
}
#endif

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
