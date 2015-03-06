#include "Utils.h"

//#include "EndDevice.c"
PUBLIC void Sleep(uint8 time)
    {
    uint8 counter = time * 10;//задержка в 100 мс*10=1 с.
    uint16 i;
    for (i = 0; i < counter; i++)
	{
	//---------------------задержка в 100 мс------------------------------
	vAHI_TimerEnable(E_JPI_TIMER_1, 7, E_JPI_TIMER_INT_PERIOD, FALSE,
		FALSE, E_JPI_TIMER_CLOCK_INTERNAL_NORMAL);
	/* Start timer for 100ms */
	vAHI_TimerStart(E_JPI_TIMER_1, E_JPI_TIMER_MODE_SINGLESHOT, 1, 12500);
	/* Wait for timer to end */
	while ((u8AHI_TimerFired(E_JPI_TIMER_1) & E_JPI_TIMER_INT_PERIOD) == 0)
	    ;
	}
        //--------------------------------------------------------------------
    }
//----------------------------------------------------------------------------
