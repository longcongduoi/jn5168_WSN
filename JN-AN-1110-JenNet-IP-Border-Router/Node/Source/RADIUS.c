/****************************************************************************
 *
 * MODULE:             JenNet-IP Border Router
 *
 * COMPONENT:          RADIUS Protocol implementation
 *
 * VERSION:            $Name$
 *
 * REVISION:           $Revision: 31870 $
 *
 * DATED:              $Date: 2011-05-26 12:30:11 +0100 (Thu, 26 May 2011) $
 *
 * STATUS:             $State$
 *
 * AUTHOR:             Matt Redfearn
 *
 * DESCRIPTION:
 *
 * LAST MODIFIED BY:   $Author: nxp29781 $
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

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <6LP.h>

#include "md5.h"
#include "log.h"
#include "RADIUS.h"

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/** Logging streams for RADIUS client */
#ifndef RADIUS_INFO_STREAM
#define RADIUS_INFO_STREAM TRUE
#endif

#ifndef RADIUS_DBG_STREAM
#define RADIUS_DBG_STREAM FALSE
#endif

/* Port number to send RADIUS authentication requests to */
#define RADIUS_AUTHENTICATION_PORT 1812


/** Endianness conversion macros */
#if __BYTE_ORDER == __BIG_ENDIAN
#define htons(a)	(a)
#define htonl(a)	(a)
#define ntohs(a)	(a)
#define ntohl(a)	(a)
#else
#error Little endian not implemented
#endif /* BIG ENDIAN */


/** Vendor ID to use for commissioning key information */
#define IANA_VENDOR_ID_NXP 28137

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/** RADIUS Packet codes */
typedef enum
{
	E_RADIUS_ACCESS_REQUEST 		= 1,
	E_RADIUS_ACCESS_ACCEPT 			= 2,
	E_RADIUS_ACCESS_REJECT   		= 3,
} PACK teRADIUS_Packet_Code;


/** RADIUS Attribute-Value pair types */
typedef enum
{
	E_RADIUS_USER_NAME				= 1,
	E_RADIUS_USER_PASSWORD			= 2,
	E_RADIUS_VENDOR_SPECIFIC		= 26,
	E_RADIUS_802154_COMMISIONING_KEY = 100,
} PACK teRADIUS_AVP_Type;


/** RADIUS Attribute - Value pair structure */
typedef struct
{
	teRADIUS_AVP_Type				eType;
	uint8							u8Length;
	uint8                   		au8Data[0];
} PACK tsRADIUS_AVP;


/** RADIUS Packet structure */
typedef struct
{
	teRADIUS_Packet_Code			eCode;
	uint8							u8PacketIdentifier;
	uint16							u16Length;
	uint8							au8Authenticator[16];
	tsRADIUS_AVP					asRADIUS_Attribute_Value_Pair[0];
} PACK tsRADIUS_Packet;


/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

PRIVATE void vRADIUS_HandleDataEvent(int iSocket,
                              te6LP_DataEvent eEvent,
                              ts6LP_SockAddr *psAddr,
                              uint8 u8AddrLen);

PRIVATE teRADIUS_Status eRADIUS_Send_Access_Request(uint64 u64MacAddress, uint8 *pu8Handle);

PRIVATE teRADIUS_Status eRADIUS_Handle_Incoming_Packet(tsRADIUS_Packet *psPacket);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/** Socket file descriptor */
PRIVATE int iRadiusSocket = -1;

/** Configured RADIUS server address */
PRIVATE ts6LP_SockAddr sServerAddr;

/** Private data structure */
PRIVATE struct
{
	tprvRADIUS_AuthoriseCb 	prvAuthenticateCallback;    /**< Configured callback after authentication */

	tsSecurityKey 			*psNetworkKey;				/**< Network key is used as shared secret with radius server */

	struct
	{
		uint64				u64MacAddress;              /**< Address of node */
		uint8 				u8PacketIdentifier;         /**< Packet identifer to match response */
	} sLastRequest;                                     /**< Details of last request */

#define RADIUS_SERVER_TIMEOUT_TICKS (100 * 1) /* 1s */
	uint8					u8Ticks;                    /**< Countdown timer to time out failed requests */

	enum
	{
		E_RADIUS_CLIENT_INIT,                           /**< RADIUS client needs initialising */
		E_RADIUS_CLIENT_IDLE,                           /**< RADIUS client is idle */
		E_RADIUS_CLIENT_WAITING_RESPONSE,               /**< RADIUS client is busy waiting for a response */
	} eState;                                           /**< State of the radius client */

} sState = { NULL, NULL, { 0ll, 0 }, 0, E_RADIUS_CLIENT_INIT };


/** String to prepend to RADIUS client debug messages */
static const char *pcDebugPrefix = "Radius Client:";

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/


/****************************************************************************
 *
 * NAME: eRADIUS_Init
 *
 * DESCRIPTION:
 * Initialise RADIUS client
 *
 * PARAMETERS  Name                    RW  Usage
 *             eRADIUS_Auth            R   Authentication method to use
 *             psServerIP              R   IPv6 address of RADIUS server
 *             psNetworkKey            R   Pointer to network key - used as shared secret with RADIUS server
 *             prvRADIUS_AuthoriseCb   R   Function to call upon completion of authentication
 *
 * RETURNS:
 * E_RADIUS_OK on success
 *
 ****************************************************************************/
PUBLIC teRADIUS_Status eRADIUS_Init(teRADIUS_Auth eRADIUS_Auth, in6_addr *psServerIP,
		                            tsSecurityKey *psNetworkKey,
		                            tprvRADIUS_AuthoriseCb prvRADIUS_AuthoriseCb)
{
	ts6LP_SockAddr sAddr;

	/* Create socket */
	iRadiusSocket = i6LP_Socket(E_6LP_PF_INET6,
							 E_6LP_SOCK_DGRAM,
							 E_6LP_PROTOCOL_ONLY_ONE);

	if (iRadiusSocket == -1)
	{
		/* Error! */
		vLog_Printf(RADIUS_INFO_STREAM, LOG_ERR, "%s unable to create Radius socket\n", pcDebugPrefix);
		return E_RADIUS_ERROR_FAILED;
	}

	/* Set up server address structure */
	memset (&sServerAddr, 0, sizeof(ts6LP_SockAddr));
	sServerAddr.sin6_family = E_6LP_PF_INET6;
	sServerAddr.sin6_port = htons(RADIUS_AUTHENTICATION_PORT);
	sServerAddr.sin6_addr = *psServerIP;

	/* Bind socket to receive incoming packets */
	(void)i6LP_GetOwnDeviceAddress(&sAddr, TRUE);
	sAddr.sin6_port = htons(40000);
	if (i6LP_Bind(iRadiusSocket, &sAddr, sizeof(ts6LP_SockAddr)) == -1)
	{
		/* Error! */
		vLog_Printf(RADIUS_INFO_STREAM, LOG_ERR, "%s unable to bind socket\n", pcDebugPrefix);
		return E_RADIUS_ERROR_FAILED;
	}

	/* Set packet handler for the RADIUS socket */
	if (i6LP_SetSocketDataEventHandler(iRadiusSocket, vRADIUS_HandleDataEvent) == -1)
	{
		/* Error! */
		vLog_Printf(RADIUS_INFO_STREAM, LOG_ERR, "%s unable to set data handler\n", pcDebugPrefix);
		return E_RADIUS_ERROR_FAILED;
	}

	vLog_Printf(RADIUS_INFO_STREAM, LOG_INFO, "\n%s started\n", pcDebugPrefix);

	/* Once socket is set up, configure our private information */
	sState.psNetworkKey = psNetworkKey;
	sState.prvAuthenticateCallback = prvRADIUS_AuthoriseCb;
	sState.u8Ticks = 0;
	sState.eState = E_RADIUS_CLIENT_IDLE;

	return E_RADIUS_OK;
}


/****************************************************************************
 *
 * NAME: vRADIUS_Tick
 *
 * DESCRIPTION:
 * Housekeeping function to be called every 10ms tick
 *
 * PARAMETERS
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vRADIUS_Tick(void)
{
	if (sState.eState != E_RADIUS_CLIENT_INIT)
	{
		/* Make sure we've been initialised */
		if (sState.u8Ticks)
		{
			/* Decrement tick counter. If it reaches 0, the last request timed out */
			if (--sState.u8Ticks == 0)
			{
				DBG_vPrintf(RADIUS_DBG_STREAM, "%s Request timed out\n", pcDebugPrefix);
				sState.eState = E_RADIUS_CLIENT_IDLE;
				if (sState.prvAuthenticateCallback)
				{
					sState.prvAuthenticateCallback(sState.sLastRequest.u64MacAddress,
							                    E_RADIUS_ERROR_TIMEOUT, NULL);
				}
			}
		}
	}
}


/****************************************************************************
 *
 * NAME: eRADIUS_Request_Authentication
 *
 * DESCRIPTION:
 * Ask RADIUS client to authenticate a node
 *
 * PARAMETERS  Name                    RW  Usage
 *             u64MacAddress           R   MAC address of node to authenticate
 *
 * RETURNS:
 * E_RADIUS_OK on success
 *
 ****************************************************************************/
PUBLIC teRADIUS_Status eRADIUS_Request_Authentication(uint64 u64MacAddress)
{
	uint8 u8Handle;

	/* If no transaction in progress */
	if (sState.eState == E_RADIUS_CLIENT_IDLE)
	{
		/* Send access request packet */
		if (eRADIUS_Send_Access_Request(u64MacAddress, &u8Handle) == E_RADIUS_OK)
		{
			/* Set internal state to allow us to check response or timeout the request */
			sState.sLastRequest.u64MacAddress = u64MacAddress;
			sState.sLastRequest.u8PacketIdentifier = u8Handle;
			sState.eState = E_RADIUS_CLIENT_WAITING_RESPONSE;
			sState.u8Ticks = RADIUS_SERVER_TIMEOUT_TICKS;
		}
	}
	return E_RADIUS_OK;
}


/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: vRADIUS_HandleDataEvent
 *
 * DESCRIPTION:
 * Handle incoming event from the socket
 *
 * PARAMETERS  Name                    RW  Usage
 *             iSocket                 R   Socket that the event arrived on
 *             eEvent                  R   Socket event that occured
 *             psAddr                  R   Address that event occured on
 *             u8AddrLen               R   Length of address structure
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vRADIUS_HandleDataEvent(int iSocket,
                              te6LP_DataEvent eEvent,
                              ts6LP_SockAddr *psAddr,
                              uint8 u8AddrLen)
{
	int iDataLen = 0;
	uint8 au8Buffer[256];
	ts6LP_SockAddr sSrcAddr;

	DBG_vPrintf(RADIUS_DBG_STREAM,  "%s data event from = %04hx:%04hx:%04hx:%04hx:%04hx:%04hx:%04hx:%04hx\n", pcDebugPrefix,
	             psAddr->sin6_addr.s6_addr32[0] >> 16,
	             psAddr->sin6_addr.s6_addr32[0] >> 0,
	             psAddr->sin6_addr.s6_addr32[1] >> 16,
	             psAddr->sin6_addr.s6_addr32[1] >> 0,
	             psAddr->sin6_addr.s6_addr32[2] >> 16,
	             psAddr->sin6_addr.s6_addr32[2] >> 0,
	             psAddr->sin6_addr.s6_addr32[3] >> 16,
	             psAddr->sin6_addr.s6_addr32[3] >> 0
	            );


	switch(eEvent)
	{
	case E_DATA_RECEIVED:
		DBG_vPrintf(RADIUS_DBG_STREAM, "%s E_DATA_RECEIVED\n", pcDebugPrefix);

		/* Received a packet - read it out of 6LP into local buffer */
		iDataLen = i6LP_RecvFrom(iSocket,
								 au8Buffer,
								 sizeof(au8Buffer),
								 0,
								 &sSrcAddr,
								 &u8AddrLen);

		if(iDataLen == 0)
		{
			/* Empty packet */
			break;
		}
		DBG_vPrintf(RADIUS_DBG_STREAM,  "%s got %d bytes\n", pcDebugPrefix, iDataLen);

		/* Handle the packet */
		eRADIUS_Handle_Incoming_Packet((tsRADIUS_Packet *)au8Buffer);
		break;

	case E_DATA_SENT:
	case E_DATA_SEND_FAILED:
		/* Event after sending packet - just log the event if enabled */
		DBG_vPrintf(RADIUS_DBG_STREAM, "%s %s\n", pcDebugPrefix,
							 eEvent == E_DATA_SENT ? "E_DATA_SENT" : "E_DATA_SEND_FAILED");
		break;

	default:
		DBG_vPrintf(RADIUS_DBG_STREAM, "%s default\n", pcDebugPrefix);
		/* any others, pass direct to the user handler */
		v6LP_DataEvent(iSocket, eEvent, psAddr, u8AddrLen);
		break;
	}

	return;
}


/****************************************************************************
 *
 * NAME: vNum2String64
 *
 * DESCRIPTION:
 * Convert 64bit integer into string
 *
 * PARAMETERS  Name                    RW  Usage
 *             pcString                W   Output buffer
 *             u64Data                 R   Input integer
 *             u32Size                 R   Size of integer (number of bytes)
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vNum2String64(char *pcString, uint64 u64Data, uint32 u32Size)
{
	uint8 u8Nibble;
	int32 i;

	for (i = (u32Size << 2)-4; i >= 0; i -= 4)
	{
		u8Nibble = (uint8)((u64Data >> i) & 0x0f);
		u8Nibble += 0x30;
		if (u8Nibble > 0x39) u8Nibble += 0x07;
		*pcString = u8Nibble;
		pcString++;
	}
	*pcString = 0;
}


/****************************************************************************
 *
 * NAME: vEncryptPassword
 *
 * DESCRIPTION:
 * Obfuscate user password using md5 hash, according to RADIUS PAP specification.
 *
 * PARAMETERS  Name                    RW  Usage
 *             pu8Authenticator        R   Pointer to 128 bit Authenticator (Random data)
 *             pu8Secret               R   Pointer to 128 bit Shared Secret (JenNet-IP network key)
 *             pu8Password             RW  Pointer to 128 bit password. This is input, and output of the password
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vEncryptPassword(uint8 *pu8Authenticator, uint8 *pu8Secret, uint8 *pu8Password)
{
	md5_state_t sMd5;
	uint8 au8Digest[16];
	uint8 i;

	DBG_vPrintf(RADIUS_DBG_STREAM, "Encrypt Secret: ");
	for (i=0; i < 32; i++) DBG_vPrintf(RADIUS_DBG_STREAM, "%02x", pu8Secret[i]);
	DBG_vPrintf(RADIUS_DBG_STREAM, "\n");

	DBG_vPrintf(RADIUS_DBG_STREAM, "Encrypt Auth: ");
	for (i=0; i < 16; i++) DBG_vPrintf(RADIUS_DBG_STREAM, "%02x", pu8Authenticator[i]);
	DBG_vPrintf(RADIUS_DBG_STREAM, "\n");

	/* Use the md5 engine to hash the secret and authenticator into au8Digest */
	md5_init(&sMd5);
	md5_append(&sMd5, pu8Secret, 32);
	md5_append(&sMd5, pu8Authenticator, 16);
	md5_finish(&sMd5, au8Digest);

	DBG_vPrintf(RADIUS_DBG_STREAM, "Encrypt Digest: ");
	for (i=0; i < 16; i++) DBG_vPrintf(RADIUS_DBG_STREAM, "%02x", au8Digest[i]);
	DBG_vPrintf(RADIUS_DBG_STREAM, "\n");

	/* Now XOR the digest with the password */
	for (i = 0; i < 16; i++)
	{
		au8Digest[i] ^= pu8Password[i];
	}

	DBG_vPrintf(RADIUS_DBG_STREAM, "Encrypt XOR: ");
	for (i=0; i < 16; i++) DBG_vPrintf(RADIUS_DBG_STREAM, "%02x", au8Digest[i]);
	DBG_vPrintf(RADIUS_DBG_STREAM, "\n");

	/* Copy the output */
	memcpy(pu8Password, au8Digest, 16);

	DBG_vPrintf(RADIUS_DBG_STREAM, "Encrypt Output. 0x%02x %02x %02x\n", pu8Password[0], pu8Password[1], pu8Password[2]);
}


/****************************************************************************
 *
 * NAME: eRADIUS_Send_Access_Request
 *
 * DESCRIPTION:
 * Generate and send a RADIUS ACCESS_REQUEST message for the node
 *
 * PARAMETERS  Name                    RW  Usage
 *             u64MacAddress           R   Mac address of node to authenticate
 *             pu8Handle               W   Location to store the handle that was used
 *
 * RETURNS:
 * E_RADIUS_OK on success
 *
 ****************************************************************************/
PRIVATE teRADIUS_Status eRADIUS_Send_Access_Request(uint64 u64MacAddress, uint8 *pu8Handle)
{
	uint8* pu8Buffer;
	tsRADIUS_Packet *psPacket = NULL;
	tsRADIUS_AVP *psAVP;

	DBG_vPrintf(RADIUS_DBG_STREAM, "\n%s send access request\n", pcDebugPrefix);

	/* Get a packet buffer from 6LP */
	if(i6LP_GetDataBuffer(&pu8Buffer) != 0)
	{
		DBG_vPrintf(RADIUS_DBG_STREAM, "buffer is unavailable\n");
		return E_RADIUS_ERROR_FAILED;
	}

	/* Cast the buffer to a RADIUS packet structure */
	psPacket = (tsRADIUS_Packet*)pu8Buffer;

	/* Construct the outgoing access request packet */
	psPacket->eCode = E_RADIUS_ACCESS_REQUEST;
	psPacket->u8PacketIdentifier = *pu8Handle = rand();
	psPacket->u16Length = sizeof(tsRADIUS_Packet);

	{
		/* Generate a random Authenticator */
		int i;
		for (i = 0; i < 16; i++)
		{
			psPacket->au8Authenticator[i] = rand();
		}
	}
	/* Now add the username attribute-value pair */
	psAVP = (tsRADIUS_AVP*)(((uint8*)psPacket) + psPacket->u16Length);
	psAVP->eType = E_RADIUS_USER_NAME;
	vNum2String64((char *)psAVP->au8Data, u64MacAddress, 16);
	psAVP->u8Length = 2 + 16;

	psPacket->u16Length += psAVP->u8Length; /* Add length of username attribute-value pair */

	/* Now add the password attribute-value pair */
	{
		uint8 au8Secret  [32];
		uint64 u64Temp;

		psAVP = (tsRADIUS_AVP*)(((uint8*)psPacket) + psPacket->u16Length);
		psAVP->eType = E_RADIUS_USER_PASSWORD;

		/* The shared secret with the RADIUS server is the Network key in ASCII */
		u64Temp = ((((uint64)sState.psNetworkKey->u32KeyVal_1) << 32) | (((uint64)sState.psNetworkKey->u32KeyVal_2) << 0));
		vNum2String64((char *)&au8Secret[0], u64Temp, 16);
		u64Temp = ((((uint64)sState.psNetworkKey->u32KeyVal_3) << 32) | (((uint64)sState.psNetworkKey->u32KeyVal_4) << 0));
		vNum2String64((char *)&au8Secret[16], u64Temp, 16);

		/* The password for each node is it's MAC address */
		vNum2String64((char *)psAVP->au8Data, u64MacAddress, 16);

		/* Encrypt the password as in the RADIUS PAP specification */
		vEncryptPassword(psPacket->au8Authenticator, au8Secret, psAVP->au8Data);
		psAVP->u8Length = 2 + 16;
	}
	psPacket->u16Length += psAVP->u8Length; /* Add length of password attribute-value pair */

	DBG_vPrintf(RADIUS_DBG_STREAM, "%s send packet\n", pcDebugPrefix);

	/* Send the packet to the configured RADIUS server */
	if(i6LP_SendTo(iRadiusSocket,
					(uint8 *)pu8Buffer,
					psPacket->u16Length,
					0,
					&sServerAddr,
					sizeof(ts6LP_SockAddr)) < 0)
	{
		DBG_vPrintf(RADIUS_DBG_STREAM, "send failed\n");
		return E_RADIUS_ERROR_FAILED;
	}

	vLog_Printf(RADIUS_INFO_STREAM, LOG_DEBUG, "%s Access Request\n", pcDebugPrefix);

	return E_RADIUS_OK;
}


/****************************************************************************
 *
 * NAME: eRADIUS_Handle_Incoming_Packet
 *
 * DESCRIPTION:
 * Handle incoming RADIUS packet from the RADIUS server
 *
 * PARAMETERS  Name                    RW  Usage
 *             psPacket                R   Received packet
 *
 * RETURNS:
 * E_RADIUS_OK on success
 *
 ****************************************************************************/
PRIVATE teRADIUS_Status eRADIUS_Handle_Incoming_Packet(tsRADIUS_Packet *psPacket)
{
	tsRADIUS_AVP *psAVP;
	switch (psPacket->eCode)
	{
	case(E_RADIUS_ACCESS_ACCEPT):
	{
		uint32 u32Position;
		vLog_Printf(RADIUS_INFO_STREAM, LOG_DEBUG, "%s Node is accepted\n", pcDebugPrefix);

		/* We could verify the Authenticator matches here */

		u32Position = sizeof(tsRADIUS_Packet);
		while (u32Position < psPacket->u16Length)
		{
			/* Grab each attribute-value pair in the response */
			psAVP = (tsRADIUS_AVP *)((uint8 *)(psPacket->asRADIUS_Attribute_Value_Pair) + u32Position - sizeof(tsRADIUS_Packet));

			DBG_vPrintf(RADIUS_DBG_STREAM, "%s Got AVP\n", pcDebugPrefix);

			DBG_vPrintf(RADIUS_DBG_STREAM, "%s Attribute pair len : %d\n", pcDebugPrefix, psAVP->u8Length);

			switch (psAVP->eType)
			{
			case (E_RADIUS_VENDOR_SPECIFIC):
			{
				/* We use the vendor specific information to carry the commissioning key of a node */
				uint32 u32Position;
				uint32 u32VendorId;
				memcpy(&u32VendorId, (uint32 *)psAVP->au8Data, sizeof(uint32));
				u32VendorId = ntohl(u32VendorId);

				DBG_vPrintf(RADIUS_DBG_STREAM, "%s Vendor ID: %d\n", pcDebugPrefix, u32VendorId);

				u32Position = 4; /* Skip Vendor ID */
				while ((2 + u32Position) < psAVP->u8Length) /* Add 2 for Vendor specific header */
				{
					tsRADIUS_AVP *psVendorAVP;
					psVendorAVP = (tsRADIUS_AVP *)((uint8 *)(psAVP->au8Data) + u32Position); /* Get pointer to the first Vendor AVP */

					DBG_vPrintf(RADIUS_DBG_STREAM, "%s Vendor attribute pair type: %d\n", pcDebugPrefix, psVendorAVP->eType);
					DBG_vPrintf(RADIUS_DBG_STREAM, "%s Vendor attribute pair len : %d\n", pcDebugPrefix, psVendorAVP->u8Length);

					/* Verify this is an attribute we want to use */
					if ((u32VendorId == IANA_VENDOR_ID_NXP) &&
						(psVendorAVP->eType == E_RADIUS_802154_COMMISIONING_KEY) &&
						(psVendorAVP->u8Length == (2 + 16)))
					{
						DBG_vPrintf(RADIUS_DBG_STREAM, "%s Got commisioning key\n", pcDebugPrefix);
						if (sState.prvAuthenticateCallback)
						{
							/* Generate a tsSecurityKey structure from the incoming commissioning key data */
							tsSecurityKey sSecurityKey;

							sSecurityKey.u32KeyVal_1 = ntohl(((uint32*)(psVendorAVP->au8Data))[0]);
							sSecurityKey.u32KeyVal_2 = ntohl(((uint32*)(psVendorAVP->au8Data))[1]);
							sSecurityKey.u32KeyVal_3 = ntohl(((uint32*)(psVendorAVP->au8Data))[2]);
							sSecurityKey.u32KeyVal_4 = ntohl(((uint32*)(psVendorAVP->au8Data))[3]);

							DBG_vPrintf(RADIUS_DBG_STREAM, "%s key 0x%08x%08x%08x%08x\n", pcDebugPrefix,
									sSecurityKey.u32KeyVal_1, sSecurityKey.u32KeyVal_2,
									sSecurityKey.u32KeyVal_3, sSecurityKey.u32KeyVal_4);

							/* Call the authentication callback */
							sState.prvAuthenticateCallback(sState.sLastRequest.u64MacAddress, E_RADIUS_OK, &sSecurityKey);
						}
					}

					/* Next vendor specific attribute */
					u32Position += psVendorAVP->u8Length;
				}
				break;
			}
			default:
				DBG_vPrintf(RADIUS_DBG_STREAM, "%s Unhandled attribute pair: %d\n", pcDebugPrefix, psAVP->eType);
			}

			/* Next attribute-value pair */
			u32Position += psAVP->u8Length;
		}

		break;
	}
	case(E_RADIUS_ACCESS_REJECT):
		vLog_Printf(RADIUS_INFO_STREAM, LOG_DEBUG, "%s Node is rejected\n", pcDebugPrefix);
		sState.prvAuthenticateCallback(sState.sLastRequest.u64MacAddress, E_RADIUS_ERROR_ACCESS_DENIED, NULL);
		break;

	default:
		break;
	}

	/* Reset state machint o IDLE */
	sState.eState = E_RADIUS_CLIENT_IDLE;
	sState.u8Ticks = 0;

	return E_RADIUS_OK;
}


/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
