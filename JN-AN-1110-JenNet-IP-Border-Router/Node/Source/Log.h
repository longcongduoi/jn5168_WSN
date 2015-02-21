/****************************************************************************
 *
 * MODULE:             JenNet-IP Border Router
 *
 * COMPONENT:          Serial logging functions
 *
 * VERSION:            $Name$
 *
 * REVISION:           $Revision: 31770 $
 *
 * DATED:              $Date: 2011-05-19 17:45:21 +0100 (Thu, 19 May 2011) $
 *
 * STATUS:             $State$
 *
 * AUTHOR:             Thomas Haydon
 *
 * DESCRIPTION:
 *
 * LAST MODIFIED BY:    $Author: nxp29781 $
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

#ifndef LOG_H_
#define LOG_H_

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/

#include <dbg.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/** @{ Define log levels corresponding to Linux syslog */
#define LOG_EMERG	0
#define LOG_ALERT	1
#define LOG_CRIT	2
#define LOG_ERR		3
#define LOG_WARNING	4
#define LOG_NOTICE	5
#define LOG_INFO	6
#define LOG_DEBUG	7
/** @} */


/* Default log level */
#define LOG_LEVEL   LOG_INFO

/* The log macros filter out log messages that have a higher log level than that set in LOG_LEVEL */
#ifdef UART_DEBUG
/* When logging via UART, we don't print the level */
#define vLog_Printf(STREAM, LEVEL, FORMAT, ARGS...)  DBG_vPrintf((STREAM && (LEVEL <= LOG_LEVEL)), FORMAT, ##ARGS)

#else
/* When logging via Serial link to host syslog, send the log level as a char integer at the start of the message */
#define QUOTE(A) #A
#define CHAR(A) QUOTE(\x##A)
#define vLog_Printf(STREAM, LEVEL, FORMAT, ARGS...)  DBG_vPrintf((STREAM && (LEVEL <= LOG_LEVEL)), CHAR(LEVEL) FORMAT, ##ARGS)

#endif


/* Set up trace macros based on flags set by Makefile */

#ifdef DEBUG_BORDER_ROUTER
#define TRACE_BR    TRUE
#else
#define TRACE_BR    FALSE
#endif

#ifdef DEBUG_BORDER_ROUTER_COMMS
#define TRACE_BR_COMMS  TRUE
#else
#define TRACE_BR_COMMS  FALSE
#endif

#ifdef DEBUG_OND
#define TRACE_OND TRUE
#else
#define TRACE_OND FALSE
#endif

#ifdef DEBUG_JENNET_STATE
#define TRACE_JENNET_STATE TRUE
#endif /* DEBUG_JENNET_STATE */

#ifndef DBG_CALLBACKS
#define DBG_CALLBACKS               TRUE
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

PUBLIC void vSL_LogSend(void);
PUBLIC void vSL_LogInit(void);
PUBLIC void vSL_LogFlush(void);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif /* LOG_H_ */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

