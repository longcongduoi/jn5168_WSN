/****************************************************************************
 *
 * MODULE:             JenNet-IP Border Router
 *
 * COMPONENT:          Queue implementation
 *
 * VERSION:            $Name:  $
 *
 * REVISION:           $Revision: 1.2 $
 *
 * DATED:              $Date: 2008/10/22 09:53:07 $
 *
 * STATUS:             $State: Exp $
 *
 * AUTHOR:             Lee Mitchell
 *
 * DESCRIPTION:
 *
 * CHANGE HISTORY:
 *
 * LAST MODIFIED BY:   $Author: lmitch $
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

#include <jendefs.h>
#include "Queue.h"

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

/****************************************************************************
 *
 * NAME:       vQueue_Init
 *
 * DESCRIPTION:
 * Initialise a queue data structure
 *
 * PARAMETERS: Name         RW  Usage
 *             psQueue      W   Pointer to queue structure to initialise
 *             pau8Buffer   R   Location of storage
 *             u32Length    R   Length of storage buffer
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vQueue_Init(tsQueue *psQueue, uint8 *pau8Buffer, uint32 u32Length)
{
	/* Initialise the event queue */
	psQueue->u32ReadPtr = 0;
	psQueue->u32WritePtr = 0;
	psQueue->u32Length = u32Length;
	psQueue->pau8Buffer = pau8Buffer;
}


/****************************************************************************
 *
 * NAME:       vQueue_Flush
 *
 * DESCRIPTION:
 * Flush all data from queue
 *
 * PARAMETERS: Name         RW  Usage
 *             psQueue      W   Pointer to queue structure to flush
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vQueue_Flush(tsQueue *psQueue)
{

	/* Initialise the event queue */
	psQueue->u32ReadPtr = 0;
	psQueue->u32WritePtr = 0;

}


/****************************************************************************
 *
 * NAME:       bPPP_QueueWrite
 *
 * DESCRIPTION:
 * Write a byte of data to the queue
 *
 * PARAMETERS: Name         RW  Usage
 *             psQueue      RW  Pointer to queue structure to add data to
 *             u8Item       R   byte of data to put into queue
 *
 * RETURNS:
 * TRUE is data added successfully, FALSE otherwise
 *
 ****************************************************************************/
PUBLIC bool bQueue_Write(tsQueue *psQueue, uint8 u8Item)
{

	/* Make a copy of the write pointer */
	uint32 u32NewWritePtr = psQueue->u32WritePtr;

	u32NewWritePtr++;
	if(u32NewWritePtr == psQueue->u32Length)
	{
		/* Check if the write location has reached the end of the buffer */
		u32NewWritePtr = 0;
	}

	/* If new incremented pointer is same as read pointer, queue is full */
	if(u32NewWritePtr == psQueue->u32ReadPtr)
	{
		return(FALSE);
	}

	psQueue->pau8Buffer[psQueue->u32WritePtr] = u8Item;	/* Add item to queue */
	psQueue->u32WritePtr = u32NewWritePtr;				/* Write new pointer */

	return(TRUE);
}


/****************************************************************************
 *
 * NAME:       bPPP_QueueRead
 *
 * DESCRIPTION:
 * Read a byte of data from the queue
 *
 * PARAMETERS: Name         RW  Usage
 *             psQueue      RW  Pointer to queue structure to get data from
 *             u8Item       W   Location to store char of data retrieved from queue
 *
 * RETURNS:
 * TRUE is data retrieved successfully, FALSE otherwise
 *
 ****************************************************************************/
PUBLIC bool bQueue_Read(tsQueue *psQueue, uint8 *pu8Item)
{

	/* If pointers are same, nothing in the queue */
	if(psQueue->u32ReadPtr == psQueue->u32WritePtr)
	{
		return(FALSE);
	}

	/* Read an event from the queue */
	*pu8Item = psQueue->pau8Buffer[psQueue->u32ReadPtr++];

	if(psQueue->u32ReadPtr == psQueue->u32Length)
	{
		/* Check if the read location has reached the end of the buffer */
		psQueue->u32ReadPtr = 0;
	}

	return(TRUE);

}


/****************************************************************************
 *
 * NAME:       bQueue_IsEmpty
 *
 * DESCRIPTION:
 * Determine if the queue is empty
 *
 * PARAMETERS: Name         RW  Usage
 *             psQueue      RW  Pointer to queue structure to check
 *
 * RETURNS:
 * bool:	TRUE if the queue is empty
 *			FALSE if it contains data
 *
 ****************************************************************************/
PUBLIC bool bQueue_IsEmpty(tsQueue *psQueue)
{
	/* If pointers are same, nothing in the queue */
	if(psQueue->u32ReadPtr == psQueue->u32WritePtr)
	{
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}


/****************************************************************************
 *
 * NAME:       bQueue_IsFull
 *
 * DESCRIPTION:
 *	Checks if the queue is full or not
 *
 * PARAMETERS: Name         RW  Usage
 *             psQueue      RW  Pointer to queue structure to check
 *
 * RETURNS:
 * bool:	TRUE if the queue is full
 *			FALSE if it contains data
 *
 ****************************************************************************/
PUBLIC bool bQueue_IsFull(tsQueue *psQueue)
{

	/* Queue can only ever hold u32Length -1 bytes max */
	if(u32Queue_GetDepth(psQueue) == psQueue->u32Length - 1)
	{
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}

}


/****************************************************************************
 *
 * NAME:       u32Queue_GetDepth
 *
 * DESCRIPTION:
 *	Returns the number of bytes in the queue
 *
 * PARAMETERS: Name         RW  Usage
 *             psQueue      RW  Pointer to queue structure to check
 *
 * RETURNS:
 *	uint32:		Number of bytes in the queue
 *
 ****************************************************************************/
PUBLIC uint32 u32Queue_GetDepth(tsQueue *psQueue)
{

	/* If pointers are same, nothing in the queue */
	if(psQueue->u32ReadPtr == psQueue->u32WritePtr)
	{
		return(0);
	}

	/* Check distance between read & write locations, handling wrap. */
	if(psQueue->u32WritePtr > psQueue->u32ReadPtr)
	{
		return(psQueue->u32WritePtr - psQueue->u32ReadPtr);
	}
	else
	{
		return(psQueue->u32Length - (psQueue->u32ReadPtr - psQueue->u32WritePtr));
	}
}


/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
