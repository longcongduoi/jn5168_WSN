/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         SSL2108 Bulb Driver (SYNCHRONOUS VARIANT)
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


#define MICRO_DISABLE_AND_SAVE_INTERRUPTS(u32Store);                        \
    {                                                                       \
        asm volatile ("b.mfspr %0, r0, 17;" :"=r"(u32Store) : );            \
        asm volatile ("b.di;" : : );                                        \
    }

#define MICRO_RESTORE_INTERRUPTS(u32Store);                                 \
        asm volatile ("b.mtspr r0, %0, 17;" : :"r"(u32Store));

/* Lamp On/Off control pins (13/14/15) REF: ticket lpap87                  */

#define LAMP_ON_OFF_MASK        0xe000UL

#define LAMP_LEVEL_MAX          255             /* Max value for level     */
#define LAMP_TIMER_PRESCALE     2               /* Prescale value to use   */
#define LAMP_TIMER              E_AHI_TIMER_0   /* Which timer to use      */
#define LAMP_TIMER_CLK_FREQ     4E6
#define LAMP_TIMER_PWM_FREQ     400

#define PWM_SYNC_TIMER          E_AHI_TIMER_2
#define PWM_SYNC_TIMER_CTRL     0x0200B00C
#define PWM_SYNC_TIMER_PRESCALE 5
#define PWM_SYNC_TIMER_MAX      15000U


#define COMP_DIO_MASK           ( (1 << 16) | (1 <<17) )

#define SYNC_LOSS_10MS_TICKS    10 /* 100ms to detect sync loss */


#define ADDR_AP_COMPCTRL  0x02001F20

#define CURRENT_BLEED_DIO_MASK  0x00001b00UL              /* bits 12+11+8+9 */
#define RX_ACTIVE_DIO           2
#define TX_ACTIVE_DIO           3
#define TX_RX_DIO_MASK          ((1<<TX_ACTIVE_DIO) | (1<<RX_ACTIVE_DIO) )

#define GPIO_DOUT_REG           0x02002004UL
#define GPIO_DIN_REG			0x02002008UL
#define SYSCON_WK_ET            0x02000008UL
#define SYSCON_SYS_IM           0x0200000CUL
#define SYSCON_SYS_IS           0x02000010UL

#define TIMER0_T_HI             0x02005004UL
#define TIMER0_T_LO             0x02005008UL


#define REG_SYS_PWR_CTRL        0x02000000
#define RFRXEN_MASK             (1 << 24)
#define RFTXEN_MASK             (1 << 25)

/* Debug DIO */
#define ENABLE_TRACE_COMPARATOR
#define SYSCON_ISR_ASSERT vAHI_DioSetOutput(LAMP_ON_OFF_MASK,0)
#define SYSCON_ISR_NEGATE vAHI_DioSetOutput(0,LAMP_ON_OFF_MASK)

#define TRACE_COMP_IE_DIO 1
#ifdef TRACE_COMP_IE_DIO
#define TRACE_COMP_IE_ON     vAHI_DioSetOutput( (1<<TRACE_COMP_IE_DIO),0);
#define TRACE_COMP_IE_OFF    vAHI_DioSetOutput( 0,(1<<TRACE_COMP_IE_DIO));
#else
#define TRACE_COMP_IE_ON
#define TRACE_COMP_IE_OFF
#endif

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

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

PRIVATE void vCbSystemController(uint32 u32DeviceId, uint32 u32ItemBitmap);
PRIVATE void vCbTimer2(uint32 u32DeviceId, uint32 u32ItemBitmap);
extern void vAHI_InterruptSetPriority(uint32 u32Mask, uint32 u32Value);

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
	volatile uint32 u32SyncLockLossCount;
	volatile uint16 u16PwmPeriod;
	         uint16 u16PwmLastValue;
	         bool_t bIsOn;

} sBulb;
/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME:       		LAMP_vInit
 *
 * DESCRIPTION:		Initialises the lamp drive system
 *
 ****************************************************************************/
PUBLIC void DriverBulb_vInit(void)
{
	static bool_t bFirstCall = TRUE;

	if (bFirstCall == TRUE)
	{
		bFirstCall = FALSE;
		/* Configure DIO pins */
		vAHI_DioSetDirection(COMP_DIO_MASK, LAMP_ON_OFF_MASK );
	    vAHI_DioSetPullup(0,COMP_DIO_MASK);
	    vAHI_DioSetDirection(TX_RX_DIO_MASK ,CURRENT_BLEED_DIO_MASK);
	    vAHI_DioInterruptEdge((1<<TX_ACTIVE_DIO),(1<<RX_ACTIVE_DIO));

#ifdef TRACE_COMP_IE_DIO
    vAHI_DioSetDirection(0,(1<<TRACE_COMP_IE_DIO));
#endif

	    /*release DIO 8 & 9 off timer 0 (before timer enabled to avoid glitches */
	    vAHI_TimerFineGrainDIOControl(0x13);

		/* Configure timer 0 to generate a PWM output on its output pin */
		vAHI_TimerEnable(LAMP_TIMER, LAMP_TIMER_PRESCALE, FALSE, FALSE, TRUE);
		vAHI_TimerConfigureOutputs(LAMP_TIMER, TRUE, TRUE); /* Invert PWM and disable gating */
		vAHI_TimerClockSelect(LAMP_TIMER, FALSE, TRUE);

		  /* Set up a timer to work out number of cycles between comparator IRQs */
		vAHI_TimerEnable(PWM_SYNC_TIMER,PWM_SYNC_TIMER_PRESCALE,TRUE, FALSE, FALSE);
		vAHI_TimerConfigureOutputs(PWM_SYNC_TIMER, FALSE, TRUE);
		vAHI_TimerClockSelect(PWM_SYNC_TIMER, FALSE, TRUE);

		vAHI_Timer2RegisterCallback(vCbTimer2);

	    vAHI_SysCtrlRegisterCallback(vCbSystemController);

	    /* Raise the system controller and timer2 prority levels above the stack */
		vAHI_InterruptSetPriority( (1 << E_AHI_DEVICE_SYSCTRL) , 13);
		vAHI_InterruptSetPriority( (1 << E_AHI_DEVICE_TIMER2 ) , 12);

		/* Initialise the lamp data structure to ON (max brightness) */
        sBulb.u16PwmPeriod    = LAMP_TIMER_CLK_FREQ / LAMP_TIMER_PWM_FREQ;
        sBulb.u16PwmLastValue = LAMP_TIMER_CLK_FREQ / LAMP_TIMER_PWM_FREQ;
        sBulb.u32SyncLockLossCount =  0;
        sBulb.bIsOn = TRUE;

	    /* now turn on lamp - this is the default state */
		vAHI_TimerStartRepeat( LAMP_TIMER, sBulb.u16PwmLastValue, sBulb.u16PwmPeriod );
	}
	else /* 2nd call to  function is after stack starts so safe to enable high power */
	{
		*(uint32 *)REG_SYS_PWR_CTRL |= (RFRXEN_MASK | RFTXEN_MASK);
		 vAHI_DioInterruptEnable(((1<<TX_ACTIVE_DIO) | (1<<RX_ACTIVE_DIO)),0);
	}
}

/****************************************************************************
 *
 * NAME:       		LAMP_bReady
 *
 * DESCRIPTION:		Returns if lamp is ready to be operated
 *
 * RETURNS:         TRUE
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
 * DESCRIPTION:		Updates the lamp level
 *
 * PARAMETERS:      Name     RW  Usage
 *         	        u8Level  R   Light level 0-LAMP_LEVEL_MAX
 *
 ****************************************************************************/
PUBLIC void DriverBulb_vSetLevel(uint8 u8Level)
{

	sBulb.u16PwmLastValue = ((au8Correction[(u8Level >> 1)]) * sBulb.u16PwmPeriod) >> 8;

    if (sBulb.u32SyncLockLossCount >= SYNC_LOSS_10MS_TICKS)
    {
    	if(sBulb.bIsOn == TRUE)
    	{
    		vAHI_TimerStartRepeat( LAMP_TIMER, sBulb.u16PwmLastValue, sBulb.u16PwmPeriod );
    	}
    }
}

/****************************************************************************
 *
 * NAME:            LAMP_vOn
 *
 * DESCRIPTION:     Turns the lamp on
 *
 ****************************************************************************/
PUBLIC void DriverBulb_vOn(void)
{
#ifndef ENABLE_TRACE_COMPARATOR
	vAHI_DioSetOutput(LAMP_ON_OFF_MASK,0);
#endif
    sBulb.bIsOn = TRUE;

    if (sBulb.u32SyncLockLossCount >= SYNC_LOSS_10MS_TICKS)
	{
    	vAHI_TimerStartRepeat( LAMP_TIMER, sBulb.u16PwmLastValue, sBulb.u16PwmPeriod );
	}
}

/****************************************************************************
 *
 * NAME:            LAMP_vOff
 *
 * DESCRIPTION:     Turns the lamp off
 *
 ****************************************************************************/
PUBLIC void DriverBulb_vOff(void)
{
	uint32 u32Ints;

	MICRO_DISABLE_AND_SAVE_INTERRUPTS(u32Ints);
#ifndef ENABLE_TRACE_COMPARATOR
	vAHI_DioSetOutput(0, LAMP_ON_OFF_MASK);
#endif
	sBulb.bIsOn = FALSE;
    vAHI_TimerStartRepeat(LAMP_TIMER,0,sBulb.u16PwmPeriod);
    MICRO_RESTORE_INTERRUPTS(u32Ints);

}

/****************************************************************************
 *
 * NAMES:           DriverBulb_bOn
 *
 * DESCRIPTION:		Access functions for Monitored Lamp Parmeters
 *
 * RETURNS:
 * Lamp state, Bus Voltage (Volts), Chip Temperature (Degrees C)
 *
 ****************************************************************************/
PUBLIC bool_t DriverBulb_bOn(void)
{
	return (sBulb.bIsOn);
}

/****************************************************************************
 *
 * NAMES:           DriverBulb_vTick
 *
 * DESCRIPTION:		Accepts 10 ms Ticks from higher layer for timing
 *                  Implements a sync-loss detector by incrementing
 *                  a counter that is reset by the sync pulses so under
 *                  sync conditions this counter will remain below a threshold
 *                  If we lose sync the counter exceeds this and we reset
 *                  the PWM to either last value (On) or GND (off)
 *
 ****************************************************************************/

PUBLIC void DriverBulb_vTick(void)
{
	static uint32 u32RestartFlag;

	if (sBulb.u32SyncLockLossCount <SYNC_LOSS_10MS_TICKS)
	{
		sBulb.u32SyncLockLossCount++;
		u32RestartFlag = 0;
	}
	else
	{
		if (u32RestartFlag==0)
		{
			sBulb.u16PwmPeriod = LAMP_TIMER_CLK_FREQ / LAMP_TIMER_PWM_FREQ;
			sBulb.bIsOn ? DriverBulb_vOn() : DriverBulb_vOff();
			/* stop the sync timer and the reset */
			vAHI_TimerStop(PWM_SYNC_TIMER);
			*(uint32 *)PWM_SYNC_TIMER_CTRL |= (1 <<5);
			/* make sure comparator interrupt is enabled so we can re-acquire lock */
			*(uint32 *)SYSCON_SYS_IM |= E_AHI_SYSCTRL_COMP0_MASK;

			u32RestartFlag = -1;
		}
	}

}
/****************************************************************************
 *
 * NAME:			DriverBulb_i16Analogue
 *
 * DESCRIPTION:     Retained for upper layer compatibility
 *                  The Synchronous driver doesn't use bus voltage
 *                  so always returns zero. However once called it indicates
 *                  the analogue peripheral are up & running so it's safe
 *                  to enable the comparator for synch pulse detection
 *
 * PARAMETERS:      Name	     RW  Usage
 *                  u8Adc        R   Identity of the ADC suppling the data
 *                  u16AdcRead   R   Value of last conversion
 *
 * RETURNS:         0
 *
 ****************************************************************************/
PUBLIC int16 DriverBulb_i16Analogue(uint8 u8Adc, uint16 u16AdcRead)
{
	static bool_t bFirstCall = TRUE;

	if (bFirstCall == TRUE)
	{
		bFirstCall = FALSE;
		vAHI_ComparatorEnable(E_AHI_AP_COMPARATOR_1, E_AHI_COMP_HYSTERESIS_40MV, E_AHI_COMP_SEL_BANDGAP);
		vAHI_ComparatorLowPowerMode(FALSE);
		vAHI_ComparatorIntEnable(E_AHI_AP_COMPARATOR_1,TRUE,TRUE);
		*(uint32 *)ADDR_AP_COMPCTRL  &= 0xfffffffb;	/* clear bit 2 Ref Ticket [lpsw3219] */
	}
	return (0);
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

/****************************************************************************
 *
 * NAME:			vCbSystemController
 *
 * DESCRIPTION:     Call-back Interrupt service routine.
 *                  Times the duration between successive interrupts
 *                  for use as the frequency multiplier for the
 *                  phase-locked loop
 *
 ****************************************************************************/

PRIVATE void vCbSystemController(uint32 u32DeviceId, uint32 u32ItemBitmap)
{
	uint32 * const pu32DioOutputRegister   = (uint32 *) GPIO_DOUT_REG;
	uint32 * const pu32SysConWakeEventType = (uint32 *) SYSCON_WK_ET;

	static uint32 u32Toggle = 0;

	uint32 u16Timer2RiseCount;

	if (  (u32ItemBitmap & TX_RX_DIO_MASK) !=0 ) /*check if any DIO events occurred */
	{
		*pu32SysConWakeEventType = (*(uint32 *) GPIO_DIN_REG) & 0x0c;

		if (*pu32SysConWakeEventType ==0)
		{
			*pu32DioOutputRegister |= CURRENT_BLEED_DIO_MASK;
		}
		else
		{
			*pu32DioOutputRegister &= ~CURRENT_BLEED_DIO_MASK;
		}
	}

	if ( (u32ItemBitmap & E_AHI_SYSCTRL_COMP0_MASK) !=0) /* check if the comparator has fired */
	{
		/* disable the interrupts for until re-enabled in future       */
		 TRACE_COMP_IE_OFF;
		 *(uint32 *)SYSCON_SYS_IM &= ~E_AHI_SYSCTRL_COMP0_MASK;

        /* optionally output comparator switching events on the ON DIO */
#ifdef ENABLE_TRACE_COMPARATOR
		u32Toggle ^= 0x01UL;
		u32Toggle ? SYSCON_ISR_ASSERT : SYSCON_ISR_NEGATE;
#endif
		sBulb.u32SyncLockLossCount = 0;
		sBulb.u16PwmPeriod = u16AHI_TimerReadCount(PWM_SYNC_TIMER);

		/* 7/8 of way through a single cycle schedule the comparator interrupt re-enabling */

		u16Timer2RiseCount = (sBulb.u16PwmPeriod >> 4) * 15;
        vAHI_TimerStartSingleShot(PWM_SYNC_TIMER,u16Timer2RiseCount,PWM_SYNC_TIMER_MAX);

		if ((sBulb.u16PwmPeriod > 0) && (sBulb.bIsOn))
		{
			vAHI_TimerStartRepeat(LAMP_TIMER, sBulb.u16PwmLastValue, sBulb.u16PwmPeriod );
		}
	}
}

PRIVATE void vCbTimer2(uint32 u32DeviceId, uint32 u32ItemBitmap)
{
	if ((u32ItemBitmap & E_AHI_TIMER_RISE_MASK) !=0)
	{
		TRACE_COMP_IE_ON;
		/* Clear any comparator interrupts before re-enabling */
		*(uint32 *)SYSCON_SYS_IS |= E_AHI_SYSCTRL_COMP0_MASK;
		*(uint32 *)SYSCON_SYS_IM |= E_AHI_SYSCTRL_COMP0_MASK;
	}
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
