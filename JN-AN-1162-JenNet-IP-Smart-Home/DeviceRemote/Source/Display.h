/****************************************************************************/
/*
 * MODULE              JN-AN-1162 JenNet-IP Smart Home
 *
 * DESCRIPTION         Display - DR1047 Controller Board LCD
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
#ifndef DISPLAY_H_
#define DISPLAY_H_

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/
#define DISPLAY_LCD		TRUE /* Display to LCD */
#define DISPLAY_TRM		TRUE /* Display to Terminal */

#define DISPLAY_COLS	  21 /* Display columns in characters */
#define DISPLAY_ROWS	   8 /* Display rows in characters */
#define DISPLAY_COL_PIXELS 6 /* Number of pixels per column */

/****************************************************************************/
/***        Type Definitions                                              ***/
/****************************************************************************/

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PUBLIC void vDisplay_Init(void);
PUBLIC void vDisplay_Draw(void);
PUBLIC void vDisplay_Clear(void);
PUBLIC void vDisplay_ClearRow(uint8 u8Row);
PUBLIC void vDisplay_String(uint8 u8Row, uint8 u8Col, char *pcString);
PUBLIC void vDisplay_Character(uint8 u8Row, uint8 u8Col, char cCharacter);
PUBLIC void vDisplay_Number (uint8 	u8Row,
							 uint8 	u8Col,
							 uint8  u8Base,			/**< Number base to use */
							 uint8  u8MinDigits,		/**< Minimum digits */
							 uint32 u32Num);		/**< Number to transmit */
PUBLIC void vDisplay_Logo(bool_t bLogo);
PUBLIC void vDisplay_Append(char cCharacter);

/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Inlined Functions                                            ***/
/****************************************************************************/


#endif /*DISPLAY_H_*/
