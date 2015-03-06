#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H
//---------------------------------------------------------------------------

#include <jendefs.h>
//---------------------------------------------------------------------------

#define BUFF_MOD_ID 					2
#define UART_MOD_ID 					3
#define PACKET_MOD_ID 					5
#define OBJECT_MOD_ID 					6
#define COMMANDHANDLER_MOD_ID 			8
#define RADIO_MOD_ID 					9
#define DEVICEDESCRIPTOR_MOD_ID 		11
#define DRIVERI2C_MOD_ID 				12
#define MATHSIMPLE_MOD_ID 				13
#define LCD_MOD_ID 						15
#define VECTOR_MOD_ID 					16
#define COORDINATOR_AS_DEVICE_MOD_ID 	17
#define FLASH_MOD_ID 					18


PUBLIC typedef struct{

	uint8 ModuleId;
	uint8 ErrorCode;
}ErrorDesc;

extern ErrorDesc g_ErrorDesc;

void EHCall(uint8 modid, uint8 errorCode);

//PUBLIC void EHInit();

#endif
