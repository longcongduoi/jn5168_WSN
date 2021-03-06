/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <AppHardwareApi_JN516x.h>

#include "Led.h"
#include "Config.h"

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

//----------------------------------------------------------------------------
// ���������������� ������
//----------------------------------------------------------------------------
PUBLIC void vLed_init(void){

	vAHI_DioSetDirection(0, LED_RED|LED_GREEN|LED_BLUE);
	vAHI_DioSetPullup(0, LED_RED|LED_GREEN|LED_BLUE);
}

//----------------------------------------------------------------------------
// ��������� ��������� ���������
//----------------------------------------------------------------------------
PUBLIC void vLed_off(void){

	vAHI_DioSetOutput(0, LED_RED|LED_GREEN|LED_BLUE);
}

//----------------------------------------------------------------------------
// ��������� �������� ���������
//----------------------------------------------------------------------------
PUBLIC void vLed_on(void){

	vAHI_DioSetOutput(LED_RED|LED_GREEN|LED_BLUE, 0);
}

//----------------------------------------------------------------------------
// ������� ���� ��������
//----------------------------------------------------------------------------
PUBLIC void vLed_selectColor(Colors_e Color){

    // ������� ������ ���� (�������� ������ ����������)
    switch (Color){

	case cBlack:
	    {

	    	vAHI_DioSetOutput(0, LED_RED|LED_GREEN|LED_BLUE);
	    	break;
	    }

	case cRed:
	    {

	    	vAHI_DioSetOutput(LED_RED, LED_GREEN|LED_BLUE);
			break;
	    }

	case cBlue:
	    {

	    	vAHI_DioSetOutput(LED_BLUE, LED_RED|LED_GREEN);
			break;
	    }

	case cGreen:
	    {

	    	vAHI_DioSetOutput(LED_GREEN, LED_RED|LED_BLUE);
			break;
	    }

	case cMagenta:
	    {

	    	vAHI_DioSetOutput(LED_RED|LED_BLUE, LED_GREEN);
			break;
	    }

   	case cYellow:
   	    {

	    	vAHI_DioSetOutput(LED_RED|LED_GREEN, LED_BLUE);
			break;
   	    }

   	case cCyan:
   	    {

	    	vAHI_DioSetOutput(LED_GREEN|LED_BLUE, LED_RED);
			break;
   	    }

   	case cWhite:
   	    {

   	    	vAHI_DioSetOutput(LED_RED|LED_GREEN|LED_BLUE, 0);
   	    	break;
   	    }

   	default:
   	    {

   	    	// �������� �����-�� ����, ��� ������
   	    }
    }
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
