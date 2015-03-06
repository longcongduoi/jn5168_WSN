#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include <jendefs.h>
#include "AACPFrame.h"
#include "Vector.h"
#include "Packet.h"

extern bool g_NetworkUp;

#define RADIO_MAXPACK_SIZE 60

// ������������� ����������� ������
PUBLIC void CommandHandlerInit();
// ���������� ���� �� �������
PUBLIC void OnUartData( Vector const* const data );
// ���������� ������ �� �����������
//PUBLIC void OnRadioData(tsData* data);
// ���������� ��������� ����� � �����������
PUBLIC void OnDeviceConnectedStateChanged(uint64 mac, bool isConnected);
// ����������� �� ����� �� ��������/�� �������� ������
PUBLIC void OnRadioSendNotification(bool isSent);
// ��������� ���� �������
PUBLIC void SendDebugMessage( char* message );
// ��������� ��������� �����
PUBLIC void SendDebugMessageForce( char* message );
// ��������� �����
PUBLIC void SendPackToServer(Packet* p);

// ���������� ������� - ������� ����� � ����� ���� OnDataReceived
PUBLIC void SendEcho( uint8 value );

#endif
