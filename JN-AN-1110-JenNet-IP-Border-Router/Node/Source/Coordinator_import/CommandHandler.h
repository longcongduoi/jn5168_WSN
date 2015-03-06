#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include <jendefs.h>
#include "AACPFrame.h"
#include "Vector.h"
#include "Packet.h"

extern bool g_NetworkUp;

#define RADIO_MAXPACK_SIZE 60

// Инициализация обработчика команд
PUBLIC void CommandHandlerInit();
// Обработать кадр от сервера
PUBLIC void OnUartData( Vector const* const data );
// Обработать данные из радиоканала
//PUBLIC void OnRadioData(tsData* data);
// Изменилось состояние связи с устройством
PUBLIC void OnDeviceConnectedStateChanged(uint64 mac, bool isConnected);
// Уведомление от стека об отправке/не отправки пакета
PUBLIC void OnRadioSendNotification(bool isSent);
// Отправить кадр отладки
PUBLIC void SendDebugMessage( char* message );
// Отправить сообщение сразу
PUBLIC void SendDebugMessageForce( char* message );
// Отправить пакет
PUBLIC void SendPackToServer(Packet* p);

// Отладочная функция - посылка байта в кадре типа OnDataReceived
PUBLIC void SendEcho( uint8 value );

#endif
