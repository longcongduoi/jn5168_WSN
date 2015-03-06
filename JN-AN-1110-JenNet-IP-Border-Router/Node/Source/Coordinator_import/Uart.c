
#include <AppHardwareApi_JN516x.h>

#include "Uart.h"
#include "Config.h"
#include "ErrorHandler.h"
#include "Buffer.h"
#include "Converter.h"
#include "Vector.h"
//----------------------------------------------------------------------------

#define UART_SEND_TIMEOUT 	255
#define BAUDRATE    		E_AHI_UART_RATE_19200
#define PARITY      		E_AHI_UART_EVEN_PARITY
#define PARITYSTATE			E_AHI_UART_PARITY_DISABLE
#define STOPBIT     		E_AHI_UART_1_STOP_BIT

#define AACP_START			0xFC
#define AACP_STOP			0xFF

// Коды ошибок
#define E_UART_MODULE_E_JPI_UART_LS_ERROR		3
#define E_UART_MODULE_E_JPI_UART_LS_BI			4
#define E_UART_MODULE_E_JPI_UART_LS_FE			5
#define E_UART_MODULE_E_JPI_UART_LS_PE			6
#define E_UART_MODULE_E_JPI_UART_LS_OE			7

#define E_UART_MODULE_SEND_STRING_BUFFER_FULL		8
#define E_UART_MODULE_SEND_PACKET_BUFFER_FULL		9
#define E_UART_MODULE_SEND_BYTE						10
#define E_UART_MODULE_RECEIVE_PACKET_BUFFER_FULL	11
#define E_UART_MODULE_RX_BUFFER_FULL				12
#define E_UART_MODULE_FRAME_SIZE_MORE_EXPECTED		13
#define E_UART_MODULE_UNEXPECTED_START_PACKET		14
#define E_UART_MODULE_UNEXPECTED_END_PACKET			15
#define E_UART_MODULE_INVALID_CRC	  				16

// Состояние приемного буфера
PRIVATE enum ByteStuffingState
{
	bssNotInitialized, bssControlSymbol, bssNormal
} state = bssNotInitialized;

PRIVATE void UartSendByte(uint8 val);
PRIVATE uint8 UartGetStatus(void);
PRIVATE void UartCallBackHandler(uint32 device, uint32 itemBitmap);
PRIVATE bool WaitTxBufFree(void);
PRIVATE uint16 MbCRC16 ( const uint8 *buffer, uint16 size );
PRIVATE bool PushToRxBuffer(uint8 t);
PRIVATE void ClearRxBuffer();

PRIVATE Vector m_UartRxBuf;
PRIVATE Buffer m_UartTxBuf;
PRIVATE bool m_TxOn;
PRIVATE int16 m_UartTrashCount = 0;
PRIVATE bool m_isFrameReceived = FALSE;

/* Table of CRC values for high–order byte */
PRIVATE uint8 auchCRCHi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40
} ;

/* Table of CRC values for low–order byte */
PRIVATE uint8 auchCRCLo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
    0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
    0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
    0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
    0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
    0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
    0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
    0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
    0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
    0x40
};

PUBLIC void vUart_init(void)
{
	if (MAIN_DEVICE_TYPE == dtEndDevice && !MAIN_EDUART_ENABLE)
		return;

	VectorClear(&m_UartRxBuf);
	BufInit(&m_UartTxBuf);
	m_TxOn = FALSE;
	m_isFrameReceived = FALSE;
	vAHI_UartEnable(E_AHI_UART_0);
	vAHI_UartSetClockDivisor(E_AHI_UART_0, BAUDRATE);
	vAHI_UartSetControl(E_AHI_UART_0, PARITY, PARITYSTATE, E_AHI_UART_WORD_LEN_8, STOPBIT, E_AHI_UART_RTS_HIGH);
	vAHI_Uart0RegisterCallback(&UartCallBackHandler);
	vAHI_UartSetInterrupt(E_AHI_UART_0, FALSE, FALSE, FALSE, TRUE, E_AHI_UART_FIFO_LEVEL_8);
	vAHI_UartSetRTSCTS(E_AHI_UART_0, TRUE);
	vAHI_UartReset(E_AHI_UART_0, FALSE, FALSE);
}

PUBLIC void UartSendFrame(Vector* v)
{
	if (MAIN_DEVICE_TYPE == dtEndDevice && !MAIN_EDUART_ENABLE)
		return;

	uint16 i;

	Frame* frame = (Frame*)&v->buf[0];
	frame->start = AACP_START;
	v->buf[v->size-1] = AACP_STOP;

	frame->crc = hton16(MbCRC16(&frame->ns, v->size - 4));

	for (i = 0; i < v->size; i++)
	{
		if ( BufAvailSize(&m_UartTxBuf) == 0 )
		{
			EHCall(UART_MOD_ID, E_UART_MODULE_SEND_PACKET_BUFFER_FULL);
			return;
		}

		BufPush(&m_UartTxBuf, v->buf[i]);

		if (i == 0 || i == v->size - 1)
			continue;

		if ((v->buf[i] == AACP_START) || (v->buf[i] == AACP_STOP))
		{
			if ( BufAvailSize(&m_UartTxBuf) == 0 )
			{
				EHCall(UART_MOD_ID, E_UART_MODULE_SEND_PACKET_BUFFER_FULL);
				return;
			}

			BufPush(&m_UartTxBuf, v->buf[i]);
		}
	}
}

PUBLIC void UartSend(void)
{
	if (MAIN_DEVICE_TYPE == dtEndDevice && !MAIN_EDUART_ENABLE)
		return;

	uint8 val;

	while (BufPop(&m_UartTxBuf, &val))
		UartSendByte(val);
}

PUBLIC void vUart_putChar(char c){

	UartSendByte((uint8)c);
}

PUBLIC Vector* UartGetFrameBuffer()
{
	if (MAIN_DEVICE_TYPE == dtEndDevice && !MAIN_EDUART_ENABLE)
		return NULL;

	if(!m_isFrameReceived)
		return NULL;

	return &m_UartRxBuf;
}

PUBLIC void UartIncommingFrameHandled()
{
	VectorClear(&m_UartRxBuf);
	m_isFrameReceived = FALSE;
}

PUBLIC void UartSendByte(uint8 val)
{
	if (WaitTxBufFree())
		vAHI_UartWriteData(E_AHI_UART_0, val);
	else
		EHCall(UART_MOD_ID, E_UART_MODULE_SEND_BYTE);
}

PRIVATE bool WaitTxBufFree(void)
{
	uint8 status;
	uint16 timer = UART_SEND_TIMEOUT;

	do
	{
		status = UartGetStatus();
		timer--;
		if (timer == 0)
			return FALSE;
	} while (!((status & E_AHI_UART_LS_THRE) && (status & E_AHI_UART_LS_TEMT)));

	return TRUE;
}

PRIVATE void UartCallBackHandler(uint32 device, uint32 itemBitmap)
{
	if ((itemBitmap & 0xFF) != E_AHI_UART_INT_RXDATA)
		return;

	uint8 t = u8AHI_UartReadData(E_AHI_UART_0);

	// Пока предыдущий кадр не обработан - ничего не принимаем
	if(m_isFrameReceived)
	{
		m_UartTrashCount++;
		return;
	}

	switch (state)
	{
		// Если буфер пуст...
		case bssNotInitialized:

			bssNotInitializedCase:

			// текущий байт старт, то это наш стартовый байт
			if (t == AACP_START)
			{
				if (PushToRxBuffer(t))
					state = bssNormal;
				else
					ClearRxBuffer();
			}
			else
				m_UartTrashCount++;
			// иначе мы ещё не засинхронизировались
			break;

			// Прошлый символ был обычным символом
		case bssNormal:

			bssNormalCase:

			if (!PushToRxBuffer(t))
			{
				state = bssNotInitialized;
				break;
			}

			state = (t == AACP_START || t == AACP_STOP) ? bssControlSymbol : bssNormal;

			// Если байт совпадает со стоповым, то стоит проверить, а не приняли ли мы уже весь пакет
			if (t == AACP_STOP)
			{
				if ( m_UartRxBuf.size >= SizeOfFrame)
				{
					Frame* frame = (Frame*)&m_UartRxBuf.buf[0];

					// Пакет получен
					if (m_UartRxBuf.size == ntoh16(frame->size) + SizeOfFrame)
					{
						// Проверяем crc
						uint16 crc = MbCRC16(&frame->ns, SizeOfFrame + ntoh16(frame->size) - 4);

						if(crc != ntoh16(frame->crc))
						{
							EHCall(UART_MOD_ID, E_UART_MODULE_INVALID_CRC);
							ClearRxBuffer();
							break;
						}

						m_isFrameReceived = TRUE;

						state = bssNotInitialized;
					}
					// Если в буфере оказалось больше положенного байт
					else if (m_UartRxBuf.size > ntoh16( frame->size ) + SizeOfFrame)
					{
						EHCall(UART_MOD_ID, E_UART_MODULE_FRAME_SIZE_MORE_EXPECTED);

						ClearRxBuffer();
					}
					// недостаточно динны, скорее всего данный байт не является стоповым - узнаем обработав следующий байт
				}
			}

			break;

			// Прошлый символ был управляющим
		case bssControlSymbol:
		{
			uint8 previous = VectorGet( &m_UartRxBuf, m_UartRxBuf.size - 1);

			// Если совпадают, то дублированный байт (в байтстаффинге) и просто его пропускаем
			if (t == previous)
			{
				state = bssNormal;
				break;
			}

			// Дублирования небыло, а это может означать только:
			// 1. ошибка в виде начала нового пакета, при неоконченном старом
			// 2. неожиданное окончание пакета, т.е. длина пакета меньше заявленной в пакете
			if (previous == AACP_START)
			{
				m_UartTrashCount += m_UartRxBuf.size - 1;

				EHCall(UART_MOD_ID, E_UART_MODULE_UNEXPECTED_START_PACKET);

				ClearRxBuffer();

				PushToRxBuffer(AACP_START);

				state = bssNormal;

				goto bssNormalCase;
			}
			else
			{
				// неожиданное окончание текущего пакета
				m_UartTrashCount += m_UartRxBuf.size;

				EHCall(UART_MOD_ID, E_UART_MODULE_UNEXPECTED_END_PACKET);

				ClearRxBuffer();

				goto bssNotInitializedCase;
			}
		}
		break;
	}
}

PRIVATE uint8 UartGetStatus(void)
{
	uint8 status = u8AHI_UartReadLineStatus(E_AHI_UART_0);
	if (status & E_AHI_UART_LS_ERROR)
		EHCall(UART_MOD_ID, E_UART_MODULE_E_JPI_UART_LS_ERROR);
	if (status & E_AHI_UART_LS_BI)
		EHCall(UART_MOD_ID, E_UART_MODULE_E_JPI_UART_LS_BI);
	if (status & E_AHI_UART_LS_FE)
		EHCall(UART_MOD_ID, E_UART_MODULE_E_JPI_UART_LS_FE);
	if (status & E_AHI_UART_LS_PE)
		EHCall(UART_MOD_ID, E_UART_MODULE_E_JPI_UART_LS_PE);
	if (status & E_AHI_UART_LS_OE)
		EHCall(UART_MOD_ID, E_UART_MODULE_E_JPI_UART_LS_OE);

	return status;
}

PRIVATE bool PushToRxBuffer(uint8 t)
{
    if (VectorAvailSize(&m_UartRxBuf) > 0)
    {
    	VectorPushBack(&m_UartRxBuf, t);
    	return TRUE;
    }
    else
    {
    	m_UartTrashCount += m_UartRxBuf.size;

    	VectorClear(&m_UartRxBuf);

    	EHCall(UART_MOD_ID, E_UART_MODULE_RX_BUFFER_FULL);

    	return FALSE;
    }
}

PRIVATE void ClearRxBuffer()
{
	VectorClear(&m_UartRxBuf);

	state = bssNotInitialized;
}

PRIVATE uint16 MbCRC16 ( const uint8 *buffer, uint16 size )
{
    uint8 crcHi = 0xFF;   // high byte of CRC initialized
    uint8 crcLo = 0xFF;   // low byte of CRC initialized

    while ( size-- )
    {
        uint8 index = crcLo ^ *buffer++; // calculate the CRC

        crcLo  = auchCRCHi[index] ^ crcHi;
        crcHi  = auchCRCLo[index];
    }

    return (crcHi << 8) | crcLo;
}


