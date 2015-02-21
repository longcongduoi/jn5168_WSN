/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         Display - DR1047 Controller Board LCD
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
#include "jendefs.h"
#include "string.h"
#include "AppHardwareApi.h"
#include "LcdDriver.h"
#include "Display.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

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
/***        Local Variables                                               ***/
/****************************************************************************/
PRIVATE char 		acDisplay[DISPLAY_ROWS][DISPLAY_COLS+1];	/* Display text */
PRIVATE bool_t 		 bChanged;									/* Display changed */
PRIVATE bool_t		 bDisplayLogo = FALSE;								/* Draw logo */
PRIVATE uint8       u8AppendRow;
PRIVATE uint8		u8AppendCol;

PRIVATE const uint8 au8NxpBitmap[992] = {
        0x0, 0xfe, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
        0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x6,
        0xc, 0x18, 0x30, 0x60, 0xc0, 0x80, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0xfe, 0xe, 0x1a,
        0x72, 0xc2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
        0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x6,
        0xc, 0x18, 0x70, 0xc0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0xc0, 0x70, 0x18,
        0xc, 0x6, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
        0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0xc2,
        0x72, 0x1a, 0xe, 0x6, 0x2, 0x2, 0x2, 0x2,
        0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
        0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
        0x2, 0x2, 0x2, 0x2, 0x2, 0x6, 0x4, 0xc,
        0x8, 0x18, 0x30, 0x60, 0xc0, 0x0, 0x0, 0x0,
        0x0, 0xff, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0x6, 0xc,
        0x18, 0x30, 0x60, 0xc0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0xff, 0x0, 0x0,
        0x0, 0x1, 0x3, 0xe, 0x38, 0x60, 0xc0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x1, 0x3, 0xe, 0x38, 0x60,
        0x40, 0x60, 0x38, 0xe, 0x3, 0x1, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0xc0, 0x60, 0x38, 0xe, 0x3, 0x1,
        0x0, 0x0, 0x0, 0xe0, 0x30, 0x10, 0x10, 0x10,
        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
        0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x30, 0x20,
        0x60, 0xc0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x1, 0xf, 0xf8, 0x0,
        0x0, 0xff, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xf0,
        0x30, 0x60, 0xc0, 0x80, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x1, 0x3, 0x6, 0xc, 0x18,
        0x30, 0xe0, 0x80, 0x0, 0x0, 0xff, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x7,
        0xc, 0x38, 0xe0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xe0, 0x38,
        0xc, 0x7, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0xff, 0x80, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x80,
        0xe0, 0x3f, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xff, 0x0,
        0x0, 0xff, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xff,
        0x0, 0x0, 0x0, 0x1, 0x7, 0xc, 0x18, 0x30,
        0x60, 0xc0, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x1, 0x3, 0x6, 0x3, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x80, 0xc0, 0x70,
        0x1c, 0x6, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0x6,
        0x1c, 0x70, 0xc0, 0x80, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x1, 0x1, 0x1, 0x1,
        0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
        0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x80, 0xc0, 0x70, 0x1e, 0x3, 0x0,
        0x0, 0xff, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xff,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x3, 0x6, 0xc, 0x18, 0x30, 0x60,
        0xc0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0xc0, 0x70, 0x18, 0xe, 0x3, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0xc0, 0x70, 0x18, 0xe, 0x3,
        0x1, 0x3, 0xe, 0x18, 0x70, 0xc0, 0x80, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x3, 0xe, 0x18, 0x70, 0xc0,
        0x0, 0x0, 0x0, 0xf0, 0x18, 0x8, 0x8, 0x8,
        0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
        0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
        0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0xc, 0x4,
        0x6, 0x2, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x3f, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3f,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x1, 0x3, 0x6, 0xc, 0x18, 0x30, 0x38, 0x2e,
        0x23, 0x21, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x30,
        0x18, 0xe, 0x3, 0x1, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3, 0xe,
        0x18, 0x30, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x21,
        0x23, 0x2e, 0x38, 0x3f, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
        0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};

#ifdef JENNIC_CHIP_FAMILY_JN514x
PRIVATE tsBitmap sNxpLogo = {(uint8 *)au8NxpBitmap, 128, 6};
#endif

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: vDisplay_Init
 *
 * DESCRIPTION:
 * Clears display
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vDisplay_Init(void)
{
	/* Driving LCD ? */
	#if DISPLAY_LCD
	{
		/* Reset */
		vLcdResetDefault();
		/* Clear the display */
		vLcdClear();
		#ifdef JENNIC_CHIP_FAMILY_JN514x
		/* Draw logo if required */
		vLcdWriteBitmap(&sNxpLogo, 0, 0);
		#endif
		/* Refresh the display */
		vLcdRefreshAll();
	}
	#endif
}

/****************************************************************************
 *
 * NAME: vDisplay_Draw
 *
 * DESCRIPTION:
 * Draws display
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vDisplay_Draw (void)
{
	/* Driving LCD ? */
	#if DISPLAY_LCD
	{
		char  acText[DISPLAY_COLS+1];
		uint8 u8Text;
		uint8 u8Row;
		uint8 u8Col;
		uint8 u8ColPix=0;

		/* Clear the display */
		vLcdResetDefault();
		vLcdClear();

		#ifdef JENNIC_CHIP_FAMILY_JN514x
		/* Draw logo if required */
		if (bDisplayLogo) vLcdWriteBitmap(&sNxpLogo, 0, 0);
		#endif

		/* Work through the rows */
		for (u8Row = 0; u8Row < DISPLAY_ROWS; u8Row++)
		{
			/* Start with no text to display */
			u8Text = 0;
			/* Work through columns */
			for (u8Col = 0; u8Col < DISPLAY_COLS+1; u8Col++)
			{
				/* Is the character a space or end of line ? */
				if (acDisplay[u8Row][u8Col] == ' ' || u8Col == DISPLAY_COLS)
				{
					/* Have we got some text to display ? */
					if (u8Text > 0)
					{
						/* Terminate string */
						acText[u8Text] = '\0';
						/* Output to display */
						vLcdWriteText(acText, u8Row, u8ColPix);
						/* Zero text size */
						u8Text = 0;
					}
				}
				/* Character is not a space ? */
				else
				{
					/* First character of output text - note pixel position for display */
					if (u8Text == 0) u8ColPix = u8Col * DISPLAY_COL_PIXELS;
					/* Need to swap some characters for others on the LCD display */
					switch (acDisplay[u8Row][u8Col])
					{
						case '+': acText[u8Text++] = '\\';					  break;
						case '-': acText[u8Text++] = ']';					  break;
						default:  acText[u8Text++] = acDisplay[u8Row][u8Col]; break;
					}
				}
			}
		}
		/* Refresh the display */
		vLcdRefreshAll();
	}
	#endif
}

/****************************************************************************
 *
 * NAME: vDisplay_Clear
 *
 * DESCRIPTION:
 * Clears display
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vDisplay_Clear(void)
{
	uint8 u8Row;
	/* Set all characters to spaces */
	memset (acDisplay, ' ', sizeof (acDisplay));
	/* Loop through rows setting last character to terminator */
	for (u8Row = 0; u8Row < DISPLAY_ROWS; u8Row++)
	{
		acDisplay[u8Row][DISPLAY_COLS] = '\0';
	}
	/* Display has been changed */
	bChanged = TRUE;
}

/****************************************************************************
 *
 * NAME: vDisplay_ClearRow
 *
 * DESCRIPTION:
 * Clears row
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vDisplay_ClearRow(uint8 u8Row)
{
	/* Set all characters to spaces */
	memset (acDisplay[u8Row], ' ', DISPLAY_COLS);
	/* Ensure termination */
	acDisplay[u8Row][DISPLAY_COLS] = '\0';
	/* Display has been changed */
	bChanged = TRUE;
}

/****************************************************************************
 *
 * NAME: vDisplay_Character
 *
 * DESCRIPTION:
 * Adds string to display
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vDisplay_Character(uint8 u8Row, uint8 u8Col, char cCharacter)
{
	/* Valid row and column ? */
	if (u8Row < DISPLAY_ROWS && u8Col < DISPLAY_COLS)
	{
		/* Copy character */
		acDisplay[u8Row][u8Col] = cCharacter;
	}
	/* Display has been changed */
	bChanged = TRUE;
}

/****************************************************************************
 *
 * NAME: vDisplay_String
 *
 * DESCRIPTION:
 * Adds string to display
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vDisplay_String(uint8 u8Row, uint8 u8Col, char *pcString)
{
	/* Valid row and column ? */
	if (u8Row < DISPLAY_ROWS && u8Col < DISPLAY_COLS)
	{
		/* Loop copying string until end of row or string */
		while (u8Col < DISPLAY_COLS && *pcString != '\0')
		{
			/* Printable character ? */
			if (*pcString >= ' ' && *pcString <= '~')
			{
				/* Copy character */
				acDisplay[u8Row][u8Col] = *pcString;
			}
			/* Not printable */
			else
			{
				/* Substitute a space */
				acDisplay[u8Row][u8Col] = ' ';
			}
			/* Go to next row and character */
			u8Col++;
			pcString++;
		}
	}
	/* Display has been changed */
	bChanged = TRUE;
}

/****************************************************************************
 *
 * NAME: vDisplay_Number
 *
 * DESCRIPTION:
 * Adds number to display
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vDisplay_Number (uint8 	u8Row,
							 uint8 	u8Col,
							 uint8  u8Base,			/**< Number base to use */
							 uint8  u8MinDigits,		/**< Minimum digits */
							 uint32 u32Num)			/**< Number to transmit */
{
	uint8 u8Digits = 0;
	uint8 u8Index  = 10;
	char acNum[11];

	/* Initialise */
	acNum[u8Index] = '\0';

	/* Loop converting data */
	do
	{
		/* Place this digit in the character array */
		acNum[--u8Index] = u32Num % u8Base;
		/* Convert to character */
		if (acNum[u8Index] < 0xa)  acNum[u8Index] += '0';
		else 					   acNum[u8Index] += 'A' - 10;
		/* Increment number of digits */
		u8Digits++;
		/* Divide the value for the next digit */
		u32Num /= u8Base;
	} while ((u32Num != 0 || u8Digits < u8MinDigits) && u8Index != 0);

	/* Put string on display */
	vDisplay_String(u8Row, u8Col, &acNum[u8Index]);
}

/****************************************************************************
 *
 * NAME: vDisplay_Logo
 *
 * DESCRIPTION:
 * Controls display of logo
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vDisplay_Logo(bool_t bLogo)
{
	bDisplayLogo = bLogo;
}

/****************************************************************************
 *
 * NAME: vDisplay_Append
 *
 * DESCRIPTION:
 * Adds string to display
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vDisplay_Append(char cCharacter)
{
	/* Display character */
	vDisplay_Character(u8AppendRow, u8AppendCol, cCharacter);
	/* Increment column */
	u8AppendCol++;
	/* Need to go to new row ? */
	if (u8AppendCol >= DISPLAY_COLS)
	{
		/* Zero column */
		u8AppendCol = 0;
		/* Increment row */
		u8AppendRow++;
		/* Need to rollback to first row ? */
		if (u8AppendRow >= DISPLAY_ROWS-2)
		{
			/* Zero row */
			u8AppendRow = 0;
		}
		/* Clear the new row */
		vDisplay_ClearRow(u8AppendRow);
	}
	/* Draw the display */
	vDisplay_Draw();
}
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
