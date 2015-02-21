/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         Capacitance Touch Driver
 *
 * Replaces capacitance touch algorithms with DIO buttons and LCD display
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
#include <AppHardwareApi.h>
#include <string.h>
#include <button.h>
#include "dbg.h"
#include "DriverCapTouch.h"
#include "Key.h"
#include "Display.h"


/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define BUTTON_P_MASK 0x80

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/


/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/
PRIVATE void vDisplayScreen();

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE uint8   u8LastRead;
PRIVATE uint8   u8Screen;
PRIVATE uint16 u16StackMode;

/****************************************************************************/
/***        Local Constants                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                      ***/
/****************************************************************************/
PUBLIC teTouchStatus eTouchInit(void)
{
	/* Set pins used by LCD to defaults, input, pullup enabled */
	vAHI_DioSetDirection(0x103, 0);
	vAHI_DioSetPullup(0x103, 0);

	/* Initialise buttons */
	vButtonInitFfd();
	u8LastRead = u8ButtonReadFfd();
	DBG_vPrintf(TRUE, "\nu8LastRead=%x", u8LastRead);

	vDisplay_Init();
	#ifdef JENNIC_CHIP_FAMILY_JN514x
	vDisplay_Logo(TRUE);
	#endif
	vDisplay_String(6,  0, "JenNet-IP Remote");
	vDisplay_String(7,  0, "Starting...");
	vDisplay_Draw();

	return TOUCH_STATUS_OK;
}


PUBLIC teTouchStatus eTouchSleep(void)
{
	return TOUCH_STATUS_OK;
}

PUBLIC teTouchStatus eTouchWake(void)
{
	return TOUCH_STATUS_OK;
}

PUBLIC teTouchStatus eTouchProcess(void)
{
	uint8 u8ThisRead;
	teTouchStatus eStatus = TOUCH_STATUS_OK;
	uint8 u8Changed;
	uint8 u8Pressed;
	uint8 u8Released;

	/* Displaying screen 0 - update to screen 1 */
	if (u8Screen == 0)
	{
		u16StackMode = u16Api_GetStackMode();
		u8Screen = 1;
		vDisplayScreen();
	}

	/* Read buttons */
	u8ThisRead = u8ButtonReadFfd();
	u8Changed  = (u8ThisRead ^ u8LastRead);
	u8Pressed  = (u8ThisRead & u8Changed);
	u8Released = (u8LastRead & u8Changed);

	/* Buttons pressed ? */
	if (u8Pressed != 0)
	{
		/* Button 0 ? */
		if (u8Pressed & BUTTON_0_MASK)
		{
			/* Screen 1 */
			if (u8Screen == 1)
			{
				/* Cycle to last screen */
				u8Screen = 5;
			}
			/* Other screens ? */
			else
			{
				/* Decrement screen */
				u8Screen--;
			}
			/* Draw new screen */
			vDisplayScreen();
		}
		/* Button 1 ? */
		if (u8Pressed & BUTTON_1_MASK)
		{
			/* Which screen ? */
			switch (u8Screen)
			{
				case 1: vCbTouchEventButton(TOUCH_BUTTON_EVENT_PRESSED,E_KEY_ON);  break;
				case 2: vCbTouchEventButton(TOUCH_BUTTON_EVENT_PRESSED,E_KEY_UP);  break;
				case 3: vCbTouchEventButton(TOUCH_BUTTON_EVENT_PRESSED,E_KEY_A);   break;
				case 4: vCbTouchEventButton(TOUCH_BUTTON_EVENT_PRESSED,E_KEY_C);   break;
				case 5: vCbTouchEventButton(TOUCH_BUTTON_EVENT_PRESSED,E_KEY_ALL); break;
			}
		}
		/* Button 2 ? */
		if (u8Pressed & BUTTON_2_MASK)
		{
			/* Which screen ? */
			switch (u8Screen)
			{
				case 1: vCbTouchEventButton(TOUCH_BUTTON_EVENT_PRESSED,E_KEY_OFF);  break;
				case 2: vCbTouchEventButton(TOUCH_BUTTON_EVENT_PRESSED,E_KEY_DOWN); break;
				case 3: vCbTouchEventButton(TOUCH_BUTTON_EVENT_PRESSED,E_KEY_B);    break;
				case 4: vCbTouchEventButton(TOUCH_BUTTON_EVENT_PRESSED,E_KEY_D);    break;
				case 5: vCbTouchEventButton(TOUCH_BUTTON_EVENT_PRESSED,E_KEY_PROG); break;
			}
		}
		/* Button 3 ? */
		if (u8Pressed & BUTTON_3_MASK)
		{
			/* Screen 5 */
			if (u8Screen == 5)
			{
				/* Cycle to first screen */
				u8Screen = 1;
			}
			/* Other screens ? */
			else
			{
				/* Increment screen */
				u8Screen++;
			}
			/* Draw new screen */
			vDisplayScreen();
		}
	}

	/* Buttons released ? */
	if (u8Released != 0)
	{
		/* Button 1 ? */
		if (u8Released & BUTTON_1_MASK)
		{
			/* Which screen ? */
			switch (u8Screen)
			{
				case 1: vCbTouchEventButton(TOUCH_BUTTON_EVENT_RELEASED,E_KEY_ON);  break;
				case 2: vCbTouchEventButton(TOUCH_BUTTON_EVENT_RELEASED,E_KEY_UP);  break;
				case 3: vCbTouchEventButton(TOUCH_BUTTON_EVENT_RELEASED,E_KEY_A);   break;
				case 4: vCbTouchEventButton(TOUCH_BUTTON_EVENT_RELEASED,E_KEY_C);   break;
				case 5: vCbTouchEventButton(TOUCH_BUTTON_EVENT_RELEASED,E_KEY_ALL); break;
			}
		}
		/* Button 2 ? */
		if (u8Released & BUTTON_2_MASK)
		{
			/* Which screen ? */
			switch (u8Screen)
			{
				case 1: vCbTouchEventButton(TOUCH_BUTTON_EVENT_RELEASED,E_KEY_OFF);  break;
				case 2: vCbTouchEventButton(TOUCH_BUTTON_EVENT_RELEASED,E_KEY_DOWN); break;
				case 3: vCbTouchEventButton(TOUCH_BUTTON_EVENT_RELEASED,E_KEY_B);    break;
				case 4: vCbTouchEventButton(TOUCH_BUTTON_EVENT_RELEASED,E_KEY_D);    break;
				case 5: vCbTouchEventButton(TOUCH_BUTTON_EVENT_RELEASED,E_KEY_PROG); break;
			}
		}
		/* All buttons released ? */
		if (u8ThisRead == 0)
		{
			vCbTouchEventButton(TOUCH_BUTTON_EVENT_ALL_RELEASED, E_KEY_NONE);
		}
	}

	/* Note last reading */
	u8LastRead = u8ThisRead;

	return eStatus;
}

PRIVATE void vDisplayScreen()
{
	vDisplay_ClearRow(6);
	vDisplay_ClearRow(7);

	/* Screen 0 - splash screen ? */
	if (u8Screen == 0)
	{
		vDisplay_String(6,  0, "JenNet-IP Remote");
		vDisplay_Draw();
	}
	else
	{
		if (u16StackMode & 0x1)
			vDisplay_String(6, 19, "SA");
		else
			vDisplay_String(6, 19, "GW");

		vDisplay_Number(6, 0, 10, 1, u8Screen);

		switch (u8Screen)
		{
			/* Screen 1 */
			case 1:
			{
				vDisplay_String(6,  2, "Control:");
				vDisplay_String(7,  0, ">");
				vDisplay_String(7,  7, "ON");
				vDisplay_String(7, 12, "OFF");
				vDisplay_String(7, 20, "<");
				vDisplay_Draw();
			}
			break;

			/* Screen 2 */
			case 2:
			{
				vDisplay_String(6,  2, "Control:");
				vDisplay_String(7,  0, ">");
				vDisplay_String(7,  7, "UP");
				vDisplay_String(7, 12, "DWN");
				vDisplay_String(7, 20, "<");
				vDisplay_Draw();
			}
			break;

			/* Screen 3 */
			case 3:
			{
				vDisplay_String(6,  2, "Group:");
				vDisplay_String(7,  0, ">");
				vDisplay_String(7,  7, "A");
				vDisplay_String(7, 13, "B");
				vDisplay_String(7, 20, "<");
				vDisplay_Draw();
			}
			break;

			/* Screen 4 */
			case 4:
			{
				vDisplay_String(6,  2, "Group:");
				vDisplay_String(7,  0, ">");
				vDisplay_String(7,  7, "C");
				vDisplay_String(7, 13, "D");
				vDisplay_String(7, 20, "<");
				vDisplay_Draw();
			}
			break;

			/* Screen 5 */
			case 5:
			{
				vDisplay_String(6,  2, "Other:");
				vDisplay_String(7,  0, ">");
				vDisplay_String(7,  6, "ALL");
				vDisplay_String(7, 12, "PRG");
				vDisplay_String(7, 20, "<");
				vDisplay_Draw();
			}
			break;

		}
	}
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
