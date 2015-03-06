/****************************************************************************
 *
 * MODULE:             JenNet-IP Border Router
 *
 * COMPONENT:          Module configuration
 *
 * VERSION:            $Name:  $
 *
 * REVISION:           $Revision: 1.3 $
 *
 * DATED:              $Date: 2008/12/09 10:29:04 $
 *
 * STATUS:             $State: Exp $
 *
 * AUTHOR:             Thomas Haydon
 *
 * DESCRIPTION:
 *
 * CHANGE HISTORY:
 *
 * LAST MODIFIED BY:   $Author: lmitch $
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

#ifndef  CONFIG_H_INCLUDED
#define  CONFIG_H_INCLUDED

#if defined __cplusplus
extern "C" {
#endif

/****************************************************************************/
/***        Include Files                                                 ***/
/****************************************************************************/

#include <jendefs.h>

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

#define ABS(Number) 	((Number < 0) ? (Number*(-1)) : (Number))// Модуль числа

// Appropriate network standard profile [0-9] or user profile [255]// Profile inherited by devices that join the network to join the network
#define CURRENT_NETWORK_JOIN_PROFILE 	0// JN-UG-3080, Section 9.2

// Appropriate network standard profile [0-9] or user profile [255]// Profile inherited by devices that join the network to run the network
#define CURRENT_NETWORK_RUN_PROFILE		0// JN-UG-3080, Section 9.2

// Bitmap of all available channels
#define NETWORK_GATEWAY_ALL_CHANNELS_BITMAP     0x7fff800

// All network nodes address prefix
#define NETWORK_ADDRESS_PREFIX		 0xfd040bd380e80002ULL

// Number of buffers for JIP stack
#define PACKET_BUFFERS                  2

// Size of routing table
#define ROUTE_TABLE_ENTRIES             300

// Maximum length of name strings
#define MAX_NAME_LENGTH					16

#define FIRMWARE "5.0.0"

//-------------------------------------------------------------------------------------------------------
// Настройки координатора

PUBLIC typedef enum{

	dtCoordinator = 0,
	dtRouter,
	dtEndDevice,
	dtPC
}MainDevType;

extern const uint8 MAIN_MAX_DEVICE_COUNT;
extern bool MAIN_EDUART_ENABLE;
extern uint8 MAIN_HARD_VERSION;

#define MAIN_DEVICE_TYPE 			0 // 0 - coordinator, 1 - router, 2 - enddevice, 3 - pc
#define HARDTYPE 					3001

#define CONFIG_DEFAULT_NETID 		19
#define CONFIG_HARD_VERSION  		2// теперь я его не использую

#define SOFT_VERSION1 				3
#define SOFT_VERSION2 				4
#define SOFT_VERSION3 				0

#define PREFMODEL 					"AG-"
#define PREFNAME 					'C'

#define WATCHDOG_INTERNAL
#define DEBUG_LIGHT
//----------------------------------------------------------------------------------------------------------------
// Вывода

//#define WATCHDOG_IMPULSE			E_AHI_DIO7_INT// Сброс внешнего сторожевого тамера
#define EXTERN_VCC_ON_OFF 			E_AHI_DIO8_INT// UPS
#define DIP_1 						E_AHI_DIO4_INT// 1 дип-переключателя
#define DIP_2 						E_AHI_DIO5_INT// 2 дип-переключателя
#define I2C_SCL 					E_AHI_DIO13_INT// SCL I2C
#define I2C_SDA 					E_AHI_DIO14_INT// SDA I2C
#define WDTIMER 					E_AHI_DIO19_INT// сторожевой таймер
#define LED_RED 					E_AHI_DIO15_INT
#define LED_GREEN		 			E_AHI_DIO16_INT
#define LED_BLUE 					E_AHI_DIO17_INT

#define RDX_48
/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

#define BCD_WHOLE_LENGTH	6// Длина целой части числа + один полубайт под знак// Только чётное число
#define BCD_FRACTION_LENGTH	2// Длина дробной части числа// Только чётное число
#define BCD_TYPE_MAXIMUM	0x9999999// Максимальное значение, которое может быть сохранено в BCD

typedef int32 Bcd_t;// Двоично-десятичный тип// Последний байт дробная часть -> максимальное число 9999.99

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

#if defined __cplusplus
}
#endif

#endif  /* CONFIG_H_INCLUDED */

/****************************************************************************/
/***        END OF FILE                                                   ***/
/****************************************************************************/

