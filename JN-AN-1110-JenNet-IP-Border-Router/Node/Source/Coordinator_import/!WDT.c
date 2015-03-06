
#include <AppHardwareApi_JN516x.h>

#include "WDT.h"
#include "Config.h"

//PRIVATE uint32 m_WDTDio;

PUBLIC void WDTInit(void){

	/*
	if (MAIN_HARD_VERSION == 2)
		m_WDTDio = E_AHI_DIO19_INT;
	else
	{
		if (MAIN_DEVICE_TYPE == dtCoordinator)
			m_WDTDio = E_AHI_DIO20_INT;
		else
			m_WDTDio = E_AHI_DIO9_INT;
	}

	vAHI_DioSetDirection(0, m_WDTDio);
	WDTPulse();
#ifdef JN5148
	vAHI_WatchdogStop();
#endif
*/
}

PUBLIC void WDTPulse(void)
{
	/*
	uint8 i = 0;
	vAHI_DioSetOutput(m_WDTDio, 0);
	vAHI_DioSetOutput(0, m_WDTDio);
	while (i != 128)
		i++;

	vAHI_DioSetOutput(m_WDTDio, 0);
	*/
}
