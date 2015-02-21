/****************************************************************************
 *
 * MODULE:             JenNet-IP Border Router
 *
 * COMPONENT:          RADIUS Protocol implementation
 *
 * VERSION:            $Name$
 *
 * REVISION:           $Revision: 31770 $
 *
 * DATED:              $Date: 2011-05-19 17:45:21 +0100 (Thu, 19 May 2011) $
 *
 * STATUS:             $State$
 *
 * AUTHOR:             Matt Redfearn
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

#ifndef RADIUS_H_
#define RADIUS_H_

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/

#include "sec2006.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/


/** Response codes from RADIUS client */
typedef enum
{
	E_RADIUS_OK,					/**< All OK */
	E_RADIUS_ERROR_FAILED,			/**< Generic error code */
	E_RADIUS_ERROR_ACCESS_DENIED,	/**< RADIUS server denied access */
	E_RADIUS_ERROR_TIMEOUT,			/**< Server did not give a timely response */
} teRADIUS_Status;


/** RADIUS Supports multiple authentication types. */
typedef enum
{
	E_RADIUS_AUTH_NONE,				/**< No auth. Not used. */
	E_RADIUS_AUTH_PAP,				/**< Use RADIUS with PAP authentication */
} teRADIUS_Auth;


/** Callback function called when an authorisation response is received from the RADIUS server
 *  \param u64MacAddress			Mac address that has been authorised
 *  \param eStatus					E_RADIUS_OK if device is authorised, E_RADIUS_ACCESS_DENIED if not
 *  \param psCommisioningKey		If \eStatus = E_STATUS_OK,Pointer to location containing the commisioning key.
 *  \return None
 */
typedef void (*tprvRADIUS_AuthoriseCb)(uint64 u64MacAddress, teRADIUS_Status eStatus, tsSecurityKey *psCommisioningKey);


/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/** Initialise RADIUS client.
 *  \param eRADIUS_Auth				Authorisation scheme to use when comminucating with server
 *  \param psServerIP				Pointer to structure containing IPv6 address of RADIUS server
 *  \param psNetworkKey				Pointer to the network key
 *  \param prvRADIUS_AuthoriseCb	Function pointer specifying Callback to call when nodes are authorised
 *  \return E_RADIUS_OK on success
 */
PUBLIC teRADIUS_Status eRADIUS_Init(teRADIUS_Auth eRADIUS_Auth, in6_addr *psServerIP, tsSecurityKey *psNetworkKey, tprvRADIUS_AuthoriseCb prvRADIUS_AuthoriseCb);


/** Function to give a regular tick to the RADIUS client module.
 *  Should be called at 10ms intervals
 *  \return None
 */
PUBLIC void vRADIUS_Tick(void);


/** Authenticate that a Node is allowed to join the network.
 *  \param u64MacAddress			MAC address of the joining node
 *  \return E_RADIUS_OK request was sent successfully
 */
PUBLIC teRADIUS_Status eRADIUS_Request_Authentication(uint64 u64MacAddress);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif /* RADIUS_H_ */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

