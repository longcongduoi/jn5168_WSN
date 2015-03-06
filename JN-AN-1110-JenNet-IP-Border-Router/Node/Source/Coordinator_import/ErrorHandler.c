
#include "ErrorHandler.h"
#include "DeviceDescriptor.h"
#include "LCD.h"
#include "Uart.h"
#include "StringSimple.h"
#include "Converter.h"
#include "CommandHandler.h"

bool IsErrorProcessing = FALSE;

char m_LCDDevStat[32];

ErrorDesc g_ErrorDesc;

PUBLIC void EHInit(){

	g_ErrorDesc.ErrorCode = 0;
	g_ErrorDesc.ModuleId = 0;
}

PUBLIC void EHCall(uint8 modid, uint8 errorCode){

	// Чтобы не зациклить вызовы в случае возникновения ошибки в процессе обработки ошибки
	if(IsErrorProcessing) return;

	IsErrorProcessing = TRUE;

	g_ErrorDesc.ModuleId = modid;
	g_ErrorDesc.ErrorCode = errorCode;

	if (MAIN_DEVICE_TYPE == dtCoordinator)
	{
		sstrcpy(m_LCDDevStat, "Error: ");
		sstrcat(m_LCDDevStat, ConverterDigToStr2(modid, 10));
		sstrcat(m_LCDDevStat, ".");
		sstrcat(m_LCDDevStat, ConverterDigToStr2(errorCode, 10));
		SendDebugMessage(m_LCDDevStat);
	}

	IsErrorProcessing = FALSE;
	return;

}
