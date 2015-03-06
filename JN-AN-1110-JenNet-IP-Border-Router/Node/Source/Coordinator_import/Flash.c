
#include <AppHardwareApi_JN516x.h>
#include <string.h>

#include "Flash.h"
#include "Converter.h"
#include "CommandHandler.h"
#include "ErrorHandler.h"
#include "DebugP.h"

//-- Свойства внутренней Flash
#define FLASH_INTERNAL_NUMBER_OF_SECTORS 				8// Количество секторов внутренней Flash памяти
#define FLASH_INTERNAL_SECTOR_SIZE_IN_BYTES 			1024*32// Вместимость одного сектора
#define FLASH_INTERNAL_PAGEWORD_SIZE_IN_BYTES 			16// Вместимость одной страницы

//-- При выборе сектора важно учитывать, что в той же памяти хранится код
#define FLASH_INTERNAL_APPROPRIATE_USER_DATA_SECTOR 	7// Сектор используемый для пользовательских данных
#define FLASH_INTERNAL_APPROPRIATE_USER_DATA_OFFSET 	FLASH_INTERNAL_SECTOR_SIZE_IN_BYTES*(FLASH_INTERNAL_APPROPRIATE_USER_DATA_SECTOR-1)// Смещение от нулевого адреса для пользовательских данных

//-- Коды ошибок
#define FLASH_ERR_CODE_SETTINGS_CONTENT_OUT_OF_BUFFER	1// Код ошибки - объём массива превышает объём буфера
#define FLASH_ERR_CODE_SETTINGS_LENGTH_IS_ZERO			2// Код ошибки - объём массива нулевой
#define FLASH_ERR_CODE_SETTINGS_CRC_IS_WRONG			3// Код ошибки - неверная контрольная сумма массива настроек

//-- Объём байт выделяемый на хранение массива настроек ([массив (CurrentSettingsLength)]+[признак наличия данных (1)]+[контрольная сумма (1)])
#define APPROPRIATE_SETTINGS_LENGTH_IN_BYTES 			256

//-- Служебные параметры
#define SETTINGS_MARK_OFFSET							0// Смещение от массива с настройками
#define SETTINGS_MARK_LENGTH_IN_BYTES					1// Объём занимаемый признаком наличия данных
#define SETTINGS_MARK_VALUE								13// Признак наличия данных// Число выбрано случайно

#define SETTINGS_CRC_OFFSET								SETTINGS_MARK_OFFSET + SETTINGS_MARK_LENGTH_IN_BYTES// Смещение от массива с настройками
#define SETTINGS_CRC_LENGTH_IN_BYTES					1// Объём занимаемый контрольной суммой

//---------------------------------------------------------------------------
// Локальные переменные и функции

uint8 FlashCash[APPROPRIATE_SETTINGS_LENGTH_IN_BYTES];

void *CurrentSettingsContent;
uint8 CurrentSettingsLength;

PRIVATE SettingsReadStatus_e eFlashSettings_checkCRC(void);
PRIVATE void vFlashSettings_calculateCRC(void);

//---------------------------------------------------------------------------
// Инициализация в режиме cold_start

PUBLIC void vFlashSettings_init(void *SettingsContent, uint8 SettingsLength){


	// Проверить превышение массивом объема заданного буфера
	if(SettingsLength > APPROPRIATE_SETTINGS_LENGTH_IN_BYTES - SETTINGS_MARK_LENGTH_IN_BYTES - SETTINGS_CRC_LENGTH_IN_BYTES){

		EHCall(FLASH_MOD_ID, FLASH_ERR_CODE_SETTINGS_CONTENT_OUT_OF_BUFFER);

	// Проверить наличие записываемых данных
	}else if(SettingsLength == 0){

		EHCall(FLASH_MOD_ID, FLASH_ERR_CODE_SETTINGS_LENGTH_IS_ZERO);

	}else{

		// Проинициализировать функции для внутренней Flash
		bAHI_FlashInit(E_FL_CHIP_INTERNAL, NULL);

		// Параметры массива
		CurrentSettingsContent = SettingsContent;
		CurrentSettingsLength = SettingsLength;
	}
}

//---------------------------------------------------------------------------
// Прочитать массив с настройками

PUBLIC SettingsReadStatus_e eFlashSettings_read(void){

	FlashCash[CurrentSettingsLength + SETTINGS_MARK_OFFSET] = 0xFF;// Установить значение по-умолчанию

	// Прочитать данные из Flash
	bAHI_FullFlashRead(FLASH_INTERNAL_APPROPRIATE_USER_DATA_OFFSET, CurrentSettingsLength, CurrentSettingsContent);

	// Проверить значение признака наличия данных
	if(FlashCash[CurrentSettingsLength + SETTINGS_MARK_OFFSET] == SETTINGS_MARK_VALUE){

		if(eFlashSettings_checkCRC() != SETTINGS_READ_CRC_FAILED){

			MSG("Flash check crc complete\n\r");

			// Копировать в назначенный массив из буфера
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
// Записать массив с настройками

PUBLIC void vFlashSettings_write(void){

	// Очистить сектор
	bAHI_FlashEraseSector(FLASH_INTERNAL_APPROPRIATE_USER_DATA_SECTOR-1);

	// Подсчитать контрольную сумму
	vFlashSettings_calculateCRC();

	// Копировать настройки в буфер
	memcpy(FlashCash, CurrentSettingsContent, CurrentSettingsLength);

	// Записать весь буфер во Flash
	bAHI_FullFlashProgram(FLASH_INTERNAL_APPROPRIATE_USER_DATA_OFFSET, APPROPRIATE_SETTINGS_LENGTH_IN_BYTES, FlashCash);
}

//---------------------------------------------------------------------------
// Проверить контрольную сумму массива настроек

PRIVATE SettingsReadStatus_e eFlashSettings_checkCRC(void){

	MSG("Flash start check crc\n\r");

	uint8 i, crc=0;
	for(i=0; i<CurrentSettingsLength; crc+=((uint8*)CurrentSettingsContent)[i++]);

	return (FlashCash[CurrentSettingsLength + SETTINGS_CRC_OFFSET] == crc) ? SETTINGS_READ_STATUS_OK:SETTINGS_READ_CRC_FAILED;
}

//---------------------------------------------------------------------------
// Расчитать контрольную сумму массива настроек

PRIVATE void vFlashSettings_calculateCRC(void){

	uint8 i, crc=0;
	for(i=0; i<CurrentSettingsLength; crc+=((uint8*)CurrentSettingsContent)[i]);

	FlashCash[CurrentSettingsLength + SETTINGS_CRC_OFFSET] = crc;
}
//----------------------------------------------------------------------------
