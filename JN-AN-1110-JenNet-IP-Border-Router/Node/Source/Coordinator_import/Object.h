#ifndef OBJECT_H
#define OBJECT_H
//---------------------------------------------------------------------------

#include <jendefs.h>
#include "Packet.h"
//---------------------------------------------------------------------------

//ID и имена приведены в соответствие п.6 "СОВМЕСТИМЫЕ ОБЪЕКТЫ ИНФОРМАЦИИ" описания протокола

#define OBJECT_CONNECTION       2//Connection
#define OBJECT_LQI   		    3//LinkQuality
#define OBJECT_BATTARY   	    4//Battery
#define OBJECT_GERKON11   	    5//Gerkon
#define OBJECT_HUMIDITY1   	    6//SHT1_Humidity
#define OBJECT_DEWPOINT   		7//SHT1_DewPoint
#define OBJECT_TEMPRETURE1      8//SHT1_Temperature
#define OBJECT_LIGHTAPDS9300    9//DigitalLight
#define OBJECT_SUGGERKON        10//SUG_DI1
#define OBJECT_SUGGERKON1       11//SUG_DI2
#define OBJECT_SUGGERKON2       12//SUG_DI3
#define OBJECT_LIGHTAPDS9002	13//AnalogLight
#define OBJECT_GERKON9			14//DI1
#define OBJECT_GERKON10			15//DI2
#define OBJECT_RELAY			16//DO1
#define OBJECT_HUMIDITY2   	    17//SHT2_Humidity
#define OBJECT_TEMPRETURE2      18//SHT2_Temperature
#define OBJECT_BEEP				19//Beep
#define TEMPERATURE2			20//Temperature1
#define TEMPERATURE3			21//Temperature2
#define TEMPERATURE4			22//Temperature3
#define TEMPERATURE5			23//Temperature4
#define OBJECT_UPS				24//UPS
#define OBJECT_RELAY2			25//DO2
#define OBJECT_RELAY3			26//DO3
#define OBJECT_RELAY4			27//DO4
#define OBJECT_TSP				28//Pt1_Temperature
#define OBJECT_TSP2				29//Pt2_Temperature
#define OBJECT_TSP3				30//Pt3_Temperature
#define OBJECT_TSP4				31//Pt4_Temperature
#define OBJECT_DEWPOINT2		32//SHT2_DewPoint
#define OBJECT_DI3				33//DI3
#define OBJECT_DI4				34//DI4
#define OBJECT_LIGHTAPDS9300_OR_LIGHTAPDS9002	0xFFFF
#define OBJECT_CHILDREN1			35// обнаружено устройств
#define OBJECT_CHILDREN2			36// зарегистрировано устройств
#define OBJECT_CHILDREN3			37// в сети в это время устройств
#define OBJECT_CHILDRENROUTER			38// подключенных к роутеру устройств

#define OBJECT_PROP_NAME       16
#define OBJECT_PROP_DIMENSION  17
#define OBJECT_PROP_VAL        18
#define OBJECT_PROP_MINVAL     19
#define OBJECT_PROP_MAXVAL     20
#define OBJECT_PROP_READTIME   21
#define OBJECT_PROP_SENDTIME   22
#define OBJECT_PROP_SENDDVAL   23
#define OBJECT_PROP_VALCONTROL 24
#define OBJECT_PROP_CONFIG     25
#define OBJECT_PROP_PRECISION  26
#define OBJECT_PROP_QUALITY	   27
#define OBJECT_PROP_SETPOINT   28
#define OBJECT_PROP_HYSTERESIS 29
#define OBJECT_PROP_MAC		   30
#define OBJECT_PROP_OBJECTNUM  31
#define OBJECT_PROP_REACTION   32

#define OBJECT_DISABLE       0
#define OBJECT_ONWAKE	     0xFFFF
//---------------------------------------------------------------------------

PUBLIC typedef struct
    {
	uint16 objectID;
	uint32 val;
	uint32 valControl;
	uint32 sendDval;
	uint16 readTime;
	uint16 sendTime;
	int32 config;
	uint8 quality;
	uint8 precision;
	ValType valType;
	//23
	//*********** не в радиопакете *************
	uint64 lastUpdTime;
	uint32 minval;
	uint32 maxval;
    //------------------------------------------
    } Object;

PUBLIC typedef void (*ObjectEnterCS)(void);
PUBLIC typedef void (*ObjectLeaveCS)(void);
PUBLIC typedef void (*ObjectGetTime)(uint64* time);
//---------------------------------------------------------------------------

PUBLIC void ObjectInit(ObjectEnterCS enterCS, ObjectLeaveCS leaveCS,
	ObjectGetTime getTime);
PUBLIC void ObjectToPack(Object object, Packet* pack, bool onlyVal);
PUBLIC bool ObjectPropValToPack(uint32 val, Object object, Packet* pack,
	uint16 propID);
PUBLIC void ObjectSetProp(Object* object, uint16 propID, ValType valType,
	uint32 val, char* strVal);
PUBLIC void ObjectSetString(char* dstString, char* srcString);
PUBLIC void ObjectSetPackString(char* dstString, char* srcString);
PUBLIC float ObjectGetPropFloat(Object object, uint16 propID);
PUBLIC void ObjectCopy(Object src, Object* dst);
#endif
