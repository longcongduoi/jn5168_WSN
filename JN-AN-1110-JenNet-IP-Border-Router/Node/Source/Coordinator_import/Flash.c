
#include <AppHardwareApi_JN516x.h>
#include <string.h>

#include "Flash.h"
#include "Converter.h"
#include "CommandHandler.h"
#include "ErrorHandler.h"
#include "DebugP.h"

//-- �������� ���������� Flash
#define FLASH_INTERNAL_NUMBER_OF_SECTORS 				8// ���������� �������� ���������� Flash ������
#define FLASH_INTERNAL_SECTOR_SIZE_IN_BYTES 			1024*32// ����������� ������ �������
#define FLASH_INTERNAL_PAGEWORD_SIZE_IN_BYTES 			16// ����������� ����� ��������

//-- ��� ������ ������� ����� ���������, ��� � ��� �� ������ �������� ���
#define FLASH_INTERNAL_APPROPRIATE_USER_DATA_SECTOR 	7// ������ ������������ ��� ���������������� ������
#define FLASH_INTERNAL_APPROPRIATE_USER_DATA_OFFSET 	FLASH_INTERNAL_SECTOR_SIZE_IN_BYTES*(FLASH_INTERNAL_APPROPRIATE_USER_DATA_SECTOR-1)// �������� �� �������� ������ ��� ���������������� ������

//-- ���� ������
#define FLASH_ERR_CODE_SETTINGS_CONTENT_OUT_OF_BUFFER	1// ��� ������ - ����� ������� ��������� ����� ������
#define FLASH_ERR_CODE_SETTINGS_LENGTH_IS_ZERO			2// ��� ������ - ����� ������� �������
#define FLASH_ERR_CODE_SETTINGS_CRC_IS_WRONG			3// ��� ������ - �������� ����������� ����� ������� ��������

//-- ����� ���� ���������� �� �������� ������� �������� ([������ (CurrentSettingsLength)]+[������� ������� ������ (1)]+[����������� ����� (1)])
#define APPROPRIATE_SETTINGS_LENGTH_IN_BYTES 			256

//-- ��������� ���������
#define SETTINGS_MARK_OFFSET							0// �������� �� ������� � �����������
#define SETTINGS_MARK_LENGTH_IN_BYTES					1// ����� ���������� ��������� ������� ������
#define SETTINGS_MARK_VALUE								13// ������� ������� ������// ����� ������� ��������

#define SETTINGS_CRC_OFFSET								SETTINGS_MARK_OFFSET + SETTINGS_MARK_LENGTH_IN_BYTES// �������� �� ������� � �����������
#define SETTINGS_CRC_LENGTH_IN_BYTES					1// ����� ���������� ����������� ������

//---------------------------------------------------------------------------
// ��������� ���������� � �������

uint8 FlashCash[APPROPRIATE_SETTINGS_LENGTH_IN_BYTES];

void *CurrentSettingsContent;
uint8 CurrentSettingsLength;

PRIVATE SettingsReadStatus_e eFlashSettings_checkCRC(void);
PRIVATE void vFlashSettings_calculateCRC(void);

//---------------------------------------------------------------------------
// ������������� � ������ cold_start

PUBLIC void vFlashSettings_init(void *SettingsContent, uint8 SettingsLength){


	// ��������� ���������� �������� ������ ��������� ������
	if(SettingsLength > APPROPRIATE_SETTINGS_LENGTH_IN_BYTES - SETTINGS_MARK_LENGTH_IN_BYTES - SETTINGS_CRC_LENGTH_IN_BYTES){

		EHCall(FLASH_MOD_ID, FLASH_ERR_CODE_SETTINGS_CONTENT_OUT_OF_BUFFER);

	// ��������� ������� ������������ ������
	}else if(SettingsLength == 0){

		EHCall(FLASH_MOD_ID, FLASH_ERR_CODE_SETTINGS_LENGTH_IS_ZERO);

	}else{

		// ������������������� ������� ��� ���������� Flash
		bAHI_FlashInit(E_FL_CHIP_INTERNAL, NULL);

		// ��������� �������
		CurrentSettingsContent = SettingsContent;
		CurrentSettingsLength = SettingsLength;
	}
}

//---------------------------------------------------------------------------
// ��������� ������ � �����������

PUBLIC SettingsReadStatus_e eFlashSettings_read(void){

	FlashCash[CurrentSettingsLength + SETTINGS_MARK_OFFSET] = 0xFF;// ���������� �������� ��-���������

	// ��������� ������ �� Flash
	bAHI_FullFlashRead(FLASH_INTERNAL_APPROPRIATE_USER_DATA_OFFSET, CurrentSettingsLength, CurrentSettingsContent);

	// ��������� �������� �������� ������� ������
	if(FlashCash[CurrentSettingsLength + SETTINGS_MARK_OFFSET] == SETTINGS_MARK_VALUE){

		if(eFlashSettings_checkCRC() != SETTINGS_READ_CRC_FAILED){

			MSG("Flash check crc complete\n\r");

			// ���������� � ����������� ������ �� ������
			memcpy(CurrentSettingsContent, FlashCash, CurrentSettingsLength);

			return SETTINGS_READ_STATUS_OK;

		}
		else{

			MSG("Flash crc is wrong\n\r");

			EHCall(FLASH_MOD_ID, FLASH_ERR_CODE_SETTINGS_CRC_IS_WRONG);

			return SETTINGS_READ_CRC_FAILED;
		}
	}
	else{

		return SETTINGS_READ_NO_DATA;
	}
}

//---------------------------------------------------------------------------
// �������� ������ � �����������

PUBLIC void vFlashSettings_write(void){

	// �������� ������
	bAHI_FlashEraseSector(FLASH_INTERNAL_APPROPRIATE_USER_DATA_SECTOR-1);

	// ���������� ����������� �����
	vFlashSettings_calculateCRC();

	// ���������� ��������� � �����
	memcpy(FlashCash, CurrentSettingsContent, CurrentSettingsLength);

	// �������� ���� ����� �� Flash
	bAHI_FullFlashProgram(FLASH_INTERNAL_APPROPRIATE_USER_DATA_OFFSET, APPROPRIATE_SETTINGS_LENGTH_IN_BYTES, FlashCash);
}

//---------------------------------------------------------------------------
// ��������� ����������� ����� ������� ��������

PRIVATE SettingsReadStatus_e eFlashSettings_checkCRC(void){

	MSG("Flash start check crc\n\r");

	uint8 i, crc=0;
	for(i=0; i<CurrentSettingsLength; crc+=((uint8*)CurrentSettingsContent)[i++]);

	return (FlashCash[CurrentSettingsLength + SETTINGS_CRC_OFFSET] == crc) ? SETTINGS_READ_STATUS_OK:SETTINGS_READ_CRC_FAILED;
}

//---------------------------------------------------------------------------
// ��������� ����������� ����� ������� ��������

PRIVATE void vFlashSettings_calculateCRC(void){

	uint8 i, crc=0;
	for(i=0; i<CurrentSettingsLength; crc+=((uint8*)CurrentSettingsContent)[i]);

	FlashCash[CurrentSettingsLength + SETTINGS_CRC_OFFSET] = crc;
}
//----------------------------------------------------------------------------
