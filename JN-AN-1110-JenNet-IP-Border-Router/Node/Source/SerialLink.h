/****************************************************************************
 *
 * MODULE:             JenNet-IP Border Router
 *
 * COMPONENT:          Serial Link to Host
 *
 * VERSION:            $Name:  $
 *
 * REVISION:           $Revision: 1.3 $
 *
 * DATED:              $Date: 2009/03/02 13:14:52 $
 *
 * STATUS:             $State: Exp $
 *
 * AUTHOR:             Lee Mitchell
 *
 * DESCRIPTION:
 *
 * LAST MODIFIED BY:   $Author: lmitch $
 *                     $Modtime: $
 *
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

#ifndef  SERIALLINK_H_INCLUDED
#define  SERIALLINK_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <jendefs.h>
#include <AppHardwareApi.h>
#include "Config.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#define SL_WRITE(DATA)		vUartWrite(HOST_UART, DATA)
#define SL_READ(PDATA)		bUartRead(HOST_UART, PDATA)

/** Macro to send a log message to the host machine
 *  First byte of the message is the level (0-7).
 *  Remainder of message is char buffer containing ascii message
 */
#define SL_LOG(level, message/*, ... */)	\
	{ \
		tsSL_Log sSL_Log = \
		{ \
			.eLevel = level, \
			.au8Message = message, \
		}; \
		/* Repoint putc to function to write into our buffer */ \
		/* DBG_vPrintf(TRUE, message, __VA_ARGS__); */ \
		vSL_WriteMessage(E_SL_MSG_LOG, strlen((char *)sSL_Log.au8Message) + 1, (uint8 *)&sSL_Log); \
	}


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/** Serial link message types */
typedef enum
{
	E_SL_MSG_VERSION_REQUEST	=   0,
	E_SL_MSG_VERSION			=   1,
	E_SL_MSG_IPV4				= 100,
	E_SL_MSG_IPV6				= 101,
	E_SL_MSG_CONFIG				= 102,
	E_SL_MSG_RUN_COORDINATOR	= 103,
	E_SL_MSG_RESET				= 104,
	E_SL_MSG_ADDR   			= 105,
	E_SL_MSG_CONFIG_REQUEST 	= 106,
	E_SL_MSG_SECURITY 			= 107,
	E_SL_MSG_LOG            	= 108,
	E_SL_MSG_PING				= 109,
	E_SL_MSG_PROFILE			= 110,
	E_SL_MSG_RUN_ROUTER			= 111,
	E_SL_MSG_RUN_COMMISSIONING	= 112,
    E_SL_MSG_ACTIVITY_LED       = 113,
    E_SL_MSG_SET_RADIO_FRONTEND = 114,
    E_SL_MSG_ENABLE_DIVERSITY   = 115,
}teSL_MsgType;


/** Structure containing a log message for passing to the host via the serial link */
typedef struct
{
	enum
	{
		E_SL_LOG_EMERG,
		E_SL_LOG_ALERT,
		E_SL_LOG_CRIT,
		E_SL_LOG_ERR,
		E_SL_LOG_WARNING,
		E_SL_LOG_NOTICE,
		E_SL_LOG_INFO,
		E_SL_LOG_DEBUG,
	}__attribute__ ((packed)) eLevel;

	uint8 au8Message[256];
} tsSL_Log;

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

PUBLIC bool bSL_ReadMessage(uint8 *pu8Type, uint16 *pu16Length, uint16 u16MaxLength, uint8 *pu8Message);
PUBLIC void vSL_WriteMessage(uint8 u8Type, uint16 u16Length, uint8 *pu8Data);

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* SERIALLINK_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

