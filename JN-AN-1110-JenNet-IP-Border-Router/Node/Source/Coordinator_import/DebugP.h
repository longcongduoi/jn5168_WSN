/*
 * DESCRIPTION
 *
 * Allows to print specific debug messages after DEBUG definition (#define DEBUG)
 * that to switch on processing of these functions
 *
 * Firstly called Debug_Init() to init hardware then you can use Debug_Printf
 * function for formatted output
 *
 */

#ifndef DEBUG_P_H
#define DEBUG_P_H

/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/

#include "Config.h"

//#define DEBUG_LIGHT
/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#ifdef DEBUG_LIGHT// Если отладка включена

    // Вывести строку (Так-как макрос PRINTF не работает без параметров, т.е. когда только форматная строка)
    #define MSG(Message){\
	if(Debug_enabled()){\
		Debug_printf("%d: %s", Debug_getLogCounter(), Message);\
	}\
    }\

    // Вывести в терминал форматированную строку
    #define PRINTF(Format,...){\
	if(Debug_enabled()){\
		Debug_printf("%d: ", Debug_getLogCounter());\
		Debug_printf(Format,__VA_ARGS__);\
	}\
    }\

#else

    #define MSG(Message) ((void)0)
    #define PRINTF(Format,...) ((void)0)

#endif

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

PUBLIC void vDebug_enable(void);
PUBLIC void Debug_disable(void);
PUBLIC bool Debug_enabled(void);

PUBLIC void Debug_printf(const char *fmt, ...);

PUBLIC uint32 Debug_getLogCounter(void);

#endif
