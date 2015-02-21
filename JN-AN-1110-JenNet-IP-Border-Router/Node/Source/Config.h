/****************************************************************************
 *
 * MODULE:             JenNet-IP Border Router
 *
 * COMPONENT:          Module configuration
 *
 * VERSION:            $Name:  $
 *
 * REVISION:           $Revision: 1.3 $
 *
 * DATED:              $Date: 2008/12/09 10:29:04 $
 *
 * STATUS:             $State: Exp $
 *
 * AUTHOR:             Thomas Haydon
 *
 * DESCRIPTION:
 *
 * CHANGE HISTORY:
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

#ifndef  CONFIG_H_INCLUDED
#define  CONFIG_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/

#include <jendefs.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/** Minimum channel number */
#define CHANNEL_MIN						11
/** Maximum channel number */
#define CHANNEL_MAX						26

/** Bitmap of all available channels */
#define GATEWAY_ALL_CHANNELS_BITMAP     0x7fff800

/** Number of buffers for JIP stack */
#define PACKET_BUFFERS                  3

/** Size of routing table */
#define ROUTE_TABLE_ENTRIES             300

/** Maximum length of name strings */
#define MAX_NAME_LENGTH					16

/** Default UART settings */
#ifndef HOST_BAUD_RATE
#define HOST_BAUD_RATE                  921600
#endif

/** Default baud rate is UART_DEBUG is defined */
#ifndef DBG_BAUD_RATE
#define DBG_BAUD_RATE                   DBG_E_UART_BAUD_RATE_115200
#endif

/** Default uart connected to host */
#ifndef HOST_UART
#define HOST_UART                       E_AHI_UART_0
#endif

/* Set debug uart to the one not connected to host */
#if (HOST_UART == 0)
#define DBG_UART                        DBG_E_UART_1
#else
#define DBG_UART                        DBG_E_UART_0
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* CONFIG_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

