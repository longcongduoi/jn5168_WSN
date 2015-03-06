#ifndef WORKMODEMANAGER_H
#define WORKMODEMANAGER_H
//---------------------------------------------------------------------------

#include <jendefs.h>
//---------------------------------------------------------------------------

PUBLIC void WMMInit (void);
PUBLIC void WMMUpdateSleepTime(void);
PUBLIC void WMMCorrectTime(uint32 realSleepTime);
PUBLIC void WMMSetNextMesurePeriodTime(uint32 mesurePeriodTime);
PUBLIC uint32 WMMGetTime(void);
PUBLIC bool WMMIsServiceMode(void);
PUBLIC bool WMMIsOnline(void);
PUBLIC bool WMMIsDemoMode(void);
PUBLIC void WMMSetOnline(bool online);
PUBLIC bool WMMButtonEvent (uint32 deviceId, uint32 itemBitmap);
PUBLIC void WMMUpdateSensorsVal(uint16 battaryVal, uint16 lightVal);

#endif
