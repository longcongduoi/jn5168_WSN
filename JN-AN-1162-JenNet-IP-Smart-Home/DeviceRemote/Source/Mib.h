/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         MIB Variable Setting Handler
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
#ifndef MIB_H
#define MIB_H

#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/******************************************************************************
**  INCLUDE FILES
*******************************************************************************/
#include "Key.h"
#include "JIP.h"

/******************************************************************************
**  MACROS
*******************************************************************************/

/******************************************************************************
**  CONSTANTS
*******************************************************************************/

/******************************************************************************
**  TYPEDEFS
*******************************************************************************/


/******************************************************************************
**  EXTERNAL VARIABLES
*******************************************************************************/

/******************************************************************************
**  GLOBAL VARIABLES
*******************************************************************************/

/******************************************************************************
**  LOCAL VARIABLES
*******************************************************************************/

/******************************************************************************
**  EXPORTED FUNCTIONS
*******************************************************************************/


PUBLIC void vSetModeMibVar(teTouchKeys eTouchKeys);

PUBLIC void vSetGroupMibVar(MAC_ExtAddr_s *psMacAddr, uint32 u32MibId, uint8 u8VarIdx, uint8 u8Group);
PUBLIC teJIP_Status eBcastGroupMibVar(uint16 u16GroupAddr, uint32 u32MibId, uint8 u8VarIdx, uint8 u8Group);
PUBLIC void vSetMibVarUint16(MAC_ExtAddr_s *psMacAddr, uint32 u32MibId, uint8 u8VarIdx, uint16 u16Val);
PUBLIC void vSetMibVarUint8(MAC_ExtAddr_s *psMacAddr, uint32 u32MibId, uint8 u8VarIdx, uint8 u8Val);

PUBLIC void vSetNodeControlMibVar(uint8 u8CountDown);

PUBLIC void vSetSafetoSleep(void);

#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  // MIB_H

/*****************************************************************************************
**  END OF FILE
*****************************************************************************************/

