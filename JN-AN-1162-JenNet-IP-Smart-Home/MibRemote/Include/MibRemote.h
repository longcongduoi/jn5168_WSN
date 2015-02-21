/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         Remote MIBs, IDs, Indicies and Values
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
#ifndef  MIBREMOTE_H_INCLUDED
#define  MIBREMOTE_H_INCLUDED

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
/* Remote MIBs ********************************************/
/* RemoteConfigGroup MIB - these are the groups the remote transmits to */
#define MIB_ID_REMOTE_CONFIG_GROUP				0xfffffe25
#define VAR_IX_REMOTE_CONFIG_GROUP_COUNT				 0
#define VAR_IX_REMOTE_CONFIG_GROUP_FINISH				 1
#define VAR_IX_REMOTE_CONFIG_GROUP_ADDR_0				 2
#define VAR_IX_REMOTE_CONFIG_GROUP_ADDR_1				 3
#define VAR_IX_REMOTE_CONFIG_GROUP_ADDR_2				 4
#define VAR_IX_REMOTE_CONFIG_GROUP_ADDR_3				 5
#define VAR_IX_REMOTE_CONFIG_GROUP_ADDR_4				 6
#define VAR_IX_REMOTE_CONFIG_GROUP_ADDR_5				 7
#define VAR_IX_REMOTE_CONFIG_GROUP_ADDR_6				 8
#define VAR_IX_REMOTE_CONFIG_GROUP_ADDR_7				 9

#ifdef MK_VAR_VAL_REMOTE_CONFIG_GROUP_COUNT
#define VAR_VAL_REMOTE_CONFIG_GROUP_COUNT MK_VAR_VAL_REMOTE_CONFIG_GROUP_COUNT
#else
#define VAR_VAL_REMOTE_CONFIG_GROUP_COUNT 5
#endif

#if defined __cplusplus
}
#endif

#endif  /* MIBREMOTE_H_INCLUDED */
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
