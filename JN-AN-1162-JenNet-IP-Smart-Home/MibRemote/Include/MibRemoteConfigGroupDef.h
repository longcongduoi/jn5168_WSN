/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         Remote Config Group MIB - Definition
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
/***        Macro Definitions                                             ***/
/****************************************************************************/
#undef MIB_HEADER
#define MIB_HEADER "MibRemoteConfigGroupDef.h"

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
/* Stack includes */
#include <jip_define_mib.h>
/* Application common includes */
#include "MibRemote.h"

/****************************************************************************/
/***        MIB definition                                                ***/
/****************************************************************************/
START_DEFINE_MIB(MIB_ID_REMOTE_CONFIG_GROUP, RemoteConfigGroupDef)
/*         ID 					   		 	  Type   Name    Disp  Flags Access 	   			 Cache Security */
DEFINE_VAR(VAR_IX_REMOTE_CONFIG_GROUP_COUNT,  UINT8, Count,	 NULL, 0,    (READ),  				 NONE, NONE)
DEFINE_VAR(VAR_IX_REMOTE_CONFIG_GROUP_FINISH, UINT8, Finish, NULL, 0,    (READ | WRITE | TRAP),  NONE, NONE)
#if VAR_VAL_REMOTE_CONFIG_GROUP_COUNT > 0
DEFINE_VAR(VAR_IX_REMOTE_CONFIG_GROUP_ADDR_0,  BLOB, Addr0,  NULL, 0,    (READ | WRITE | TRAP),  NONE, NONE)
#if VAR_VAL_REMOTE_CONFIG_GROUP_COUNT > 1
DEFINE_VAR(VAR_IX_REMOTE_CONFIG_GROUP_ADDR_1,  BLOB, Addr1,  NULL, 0,    (READ | WRITE | TRAP),  NONE, NONE)
#if VAR_VAL_REMOTE_CONFIG_GROUP_COUNT > 2
DEFINE_VAR(VAR_IX_REMOTE_CONFIG_GROUP_ADDR_2,  BLOB, Addr2,  NULL, 0,    (READ | WRITE | TRAP),  NONE, NONE)
#if VAR_VAL_REMOTE_CONFIG_GROUP_COUNT > 3
DEFINE_VAR(VAR_IX_REMOTE_CONFIG_GROUP_ADDR_3,  BLOB, Addr3,  NULL, 0,    (READ | WRITE | TRAP),  NONE, NONE)
#if VAR_VAL_REMOTE_CONFIG_GROUP_COUNT > 4
DEFINE_VAR(VAR_IX_REMOTE_CONFIG_GROUP_ADDR_4,  BLOB, Addr4,  NULL, 0,    (READ | WRITE | TRAP),  NONE, NONE)
#if VAR_VAL_REMOTE_CONFIG_GROUP_COUNT > 5
DEFINE_VAR(VAR_IX_REMOTE_CONFIG_GROUP_ADDR_5,  BLOB, Addr5,  NULL, 0,    (READ | WRITE | TRAP),  NONE, NONE)
#if VAR_VAL_REMOTE_CONFIG_GROUP_COUNT > 6
DEFINE_VAR(VAR_IX_REMOTE_CONFIG_GROUP_ADDR_6,  BLOB, Addr6,  NULL, 0,    (READ | WRITE | TRAP),  NONE, NONE)
#if VAR_VAL_REMOTE_CONFIG_GROUP_COUNT > 7
DEFINE_VAR(VAR_IX_REMOTE_CONFIG_GROUP_ADDR_7,  BLOB, Addr7,  NULL, 0,    (READ | WRITE | TRAP),  NONE, NONE)
#if VAR_VAL_REMOTE_CONFIG_GROUP_COUNT > 8
#error Only a maximum of 8 remote groups can be configured by simply updating VAR_VAL_REMOTE_CONFIG_GROUP_COUNT
#endif /* >8 */
#endif /* >7 */
#endif /* >6 */
#endif /* >5 */
#endif /* >4 */
#endif /* >3 */
#endif /* >2 */
#endif /* >1 */
#endif /* >0 */
END_DEFINE_MIB(RemoteConfigGroupDef)

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
