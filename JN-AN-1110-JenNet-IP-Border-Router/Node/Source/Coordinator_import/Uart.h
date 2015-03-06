#ifndef UART_H
#define UART_H
//----------------------------------------------------------------------------

#include <jendefs.h>

#include "AACPFrame.h"
#include "Vector.h"
#include "Packet.h"
//----------------------------------------------------------------------------

PUBLIC void vUart_init(void);
PUBLIC void UartSendFrame(Vector* v);
PUBLIC void UartSendPack(Packet* p);
PUBLIC void UartSend(void);

PUBLIC void vUart_putChar(char c);

PUBLIC Vector* UartGetFrameBuffer();
PUBLIC void UartIncommingFrameHandled();

#endif
