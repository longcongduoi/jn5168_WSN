/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include "DebugP.h"

#include "Fio.h"
#include "Uart.h"

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

PRIVATE uint32 LogCounter = 0;// ������� ���������
PRIVATE bool enabled = FALSE;// ������� ��������

PRIVATE char* OnMessage = "\n\r--- DEBUG ---\r\n\n\r";
PRIVATE char* OffMessage = "\n\r-------------\r\n";

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

PUBLIC void vDebug_enable(){

    Debug_printf(OnMessage);

    enabled = TRUE;
}

PUBLIC bool Debug_enabled(){

    return enabled;
}

PUBLIC void Debug_printf(const char *fmt, ...){

    // ������ �� �������
    static bool isPrintfExecute = FALSE;

    // ���� ������� ����� ���� ������� � ���������� ������� (�������� ��-�� ���������� � ������ �����������, ���������� ��� �� �������)
    // ����� ��������������, ���� �� ���������� � ������, ����������� � ������ (��������, ���� ��������� ����������, ��� ���������� printf)
    if(!isPrintfExecute){

	va_list ap;
	va_start(ap, fmt);

	Fio_printf(fmt, ap, vUart_putChar);

	va_end(ap);

	isPrintfExecute=FALSE;
    }
}

PUBLIC uint32 Debug_getLogCounter(void){

    return ++LogCounter;
}