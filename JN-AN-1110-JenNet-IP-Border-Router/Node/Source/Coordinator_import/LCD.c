
#include <Api.h>
#include <AppHardwareApi_JN516x.h>

#include "LCD.h"
#include "Config.h"
#include "ErrorHandler.h"
#include "DriverI2C.h"
#include "StringSimple.h"
#include "Converter.h"
#include "Uart.h"
#include "DeviceDescriptor.h"

//---------------------------------------------------------------------------
#ifdef  TIC_149
#define LCD_WIDTH          132
#define LCD_HEIGHT         8
#define LCD_MAXSTRING      64
#define LCD_ADRESS         0x3c
#endif

#ifdef  TIC_48
#define LCD_WIDTH          128
#define LCD_HEIGHT         8//4
#define LCD_MAXSTRING      32
#define LCD_ADRESS         0x3c
#endif

#ifdef  RDX_154
#define LCD_WIDTH          132
#define LCD_HEIGHT         8
#define LCD_MAXSTRING      64
#define LCD_ADRESS         0x38
#define LCD_VBIAS          0x82
#endif

#ifdef  RDX_48
#define LCD_WIDTH          128
#define LCD_HEIGHT         8
#define LCD_MAXSTRING      32
#define LCD_ADRESS         0x38
#define LCD_VBIAS          0x82
#endif
PRIVATE uint8 m_LCDLinkTrueImg[1][6] = { { 0xC0, 0xA0, 0xBF, 0xBF, 0xA0, 0xC0 } };
PRIVATE uint8 m_LCDLinkFalseImg[1][6] =	{ { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 } };
PRIVATE uint8 m_LCDLinkCharge0Img[1][11] = { { 0x01, 0x01, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01 } };
PRIVATE uint8 m_LCDLinkCharge1Img[1][2] = { { 0x03, 0x03 } };
PRIVATE uint8 m_LCDLinkCharge2Img[1][2] = { { 0x0F, 0x0F } };
PRIVATE uint8 m_LCDLinkCharge3Img[1][2] = { { 0x3F, 0x3F } };
PRIVATE uint8 m_LCDLinkCharge4Img[1][2] = { { 0xFF, 0xFF } };
PRIVATE uint8 m_LCDBlinkOnImg[1][2] = { { 0xC0, 0xC0 } };
PRIVATE uint8 m_LCDBlinkOffImg[1][2] = { { 0x03, 0x03 } };
PRIVATE uint8 m_LCDChar0Img8[1][6] = { { 0x7E, 0xFF, 0x81, 0x81, 0xFF, 0x7E } };
PRIVATE uint8 m_LCDChar1Img8[1][4] = { { 0x40, 0x40, 0xFF, 0xFF } };
PRIVATE uint8 m_LCDChar2Img8[1][6] = { { 0x41, 0xC3, 0x87, 0x8D, 0xF9, 0x71 } };
PRIVATE uint8 m_LCDChar3Img8[1][6] = { { 0x42, 0xC3, 0x91, 0x91, 0xFF, 0x6E } };
PRIVATE uint8 m_LCDChar4Img8[1][6] = { { 0x0C, 0x3C, 0x74, 0xFF, 0xFF, 0x04 } };
PRIVATE uint8 m_LCDChar5Img8[1][6] = { { 0xF2, 0xF3, 0xA1, 0xA1, 0xBF, 0x9E } };
PRIVATE uint8 m_LCDChar6Img8[1][6] = { { 0x7E, 0xFF, 0x91, 0x91, 0xDF, 0x4E } };
PRIVATE uint8 m_LCDChar7Img8[1][6] = { { 0x80, 0x87, 0x8F, 0xBC, 0xF0, 0xC0 } };
PRIVATE uint8 m_LCDChar8Img8[1][6] = { { 0x6E, 0xFF, 0x91, 0x91, 0xFF, 0x6E } };
PRIVATE uint8 m_LCDChar9Img8[1][6] = { { 0x62, 0xF3, 0x91, 0x91, 0xFF, 0x7E } };
PRIVATE uint8 m_LCDCharAImg8[1][8] = { { 0x03, 0x1F, 0x3C, 0xE4, 0xE4, 0x3C, 0x1F, 0x03 } };
PRIVATE uint8 m_LCDCharBImg8[1][6] = { { 0xFF, 0xFF, 0x91, 0x91, 0xFF, 0x6E } };
PRIVATE uint8 m_LCDCharCImg8[1][7] = { { 0x7E, 0xFF, 0x81, 0x81, 0x81, 0xC3, 0x42 } };
PRIVATE uint8 m_LCDCharDImg8[1][7] = { { 0xFF, 0xFF, 0x81, 0x81, 0xC3, 0x7E, 0x3C } };
PRIVATE uint8 m_LCDCharEImg8[1][6] = { { 0xFF, 0xFF, 0x91, 0x91, 0x91, 0x81 } };
PRIVATE uint8 m_LCDCharFImg8[1][6] = { { 0xFF, 0xFF, 0x88, 0x88, 0x88, 0x80 } };
PRIVATE uint8 m_LCDCharGImg8[1][7] = { { 0x7E, 0xFF, 0x81, 0x89, 0x8B, 0xCF, 0x4E } };
PRIVATE uint8 m_LCDCharHImg8[1][7] = { { 0xFF, 0xFF, 0x10, 0x10, 0x10, 0xFF, 0xFF } };
PRIVATE uint8 m_LCDCharIImg8[1][2] = { { 0xFF, 0xFF } };
PRIVATE uint8 m_LCDCharJImg8[1][5] = { { 0x06, 0x07, 0x01, 0xFF, 0xFE } };
PRIVATE uint8 m_LCDCharKImg8[1][7] = { { 0xFF, 0xFF, 0x38, 0x6C, 0xC6, 0x83, 0x01 } };
PRIVATE uint8 m_LCDCharLImg8[1][6] = { { 0xFF, 0xFF, 0x01, 0x01, 0x01, 0x01 } };
PRIVATE uint8 m_LCDCharMImg8[1][8] = { { 0xFF, 0xFF, 0x38, 0x1E, 0x1E, 0x38, 0xFF, 0xFF } };
PRIVATE uint8 m_LCDCharNImg8[1][7] = { { 0xFF, 0xFF, 0x70, 0x1C, 0x06, 0xFF, 0xFF } };
PRIVATE uint8 m_LCDCharOImg8[1][7] = { { 0x7E, 0xFF, 0x81, 0x81, 0x81, 0xFF, 0x7E } };
PRIVATE uint8 m_LCDCharPImg8[1][7] = { { 0xFF, 0xFF, 0x88, 0x88, 0x88, 0xF8, 0x70 } };
PRIVATE uint8 m_LCDCharQImg8[1][7] = { { 0x7E, 0xFF, 0x81, 0x85, 0x87, 0xFF, 0x7D } };
PRIVATE uint8 m_LCDCharRImg8[1][7] = { { 0xFF, 0xFF, 0x90, 0x90, 0x90, 0xFF, 0x6F } };
PRIVATE uint8 m_LCDCharSImg8[1][6] = { { 0x62, 0xF3, 0x91, 0x91, 0xDF, 0x4E } };
PRIVATE uint8 m_LCDCharTImg8[1][6] = { { 0x80, 0x80, 0xFF, 0xFF, 0x80, 0x80 } };
PRIVATE uint8 m_LCDCharUImg8[1][7] = { { 0xFE, 0xFF, 0x01, 0x01, 0x01, 0xFF,0xFE } };
PRIVATE uint8 m_LCDCharVImg8[1][8] = { { 0xC0, 0xF0, 0x3C, 0x0F, 0x0F, 0x3C, 0xF0, 0xC0 } };
PRIVATE uint8 m_LCDCharWImg8[1][12] = { { 0xC0, 0xF0, 0x3C, 0x0F, 0x0F, 0x3C, 0x3C, 0x0F, 0x0F, 0x3C, 0xF0, 0xC0 } };
PRIVATE uint8 m_LCDCharXImg8[1][7] = { { 0xC3, 0xE7, 0x3C, 0x18, 0x3C, 0xE7, 0xC3 } };
PRIVATE uint8 m_LCDCharYImg8[1][8] = { { 0xC0, 0xE0, 0x30, 0x1F, 0x1F, 0x30, 0xE0, 0xC0 } };
PRIVATE uint8 m_LCDCharZImg8[1][7] = { { 0x83, 0x87, 0x8D, 0x99, 0xB1, 0xE1, 0xC1 } };
PRIVATE uint8 m_LCDCharPointImg8[1][2] = { { 0x03, 0x03 } };
PRIVATE uint8 m_LCDChar0Img24[3][11] =
		{
				{ 0x03, 0x03, 0x0F, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0F, 0x03, 0x03 },
				{ 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF },
				{ 0xFC, 0xFC, 0xFF, 0x03, 0x03, 0x03, 0x03, 0x03, 0xFF, 0xFC, 0xFC }
		};
PRIVATE uint8 m_LCDChar1Img24[3][7] =
		{
				{ 0x03, 0x03, 0x03, 0x03, 0x0F, 0x0F, 0x0F },
				{ 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF },
				{ 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF }
		};
PRIVATE uint8 m_LCDChar2Img24[3][11] =
		{
				{ 0x03, 0x03, 0x0F, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0F, 0x03, 0x03 },
				{ 0xC0, 0xC0, 0xC0, 0x00, 0x03, 0x03, 0x0F, 0x0C, 0xFC, 0xF0, 0xF0 },
				{ 0x3F, 0x3F, 0xFF, 0xC3, 0xC3, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03 }
		};
PRIVATE uint8 m_LCDChar3Img24[3][11] =
		{
				{ 0x03, 0x03, 0x0F, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0F, 0x03, 0x03 },
				{ 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x0C, 0x0C, 0xFF, 0xF3, 0xF3 },
				{ 0x0C, 0x0C, 0x0F, 0x03, 0x03, 0x03, 0x03, 0x03, 0xFF, 0xFC, 0xFC }
		};
PRIVATE uint8 m_LCDChar4Img24[3][11] =
		{
				{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x0F, 0x0F, 0x0F },
				{ 0x03, 0x03, 0x1F, 0x1C, 0xFC, 0xF0, 0xF0, 0x80, 0xFF, 0xFF, 0xFF },
				{ 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xFF, 0xFF, 0xFF }
		};
PRIVATE uint8 m_LCDChar5Img24[3][11] =
		{
				{ 0x0F, 0x0F, 0x0F, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C },
				{ 0xFF, 0xFF, 0xFF, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0F, 0x03, 0x03 },
				{ 0x0C, 0x0C, 0x0F, 0x03, 0x03, 0x03, 0x03, 0x03, 0xFF, 0xFC, 0xFC }
		};
PRIVATE uint8 m_LCDChar6Img24[3][11] =
		{
				{ 0x03, 0x03, 0x0F, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0F, 0x03, 0x03 },
				{ 0xFF, 0xFF, 0xFF, 0x03, 0x0F, 0x0C, 0x0C, 0x0C, 0x0F, 0x03, 0x03 },
				{ 0xFC, 0xFC, 0xFF, 0x03, 0x03, 0x03, 0x03, 0x03, 0xFF, 0xFC, 0xFC }
		};
PRIVATE uint8 m_LCDChar7Img24[3][11] =
		{
				{ 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0F, 0x0F, 0x0F },
				{ 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x3F, 0x3C, 0xFC, 0xC0, 0xC0 },
				{ 0x00, 0x00, 0x3F, 0x3F, 0xFF, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00 }
		};
PRIVATE uint8 m_LCDChar8Img24[3][11] =
		{
				{ 0x03, 0x03, 0x0F, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0F, 0x03, 0x03 },
				{ 0xF3, 0xF3, 0xFF, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0xFF, 0xF3, 0xF3 },
				{ 0xFC, 0xFC, 0xFF, 0x03, 0x03, 0x03, 0x03, 0x03, 0xFF, 0xFC, 0xFC }
		};
PRIVATE uint8 m_LCDChar9Img24[3][11] =
		{
				{ 0x03, 0x03, 0x0F, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0F, 0x03, 0x03 },
				{ 0xFC, 0xFC, 0xFF, 0x03, 0x03, 0x03, 0x03, 0x06, 0xFF, 0xFF, 0xFF },
				{ 0x0C, 0x0C, 0x0F, 0x03, 0x03, 0x03, 0x03, 0x03, 0xFF, 0xFC, 0xFC }
		};

PRIVATE uint8 m_LCDCharCImg24[3][13] =
		{
				{ 0x00, 0x00, 0x03, 0x0F, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0F, 0x03, 0x03 },
				{ 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
				{ 0xF0, 0xF0, 0xFC, 0x0F, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x0F, 0x0C, 0x0C }
		};

PRIVATE uint8 m_LCDCharEImg24[3][10] =
		{
				{ 0x0F, 0x0F, 0x0F, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C },
				{ 0xFF, 0xFF, 0xFF, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x00 },
				{ 0xFF, 0xFF, 0xFF, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03 }
		};

PRIVATE uint8 m_LCDCharGradImg24[3][9] =
		{
				{ 0x03, 0x03, 0x0F, 0x0C, 0x0C, 0x0C, 0x0F, 0x03, 0x03 },
				{ 0xC0, 0xC0, 0xF0, 0x30, 0x30, 0x30, 0xF0, 0xC0, 0xC0 },
				{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
		};
PRIVATE uint8 m_LCDCharProcImg24[3][21] =
		{
				{ 0x03, 0x03, 0x0F, 0x0C, 0x0C, 0x0C, 0x0F, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x03, 0x00, 0x00 },
				{ 0xC0, 0xC0, 0xF0, 0x30, 0x30, 0x30, 0xF0, 0xC0, 0xC3, 0x03, 0x0F, 0x0C, 0x3C, 0x30, 0xF0, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00 },
				{ 0x00, 0x00, 0x0C, 0x0C, 0x3C, 0x30, 0xF0, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x3C, 0x3C, 0xFF, 0xC3, 0xC3, 0xC3, 0xFF, 0x3C, 0x3C }
		};

PRIVATE uint8 m_LCDCharPointImg24[3][3] =
		{
				{ 0x00, 0x00, 0x00 },
				{ 0x00, 0x00, 0x00 },
				{ 0x07, 0x07, 0x07 }
		};
PRIVATE uint8 m_LCDCharDashImg24[3][9] =
		{
				{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
				{ 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03 },
				{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
		};

typedef enum
{
	ctSingle = 0, ctStartSequence,
	ctSequence
} LCDComandType;

typedef enum
{
	ffMSSansSerif8 = 0, ffMSSansSerif24
} LCDFontFamily;

typedef enum
{
	aLeft = 0, aRight, aCenter
} LCDAlign;

typedef struct
{
	uint8 height;
	uint8 width;
	uint8* img;
} LCDImage;
PRIVATE LCDImage m_LCDLinkTrueImage, m_LCDLinkFalseImage, m_LCDLinkCharge0Image,
		m_LCDLinkCharge1Image, m_LCDLinkCharge2Image, m_LCDLinkCharge3Image,
		m_LCDLinkCharge4Image;
PRIVATE LCDImage m_LCDBlinkOnImage, m_LCDBlinkOffImage;
PRIVATE LCDImage m_LCDChar0Image8, m_LCDChar1Image8, m_LCDChar2Image8,
		m_LCDChar3Image8, m_LCDChar4Image8, m_LCDChar5Image8, m_LCDChar6Image8,
		m_LCDChar7Image8, m_LCDChar8Image8, m_LCDChar9Image8, m_LCDCharAImage8,
		m_LCDCharBImage8, m_LCDCharCImage8, m_LCDCharDImage8, m_LCDCharEImage8,
		m_LCDCharFImage8, m_LCDCharGImage8, m_LCDCharHImage8, m_LCDCharIImage8,
		m_LCDCharJImage8, m_LCDCharKImage8, m_LCDCharLImage8, m_LCDCharMImage8,
		m_LCDCharNImage8, m_LCDCharOImage8, m_LCDCharPImage8, m_LCDCharQImage8,
		m_LCDCharRImage8, m_LCDCharSImage8, m_LCDCharTImage8, m_LCDCharUImage8,
		m_LCDCharVImage8, m_LCDCharWImage8, m_LCDCharXImage8, m_LCDCharYImage8,
		m_LCDCharZImage8, m_LCDCharPointImage8;
PRIVATE LCDImage m_LCDChar0Image24, m_LCDChar1Image24, m_LCDChar2Image24,
		m_LCDChar3Image24, m_LCDChar4Image24, m_LCDChar5Image24,
		m_LCDChar6Image24, m_LCDChar7Image24, m_LCDChar8Image24,
		m_LCDChar9Image24, m_LCDCharCImage24, m_LCDCharGradImage24,
		m_LCDCharProcImage24, m_LCDCharPointImage24, m_LCDCharDashImage24, m_LCDCharEImage24;
PRIVATE uint8 m_LCDMonitor[LCD_HEIGHT][LCD_WIDTH];
PRIVATE LCDImage *m_LCDFont8[37], *m_LCDFont24[16];
PRIVATE char m_LCDNetID[4];
PRIVATE char m_LCDDevName[32];
PRIVATE char m_LCDDevStat[16];
PRIVATE DriverI2CInfo m_LCDI2CInfo;
PRIVATE uint8 m_X = 0, m_Y = 0;
PRIVATE char m_LCDText[32];
PRIVATE char m_LCDDebug[3][16];
PRIVATE bool m_LCDLinkAsText;
PRIVATE bool m_LCDBlink;
PRIVATE uint32 m_WDTDio; // для WDT
PRIVATE void LCDSetAddress(uint8 x, uint8 y);
PRIVATE void LCDImageInit(LCDImage* image, uint8 height, uint8 width, uint8* img);
PRIVATE void LCDClear(void);
PRIVATE void LCDSend(LCDComandType type, uint8 val);
PRIVATE void LCDDrawImage(LCDImage lcdImage, uint8 x, uint8 y, bool fromLastPos);
PRIVATE void LCDDrawText(char* text, uint8 row, LCDAlign align,
		LCDFontFamily font);
PRIVATE uint8 LCDGetFontIndex(char ch, LCDFontFamily font);
PRIVATE uint16 LCDTextWidth(char* str, LCDFontFamily font);
PRIVATE uint8 LCDGetMaxWidth(uint8 row, LCDFontFamily font);
PRIVATE uint8 LCDGetLeftStartPos(uint8 row, LCDFontFamily font);
PRIVATE uint8 LCDGetRightStopPos(uint8 row, LCDFontFamily font);
PRIVATE uint8 LCDGetAlignStart(uint8 row, LCDFontFamily font, LCDAlign align,
		uint8 textWidth);
PRIVATE void LCDFormatText(char* frmtText, char* text, uint8 maxLength,
		LCDFontFamily font, uint8 maxWidth);
PRIVATE void LCDDrawCoordGUI(void);
PRIVATE void LCDMakeVersion(char* str);
//---------------------------------------------------------------------------

PUBLIC void LCDInit(void)
{
	m_WDTDio = WDTIMER; // для WDT
	m_LCDDebug[0][0] = '\0';
	m_LCDDebug[1][0] = '\0';
	m_LCDDebug[2][0] = '\0';
	m_LCDText[0] = '\0';
	m_LCDNetID[0] = '\0';
	m_LCDDevName[0] = '\0';
	m_LCDDevStat[0] = '\0';
	m_LCDLinkAsText = FALSE;
	m_LCDBlink = FALSE;

	// Link
	LCDImageInit(&m_LCDLinkTrueImage, 1, 6, &m_LCDLinkTrueImg[0][0]);
	LCDImageInit(&m_LCDLinkFalseImage, 1, 6, &m_LCDLinkFalseImg[0][0]);
	LCDImageInit(&m_LCDLinkCharge0Image, 1, 11, &m_LCDLinkCharge0Img[0][0]);
	LCDImageInit(&m_LCDLinkCharge1Image, 1, 2, &m_LCDLinkCharge1Img[0][0]);
	LCDImageInit(&m_LCDLinkCharge2Image, 1, 2, &m_LCDLinkCharge2Img[0][0]);
	LCDImageInit(&m_LCDLinkCharge3Image, 1, 2, &m_LCDLinkCharge3Img[0][0]);
	LCDImageInit(&m_LCDLinkCharge4Image, 1, 2, &m_LCDLinkCharge4Img[0][0]);

	// Blink
	LCDImageInit(&m_LCDBlinkOnImage, 1, 2, &m_LCDBlinkOnImg[0][0]);
	LCDImageInit(&m_LCDBlinkOffImage, 1, 2, &m_LCDBlinkOffImg[0][0]);

	// Font8
	LCDImageInit(&m_LCDChar0Image8, 1, 6, &m_LCDChar0Img8[0][0]);
	m_LCDFont8[0] = &m_LCDChar0Image8;
	LCDImageInit(&m_LCDChar1Image8, 1, 4, &m_LCDChar1Img8[0][0]);
	m_LCDFont8[1] = &m_LCDChar1Image8;
	LCDImageInit(&m_LCDChar2Image8, 1, 6, &m_LCDChar2Img8[0][0]);
	m_LCDFont8[2] = &m_LCDChar2Image8;
	LCDImageInit(&m_LCDChar3Image8, 1, 6, &m_LCDChar3Img8[0][0]);
	m_LCDFont8[3] = &m_LCDChar3Image8;
	LCDImageInit(&m_LCDChar4Image8, 1, 6, &m_LCDChar4Img8[0][0]);
	m_LCDFont8[4] = &m_LCDChar4Image8;
	LCDImageInit(&m_LCDChar5Image8, 1, 6, &m_LCDChar5Img8[0][0]);
	m_LCDFont8[5] = &m_LCDChar5Image8;
	LCDImageInit(&m_LCDChar6Image8, 1, 6, &m_LCDChar6Img8[0][0]);
	m_LCDFont8[6] = &m_LCDChar6Image8;
	LCDImageInit(&m_LCDChar7Image8, 1, 6, &m_LCDChar7Img8[0][0]);
	m_LCDFont8[7] = &m_LCDChar7Image8;
	LCDImageInit(&m_LCDChar8Image8, 1, 6, &m_LCDChar8Img8[0][0]);
	m_LCDFont8[8] = &m_LCDChar8Image8;
	LCDImageInit(&m_LCDChar9Image8, 1, 6, &m_LCDChar9Img8[0][0]);
	m_LCDFont8[9] = &m_LCDChar9Image8;
	LCDImageInit(&m_LCDCharAImage8, 1, 8, &m_LCDCharAImg8[0][0]);
	m_LCDFont8[10] = &m_LCDCharAImage8;
	LCDImageInit(&m_LCDCharBImage8, 1, 6, &m_LCDCharBImg8[0][0]);
	m_LCDFont8[11] = &m_LCDCharBImage8;
	LCDImageInit(&m_LCDCharCImage8, 1, 7, &m_LCDCharCImg8[0][0]);
	m_LCDFont8[12] = &m_LCDCharCImage8;
	LCDImageInit(&m_LCDCharDImage8, 1, 7, &m_LCDCharDImg8[0][0]);
	m_LCDFont8[13] = &m_LCDCharDImage8;
	LCDImageInit(&m_LCDCharEImage8, 1, 6, &m_LCDCharEImg8[0][0]);
	m_LCDFont8[14] = &m_LCDCharEImage8;
	LCDImageInit(&m_LCDCharFImage8, 1, 6, &m_LCDCharFImg8[0][0]);
	m_LCDFont8[15] = &m_LCDCharFImage8;
	LCDImageInit(&m_LCDCharGImage8, 1, 7, &m_LCDCharGImg8[0][0]);
	m_LCDFont8[16] = &m_LCDCharGImage8;
	LCDImageInit(&m_LCDCharHImage8, 1, 7, &m_LCDCharHImg8[0][0]);
	m_LCDFont8[17] = &m_LCDCharHImage8;
	LCDImageInit(&m_LCDCharIImage8, 1, 2, &m_LCDCharIImg8[0][0]);
	m_LCDFont8[18] = &m_LCDCharIImage8;
	LCDImageInit(&m_LCDCharJImage8, 1, 5, &m_LCDCharJImg8[0][0]);
	m_LCDFont8[29] = &m_LCDCharJImage8;
	LCDImageInit(&m_LCDCharKImage8, 1, 7, &m_LCDCharKImg8[0][0]);
	m_LCDFont8[20] = &m_LCDCharKImage8;
	LCDImageInit(&m_LCDCharLImage8, 1, 6, &m_LCDCharLImg8[0][0]);
	m_LCDFont8[21] = &m_LCDCharLImage8;
	LCDImageInit(&m_LCDCharMImage8, 1, 8, &m_LCDCharMImg8[0][0]);
	m_LCDFont8[22] = &m_LCDCharMImage8;
	LCDImageInit(&m_LCDCharNImage8, 1, 7, &m_LCDCharNImg8[0][0]);
	m_LCDFont8[23] = &m_LCDCharNImage8;
	LCDImageInit(&m_LCDCharOImage8, 1, 7, &m_LCDCharOImg8[0][0]);
	m_LCDFont8[24] = &m_LCDCharOImage8;
	LCDImageInit(&m_LCDCharPImage8, 1, 7, &m_LCDCharPImg8[0][0]);
	m_LCDFont8[25] = &m_LCDCharPImage8;
	LCDImageInit(&m_LCDCharQImage8, 1, 7, &m_LCDCharQImg8[0][0]);
	m_LCDFont8[26] = &m_LCDCharQImage8;
	LCDImageInit(&m_LCDCharRImage8, 1, 7, &m_LCDCharRImg8[0][0]);
	m_LCDFont8[27] = &m_LCDCharRImage8;
	LCDImageInit(&m_LCDCharSImage8, 1, 6, &m_LCDCharSImg8[0][0]);
	m_LCDFont8[28] = &m_LCDCharSImage8;
	LCDImageInit(&m_LCDCharTImage8, 1, 6, &m_LCDCharTImg8[0][0]);
	m_LCDFont8[29] = &m_LCDCharTImage8;
	LCDImageInit(&m_LCDCharUImage8, 1, 7, &m_LCDCharUImg8[0][0]);
	m_LCDFont8[30] = &m_LCDCharUImage8;
	LCDImageInit(&m_LCDCharVImage8, 1, 8, &m_LCDCharVImg8[0][0]);
	m_LCDFont8[31] = &m_LCDCharVImage8;
	LCDImageInit(&m_LCDCharWImage8, 1, 12, &m_LCDCharWImg8[0][0]);
	m_LCDFont8[32] = &m_LCDCharWImage8;
	LCDImageInit(&m_LCDCharXImage8, 1, 7, &m_LCDCharXImg8[0][0]);
	m_LCDFont8[33] = &m_LCDCharXImage8;
	LCDImageInit(&m_LCDCharYImage8, 1, 8, &m_LCDCharYImg8[0][0]);
	m_LCDFont8[34] = &m_LCDCharYImage8;
	LCDImageInit(&m_LCDCharZImage8, 1, 7, &m_LCDCharZImg8[0][0]);
	m_LCDFont8[35] = &m_LCDCharZImage8;
	LCDImageInit(&m_LCDCharPointImage8, 1, 2, &m_LCDCharPointImg8[0][0]);
	m_LCDFont8[36] = &m_LCDCharPointImage8;

	// Font24
	LCDImageInit(&m_LCDChar0Image24, 3, 11, &m_LCDChar0Img24[0][0]);
	m_LCDFont24[0] = &m_LCDChar0Image24;
	LCDImageInit(&m_LCDChar1Image24, 3, 7, &m_LCDChar1Img24[0][0]);
	m_LCDFont24[1] = &m_LCDChar1Image24;
	LCDImageInit(&m_LCDChar2Image24, 3, 11, &m_LCDChar2Img24[0][0]);
	m_LCDFont24[2] = &m_LCDChar2Image24;
	LCDImageInit(&m_LCDChar3Image24, 3, 11, &m_LCDChar3Img24[0][0]);
	m_LCDFont24[3] = &m_LCDChar3Image24;
	LCDImageInit(&m_LCDChar4Image24, 3, 11, &m_LCDChar4Img24[0][0]);
	m_LCDFont24[4] = &m_LCDChar4Image24;
	LCDImageInit(&m_LCDChar5Image24, 3, 11, &m_LCDChar5Img24[0][0]);
	m_LCDFont24[5] = &m_LCDChar5Image24;
	LCDImageInit(&m_LCDChar6Image24, 3, 11, &m_LCDChar6Img24[0][0]);
	m_LCDFont24[6] = &m_LCDChar6Image24;
	LCDImageInit(&m_LCDChar7Image24, 3, 11, &m_LCDChar7Img24[0][0]);
	m_LCDFont24[7] = &m_LCDChar7Image24;
	LCDImageInit(&m_LCDChar8Image24, 3, 11, &m_LCDChar8Img24[0][0]);
	m_LCDFont24[8] = &m_LCDChar8Image24;
	LCDImageInit(&m_LCDChar9Image24, 3, 11, &m_LCDChar9Img24[0][0]);
	m_LCDFont24[9] = &m_LCDChar9Image24;

	LCDImageInit(&m_LCDCharCImage24, 3, 13, &m_LCDCharCImg24[0][0]);
	m_LCDFont24[10] = &m_LCDCharCImage24;
	LCDImageInit(&m_LCDCharGradImage24, 3, 9, &m_LCDCharGradImg24[0][0]);
	m_LCDFont24[11] = &m_LCDCharGradImage24;
	LCDImageInit(&m_LCDCharProcImage24, 3, 21, &m_LCDCharProcImg24[0][0]);
	m_LCDFont24[12] = &m_LCDCharProcImage24;
	LCDImageInit(&m_LCDCharPointImage24, 3, 3, &m_LCDCharPointImg24[0][0]);
	m_LCDFont24[13] = &m_LCDCharPointImage24;
	LCDImageInit(&m_LCDCharDashImage24, 3, 9, &m_LCDCharDashImg24[0][0]);
	m_LCDFont24[14] = &m_LCDCharDashImage24;
	LCDImageInit(&m_LCDCharEImage24, 3, 10, &m_LCDCharEImg24[0][0]);
	m_LCDFont24[15] = &m_LCDCharEImage24;
	DriverI2CInit(&m_LCDI2CInfo, I2C_SDA, I2C_SCL, LCD_ADRESS, "110100", "011110");

		LCDPower(FALSE);
		LCDPower(TRUE);

		// set modes
		if (DriverI2CStart(&m_LCDI2CInfo, TRUE))
		{
		#ifdef TIC_149
		LCDSend(ctSingle, 0x01); // switch to function and RAM command page   //i2c_write(0x01);      на основн стр
		LCDSend(ctSingle, 0xe); // switch to display setting command page     //i2c_write(0b00001110); на стр 110
		LCDSend(ctSingle, 0x84); // Multiplex rate = 1/34 		      //i2c_write(0b10000100); MUX = 1/65
		LCDSend(ctSingle, 0x6); // Normal mode
		LCDSend(ctSingle, 0xe); // Mirror X Mirror Y
		LCDSend(ctSingle, 0x12); // Bias system 3
		LCDSend(ctSingle, 0x24); // IB <- 1				      //i2c_write(0b00100100); IB = 1
		LCDSend(ctSingle, 0x01); // switch to function and RAM command page
		LCDSend(ctSingle, 0xD); // switch to HV-gen command page
		LCDSend(ctSingle, 0x5); // VLCD programming range HIGH, voltage multiplier enabled
		LCDSend(ctSingle, 0x17); // temperature coeficient TC0///17
		LCDSend(ctSingle, 0x9); // 2 * voltage multiplier//0x8
		LCDSend(ctSingle, (1 << 7) | 56); //отличная контрастность, если больше, то будет засветка черным фоном
		LCDSend(ctSingle, 0x01); // switch to function and RAM command page
		LCDSend(ctSingle, 0xB); // switch to special-fit command page
		LCDSend(ctSingle, (1 << 6) | (1 << 4) | (1 << 3));
		#endif

		#ifdef 	RDX_154
		//настройка контраста
	        LCDSend(ctSingle, 0xEB);//установить отношение BIAS=9
	        LCDSend(ctSingle, 0x81);//команда настройки значения потенциомера VBIAS
	        LCDSend(ctSingle, LCD_VBIAS);//рекомендуемое значение для угла обзора 90 градусов
	        //развёртка дисплея
	        LCDSend(ctSingle, 0xC2);//сверху вниз, слева направо
			LCDSend(ctSingle, 0x24); //set temperature compensation, TC[1:0]
			LCDSend(ctSingle, 0x03);
			//TC[1:0] = 0: 0x00 temp. coeff.  -0.05%/°C (default)
			//TC[1:0] = 1: 0x01 temp. coeff.  -0.10%/°C
			//TC[1:0] = 2: 0x02 temp. coeff.  -0.15%/°C
			//TC[1:0] = 3: 0x03 temp. coeff.  -0.00%/°C
		#endif

		#ifdef 	RDX_48
		//настройка контраста
	        LCDSend(ctSingle, 0xE9);//установить отношение BIAS=7
	        LCDSend(ctSingle, 0x81);//команда настройки значения потенциомера VBIAS
	        LCDSend(ctSingle, LCD_VBIAS);//рекомендуемое значение для угла обзора 90 градусов
	        //развёртка дисплея
	        LCDSend(ctSingle, 0xC0);//сверху вниз, слева направо
	        //LCDSend(ctSingle, 0x81);//команда порядок записи в RAM
	        LCDSend(ctSingle, 0x89);
	        LCDSend(ctSingle, 0x24); //set temperature compensation, TC[1:0]
	        LCDSend(ctSingle, 0x03);
	        //TC[1:0] = 0: 0x00 temp. coeff.  -0.05%/°C (default)
	        //TC[1:0] = 1: 0x01 temp. coeff.  -0.10%/°C
	        //TC[1:0] = 2: 0x02 temp. coeff.  -0.15%/°C
	        //TC[1:0] = 3: 0x03 temp. coeff.  -0.00%/°C
		#endif

		#ifdef  TIC_48
		//LCDSend(ctSingle, 0x01); // switch to function and RAM command page
		LCDSend(ctSingle, 0x9); // switch to display setting command page
		LCDSend(ctSingle, 0x5); // Multiplex rate = 1/34
		LCDSend(ctSingle, 0xC); // Normal mode
		LCDSend(ctSingle, 0x14); // Bias system 3
		LCDSend(ctSingle, 0x01); // switch to function and RAM command page
		LCDSend(ctSingle, 0xA); // switch to HV-gen command page
		LCDSend(ctSingle, 0x5); // VLCD programming range HIGH, voltage multiplier enabled
		LCDSend(ctSingle, 0x27); // temperature coefficient TC0//20
		LCDSend(ctSingle, 0x09); // 3 * voltage multiplier
		LCDSend(ctSingle, (1 << 7) | 64);
		#endif

			DriverI2CStop(&m_LCDI2CInfo);
		} else
			EHCall(LCD_MOD_ID, 1);

		// clear lcd
		LCDUpdate();
		return;
	}
	//---------------------------------------------------------------------------

	PUBLIC void LCDPower(bool on){

	    DriverI2CStart(&m_LCDI2CInfo, TRUE);

	    if (DriverI2CStart(&m_LCDI2CInfo, TRUE)){

		#ifdef TIC_48
		LCDSend(ctSingle, 0x01); // switch to function and RAM command page
		LCDSend(ctSingle, (1 << 5) | (((on) ? 0 : 1) << 2) | (0 << 1)); // PowerDown, Vertical addressing
		#endif

		#ifdef TIC_149
		LCDSend(ctSingle, 0x01); // switch to function and RAM command page
		LCDSend(ctSingle, (1 << 4) | (((on) ? 0 : 1) << 2) | (0 << 1)); // PowerDown, Vertical addressing
		#endif

		#ifdef RDX_154
	        LCDSend(ctSingle, 0xAE|on); //включение дисплея
		#endif

		#ifdef RDX_48
	        LCDSend(ctSingle, 0xAE|on); //включение дисплея
		#endif
		DriverI2CStop(&m_LCDI2CInfo);
		}
	}
//---------------------------------------------------------------------------

PUBLIC void LCDSetNet(uint8 netID)
{
	ConverterDigToStr(netID, m_LCDNetID, 10);
}
//---------------------------------------------------------------------------

PUBLIC void LCDSetDevStat(uint8 devCount, uint8 Routing, uint8 errorModule, uint8 errorCode)
{
	char digit[4];
	sstrcpy(m_LCDDevStat, ConverterDigToStr(devCount, digit, 10));
		sstrcat(m_LCDDevStat, "-");
		sstrcat(m_LCDDevStat, ConverterDigToStr( u16Api_GetRoutingTableSize(), digit, 10));
	if (MAIN_DEVICE_TYPE == dtCoordinator && errorModule != 0)
	{
		sstrcat(m_LCDDevStat, "  E");
		sstrcat(m_LCDDevStat, ConverterDigToStr(errorModule, digit, 10));
		sstrcat(m_LCDDevStat, "-");
		sstrcat(m_LCDDevStat, ConverterDigToStr(errorCode, digit, 10));
	}
}

//---------------------------------------------------------------------------

PUBLIC void LCDSetDevName(char* name)
{
	sstrcpy(m_LCDDevName, name);
}
//---------------------------------------------------------------------------

PUBLIC void LCDSetLinkDrawAsText(bool asText)
{
	m_LCDLinkAsText = asText;
}
//---------------------------------------------------------------------------

PUBLIC void LCDSetText(char* str)
{
	sstrcpy(m_LCDText, str);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
PUBLIC void LCDDebug(char* str, uint8 line) // отладка, вывод выбранной строки
{
	sstrcpy(m_LCDDebug[line], str);
}
//---------------------------------------------------------------------------
PUBLIC void LCDDebugWithDig(char* str, uint64 dig, uint8 line) // вывод выбранного значения
{
	char strMsg[64];
	sstrcpy(strMsg, str);
	char strDig[64];
	ConverterDigToStr(dig, strDig, 10);
	sstrcat(strMsg, strDig);
	LCDDebug(strMsg, line);
}
//---------------------------------------------------------------------------

PUBLIC void LCDUpdate(void){

    static bool first = TRUE;

    LCDClear();
    if (!first) LCDDrawCoordGUI();

    #ifndef RDX_48
    LCDSetAddress(0, 0);
    #else
    LCDSetAddress(0, 40);
	#endif

    #ifdef TIC_149//|TIC_48
    static uint8 m_LCDPrevMonitor[LCD_HEIGHT][LCD_WIDTH];
    uint8 r, c, val;
        bool sequence = FALSE;
    for (r = 0; r < LCD_HEIGHT; r++)
	{
	for (c = 0; c < (LCD_WIDTH - 0); c++)
	    {
	    if (first || m_LCDMonitor[r][c] != m_LCDPrevMonitor[r][c])
		{
		val = m_LCDMonitor[r][c];

#ifndef TIC_149
		val = (((val >> 0) & 1) << 7) | (((val >> 1) & 1) << 6)
			| (((val >> 2) & 1) << 5) | (((val >> 3) & 1) << 4)
			| (((val >> 4) & 1) << 3) | (((val >> 5) & 1) << 2)
			| (((val >> 6) & 1) << 1) | (((val >> 7) & 1) << 0); // first bit = last bit

#endif
		m_LCDPrevMonitor[r][c] = m_LCDMonitor[r][c];
		if (!sequence)
		    {
		    LCDSetAddress(c, r);
		    DriverI2CStart(&m_LCDI2CInfo, TRUE);
		    LCDSend(ctStartSequence, val);
		    sequence = TRUE;
		    }
		else
		    LCDSend(ctSequence, val);
		}
	    else
		{
		if (sequence)
		DriverI2CStop(&m_LCDI2CInfo);
		sequence = FALSE;
		}
	    }
	}
    if (sequence)
    DriverI2CStop(&m_LCDI2CInfo);
    first = FALSE;
    #endif

#ifdef TIC_48
   static uint8 m_LCDPrevMonitor[LCD_HEIGHT][LCD_WIDTH];
   uint8 r, c, val;
       bool sequence = FALSE;
   for (r = 0; r < LCD_HEIGHT; r++)
	{
	for (c = 0; c < (LCD_WIDTH - 0); c++)
	    {
	    if (first || m_LCDMonitor[r][c] != m_LCDPrevMonitor[r][c])
		{
		val = m_LCDMonitor[r][c];

#ifndef TIC_149
		val = (((val >> 0) & 1) << 7) | (((val >> 1) & 1) << 6)
			| (((val >> 2) & 1) << 5) | (((val >> 3) & 1) << 4)
			| (((val >> 4) & 1) << 3) | (((val >> 5) & 1) << 2)
			| (((val >> 6) & 1) << 1) | (((val >> 7) & 1) << 0); // first bit = last bit

#endif
		m_LCDPrevMonitor[r][c] = m_LCDMonitor[r][c];
		if (!sequence)
		    {
		    LCDSetAddress(c, r);
		    DriverI2CStart(&m_LCDI2CInfo, TRUE);
		    LCDSend(ctStartSequence, val);
		    sequence = TRUE;
		    }
		else
		    LCDSend(ctSequence, val);
		}
	    else
		{
		if (sequence)
		DriverI2CStop(&m_LCDI2CInfo);
		sequence = FALSE;
		}
	    }
	}
   if (sequence)
   DriverI2CStop(&m_LCDI2CInfo);
   first = FALSE;
   #endif

    #ifdef RDX_154
    m_LCDI2CInfo.address=LCD_ADRESS+1;
    uint8 r, c;
    for (r = 0; r < LCD_HEIGHT; r++){
	for (c = 0; c < LCD_WIDTH ; c++) {
	    DriverI2CStart(&m_LCDI2CInfo, TRUE);
	    LCDSend(ctStartSequence, m_LCDMonitor[(LCD_HEIGHT-1)-r][c]);
	    }
	}

    DriverI2CStop(&m_LCDI2CInfo);
    first = FALSE;

    m_LCDI2CInfo.address=LCD_ADRESS;
    #endif

	#ifdef RDX_48
    m_LCDI2CInfo.address=LCD_ADRESS+1;
    uint8 r, c;
    for (r = 0; r < LCD_HEIGHT; r++){
	for (c = 0; c < LCD_WIDTH ; c++) {
	    DriverI2CStart(&m_LCDI2CInfo, TRUE);
	    LCDSend(ctStartSequence, m_LCDMonitor[(LCD_HEIGHT-1)-r][c]);
	    }
	for (c = 0; c < 4 ; c++) {
	    DriverI2CStart(&m_LCDI2CInfo, TRUE);
	    LCDSend(ctStartSequence, 0);
	    }
	}

    DriverI2CStop(&m_LCDI2CInfo);
    first = FALSE;

    m_LCDI2CInfo.address=LCD_ADRESS;
    #endif
    }
//---------------------------------------------------------------------------

PUBLIC void LCDTick() // точка на экране, привязанная к WDT
    {
    vAHI_DioSetOutput (WDTIMER, 0);
    vAHI_DioSetOutput (0, WDTIMER);
    m_LCDBlink = !m_LCDBlink;
    if (m_LCDBlink == TRUE)
    vAHI_DioSetOutput (WDTIMER, 0);
    else
	vAHI_DioSetOutput (0, WDTIMER);

    //LCDDebugWithDig("val_",m_LCDBlink, 2);
    }
//---------------------------------------------------------------------------

PRIVATE void LCDSetAddress(uint8 x, uint8 y){

    #ifdef TIC_149
    uint8 xm = 0;
    if (x > 127){
	x = x - 127; //По default стояло 127, но в идеале должно быть 10000 ?????
	xm = 4; //По default стояло 4, но в идеале должно быть 2 ?????
	}
    DriverI2CStart(&m_LCDI2CInfo, TRUE);
    if (DriverI2CStart(&m_LCDI2CInfo, TRUE))
    {
	LCDSend(ctSingle, 0x01);
	LCDSend(ctSingle, 0x20 | xm); // Xm -> 0
	LCDSend(ctSingle, 0x50 | y); //По default стояло 0x40, но в идеале должно быть 0x50, оставил
	LCDSend(ctSingle, 0x80 | x);
	DriverI2CStop(&m_LCDI2CInfo);
	}
    #endif

    #ifdef TIC_48
    //DriverI2CStart(&m_LCDI2CInfo, TRUE);
    if (DriverI2CStart(&m_LCDI2CInfo, TRUE))
    {
	LCDSend(ctSingle, 0x01);
	LCDSend(ctSingle, 0x40 | y);
	LCDSend(ctSingle, 0x80 | x);
	DriverI2CStop(&m_LCDI2CInfo);
	}
    #endif

    #ifdef RDX_154
    //DriverI2CStart(&m_LCDI2CInfo, TRUE);

    if (DriverI2CStart(&m_LCDI2CInfo, TRUE))
    {
	if ((y/8)==0) y=(y/8); else y=(y/8)-1;
	LCDSend(ctSingle, 0xB0 | y);
	LCDSend(ctSingle, x & 0x0F);
	LCDSend(ctSingle, (x >> 4) | 0x10);
	DriverI2CStop(&m_LCDI2CInfo);
	}
    #endif

	#ifdef RDX_48
    DriverI2CStart(&m_LCDI2CInfo, TRUE);
    if (DriverI2CStart(&m_LCDI2CInfo, TRUE))
    {
        if ((y % 8)==0){

        	y=y/8;
        }
        else{

        	y=(y/8)-1;
        }

        LCDSend(ctSingle, 0xB0 | y);
        LCDSend(ctSingle, x & 0x0F);
        LCDSend(ctSingle, (x >> 4) | 0x10);
        DriverI2CStop(&m_LCDI2CInfo);
	}
    #endif

    }
//---------------------------------------------------------------------------

PRIVATE void LCDImageInit(LCDImage* image, uint8 height, uint8 width,
		uint8* img)
{
	image->height = height;
	image->width = width;
	image->img = img;
}
//---------------------------------------------------------------------------

PRIVATE void LCDClear(void)
{
	uint8 w, h;

	for (h = 0; h < LCD_HEIGHT; h++)
		for (w = 0; w < LCD_WIDTH; w++)
			m_LCDMonitor[h][w] = 0;
}
//---------------------------------------------------------------------------

PRIVATE void LCDSend(LCDComandType type, uint8 val){

    #ifdef TIC_149//|TIC_48
    switch (type){
    case ctSingle:
	DriverI2CSend(&m_LCDI2CInfo, 0x80);
	break;
    case ctStartSequence:
	DriverI2CSend(&m_LCDI2CInfo, 0x40);
	break;
    case ctSequence:
	break;
    }
    #endif

    #ifdef TIC_48
    switch (type){
    case ctSingle:
	DriverI2CSend(&m_LCDI2CInfo, 0x80);
	break;
    case ctStartSequence:
	DriverI2CSend(&m_LCDI2CInfo, 0x40);
	break;
    case ctSequence:
	break;
    }
    #endif

    DriverI2CSend(&m_LCDI2CInfo, val);
}
//---------------------------------------------------------------------------

PRIVATE void LCDDrawImage(LCDImage lcdImage, uint8 x, uint8 y, bool fromLastPos)
{
	uint8 w = 0, h = 0;
	if (fromLastPos)
	{
		x = m_X;
		y = m_Y;
	}
	m_Y = y;

	for (h = y; h < y + lcdImage.height && h < LCD_HEIGHT; h++)
		for (w = x; w < x + lcdImage.width && w < LCD_WIDTH; w++)
			m_LCDMonitor[h][w] =
					lcdImage.img[(h - y) * lcdImage.width + (w - x)];
	m_X = w + 1;
}
//---------------------------------------------------------------------------

PRIVATE void LCDDrawText(char* text, uint8 row, LCDAlign align,
		LCDFontFamily font)
{
	char textFormatted[LCD_MAXSTRING];
	uint8 pos = 0;
	uint8 startX;
	uint8 textWidth;
	uint8 maxWidth = LCDGetMaxWidth(row, font);

	LCDFormatText(textFormatted, text, LCD_MAXSTRING, font, maxWidth);
	textWidth = LCDTextWidth(textFormatted, font);
	startX = LCDGetAlignStart(row, font, align, textWidth);

	while (textFormatted[pos] != '\0')
	{
		switch (font)
		{
		case ffMSSansSerif8:
			LCDDrawImage(*m_LCDFont8[LCDGetFontIndex(textFormatted[pos], font)],
					startX, row, pos != 0);
			break;
		case ffMSSansSerif24:
			LCDDrawImage(
					*m_LCDFont24[LCDGetFontIndex(textFormatted[pos], font)],
					startX, row, pos != 0);
			break;
		}
		pos++;
	}
}
//---------------------------------------------------------------------------

PRIVATE uint8 LCDGetFontIndex(char ch, LCDFontFamily font)
{
	uint8 pos = 0;

	switch (ch)
	{
	case 'q':
		ch = 'Q';
		break;
	case 'w':
		ch = 'W';
		break;
	case 'e':
		ch = 'E';
		break;
	case 'r':
		ch = 'R';
		break;
	case 't':
		ch = 'T';
		break;
	case 'y':
		ch = 'Y';
		break;
	case 'u':
		ch = 'U';
		break;
	case 'i':
		ch = 'I';
		break;
	case 'o':
		ch = 'O';
		break;
	case 'p':
		ch = 'P';
		break;
	case 'a':
		ch = 'A';
		break;
	case 's':
		ch = 'S';
		break;
	case 'd':
		ch = 'D';
		break;
	case 'f':
		ch = 'F';
		break;
	case 'g':
		ch = 'G';
		break;
	case 'h':
		ch = 'H';
		break;
	case 'j':
		ch = 'J';
		break;
	case 'k':
		ch = 'K';
		break;
	case 'l':
		ch = 'L';
		break;
	case 'z':
		ch = 'Z';
		break;
	case 'x':
		ch = 'X';
		break;
	case 'c':
		ch = 'C';
		break;
	case 'v':
		ch = 'V';
		break;
	case 'b':
		ch = 'B';
		break;
	case 'n':
		ch = 'N';
		break;
	case 'm':
		ch = 'M';
		break;
	}

	switch (font)
	{
	case ffMSSansSerif8:
		if (ch >= 48 && ch <= 57)
			pos = ch - 48;
		else if (ch >= 65 && ch <= 90)
			pos = 10 + ch - 65;
		else
			pos = 36;
		break;
	case ffMSSansSerif24:
		if (ch >= 48 && ch <= 57)
			pos = ch - 48;
		else if (ch == 'C')
			pos = 10;
		else if (ch == (char) 176)
			pos = 11;
		else if (ch == '%')
			pos = 12;
		else if (ch == '-')
			pos = 14;
		else if (ch == 'E')
			pos = 15;
		else
			pos = 13;
		break;
	}

	return pos;
}
//---------------------------------------------------------------------------

PRIVATE uint16 LCDTextWidth(char* str, LCDFontFamily font)
{
	uint16 width = 0;
	uint8 pos = 0;
	uint8 fontPos;

	while (str[pos] != '\0')
	{
		fontPos = LCDGetFontIndex(str[pos], font);
		switch (font)
		{
		case ffMSSansSerif8:
			width += m_LCDFont8[fontPos]->width;
			break;
		case ffMSSansSerif24:
			width += m_LCDFont24[fontPos]->width;
			break;
		}
		width++;
		pos++;
	}
	return width - 1;
}
//---------------------------------------------------------------------------

PRIVATE uint8 LCDGetMaxWidth(uint8 row, LCDFontFamily font)
{
	return LCDGetRightStopPos(row, font) - LCDGetLeftStartPos(row, font);
}
//---------------------------------------------------------------------------

PRIVATE uint8 LCDGetLeftStartPos(uint8 row, LCDFontFamily font)
{
	uint8 i, j;
	uint8 max = 0;
	uint8 rowSize = (font == ffMSSansSerif8) ? 1 : 3;
	uint32 val, prevVal;

	prevVal = 0;
	for (i = 0; i < LCD_WIDTH; i++)
	{
		val = 0;
		for (j = 0; j < rowSize; j++)
			val = val | (m_LCDMonitor[row + j][i] << (j * 8));
		max++;
		if (val == 0 && val == prevVal)
			break;
		prevVal = val;
	}
	return max - 1;
}
//---------------------------------------------------------------------------

PRIVATE uint8 LCDGetRightStopPos(uint8 row, LCDFontFamily font)
{
	int16 i;
	uint8 j;
	uint8 max = 0;
	uint8 rowSize = (font == ffMSSansSerif8) ? 1 : 3;
	uint32 val, prevVal;

	prevVal = 0;
	for (i = LCD_WIDTH - 1; i >= 0; i--)
	{
		val = 0;
		for (j = 0; j < rowSize; j++)
			val = val | (m_LCDMonitor[row + j][i] << (j * 8));
		max++;
		if (val == 0 && val == prevVal)
			break;
		prevVal = val;
	}
	return LCD_WIDTH - max;
}
//---------------------------------------------------------------------------

PRIVATE uint8 LCDGetAlignStart(uint8 row, LCDFontFamily font, LCDAlign align,
		uint8 textWidth)
{
	uint8 left = LCDGetLeftStartPos(row, font);
	uint8 right = LCDGetRightStopPos(row, font);

	switch (align)
	{
	case aLeft:
		return left;
	case aRight:
		return right - textWidth + 1;
	case aCenter:
		return left + (right - left) / 2 - textWidth / 2;
	}
	return 0;
}
//---------------------------------------------------------------------------

PRIVATE void LCDFormatText(char* frmtText, char* text, uint8 maxLength,
		LCDFontFamily font, uint8 maxWidth)
{
	uint8 textWidth = LCDTextWidth(text, font);
	uint8 textLength = sstrlen(text);
	uint8 start = (textLength > maxLength) ? textLength - maxLength + 3 + 1 : 0;
	while (textWidth > maxWidth)
	{
		sstrcpy(frmtText, "...");
		sstrcat(frmtText, &text[start]);
		textWidth = LCDTextWidth(frmtText, font);
		start++;
	}
	if (start == 0)
		sstrcpy(frmtText, text);
}
//---------------------------------------------------------------------------

PRIVATE void LCDDrawCoordGUI(void)
{
	char ver[16];

	LCDMakeVersion(ver);
	LCDDrawText(ver, 0, aLeft, ffMSSansSerif8);
	if (m_LCDBlink)
		LCDDrawImage(m_LCDBlinkOnImage, LCD_WIDTH - m_LCDBlinkOnImage.width, 0,
				FALSE);
	else
		LCDDrawImage(m_LCDBlinkOffImage, LCD_WIDTH - m_LCDBlinkOffImage.width,
				0, FALSE);
	LCDDrawText(m_LCDNetID, 0, aRight, ffMSSansSerif8);
	LCDDrawText(m_LCDDevName, 0, aCenter, ffMSSansSerif8);
	LCDDrawText(m_LCDDevStat, 1, aCenter, ffMSSansSerif24);
	if (m_LCDDebug[0][0] != '\0' || m_LCDDebug[1][0] != '\0'
			|| m_LCDDebug[2][0] != '\0')
	{
		LCDDrawText(m_LCDDebug[0], 1, aLeft, ffMSSansSerif8);
		LCDDrawText(m_LCDDebug[1], 2, aLeft, ffMSSansSerif8);
		LCDDrawText(m_LCDDebug[2], 3, aLeft, ffMSSansSerif8);
	}
}
//---------------------------------------------------------------------------

PRIVATE void LCDMakeVersion(char* str)
{
	char dig[4];
	ConverterDigToStr(SOFT_VERSION1, str, 10);
	sstrcat(str, ".");
	ConverterDigToStr(SOFT_VERSION2, dig, 10);
	sstrcat(str, dig);
	sstrcat(str, ".");
	ConverterDigToStr(SOFT_VERSION3, dig, 10);
	sstrcat(str, dig);
}
//---------------------------------------------------------------------------
