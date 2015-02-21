/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         JIP Callback Functions
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


#include "dbg.h"
#include "dbg_uart.h"



#include "jip.h"


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

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/


/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/



/****************************************************************************
 *
 * NAME: vJIP_Remote_GetResponse
 *
 * DESCRIPTION:
 * Callback to handle response to remote Get request.
 *
 * PARAMETERS: Name                     RW  Usage
 *             *psAddr
 *             u8Handle
 *             u8ModuleIndex
 *             u8VarIndex
 *             eStatus
 *             eVarType                 R   Type of variable returned
 *             *pvVal                   R   Pointer to returned data
 *             u32ValSize               R   Size of returned data
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vJIP_Remote_GetResponse(ts6LP_SockAddr *psAddr,
                                    uint8 u8Handle,
                                    uint8 u8ModuleIndex,
                                    uint8 u8VarIndex,
                                    teJIP_Status eStatus,
                                    teJIP_VarType eVarType,
                                    const void *pvVal,
                                    uint32 u32ValSize)
{
//    DBG_vPrintf(DBG_CALLBACKS, "%s(%s)\n", __FUNCTION__, apcDataTypes[eVarType]);

}


/****************************************************************************
 *
 * NAME: vJIP_Remote_TrapResponse
 *
 * DESCRIPTION:
 * Callback to handle response to remote Trap request.
 *
 * PARAMETERS: Name                     RW  Usage
 *             *psAddr
 *             u8Handle
 *             u8ModuleIndex
 *             u8VarIndex
 *             eStatus
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vJIP_Remote_TrapResponse(ts6LP_SockAddr *psAddr,
                                     uint8 u8Handle,
                                     uint8 u8ModuleIndex,
                                     uint8 u8VarIndex,
                                     teJIP_Status eStatus)
{
//    DBG_vPrintf(DBG_CALLBACKS, "%s()\n", __FUNCTION__);
}


/****************************************************************************
 *
 * NAME: vJIP_Remote_TrapNotify
 *
 * DESCRIPTION:
 * Callback to handle notification of remote Trap event.
 *
 * PARAMETERS: Name                     RW  Usage
 *             *psAddr
 *             u8Handle
 *             u8ModuleIndex
 *             u8VarIndex
 *             eVarType                 R   Type of variable returned
 *             *pvVal                   R   Pointer to returned data
 *             u32ValSize               R   Size of returned data
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vJIP_Remote_TrapNotify(ts6LP_SockAddr *psAddr,
                                      uint8 u8Handle,
                                      uint8 u8ModuleIndex,
                                      uint8 u8VarIndex,
                                      teJIP_Status eStatus,
                                      teJIP_VarType eVarType,
                                      void *pvVal,
                                      uint32 u32ValSize)
{
//    DBG_vPrintf(DBG_CALLBACKS, "%s()\n", __FUNCTION__);
}





/****************************************************************************
 *
 * NAME: vJIP_Remote_DataSent
 *
 * DESCRIPTION:
 *
 *
 * PARAMETERS: Name                     RW  Usage
 *             *psAddr
 *             u8Handle
 *             eStatus                  R   Response status
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vJIP_Remote_DataSent(ts6LP_SockAddr *psAddr,
                                 teJIP_Status eStatus)
{
//    DBG_vPrintf(1, "R");


}


/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
