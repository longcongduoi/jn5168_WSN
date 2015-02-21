/****************************************************************************
 *
 * MODULE:             JenNet-IP Border Router
 *
 * COMPONENT:          Buffered, interrupt driven serial I/O
 *
 * VERSION:            $Name:  $
 *
 * REVISION:           $Revision: 1.2 $
 *
 * DATED:              $Date: 2009/01/22 14:02:35 $
 *
 * STATUS:             $State: Exp $
 *
 * AUTHOR:             Lee Mitchell
 *
 * DESCRIPTION:
 * Just some simple common uart functions
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
#include <math.h>
#include <AppHardwareApi.h>
#include <dbg.h>

#include "UartBuffered.h"
#include "Queue.h"


/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#define UART_NUM_UARTS  2

#ifdef DEBUG_UART_DRIVER
#define DEBUG_UART_BUFFERED TRUE
#else
#define DEBUG_UART_BUFFERED FALSE
#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

PUBLIC void vUartSetBaudRate(uint8 u8Uart, uint32 u32BaudRate);
PRIVATE void vUartISR(uint32 u32DeviceId, uint32 u32ItemBitmap);
PRIVATE void vUartTxIsr(uint8 u8Uart);
PRIVATE void vUartRxIsr(uint8 u8Uart);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

/** Receive and transmit queues for each UART */
PRIVATE tsQueue		asUart_TxQueue[UART_NUM_UARTS];
PRIVATE tsQueue		asUart_RxQueue[UART_NUM_UARTS];

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME:       vUartInit
 *
 * DESCRIPTION:
 * Initialises the specified UART.
 *
 * PARAMETERS:      Name            RW  Usage
 *                  u8Uart			R   UART to initialise, eg, E_AHI_UART_0
 *                  u8BaudRate      R   Baudrate to use (bps eg 921600)
 *                  psFifo          R   Pointer to a tsUartFifo struct holding
 *										all data for this UART
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vUartInit(uint8 u8Uart, uint32 u32BaudRate, uint8 *pu8TxBuffer, uint32 u32TxBufferLen, uint8 *pu8RxBuffer, uint32 u32RxBufferLen)
{
	/* Initialise Tx & Rx Queue's */
	vQueue_Init(&asUart_TxQueue[u8Uart], pu8TxBuffer, u32TxBufferLen);
	vQueue_Init(&asUart_RxQueue[u8Uart], pu8RxBuffer, u32RxBufferLen);

	/* Configure the selected Uart */
    vAHI_UartEnable(u8Uart);

    vAHI_UartReset(u8Uart, TRUE, TRUE);
    vAHI_UartReset(u8Uart, FALSE, FALSE);

	vUartSetBaudRate(u8Uart, u32BaudRate);

	/* install interrupt service callback */
    if(u8Uart == E_AHI_UART_0)
    {
		vAHI_Uart0RegisterCallback((void*)vUartISR);
	}
    else
    {
		vAHI_Uart1RegisterCallback((void*)vUartISR);
	}

	/* Enable TX Fifo empty and Rx data interrupts */
    vAHI_UartSetInterrupt(u8Uart, FALSE, FALSE, TRUE, TRUE, E_AHI_UART_FIFO_LEVEL_1);
}


/****************************************************************************
 *
 * NAME:       vUartSetBaudRate
 *
 * DESCRIPTION:
 * Sets the baud rate for the specified uart
 *
 * PARAMETERS:      Name            RW  Usage
 *                  u8Uart			R   UART to initialise, eg, E_AHI_UART_0
 *                  u8BaudRate      R   Baudrate to use (bps eg 921600)
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vUartSetBaudRate(uint8 u8Uart, uint32 u32BaudRate)
{
    uint16 u16Divisor;
    uint32 u32Remainder;
    uint8  u8ClocksPerBit = 16;

#if (defined ENABLE_ADVANCED_BAUD_SELECTION)
    /* Defining ENABLE_ADVANCED_BAUD_SELECTION in the Makefile
     * enables this code which searches for a clocks per bit setting
     * that gets closest to the configured rate.
     */
    uint32 u32CalcBaudRate = 0;
    int32  i32BaudError = 0x7FFFFFFF;

    DBG_vPrintf(DEBUG_UART_BUFFERED, "Config uart=%d, baud=%d\n", u8Uart, u32BaudRate);

    while (abs(i32BaudError) > (int32)(u32BaudRate >> 4)) /* 6.25% (100/16) error */
    {
        if (--u8ClocksPerBit < 3)
        {
            DBG_vPrintf(DEBUG_UART_BUFFERED, "Could not calculate UART settings for target baud!");
            return;
        }
#endif /* ENABLE_ADVANCED_BAUD_SELECTION */

        /* Calculate Divisor register = 16MHz / (16 x baud rate) */
        u16Divisor = (uint16)(16000000UL / ((u8ClocksPerBit+1) * u32BaudRate));

        /* Correct for rounding errors */
        u32Remainder = (uint32)(16000000UL % ((u8ClocksPerBit+1) * u32BaudRate));

        if (u32Remainder >= (((u8ClocksPerBit+1) * u32BaudRate) / 2))
        {
            u16Divisor += 1;
        }

#if (defined ENABLE_ADVANCED_BAUD_SELECTION)
        DBG_vPrintf(DEBUG_UART_BUFFERED, "Divisor=%d, cpb=%d\n", u16Divisor, u8ClocksPerBit);

        u32CalcBaudRate = (16000000UL / ((u8ClocksPerBit+1) * u16Divisor));

        DBG_vPrintf(DEBUG_UART_BUFFERED, "Calculated baud=%d\n", u32CalcBaudRate);

        i32BaudError = (int32)u32CalcBaudRate - (int32)u32BaudRate;

        DBG_vPrintf(DEBUG_UART_BUFFERED, "Error baud=%d\n", i32BaudError);
    }

    DBG_vPrintf(DEBUG_UART_BUFFERED, "Config uart=%d: Divisor=%d, cpb=%d\n", u8Uart, u16Divisor, u8ClocksPerBit);

    /* Set the calculated clocks per bit */
    vAHI_UartSetClocksPerBit(u8Uart, u8ClocksPerBit);
#endif /* ENABLE_ADVANCED_BAUD_SELECTION */

    /* Set the calculated divisor */
    vAHI_UartSetBaudDivisor(u8Uart, u16Divisor);
}

#ifdef UART_EXTRAS
/****************************************************************************
 *
 * NAME:       vUartDeInit
 *
 * DESCRIPTION:
 * Disables the specified UART.
 *
 * PARAMETERS:      Name            RW  Usage
 *                  u8Uart			R   UART to disable, eg, E_AHI_UART_0
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vUartDeInit(uint8 u8Uart)
{

	/* Wait for any transmissions to complete */
	while(!bUartTxInProgress(u8Uart));

	/* Disable TX Fifo empty and Rx data interrupts */
    vAHI_UartSetInterrupt(u8Uart, FALSE, FALSE, FALSE, FALSE, E_AHI_UART_FIFO_LEVEL_1);

	/* remove interrupt service callback */
    if(u8Uart == E_AHI_UART_0)
    {
		vAHI_Uart0RegisterCallback((void*)NULL);
	}
	else
	{
		vAHI_Uart1RegisterCallback((void*)NULL);
	}

	vAHI_UartDisable(u8Uart);
}
#endif

#ifdef UART_EXTRAS
/****************************************************************************
 *
 * NAME:       bUartReadBinary
 *
 * DESCRIPTION:
 * Reads a specified number of bytes from the uart and stores them in an area
 * of memory. Times out if no data is received for a specified time.
 *
 * PARAMETERS:      Name            RW  Usage
 *                  u8Uart			R   UART to read from, eg. E_AHI_UART_0
 *					pu8Ptr			W	Pointer to an area of memory that
 *										will receive the data.
 *					u32Len			R	Number of bytes to receive
 *					u32TimeoutTime	R	How long to wait for data before
 *										timeout
 *
 * RETURNS:
 * bool_t, TRUE if specified number of bytes were read, FALSE if timed out
 *
 ****************************************************************************/
PUBLIC bool_t bUartReadBinary(uint8 u8Uart, uint8 *pu8Ptr, uint32 u32Len, uint32 u32TimeoutTime)
{

	uint32 n;

	for(n = 0; n < u32Len; n++){
		if(!bUartReadWithTimeout(u8Uart, pu8Ptr++, u32TimeoutTime)){
			return(FALSE);
		}
	}

	return(TRUE);
}
#endif

#ifdef UART_EXTRAS
/****************************************************************************
 *
 * NAME:       bUartReadWithTimeout
 *
 * DESCRIPTION:
 * Attempts to read 1 byte from the RX buffer. If there is no data in the
 * buffer, then it will wait for u32TimeoutTime, and then exit.
 *
 * PARAMETERS:      Name            RW  Usage
 *                  u8Uart			R   UART to use, eg, E_AHI_UART_0
 *                  pu8Data      	W   pointer to 8 bit RX data destination
 *                  u32TimeoutTime  R   Time to wait for data to if there
 *										is none already in the buffer
 *
 * RETURNS:
 * bool_t: TRUE if data was read, data is left in *pu8Data,
 *		   FALSE if no data was read
 *
 ****************************************************************************/
PUBLIC bool_t bUartReadWithTimeout(uint8 u8Uart, uint8 *pu8Data, uint32 u32TimeoutTime)
{

	uint32 u32Time;

	for(u32Time = 0; u32Time < u32TimeoutTime; u32Time++){

		if(bQueue_Read(&asUart_RxQueue[u8Uart], pu8Data))
		{
			return(TRUE);
		}

	}

	*pu8Data = 0;
	return(FALSE);
}
#endif

#ifdef UART_EXTRAS
/****************************************************************************
 *
 * NAME:       u8UartRead
 *
 * DESCRIPTION:
 * Reads 1 byte from the RX buffer. If there is no data in the
 * buffer, then it will wait until there is.
 *
 * PARAMETERS:      Name            RW  Usage
 *                  u8Uart			R   UART to use, eg, E_AHI_UART_0
 *
 * RETURNS:
 * uint8: received data
 *
 ****************************************************************************/
PUBLIC uint8 u8UartRead(uint8 u8Uart)
{

	uint8 u8Data;

	while(!bQueue_Read(&asUart_RxQueue[u8Uart], &u8Data));

	return(u8Data);

}
#endif

/****************************************************************************
 *
 * NAME:       u8UartRead
 *
 * DESCRIPTION:
 * Reads 1 byte from the RX buffer. If there is no data in the
 * buffer, then return FALSE
 *
 * PARAMETERS:      Name            RW  Usage
 *                  u8Uart			R   UART to use, eg, E_AHI_UART_0
 *
 * RETURNS:
 * TRUE if a byte has been read from the queue
 *
 ****************************************************************************/
PUBLIC bool bUartRead(uint8 u8Uart, uint8 *pu8Data)
{
	return(bQueue_Read(&asUart_RxQueue[u8Uart], pu8Data));
}

#ifdef UART_EXTRAS
/****************************************************************************
 *
 * NAME:       bUartTxInProgress
 *
 * DESCRIPTION:
 * Returns the state of data transmission
 *
 * PARAMETERS:      Name            RW  Usage
 *                  u8Uart			R   UART to use, eg, E_AHI_UART_0
 *
 * RETURNS:
 * bool_t: TRUE if data in buffer is being transmitted
 *		   FALSE if all data in buffer has been transmitted by the UART
 *
 ****************************************************************************/
PUBLIC bool_t bUartTxInProgress(uint8 u8Uart)
{

	if(bQueue_IsEmpty(&asUart_TxQueue[u8Uart]))
	{

		if((u8AHI_UartReadLineStatus(u8Uart) & E_AHI_UART_LS_TEMT ) != 0)
		{
			return(FALSE);
		}

	}

	return(TRUE);

}
#endif

#ifdef UART_EXTRAS
/****************************************************************************
 *
 * NAME:       bUartRxDataAvailable
 *
 * DESCRIPTION:
 * Returns state of data reception
 *
 * PARAMETERS:      Name            RW  Usage
 *                  u8Uart			R   UART to use, eg, E_AHI_UART_0
 *
 * RETURNS:
 * bool_t: TRUE if there is received data in the buffer
 *		   FALSE if there is no received data
 *
 ****************************************************************************/
PUBLIC bool_t bUartRxDataAvailable(uint8 u8Uart)
{

	return(!bQueue_IsEmpty(&asUart_RxQueue[u8Uart]));

}
#endif

#ifdef UART_EXTRAS
/****************************************************************************
 *
 * NAME:       vUartWriteBinary
 *
 * DESCRIPTION:
 * Writes a specified number of bytes to the uart.
 *
 * PARAMETERS:      Name            RW  Usage
 *                  u8Uart			R   UART to write to, eg. E_AHI_UART_0
 *					pu8Ptr			W	Pointer to an area of memory that
 *										contains the data to send
 *					u32Len			R	Number of bytes to send
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vUartWriteBinary(uint8 u8Uart, uint8 *pu8Ptr, uint32 u32Len)
{

	uint32 n;

	for(n = 0; n < u32Len; n++){
		vUartWrite(u8Uart, *pu8Ptr++);
	}

}
#endif

#ifdef UART_EXTRAS
/****************************************************************************
 *
 * NAME:       vUartWriteString
 *
 * DESCRIPTION:
 * Writes a null terminated string to the specified UART
 *
 * PARAMETERS:      Name            RW  Usage
 *                  u8Uart			R   UART to write to, eg. E_AHI_UART_0
 *					pu8String		W	Pointer to a null terminated string
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vUartWriteString(uint8 u8Uart, uint8 *pu8String)
{

	while(*pu8String != '\0'){
		vUartWrite(u8Uart, *pu8String);
		if(*pu8String++ == '\r'){
			vUartWrite(u8Uart, '\n');
		}
	}

}
#endif

/****************************************************************************
 *
 * NAME:       vUartWrite
 *
 * DESCRIPTION:
 * Writes one byte to the specified uart for transmission
 *
 * PARAMETERS:      Name            RW  Usage
 *                  u8Uart			R   UART to use, eg, E_AHI_UART_0
 *					u8Data			R	data to transmit
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vUartWrite(uint8 u8Uart, uint8 u8Data)
{

	while(!bQueue_Write(&asUart_TxQueue[u8Uart], u8Data));

	/*
	 * if there is already a tx in progress, we can expect a TX interrupt
	 * some time in the future, in which case the data we wrote to the tx
	 * buffer will get sent in due course to the UART in the interrupt
	 * service routine.
	 * if there is no tx in progress, there won't be a tx interrupt, and the
	 * byte won't get read from the buffer in the ISR, so we must write it
	 * to the UART tx FIFO here.
	 */
    if ((u8AHI_UartReadLineStatus(u8Uart) & (E_AHI_UART_LS_THRE|E_AHI_UART_LS_TEMT)) == (E_AHI_UART_LS_THRE|E_AHI_UART_LS_TEMT))
    {
		if(bQueue_Read(&asUart_TxQueue[u8Uart], &u8Data))
		{
			vAHI_UartWriteData(u8Uart, u8Data);
		}
	}
}

#ifdef UART_EXTRAS
/****************************************************************************
 *
 * NAME:       vUartClear
 *
 * DESCRIPTION:
 * Clears the buffers of the specified UART
 *
 * PARAMETERS:      Name            RW  Usage
 *                  u8Uart			R   UART to disable, eg, E_AHI_UART_0
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vUartClear(uint8 u8Uart)
{
	/* Disable TX Fifo empty and Rx data interrupts */
    vAHI_UartSetInterrupt(u8Uart, FALSE, FALSE, FALSE, FALSE, E_AHI_UART_FIFO_LEVEL_1);

	vQueue_Flush(&asUart_TxQueue[u8Uart]);
	vQueue_Flush(&asUart_RxQueue[u8Uart]);

	/* flush hardware buffer */
    vAHI_UartReset(u8Uart, TRUE, TRUE);
    vAHI_UartReset(u8Uart, FALSE, FALSE);

	/* Re-enable TX Fifo empty and Rx data interrupts */
    vAHI_UartSetInterrupt(u8Uart, FALSE, FALSE, TRUE, TRUE, E_AHI_UART_FIFO_LEVEL_1);
}



/****************************************************************************
 *
 * NAME:       vUartFlush
 *
 * DESCRIPTION:
 * Flushes the buffers of the specified UART
 *
 * PARAMETERS:      Name            RW  Usage
 *                  u8Uart			R   UART to disable, eg, E_AHI_UART_0
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PUBLIC void vUartFlush(uint8 u8Uart)
{
	/* Disable TX Fifo empty and Rx data interrupts */
    vAHI_UartSetInterrupt(u8Uart, FALSE, FALSE, FALSE, FALSE, E_AHI_UART_FIFO_LEVEL_1);

	vQueue_Flush(&asUart_TxQueue[u8Uart]);
	vQueue_Flush(&asUart_RxQueue[u8Uart]);

	/* flush hardware buffer */
    vAHI_UartReset(u8Uart, TRUE, TRUE);
    vAHI_UartReset(u8Uart, FALSE, FALSE);

	/* Re-enable TX Fifo empty and Rx data interrupts */
    vAHI_UartSetInterrupt(u8Uart, FALSE, FALSE, TRUE, TRUE, E_AHI_UART_FIFO_LEVEL_1);
}
#endif

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME:       vUartISR
 *
 * DESCRIPTION:
 * Interrupt service callback for UART's
 *
 * PARAMETERS:      Name            RW  Usage
 *                  u32DeviceId		R   Device ID of whatever generated the
 *										interrupt
 *					u32ItemBitmap	R	Which part of the device generated
 *										the interrupt
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vUartISR(uint32 u32DeviceId, uint32 u32ItemBitmap)
{
	uint8 u8Uart;

	switch(u32DeviceId){

	case E_AHI_DEVICE_UART0:
		u8Uart = 0;
		break;

	case E_AHI_DEVICE_UART1:
		u8Uart = 1;
		break;

	default:
		return;
	}


	switch(u32ItemBitmap){

	case E_AHI_UART_INT_TX:
		vUartTxIsr(u8Uart);
		break;

	case E_AHI_UART_INT_RXDATA:
		vUartRxIsr(u8Uart);
		break;

	}
}


/****************************************************************************
 *
 * NAME:       vUartTxISR
 *
 * DESCRIPTION:
 * Interrupt service callback for UART data transmission. Checks the tx buffer
 * for any data waiting for transmission, and if any is available, will write
 * up to 16 bytes of it to the UART's hardware fifo, set the tx in progress
 * flag then exit.
 *
 * PARAMETERS:      Name            RW  Usage
 *                  u8Uart			R   Uart to write to
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vUartTxIsr(uint8 u8Uart)
{
	uint8 u8Data;
	uint32 u32Bytes = 0;

	/*
	 * if there is data in buffer waiting for tx and we've not filled the
	 * hardware fifo up
	 */

	while(u32Bytes++ < 16 && bQueue_Read(&asUart_TxQueue[u8Uart], &u8Data))
	{
		vAHI_UartWriteData(u8Uart, u8Data); /* write one byte to the UART */
    }
}


/****************************************************************************
 *
 * NAME:       vUartRxISR
 *
 * DESCRIPTION:
 * Interrupt service callback for UART data reception. Reads a received
 * byte from the UART and writes it to the reception buffer if it is not
 * full.
 *
 * PARAMETERS:      Name            RW  Usage
 *                  u8Uart			R   Uart to read from
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
PRIVATE void vUartRxIsr(uint8 u8Uart)
{
	bQueue_Write(&asUart_RxQueue[u8Uart], u8AHI_UartReadData(u8Uart));
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

