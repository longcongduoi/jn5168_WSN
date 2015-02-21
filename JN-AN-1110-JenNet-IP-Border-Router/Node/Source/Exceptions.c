/****************************************************************************
 *
 * MODULE:             JenNet-IP Border Router
 *
 * COMPONENT:          Exception handlers
 *
 * VERSION:            $Name$
 *
 * REVISION:           $Revision: 30585 $
 *
 * DATED:              $Date: 2011-04-06 22:03:34 +0100 (Wed, 06 Apr 2011) $
 *
 * STATUS:             $State$
 *
 * AUTHOR:             Thomas Haydon
 *
 * DESCRIPTION:        Exception handlers
 *
 *
 * LAST MODIFIED BY:   $Author: nxp29781 $
 *                     $Modtime: $
 *
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
#include <AppHardwareApi.h>

#include "Exceptions.h"
#include "log.h"


/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#ifdef NO_EXCEPTION_DEBUG
#define TRACE_EXC                   FALSE
#else
#define TRACE_EXC                   TRUE
#endif

/** Define to dump the stack on exception */
#ifndef EXC_DUMP_STACK
//#define EXC_DUMP_STACK
#endif /* EXC_DUMP_STACK */

/** Define to dump registers on exception */
#ifndef EXC_DUMP_REGS
//#define EXC_DUMP_REGS
#endif /* EXC_DUMP_REGS */

/* Default log level is DEBUG */
#define LOG_LEVEL                   LOG_DEBUG


/* Select whether exception vectors should be in RAM or Flash based on chip family */
#if (defined JENNIC_CHIP_FAMILY_JN514x)
#define EXCEPTION_VECTORS_LOCATION_RAM
#elif (defined JENNIC_CHIP_FAMILY_JN516x)
#define EXCEPTION_VECTORS_LOCATION_FLASH
#else
#error Unsupported chip family selected
#endif /* JENNIC_CHIP_FAMILY */


#if (defined EXCEPTION_VECTORS_LOCATION_RAM)
/* RAM exception vectors are set up at run time */
/* Addresses of exception vectors in RAM */
#define BUS_ERROR                   *((volatile uint32 *)(0x4000000))
#define TICK_TIMER                  *((volatile uint32 *)(0x4000004))
#define UNALIGNED_ACCESS            *((volatile uint32 *)(0x4000008))
#define ILLEGAL_INSTRUCTION         *((volatile uint32 *)(0x400000c))
#define EXTERNAL_INTERRUPT          *((volatile uint32 *)(0x4000010))
#define SYSCALL                     *((volatile uint32 *)(0x4000014))
#define TRAP                        *((volatile uint32 *)(0x4000018))
#define GENERIC                     *((volatile uint32 *)(0x400001c))
#define STACK_OVERFLOW              *((volatile uint32 *)(0x4000020))
#elif (defined EXCEPTION_VECTORS_LOCATION_FLASH)
/* Flash exception vectors are set up at compile time */
#else
#error Unknown exception vector location
#endif /* EXCEPTION_VECTORS_LOCATION */

/* Locations in stack trace of important information */
#define STACK_REG                   1
#define PROGRAM_COUNTER             18
#define EFFECTIVE_ADDR              19

/* Number of registers */
#define REG_COUNT                   16

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Function Prototypes                                     ***/
/****************************************************************************/

PRIVATE void vExceptionHandler(uint32 *pu32Stack, eExceptionType eType);

PRIVATE void *pvHeapAllocOverflowProtect(void *pvPointer, uint32 u32Size, bool_t bClear);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

extern uint32 heap_location;
extern void *(*prHeap_AllocFunc)(void *, uint32, bool_t);
PRIVATE void *(*prHeap_AllocOrig)(void *, uint32, bool_t);

/* Symbol defined by the linker script */
extern uint32 ram_top;

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************
 *
 * NAME: vEXC_Register
 *
 * DESCRIPTION:
 * Set up exceptions. When in RAM, overwrite the default vectors with ours.
 * We also patch the heap allocation function so that we can keep tabs on
 * the amount of free heap.
 *
 * PARAMETERS: None
 *
 * RETURNS:
 * None
 *
 ****************************************************************************/
PUBLIC void vEXC_Register(void)
{
#ifdef EXCEPTION_VECTORS_LOCATION_RAM
	/* Overwrite exception vectors, pointing them all at the generic handler */
	BUS_ERROR			= (uint32)vExceptionHandler;
	UNALIGNED_ACCESS	= (uint32)vExceptionHandler;
	ILLEGAL_INSTRUCTION	= (uint32)vExceptionHandler;
	SYSCALL				= (uint32)vExceptionHandler;
	TRAP				= (uint32)vExceptionHandler;
	GENERIC				= (uint32)vExceptionHandler;
	STACK_OVERFLOW		= (uint32)vExceptionHandler;
#endif /* EXCEPTION_VECTORS_LOCATION */

    prHeap_AllocOrig = prHeap_AllocFunc;
    prHeap_AllocFunc = pvHeapAllocOverflowProtect;
}


#ifdef EXCEPTION_VECTORS_LOCATION_FLASH
/* If exception vectors are in flash, define the handler functions here to be linked in */
/* These function names are defined in the 6x linker script for the various exceptions */
/* Point them all at the generic handler */
PUBLIC void vException_BusError(uint32 *pu32Stack, eExceptionType eType)
{
    vExceptionHandler(pu32Stack, eType);
}

PUBLIC void vException_UnalignedAccess(uint32 *pu32Stack, eExceptionType eType)
{
    vExceptionHandler(pu32Stack, eType);
}

PUBLIC void vException_IllegalInstruction(uint32 *pu32Stack, eExceptionType eType)
{
    vExceptionHandler(pu32Stack, eType);
}

PUBLIC void vException_SysCall(uint32 *pu32Stack, eExceptionType eType)
{
    vExceptionHandler(pu32Stack, eType);
}

PUBLIC void vException_Trap(uint32 *pu32Stack, eExceptionType eType)
{
    vExceptionHandler(pu32Stack, eType);
}

PUBLIC void vException_StackOverflow(uint32 *pu32Stack, eExceptionType eType)
{
    vExceptionHandler(pu32Stack, eType);
}

#endif /* EXCEPTION_VECTORS_LOCATION_FLASH */


/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/


/****************************************************************************
 *
 * NAME: vExceptionHandler
 *
 * DESCRIPTION:
 * Generic exception handler which is called whether the vectors are in RAM or flash
 *
 * PARAMETERS: None
 *
 * RETURNS:
 * None
 *
 ****************************************************************************/
PRIVATE void vExceptionHandler(uint32 *pu32Stack, eExceptionType eType)
{
#if (defined EXC_DUMP_STACK) || (defined EXC_DUMP_REGS)
	int i;
#endif
	uint32 u32EPCR, u32EEAR, u32Stack;

	char *pcString;

	switch (eType)
	{
	case E_EXC_BUS_ERROR:
		pcString = "BUS";
		break;

	case E_EXC_UNALIGNED_ACCESS:
		pcString = "ALIGN";
		break;

	case E_EXC_ILLEGAL_INSTRUCTION:
		pcString = "ILLEGAL";
		break;

	case E_EXC_SYSCALL:
		pcString = "SYSCALL";
		break;

	case E_EXC_TRAP:
		pcString = "TRAP";
		break;

	case E_EXC_GENERIC:
		pcString = "GENERIC";
		break;

	case E_EXC_STACK_OVERFLOW:
		pcString = "STACK";
		break;

	default:
		pcString = "UNKNOWN";
		break;
	}

	/* Push out anything that might be in the log buffer already */
	vSL_LogFlush();

	/* Pull the EPCR and EEAR values from where they've been saved by the ROM exception handler */
	u32EPCR = pu32Stack[PROGRAM_COUNTER];
	u32EEAR = pu32Stack[EFFECTIVE_ADDR];
	u32Stack = pu32Stack[STACK_REG];

	/* Log the exception */
	vLog_Printf(TRACE_EXC, LOG_CRIT, "\n\n\n%s EXCEPTION @ %08x (EA: %08x SK: %08x HP: %08x)", pcString, u32EPCR, u32EEAR, u32Stack, ((uint32 *)&heap_location)[0]);
    vSL_LogFlush();

#ifdef EXC_DUMP_REGS
    /* Pull and print the registers from saved locations */
    for (i = 0; i < REG_COUNT; i += 4)
    {
        vLog_Printf(TRACE_EXC, LOG_ERR, "\nR%02d: %08x R%02d: %08x R%02d: %08x R%02d: %08x\n", i, pu32Stack[i], i+1, pu32Stack[i+1], i+2, pu32Stack[i+2], i+3, pu32Stack[i+3]);
        vSL_LogFlush();
    }
#endif

#ifdef EXC_DUMP_STACK
    /* Print the stack */
    pu32Stack = (uint32 *)(u32Stack & 0xFFFFFFF0);
	for (i = 0; (pu32Stack + i) < (uint32 *)(ram_top); i += 4)
	{
		vLog_Printf(TRACE_EXC, LOG_ERR, "\n%08x: %08x %08x %08x %08x", pu32Stack + i, pu32Stack[i], pu32Stack[i+1], pu32Stack[i+2], pu32Stack[i+3]);
		vSL_LogFlush();
	}
#endif

	/* Software reset */
	vAHI_SwReset();
}


/****************************************************************************
 *
 * NAME: pvHeapAllocOverflowProtect
 *
 * DESCRIPTION:
 * New heap allocation function that sets the stack overflow location to the new
 * top address of the heap.
 *
 * PARAMETERS:  Name             RW  Usage
 *              pvPointer		 W   Location of allocated heap memory
 *              u32Size          R   Number of bytes to allocate
 *              bClear           R   Flag to set new memory to 0
 *
 * RETURNS:
 * Pointer to new memory
 *
 ****************************************************************************/
PRIVATE void *pvHeapAllocOverflowProtect(void *pvPointer, uint32 u32Size, bool_t bClear)
{
    void *pvAlloc;

    /* Call original heap allocation function */
    pvAlloc = prHeap_AllocOrig(pvPointer, u32Size, bClear);
    /* Set stack overflow address */
    vAHI_SetStackOverflow(TRUE, ((uint32 *)&heap_location)[0]);

    return pvAlloc;
}

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
