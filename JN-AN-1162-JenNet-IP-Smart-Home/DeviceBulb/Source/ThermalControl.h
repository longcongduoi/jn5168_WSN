/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         Thermal Control Loop
 *
 * Three term PID controller to provide thermal stability to Dde temperature
 * under harsh luminaire environments
 */
/****************************************************************************/
/*
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
 */
/****************************************************************************/
#ifndef  THERMALCONTROL_H_INCLUDED
#define  THERMALCONTROL_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/
#include <jendefs.h>
#include <string.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/* Only Need ever 20th sample to tick loop every 5S as we sample adc at 4Hz  */
#define DECIMATE_FACTOR 20

#define AMBIENT_TEMP_C	26


/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

#ifdef DEBUG_TCL
typedef struct
{
	/* Input/output */
	uint8 u8PwmIn;
	int8  i8ChipTemp;
	uint8 u8PwmOut;
	/* Internal states */
	int16 i16Error;
	int16 i16DiffOut;
	int32 i32IntLoopOut;
	int32 i32LpFilterOut;
}tsTclVarMon;
#endif

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

PUBLIC void vDecimator(int16 i16ChipTemp);
PUBLIC void vSetUserLevel(uint8 u8Level);

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#ifdef DEBUG_TCL
     tsTclVarMon sTclVarMon;
#endif

#if defined __cplusplus
}
#endif

#endif  /* THERMALCONTROL_H_INCLUDED */
/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/
