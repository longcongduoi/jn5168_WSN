/****************************************************************************
 *
 * MODULE:             JenNet-IP Border Router
 *
 * COMPONENT:          Serial Link to Host
 *
 * VERSION:            $Name:  $
 *
 * REVISION:           $Revision: 1.2 $
 *
 * DATED:              $Date: 2008/12/18 14:24:54 $
 *
 * STATUS:             $State: Exp $
 *
 * AUTHOR:             Lee Mitchell
 *
 * DESCRIPTION:
 *
 *
 * LAST MODIFIED BY:   $Author: lmitch $
 *                     $Modtime: $
 *
 ****************************************************************************
 *
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
 *
 ***************************************************************************/

#ifdef DEBUG_SERIAL_LINK
#define DEBUG_SL            TRUE
#else
#define DEBUG_SL            FALSE
#endif

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <jendefs.h>
#include <string.h>
#include <dbg.h>

#include "SerialLink.h"
#include "UartBuffered.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#define SL_START_CHAR	0x01
#define SL_ESC_CHAR		0x02
#define SL_END_CHAR		0x03

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/** Enumerated list of states for receive state machine */
typedef enum
{
	E_STATE_RX_WAIT_START,
	E_STATE_RX_WAIT_TYPE,
	E_STATE_RX_WAIT_LENMSB,
	E_STATE_RX_WAIT_LENLSB,
	E_STATE_RX_WAIT_CRC,
	E_STATE_RX_WAIT_DATA,
}teSL_RxState;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

PRIVATE uint8 u8SL_CalculateCRC(uint8 u8Type, uint16 u16Length, uint8 *pu8Data);

PRIVATE void vSL_TxByte(bool bSpecialCharacter, uint8 u8Data);
PRIVATE bool bSL_RxByte(uint8 *pu8Data);

PRIVATE void vLogInit(void);
PRIVATE void vLogPutch(char c);
PRIVATE void vLogFlush(void);
PRIVATE void vLogAssert(void);


/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

uint8	au8LogBuffer[256];
uint8	u8LogStart = 0;
uint8	u8LogEnd   = 0;
bool_t	bLogging = FALSE;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: bSL_ReadMessage
 *
 * DESCRIPTION:
 * Attempt to read a complete message from the serial link
 *
 * PARAMETERS  Name                    RW  Usage
 *             pu8Type                 W   Location to store incoming message type
 *             pu16Length              W   Location to store incoming message length
 *             u16MaxLength            R   Length of allocated message buffer
 *             pu8Message              W   Location to store message payload
 *
 * RETURNS:
 * TRUE if a complete valid message has been received
 *
 ****************************************************************************/
PUBLIC bool bSL_ReadMessage(uint8 *pu8Type, uint16 *pu16Length, uint16 u16MaxLength, uint8 *pu8Message)
{

	static teSL_RxState eRxState = E_STATE_RX_WAIT_START;
	static uint8 u8CRC;
	uint8 u8Data;
	static uint16 u16Bytes;
	static bool bInEsc = FALSE;
	int iBytes = 0;

	// Read up to 5 received bytes each call */
	while((iBytes < 5) && bSL_RxByte(&u8Data))
	{
		switch(u8Data)
		{

		case SL_START_CHAR:
			// Reset state machine
			u16Bytes = 0;
			bInEsc = FALSE;
			DBG_vPrintf(DEBUG_SL, "\nRX Start ");
			eRxState = E_STATE_RX_WAIT_TYPE;
			break;

		case SL_ESC_CHAR:
			// Escape next character
			bInEsc = TRUE;
			break;

		case SL_END_CHAR:
			// End message
		    DBG_vPrintf(DEBUG_SL, "\nGot END");
            eRxState = E_STATE_RX_WAIT_START;
            if(*pu16Length < u16MaxLength)
            {
                if(u8CRC == u8SL_CalculateCRC(*pu8Type, *pu16Length, pu8Message))
                {
                	/* CRC matches - valid packet */
                    DBG_vPrintf(DEBUG_SL, "\nbSL_ReadMessage(%d, %d, %02x)", *pu8Type, *pu16Length, u8CRC);
                    return(TRUE);
                }
            }
			DBG_vPrintf(DEBUG_SL, "\nCRC BAD");
			break;

		default:
			if(bInEsc)
			{
				/* Unescape the character */
				u8Data ^= 0x10;
				bInEsc = FALSE;
			}

			switch(eRxState)
			{

			case E_STATE_RX_WAIT_START:
				break;

			case E_STATE_RX_WAIT_TYPE:
			    DBG_vPrintf(DEBUG_SL, "\nType %d", u8Data);
				*pu8Type = u8Data;
				eRxState++;
				break;

			case E_STATE_RX_WAIT_LENMSB:
				*pu16Length = (uint16)u8Data << 8;
				eRxState++;
				break;

			case E_STATE_RX_WAIT_LENLSB:
				*pu16Length += (uint16)u8Data;
				DBG_vPrintf(DEBUG_SL, "\nLength %d", *pu16Length);
				if(*pu16Length > u16MaxLength)
				{
				    DBG_vPrintf(DEBUG_SL, "\nLength > MaxLength");
					eRxState = E_STATE_RX_WAIT_START;
				}
				else
				{
					eRxState++;
				}
				break;

			case E_STATE_RX_WAIT_CRC:
			    DBG_vPrintf(DEBUG_SL, "\nCRC %02x\n", u8Data);
				u8CRC = u8Data;
				eRxState++;
				break;

			case E_STATE_RX_WAIT_DATA:
				if(u16Bytes < *pu16Length)
				{
				    DBG_vPrintf(DEBUG_SL, "%02x ", u8Data);
					pu8Message[u16Bytes++] = u8Data;
				}
				break;
			}
			break;
		}
		iBytes++;
	}
	return(FALSE);
}


/****************************************************************************
 *
 * NAME: vSL_WriteMessage
 *
 * DESCRIPTION:
 * Write message to the serial link
 *
 * PARAMETERS: Name                   RW  Usage
 *             u8Type                 R   Message type
 *             u16Length              R   Message length
 *             pu8Data                R   Message payload
 * RETURNS:
 * void
 ****************************************************************************/
PUBLIC void vSL_WriteMessage(uint8 u8Type, uint16 u16Length, uint8 *pu8Data)
{

	int n;

	DBG_vPrintf(DEBUG_SL, "\nvSL_WriteMessage(%d, %d, %02x)", u8Type, u16Length, u8SL_CalculateCRC(u8Type, u16Length, pu8Data));

	/* Send start character */
	vSL_TxByte(TRUE, SL_START_CHAR);

	/* Send message type */
	vSL_TxByte(FALSE, u8Type);

	/* Send message length */
	vSL_TxByte(FALSE, (u16Length >> 8) & 0xff);
	vSL_TxByte(FALSE, (u16Length >> 0) & 0xff);

	/* Send message checksum */
	vSL_TxByte(FALSE, u8SL_CalculateCRC(u8Type, u16Length, pu8Data));

	/* Send message payload */
	for(n = 0; n < u16Length; n++)
	{
		vSL_TxByte(FALSE, pu8Data[n]);
	}

	/* Send end character */
	vSL_TxByte(TRUE, SL_END_CHAR);

}


/****************************************************************************
 *
 * NAME: vSL_LogSend
 *
 * DESCRIPTION:
 * Send log messages from the log buffer to the host
 *
 * PARAMETERS: 	Name        		RW  Usage
 *
 * RETURNS:
 * void
 ****************************************************************************/
PUBLIC void vSL_LogSend(void)
{
	int n;
	uint8 u8CRC;
	uint8 u8Length;

	while (u8LogEnd - u8LogStart != 0)
	{
		/* Send start character */
		vSL_TxByte(TRUE, SL_START_CHAR);

		/* Send message type */
		vSL_TxByte(FALSE, E_SL_MSG_LOG);

		u8CRC = E_SL_MSG_LOG;

		for (u8Length = 0; au8LogBuffer[(u8LogStart + u8Length) & 0xFF] |= '\0'; u8Length++)
		{
			u8CRC ^= au8LogBuffer[(u8LogStart + u8Length) & 0xFF];
		}

		u8CRC ^= 0;
		u8CRC ^= u8Length;

		/* Send message length */
		vSL_TxByte(FALSE, 0);
		vSL_TxByte(FALSE, u8Length);

		/* Send message checksum */
		vSL_TxByte(FALSE, u8CRC);

		/* Send message payload */
		for(n = 0; n < u8Length; n++)
		{
			vSL_TxByte(FALSE, au8LogBuffer[u8LogStart]);
			u8LogStart++;
		}

		u8LogStart++;

		/* Send end character */
		vSL_TxByte(TRUE, SL_END_CHAR);
	}
}


/****************************************************************************
 *
 * NAME: vSL_LogFlush
 *
 * DESCRIPTION:
 * Flush any log messages from the outoing queue
 *
 * PARAMETERS: 	Name        		RW  Usage
 *
 * RETURNS:
 * void
 ****************************************************************************/
PUBLIC void vSL_LogFlush(void)
{
	/* Empty whatever is currently in the queue to transmit */
	vUartClear(HOST_UART);

	/* Copy log buffer to the UART buffer for transmission */
	vSL_LogSend();

	/* Now we need to flush the UART buffers out to the host */
	vUartFlush(HOST_UART);
}


/****************************************************************************
 *
 * NAME: vSL_LogInit
 *
 * DESCRIPTION:
 * Initialise Serial Link logging
 * Set up DBG module to use serial link functions for its output
 *
 * PARAMETERS: 	Name        		RW  Usage
 *
 * RETURNS:
 * void
 ****************************************************************************/
PUBLIC void vSL_LogInit(void)
{
    tsDBG_FunctionTbl sFunctionTbl;

    sFunctionTbl.prInitHardwareCb	= vLogInit;
    sFunctionTbl.prPutchCb			= vLogPutch;
    sFunctionTbl.prFlushCb 			= vLogFlush;
    sFunctionTbl.prFailedAssertCb	= vLogAssert;

    DBG_vInit(&sFunctionTbl);
}


/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: u8SL_CalculateCRC
 *
 * DESCRIPTION:
 * Calculate CRC of packet
 *
 * PARAMETERS: Name                   RW  Usage
 *             u8Type                 R   Message type
 *             u16Length              R   Message length
 *             pu8Data                R   Message payload
 * RETURNS:
 * CRC of packet
 ****************************************************************************/
PRIVATE uint8 u8SL_CalculateCRC(uint8 u8Type, uint16 u16Length, uint8 *pu8Data)
{

	int n;
	uint8 u8CRC;

	u8CRC = u8Type;
	u8CRC ^= (u16Length >> 8) & 0xff;
	u8CRC ^= (u16Length >> 0) & 0xff;

	for(n = 0; n < u16Length; n++)
	{
		u8CRC ^= pu8Data[n];
	}

	return(u8CRC);
}


/****************************************************************************
 *
 * NAME: vSL_TxByte
 *
 * DESCRIPTION:
 * Send, escaping if required, a byte to the serial link
 *
 * PARAMETERS: 	Name        		RW  Usage
 *              bSpecialCharacter   R   TRUE if this byte should not be escaped
 *              u8Data              R   Character to send
 *
 * RETURNS:
 * void
 ****************************************************************************/
PRIVATE void vSL_TxByte(bool bSpecialCharacter, uint8 u8Data)
{
	if(!bSpecialCharacter && u8Data < 0x10)
	{
		/* Send escape character and escape byte */
		u8Data ^= 0x10;
		SL_WRITE(SL_ESC_CHAR);
	}
	SL_WRITE(u8Data);
}


/****************************************************************************
 *
 * NAME: bSL_RxByte
 *
 * DESCRIPTION:
 * Attempt to read a byte from the UART.
 *
 * PARAMETERS: 	Name        		RW  Usage
 *              pu8Data             W   Location to store byte read
 *
 * RETURNS:
 * TRUE if a byte has been read, FALSE otherwise
 ****************************************************************************/
PRIVATE bool bSL_RxByte(uint8 *pu8Data)
{
	return(SL_READ(pu8Data));
}


/****************************************************************************
 *
 * NAME: vLogInit
 *
 * DESCRIPTION:
 * Callback function for DBG module to initialise output
 *
 * PARAMETERS: 	Name        		RW  Usage
 *
 * RETURNS:
 * void
 ****************************************************************************/
PRIVATE void vLogInit(void)
{

}


/****************************************************************************
 *
 * NAME: vLogPutch
 *
 * DESCRIPTION:
 * Callback function for DBG module to write out characters
 *
 * PARAMETERS: 	Name        		RW  Usage
 *
 * RETURNS:
 * void
 ****************************************************************************/
PRIVATE void vLogPutch(char c)
{
	if (!bLogging)
	{
		/* Handle first character being the log level */
		if (c < 7)
		{
			/* Ensure log level is LOG_INFO or higher */
			au8LogBuffer[u8LogEnd] = c;
		}
		else
		{
			au8LogBuffer[u8LogEnd] = 6;
		}
		u8LogEnd++;
	}

	if (c >= 0x20 && c < 0x7F)
	{
		/* Add ASCII characters to the output buffer */
		au8LogBuffer[u8LogEnd] = c;
		u8LogEnd++;
	}

	bLogging = TRUE;
}


/****************************************************************************
 *
 * NAME: vLogFlush
 *
 * DESCRIPTION:
 * Callback function for DBG module to flush output buffer - used to terminate
 * an entry in the logbuffer
 *
 * PARAMETERS: 	Name        		RW  Usage
 *
 * RETURNS:
 * void
 ****************************************************************************/
PRIVATE void vLogFlush(void)
{
	au8LogBuffer[u8LogEnd] = '\0';
	u8LogEnd++;
	bLogging = FALSE;
}


/****************************************************************************
 *
 * NAME: vLogAssert
 *
 * DESCRIPTION:
 * Callback function for DBG module to assert - not used
 *
 * PARAMETERS: 	Name        		RW  Usage
 *
 * RETURNS:
 * void
 ****************************************************************************/
PRIVATE void vLogAssert(void)
{

}


/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

