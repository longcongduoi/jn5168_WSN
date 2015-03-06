/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include <AppHardwareApi_JN516x.h>

#include "Watchdog.h"
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

PUBLIC void vWatchdog_init(){

	#if defined WATCHDOG_INTERNAL

	    vAHI_WatchdogStart(12);// Prescaler is equal 12 -> 16392 ms timeout period

	#elif defined WATCHDOG_EXTERNAL

	    vAHI_WatchdogStop();

	    // Установить вывод через который передаётся импульс на внешний Watchdog
		vAHI_DioSetDirection(0, WATCHDOG_IMPULSE);
		vAHI_DioSetPullup(0, WATCHDOG_IMPULSE);

	#elif

	    vAHI_WatchdogStop();

	#endif
}

PUBLIC void vWatchdog_restart(){


	#if defined WATCHDOG_INTERNAL

	    vAHI_WatchdogRestart();

	#elif defined WATCHDOG_EXTERNAL

	    static bool isImpulse;

	    (isImpulse) ? vAHI_DioSetOutput(WATCHDOG_IMPULSE, 0):vAHI_DioSetOutput(WATCHDOG_IMPULSE, 0);
	    isImpulse = !isImpulse;

	#endif
}

PUBLIC void vWatchdog_stop(){

    #if defined WATCHDOG_INTERNAL

		vAHI_WatchdogStop();

    #endif
}
