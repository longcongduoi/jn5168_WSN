/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         NwkTest MIB - Implementation
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
/* Standard includes */
#include <string.h>
/* SDK includes */
#include <jendefs.h>
/* Hardware includes */
#include <AppHardwareApi.h>
#include <PeripheralRegs.h>
/* Stack includes */
#include <Api.h>
#include <AppApi.h>
#include <JIP.h>
#include <6LP.h>
#include <AccessFunctions.h>
/* JenOS includes */
#include <dbg.h>
#include <dbg_uart.h>
#include <os.h>
#include <pdm.h>
/* Application common includes */
#include "MibNwkTest.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

PUBLIC teJIP_Status eSetMacRetries(uint8 u8Value, void *pvCbData);
PUBLIC void vGetMacRetries(thJIP_Packet hPacket, void *pvCbData);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/

extern PUBLIC uint8  u8LastPktLqi;

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/****************************************************************************/
/***        MIB structure                                                 ***/
/****************************************************************************/

/****************************************************************************/
/***        MIB declaration                                                ***/
/****************************************************************************/

#define DECLARE_MIB
#include "MibNwkTestDef.h"
JIP_START_DECLARE_MIB(NwkTestDef, NwkTest)
JIP_CALLBACK(MacRetries,eSetMacRetries,             vGetMacRetries, NULL)
JIP_CALLBACK(RxLqi,     NULL,                       vGetUint8,      &u8LastPktLqi)
JIP_END_DECLARE_MIB(NwkTest, hNwkTest)

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: MibNwkTest_vInit
 *
 * DESCRIPTION:
 * Initialises data
 *
 ****************************************************************************/
PUBLIC void MibNwkTest_vInit(void)
{
    return;
}

/****************************************************************************
 *
 * NAME: MibNwkTest_vRegister
 *
 * DESCRIPTION:
 * Registers MIB
 *
 ****************************************************************************/
PUBLIC void MibNwkTest_vRegister(void)
{
	teJIP_Status eStatus;

	/* Register MIB */
	eStatus = eJIP_RegisterMib(hNwkTest);
	/* Debug */
	DBG_vPrintf(DBG_MIB_NWK_TEST, "\nMibNwkTest_vRegister()");
	DBG_vPrintf(DBG_MIB_NWK_TEST, "\neJIP_RegisterMib(NwkTest)=%d", eStatus);
	return;
}


PUBLIC teJIP_Status eSetMacRetries(uint8 u8Value, void *pvCbData)
{
    void            *pvMac;
    MAC_Pib_s       *psPib;

    /* Get MAC and PIB pointers */
    pvMac = pvAppApiGetMacHandle();
    psPib = MAC_psPibGetHandle(pvMac);

    /* Set MAC retries */
    psPib->u8MaxFrameRetries = u8Value;

    DBG_vPrintf(DBG_MIB_NWK_TEST, "\nvSetMacRetries=%d", u8Value);

    return E_JIP_OK;
}


PUBLIC void vGetMacRetries(thJIP_Packet hPacket, void *pvCbData)
{
    void            *pvMac;
    MAC_Pib_s       *psPib;
    uint8           u8MacRetries;
    teJIP_Status    eStatus;

    /* Get MAC and PIB pointers */
    pvMac = pvAppApiGetMacHandle();
    psPib = MAC_psPibGetHandle(pvMac);

    /* Get MAC retries */
    u8MacRetries = psPib->u8MaxFrameRetries;

    eStatus = eJIP_PacketAddData(hPacket, &u8MacRetries, sizeof(uint8), 0);

    DBG_vPrintf(DBG_MIB_NWK_TEST, "\nvGetMacRetries=%d, status=%d", u8MacRetries, eStatus);
}


/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
