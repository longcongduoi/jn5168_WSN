#include <AppHardwareApi_JN516x.h>

#include "WorkModeManager.h"
#include "DeviceDescriptor.h"
#include "LCD.h"
#include "Config.h"
//#include "Battary.h"
#include "Utils.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
PRIVATE uint32 m_WMMTimer;
PRIVATE uint32 m_WMMMesurePeriodTime;
PRIVATE uint32 m_WMMSleepTime;
PRIVATE bool m_WMMConnectionVal;
PRIVATE uint16 m_WMMBattaryVal;
PRIVATE uint16 m_WMMLightVal;
PRIVATE uint8 m_WMMDemoMode;
PRIVATE uint32 m_WMMLedRedDio, m_WMMLedGreenDio, m_WMMLedBlueDio;

typedef enum
{
	cBlack = 0, cRed, cBlue, cMagenta, cGreen, cYellow, cCyan, cWhite
} WMMLedColor;

PRIVATE void WMMCheckServiceMode(void);
PRIVATE void WMMUpdateLED(bool on);
PRIVATE void WMMLEDSetColor(WMMLedColor color);
//----------------------------------------------------------------------------

PUBLIC void WMMInit(void)
{
	m_WMMTimer = 0;
	m_WMMConnectionVal = FALSE;
	m_WMMMesurePeriodTime = 0;
	m_WMMDemoMode = deviceDescriptor.demoMode;
	
	vAHI_DioSetDirection(WMM_BUTTON, 0);
	vAHI_DioWake(WMM_BUTTON, 0, WMM_BUTTON, 0);
	vAHI_DioSetDirection(0, m_WMMLedRedDio | m_WMMLedGreenDio | m_WMMLedBlueDio);
	vAHI_DioSetOutput(0, m_WMMLedRedDio | m_WMMLedGreenDio | m_WMMLedBlueDio); // ��� ������ ��� ���������� 0
}
//----------------------------------------------------------------------------

PUBLIC void WMMUpdateSleepTime(void)
{
	WMMCheckServiceMode();
	if (deviceDescriptor.serviceMode == 1)
		m_WMMSleepTime = CONFIG_SERVICETIME;
	else if (!m_WMMConnectionVal)
		m_WMMSleepTime = CONFIG_LOST_CONNECTIONTIME;
	else
		m_WMMSleepTime = m_WMMMesurePeriodTime;

	gJenie_EndDeviceScanSleep = m_WMMSleepTime * 1000;
	vApi_SetScanSleep(m_WMMSleepTime * 1000);
	eJenie_SetSleepPeriod(m_WMMSleepTime * 1000);
}
//----------------------------------------------------------------------------

PUBLIC void WMMCorrectTime(uint32 realSleepTime)
{
	if (realSleepTime < m_WMMSleepTime)
		m_WMMTimer += realSleepTime;
	else
		m_WMMTimer += m_WMMSleepTime;
}
//----------------------------------------------------------------------------

PUBLIC void WMMSetNextMesurePeriodTime(uint32 mesurePeriodTime)
{
	m_WMMMesurePeriodTime = mesurePeriodTime;
}
//----------------------------------------------------------------------------

PUBLIC uint32 WMMGetTime(void)
{
	return m_WMMTimer;
}
//----------------------------------------------------------------------------

PUBLIC bool WMMIsServiceMode(void)
{
	return deviceDescriptor.serviceMode;
}
//----------------------------------------------------------------------------

PUBLIC bool WMMIsOnline(void)
{
	return m_WMMConnectionVal;
}
//----------------------------------------------------------------------------

PUBLIC bool WMMIsDemoMode(void)
{
	if (!m_WMMConnectionVal)
		m_WMMDemoMode = (m_WMMDemoMode == 0) ? 0 : m_WMMDemoMode - 1;
	else
		m_WMMDemoMode = deviceDescriptor.demoMode;

	if ((deviceDescriptor.demoMode != 0) && (m_WMMDemoMode == 0))
	{
		gJenie_EndDeviceScanSleep = ((uint32) -1);
		vApi_SetScanSleep(((uint32) -1));
		m_WMMSleepTime = CONFIG_DEMOSLEEP_TIME;
		eJenie_SetSleepPeriod(m_WMMSleepTime * 1000);
		return TRUE;
	}

	return FALSE;
}
//----------------------------------------------------------------------------

PUBLIC void WMMSetOnline(bool online)
{
	m_WMMConnectionVal = online;
	WMMUpdateSleepTime();
}
//----------------------------------------------------------------------------

PUBLIC bool WMMButtonEvent(uint32 deviceId, uint32 itemBitmap)
{
	if ((deviceId == E_AHI_DEVICE_SYSCTRL) && (itemBitmap & WMM_BUTTON))
	{
		m_WMMDemoMode = deviceDescriptor.demoMode;
		WMMUpdateSleepTime();
		WMMCheckServiceMode();
		return TRUE;
	}
	return FALSE;
}
//---------------------------------------------------------------------------

PUBLIC void WMMUpdateSensorsVal(uint16 battaryVal, uint16 lightVal)
{
	m_WMMBattaryVal = battaryVal;
	m_WMMLightVal = lightVal;
}
//---------------------------------------------------------------------------

PRIVATE void WMMCheckServiceMode(void)
{
	deviceDescriptor.serviceMode = (u32AHI_DioReadInput() & WMM_BUTTON) ? 0 : 1;
	if (!deviceDescriptor.serviceMode) //{
		vAHI_DioWake(WMM_BUTTON, 0, 0, WMM_BUTTON);
	else

		vAHI_DioWake(WMM_BUTTON, 0, WMM_BUTTON, 0);
	WMMUpdateLED((deviceDescriptor.serviceMode == 1) ? TRUE : FALSE);
	if (deviceDescriptor.testMode != 0) //
		deviceDescriptor.serviceMode = 1;
	LCDSetLinkDrawAsText((deviceDescriptor.serviceMode == 1) ? TRUE : FALSE);
	LCDUpdate();
}
//-----------��������� �������-----------------------
PRIVATE void WMMUpdateLED(bool on)
{
	if (on)
	//-----���� ������ ��������--------------------------------------------------------
	{
		{
			LCDPower(TRUE); // ������� �������
			LCDUpdate(); // ������ �����������
		}
		{
#ifdef TIC_149
			WMMLEDSetColor(cWhite); // ��� 149 ������ ��������� ��� ���������� ������
#else
			//--------�������� ����� ��� TIC-48----------------------------------------
			if (m_WMMLightVal > 1024) // ��������� ��������� ��� ������� ���������
				WMMLEDSetColor(cBlack);
			else if (m_WMMBattaryVal < ObjectHandlerBattary.object.minval) // ������� ��� ����������� �������
				WMMLEDSetColor(cRed);
			else if (m_WMMConnectionVal < 1) // ����� ��� ���������� �����
				WMMLEDSetColor(cBlue);
			else
				WMMLEDSetColor(cWhite); // �����, ���� ��� � �������
			//--------------------------------------------------------------------------
#endif
		}
	}
	//-----�����, ���� ������ ��������---------------------------------------------------
	else if (deviceDescriptor.demoMode != 1) // ���� ������ ��������� � �� ����-�����, ����� ������� ������
	{
		//-------------��������� ���������� ��������------------------------
		if (ObjectHandlerBattary.object.config == 1
				|| ObjectHandlerBattary.object.config == 2)
		{
			{
				LCDPower(TRUE); // ������� �������
				LCDUpdate(); // ������ �����������
			}
			if (ObjectHandlerBattary.object.config == 1) // ��������� ������ ������� ��������
				WMMLEDSetColor(cBlack); // ��������� ���������
			if (ObjectHandlerBattary.object.config == 2) // ��������� ������� � ��������� ��������
			{
#ifdef TIC_149
				WMMLEDSetColor(cWhite); // ��� 149 ��������� ��� ���������� ������
#else
				//--------�������� ����� ��� TIC-48----------------------------------------
				if (m_WMMLightVal > 1024) // ��������� ��������� ��� ������� ���������
					WMMLEDSetColor(cBlack);
				else if (m_WMMBattaryVal < ObjectHandlerBattary.object.minval) // ������� ��� ����������� �������
					WMMLEDSetColor(cRed);
				else if (m_WMMConnectionVal < 1) // ����� ��� ���������� �����
					WMMLEDSetColor(cBlue);
				else
					WMMLEDSetColor(cWhite); // �����, ���� ��� � �������
				//		//--------------------------------------------------------------------------
#endif
			}
		} else
		{
			LCDPower(FALSE); // ������� ��������
			WMMLEDSetColor(cBlack); // ��������� ���������
		}
	}

	//*******************�������*********************

	/*if (ObjectHandlerBattary.object.config==0)
	 {
	 LCDPower(FALSE);// ������� ��������
	 WMMLEDSetColor(cBlack);
	 }
	 if (ObjectHandlerBattary.object.config==1)
	 {
	 LCDPower(FALSE);// ������� ��������
	 WMMLEDSetColor(cWhite);
	 }

	 if (ObjectHandlerBattary.object.config==2)
	 {
	 LCDPower(TRUE);// ������� �������
	 WMMLEDSetColor(cBlack);
	 }

	 if (ObjectHandlerBattary.object.config==3)
	 {
	 LCDPower(TRUE);// ������� �������
	 WMMLEDSetColor(cWhite);
	 }*/
	//***************************************************
}
//---------------------------------------------------------------------------
PRIVATE void WMMLEDSetColor(WMMLedColor color)
{
	static WMMLedColor prevColor = cWhite + 1;
	if (color == prevColor)
		return;
	vAHI_DioSetOutput(0, m_WMMLedRedDio | m_WMMLedGreenDio | m_WMMLedBlueDio); // ��� ������ ��� ���������� 0 (������� ����� ����� ������,
	//�� ���� �� ������� ������� 1)
	//vAHI_DioSetOutput(m_WMMLedRedDio | m_WMMLedGreenDio | m_WMMLedBlueDio, 0);
	uint32 colorVal = 0;
	switch (color)
	{
	case cBlack:
		colorVal = 0;
		break;
	case cRed:
		colorVal = m_WMMLedRedDio;
		break;
	case cBlue:
		colorVal = m_WMMLedBlueDio;
		break;
	case cMagenta:
		colorVal = m_WMMLedRedDio | m_WMMLedBlueDio;
		break;
	case cGreen:
		colorVal = m_WMMLedGreenDio;
		break;
	case cYellow:
		colorVal = m_WMMLedRedDio | m_WMMLedGreenDio;
		break;
	case cCyan:
		colorVal = m_WMMLedGreenDio | m_WMMLedBlueDio;
		break;
	case cWhite:
		colorVal = m_WMMLedRedDio | m_WMMLedGreenDio | m_WMMLedBlueDio;
		break;
	}
	vAHI_DioSetOutput(colorVal, 0); // ��� ������ ��� ���������� 0
	//vAHI_DioSetOutput(0, colorVal);
	prevColor = color;
}
//---------------------------------------------------------------------------
