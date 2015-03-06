#include "Object.h"
#include "Converter.h"
#include "MathSimple.h"
#include "ErrorHandler.h"
//---------------------------------------------------------------------------

PRIVATE ObjectEnterCS m_EnterCS;
PRIVATE ObjectLeaveCS m_LeaveCS;
PRIVATE ObjectGetTime m_GetTime;
//---------------------------------------------------------------------------

PUBLIC void ObjectInit(ObjectEnterCS enterCS, ObjectLeaveCS leaveCS,
	ObjectGetTime getTime)
    {
    m_EnterCS = enterCS;
    m_LeaveCS = leaveCS;
    m_GetTime = getTime;
    }
//------------содержание свойств в объекте------------------

PUBLIC void ObjectToPack(Object object, Packet* pack, bool onlyVal)
    {
    PacketAddObject(pack, object.objectID);

    if (!onlyVal)
	{
	//*********************************убрано из пакета объекта****************************************
	//PacketAddProperty (pack, OBJECT_PROP_NAME, pvtString, 0, object.precision, object.name);
	//PacketAddProperty (pack, OBJECT_PROP_DIMENSION, pvtString, 0, object.precision, object.dimension);
	//PacketAddProperty (pack, OBJECT_PROP_MINVAL, object.valType, object.minval, object.precision, NULL);
	//PacketAddProperty (pack, OBJECT_PROP_MAXVAL, object.valType, object.maxval, object.precision, NULL);
	//**************************************************************************************************
	PacketAddProperty(pack, OBJECT_PROP_PRECISION, pvtInt8,	object.precision, object.precision, NULL);
	PacketAddProperty(pack, OBJECT_PROP_SENDDVAL, object.valType, object.sendDval, object.precision, NULL);
	PacketAddProperty(pack, OBJECT_PROP_VALCONTROL, object.valType, object.valControl, object.precision, NULL);
	PacketAddProperty(pack, OBJECT_PROP_READTIME, pvtUint16, object.readTime, object.precision, NULL);
	PacketAddProperty(pack, OBJECT_PROP_SENDTIME, pvtUint16, object.sendTime, object.precision, NULL);
	PacketAddProperty(pack, OBJECT_PROP_CONFIG, pvtInt32, object.config, object.precision, NULL);
	}
    //**********************обязательные объекты****************************
    PacketAddProperty(pack, OBJECT_PROP_VAL, object.valType, object.val, object.precision, NULL);
    PacketAddProperty(pack, OBJECT_PROP_QUALITY, pvtUint8, object.quality, object.precision, NULL);
    //**********************************************************************
    }
//---------------------------------------------------------------------------
PUBLIC bool ObjectPropValToPack(uint32 val, Object object, Packet* pack,
	uint16 propID)//что можно изменять
    {
    PacketAddObject(pack, object.objectID);
    switch (propID)
	{
    case OBJECT_PROP_VALCONTROL:
	PacketAddProperty(pack, OBJECT_PROP_VALCONTROL, object.valType, val,
		object.precision, NULL);
	return TRUE;
    case OBJECT_PROP_CONFIG:
	PacketAddProperty(pack, OBJECT_PROP_CONFIG, pvtInt32, val,
		object.precision, NULL);
	return TRUE;

    case OBJECT_PROP_SENDDVAL:
	PacketAddProperty(pack, OBJECT_PROP_SENDDVAL, object.valType, val,
		object.precision, NULL);
	return TRUE;

    case OBJECT_PROP_READTIME:
	PacketAddProperty(pack, OBJECT_PROP_READTIME, pvtUint16, val,
		object.precision, NULL);
	return TRUE;
    case OBJECT_PROP_SENDTIME:
	PacketAddProperty(pack, OBJECT_PROP_SENDTIME, pvtUint16, val,
		object.precision, NULL);
	return TRUE;
	}
    return FALSE;
    }
//---------------------------------------------------------------------------
PUBLIC void ObjectSetProp(Object* object, uint16 propID, ValType valType,
	uint32 val, char* strVal)
    {
    if (m_EnterCS != NULL)
	m_EnterCS();
    switch (propID)
	{
    //case (OBJECT_PROP_NAME) : ConverterPackStringToPackString(strVal, object->name); break;
    //case (OBJECT_PROP_DIMENSION) : ConverterPackStringToPackString(strVal, object->dimension); break;
    case (OBJECT_PROP_VAL):
	{
	object->val = val;
	object->valType = valType;
	if (m_GetTime != NULL)
	    m_GetTime(&object->lastUpdTime);
	}
	break;
	//case (OBJECT_PROP_MINVAL) : object->minval = val; break;
	//case (OBJECT_PROP_MAXVAL) : object->maxval = val; break;
    case (OBJECT_PROP_READTIME):
	object->readTime = (uint16) val;
	break;
    case (OBJECT_PROP_SENDTIME):
	object->sendTime = (uint16) val;
	break;
    case (OBJECT_PROP_CONFIG):
	object->config = (int32) val;
	break;
    case (OBJECT_PROP_PRECISION):
	object->precision = (uint8) val;
	break;

    case (OBJECT_PROP_SENDDVAL):
	object->sendDval = val;
	break;

    case (OBJECT_PROP_VALCONTROL):
	object->valControl = val;
	break;
    case (OBJECT_PROP_QUALITY):
	object->quality = (uint8) val;
	break;
    default:
	EHCall(OBJECT_MOD_ID, 1);
	break;
	}
    if (m_LeaveCS != NULL)
	m_LeaveCS();
    }
//---------------------------------------------------------------------------

PUBLIC float ObjectGetPropFloat(Object object, uint16 propID)
    {
    if (m_EnterCS != NULL)
	m_EnterCS();
    uint32 val32 = 0;
    float valFloat;

    switch (propID)
	{
    case (OBJECT_PROP_VAL):
	val32 = object.val;
	break;
    case (OBJECT_PROP_MINVAL):
	val32 = object.minval;
	break;
    case (OBJECT_PROP_MAXVAL):
	val32 = object.maxval;
	break;
    case (OBJECT_PROP_SENDDVAL):
	val32 = object.sendDval;
	break;
    default:
	EHCall(OBJECT_MOD_ID, 2);
	break;
	}
    switch (object.valType)
	{
    case pvtUint8:
    case pvtUint16:
    case pvtUint32:
	valFloat = val32;
	break;
    case pvtInt8:
    case pvtInt16:
    case pvtInt32:
    case pvtFloat:
	valFloat = (int32) val32;
	break;
    default:
	valFloat = 0;
	break;
	}
    if (m_LeaveCS != NULL)
	m_LeaveCS();
    return valFloat * MathSimplePowInt8(0.1, object.precision);
    }
//---------------------------------------------------------------------------

PUBLIC void ObjectCopy(Object src, Object* dst)
    {
    //ConverterPackStringToPackString (src.name, dst->name);
    //ConverterPackStringToPackString (src.dimension, dst->dimension);

    dst->val = src.val;
    dst->valControl = src.valControl;
    dst->minval = src.minval;
    dst->maxval = src.maxval;
    dst->sendDval = src.sendDval;
    dst->valType = src.valType;
    dst->precision = src.precision;
    dst->readTime = src.readTime;
    dst->sendTime = src.sendTime;
    dst->config = src.config;
    dst->quality = src.quality;
    }
//---------------------------------------------------------------------------
