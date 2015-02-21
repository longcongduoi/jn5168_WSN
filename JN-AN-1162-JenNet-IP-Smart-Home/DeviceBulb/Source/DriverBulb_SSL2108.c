/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         SSL2108 Bulb Driver
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

/* DIO for the NXP SSL/LED driver boards */

#define LAMP_PWM_PIN            10              /* Timer 0 output pin      */

/* Lamp On/Off control pins (13/14/15) REF: ticket lpap87                  */

#define LAMP_ON_OFF_MASK        0xe000UL

#define LAMP_BLEEDER_PIN         3              /* Bleeder activation pin  */

#define LAMP_LEVEL_MAX          255             /* Max value for level     */
#define LAMP_TIMER_FREQUENCY    1000000         /* Timer clock frequency   */
#define LAMP_TIMER_PRESCALE     4               /* Prescale value to use   */
#define LAMP_TIMER              E_AHI_TIMER_0   /* Which timer to use      */

#define SSL_PWM_FREQUENCY       300

#define PWM_COUNT               (LAMP_TIMER_FREQUENCY/SSL_PWM_FREQUENCY)


/* 0x0FFF = 2400mv  */


#define VBUS_MAXIMUM   		228   //  (=2.4 * (470K+470K+10K)/10K
#define VBUS_TRIP_HI         81
#define VBUS_TRIP_LO         76

#if (JENNIC_CHIP == JN5148) || (JENNIC_CHIP == JN5148J01)
#define ADC_USED		E_AHI_ADC_SRC_ADC_4 /* ADC to be read */
#else
#define ADC_USED		E_AHI_ADC_SRC_ADC_1 /* ADC to be read */
#endif

#if (JENNIC_CHIP == JN5148) || (JENNIC_CHIP == JN5148J01)
#define ADC_BITS 12
#elif (JENNIC_CHIP == JN5142J01)
#define ADC_BITS  8
#else
#define ADC_BITS 10
#endif

/* Set LED pin for mimicing lamp PAM on LED output (set to > 20 to disable) */
#define LAMP_LED_PIN		17

/* Correction curves */

PRIVATE const uint8 au8Correction[128]= { 13,  14,  15,  16,  17,  18,  19,  20,
                                          21,  22,  23,  24,  25,  26,  27,  28,
                                          29,  30,  31,  32,  33,  34,  35,  36,
                                          37,  38,  39,  40,  41,  42,  43,  44,
                                          45,  46,  47,  49,  50,  51,  53,  54,
                                          56,  57,  59,  60,  62,  63,  65,  66,
                                          68,  70,  71,  73,  75,  76,  78,  80,
                                          82,  83,  85,  87,  89,  91,  92,  94,
                                          96,  98, 100, 102, 104, 106, 108, 110,
                                         112, 114, 116, 119, 121, 123, 125, 127,
                                         129, 132, 134, 136, 138, 141, 143, 145,
                                         148, 150, 153, 155, 157, 160, 162, 165,
                                         167, 170, 173, 175, 178, 180, 183, 186,
                                         188, 191, 194, 196, 199, 202, 205, 207,
                                         210, 213, 216, 219, 222, 225, 228, 231,
                                         234, 237, 240, 243, 246, 249, 252, 255 };

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

typedef enum
{
	E_STATE_LAMP_OFF,
	E_STATE_LAMP_ON,
	E_STATE_BLEEDER_ON
} teDriverStates;

typedef enum
{
	E_EVENT_OFF,
	E_EVENT_ON,
	E_EVENT_TRIP_HI,
	E_EVENT_TRIP_LO
} teDriverEvents;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
#if (LAMP_LED_PIN < 21)
PRIVATE void vCbTimer0(uint32 u32DeviceId, uint32 u32ItemBitmap);
#endif
PRIVATE void vUpdateDriverState(teDriverEvents eDriverEvent);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Global Variables                                               ***/
/****************************************************************************/


/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

PRIVATE volatile uint32 gu32BusVoltage=0; /* driver Rectified Mains supply  */
PRIVATE bool_t bIsOn = FALSE;             /* Powered State of lamp          */
PRIVATE uint16 gu16PwmLastValue = 0;      /* Last PWM Duty cycle            */

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
		vAHI_DioSetDirection(0, (LAMP_ON_OFF_MASK | (1 <<LAMP_BLEEDER_PIN)));


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



		/* start bus bleeding for dimmer compatibility */

		vAHI_DioSetOutput((1 << LAMP_BLEEDER_PIN),0);


		vAHI_ApConfigure(E_AHI_AP_REGULATOR_ENABLE,
							 E_AHI_AP_INT_DISABLE,
							 E_AHI_AP_SAMPLE_8,
							 E_AHI_AP_CLOCKDIV_500KHZ,
							 E_AHI_AP_INTREF);

		while (!bAHI_APRegulatorEnabled());   /* spin on reg not enabled */

		vAHI_AdcEnable(E_AHI_ADC_SINGLE_SHOT, E_AHI_AP_INPUT_RANGE_2, ADC_USED);

		do
		{
			vAHI_AdcStartSample();
			while(bAHI_AdcPoll());
			gu32BusVoltage = ((uint32)u16AHI_AdcRead()*VBUS_MAXIMUM) >> ADC_BITS;
		} while (gu32BusVoltage< VBUS_TRIP_HI);

		vAHI_AdcDisable();

		vAHI_ApConfigure(E_AHI_AP_REGULATOR_DISABLE,
						 E_AHI_AP_INT_DISABLE,
					     E_AHI_AP_SAMPLE_8,
						 E_AHI_AP_CLOCKDIV_500KHZ,
						 E_AHI_AP_INTREF);
	    /* turn on lamp - this is the default state */

		vAHI_DioSetOutput(LAMP_ON_OFF_MASK,0);
		vAHI_TimerStartRepeat(LAMP_TIMER, 0, PWM_COUNT );

		bInit = TRUE;
		bIsOn = TRUE;

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
	return ((gu32BusVoltage > VBUS_TRIP_HI) ? TRUE : FALSE);
}

/****************************************************************************
 *
 * NAME:       		LAMP_vSetLevel
 *
 * DESCRIPTION:		Updates the lamp level via the thermal control loop
 *
 * PARAMETERS:      Name     RW  Usage
 *         	        u8Level  R   Light level 0-LAMP_LEVEL_MAX
 *
 ****************************************************************************/
PUBLIC void DriverBulb_vSetLevel(uint8 u8Level)
{

	uint32 u32DimmerPWM;

    u32DimmerPWM = (PWM_COUNT*u8Level) / LAMP_LEVEL_MAX;
	vAHI_TimerStartRepeat(LAMP_TIMER, (PWM_COUNT-u32DimmerPWM), PWM_COUNT );
	gu16PwmLastValue = PWM_COUNT-u32DimmerPWM;

}


/****************************************************************************
 *
 * NAME:            LAMP_vOn
 *
 * DESCRIPTION:     Turns the lamp on
 *
 *
 * PARAMETERS:      Name     RW  Usage
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void DriverBulb_vOn(void)
{
	vUpdateDriverState(E_EVENT_ON);
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
    vUpdateDriverState(E_EVENT_OFF);
}

/****************************************************************************
 *
 * NAMES:           DriverBulb_bOn, u16ReadBusVoltage, u16ReadChipTemperature
 *
 * DESCRIPTION:		Acess functions for Monitored Lamp Parmeters
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
 * DESCRIPTION:		10 ms Ticks from higher layer for timing
 *
 *
 ****************************************************************************/

PUBLIC void DriverBulb_vTick(void)
{

}
/****************************************************************************
 *
 * NAME:			vMonitorVoltage
 *
 * DESCRIPTION:
 *                  Reads ADC and updates voltage variable used by forground
 *                  0xFFF is 2.4V (=228V)
 *                  Also turns off lamp if Vbus to low.
 *
 *
 *
 *
 * PARAMETERS:      Name	     RW  Usage
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC int16 DriverBulb_i16Analogue(uint8 u8Adc, uint16 u16AdcRead)
{
	uint32 u32AdcSample = 0;

	/* ADC4/1 - bus voltage  */
	if (u8Adc == ADC_USED)
	{
 		u32AdcSample = ((uint32)u16AdcRead*VBUS_MAXIMUM) >> ADC_BITS;

     	gu32BusVoltage = u32AdcSample;

		if (u32AdcSample > VBUS_TRIP_HI)
		{
         	vUpdateDriverState(E_EVENT_TRIP_HI);
    	}

    	if (u32AdcSample < VBUS_TRIP_LO)
    	{
			vUpdateDriverState(E_EVENT_TRIP_LO);
		}
	}

	return (int16) gu32BusVoltage;
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
			/* Set output high, turning off LED */
			vAHI_DioSetOutput(0, (1 << LAMP_LED_PIN));
		}
		/* Fall interrupt */
		else
		{
			/* Set output low, turning on LED */
			vAHI_DioSetOutput((1 << LAMP_LED_PIN), 0);
		}
	}
}
#endif

/****************************************************************************
 *
 * NAME:            vUpdateDriverState
 *
 * DESCRIPTION:     Sequences the off-bleeder-on control logic
 *
 * PARAMETERS:      Name	     RW  Usage
 *                  eDriverEvent R	 Internal Driver Events
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/

PRIVATE void vUpdateDriverState(teDriverEvents eDriverEvent)
{
	static teDriverStates eDriverState = E_STATE_LAMP_OFF;
	static uint8 u8TripCount = 0;

	switch (eDriverState)
	{
		case E_STATE_LAMP_OFF:
		    if (eDriverEvent == E_EVENT_ON)
		    {
				eDriverState = E_STATE_BLEEDER_ON;
				vAHI_DioSetOutput((1 << LAMP_BLEEDER_PIN),0);
				u8TripCount = 0;
		    }
		    break;

		case E_STATE_BLEEDER_ON:
		   if (eDriverEvent  == E_EVENT_TRIP_HI)
		   {
			   if (u8TripCount >0)
			   {
			       eDriverState = E_STATE_LAMP_ON;
			       bIsOn=TRUE;
			       vAHI_DioSetOutput(LAMP_ON_OFF_MASK,0);
			       vAHI_TimerStartRepeat(LAMP_TIMER, gu16PwmLastValue, PWM_COUNT );
			   }
			   u8TripCount++;
		   }
		   break;

		case	E_STATE_LAMP_ON:

		   if (eDriverEvent == E_EVENT_OFF)
		   {
			   eDriverState = E_STATE_LAMP_OFF;
			   vAHI_DioSetOutput(0, (1 << LAMP_BLEEDER_PIN ));
			   vAHI_DioSetOutput(0, LAMP_ON_OFF_MASK);
			   vAHI_TimerStartRepeat(LAMP_TIMER, PWM_COUNT, PWM_COUNT );
			   bIsOn = FALSE;
		   }

		   if (eDriverEvent == E_EVENT_TRIP_LO)
		   {
			    eDriverState = E_STATE_BLEEDER_ON;
			    vAHI_DioSetOutput(0, LAMP_ON_OFF_MASK);
			    vAHI_TimerStartRepeat(LAMP_TIMER, PWM_COUNT, PWM_COUNT );
			    u8TripCount = 0;
			    bIsOn = FALSE;
	       }
	       break;

		   default : break;
   }
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
