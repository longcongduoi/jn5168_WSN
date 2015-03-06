#include "ObjectConnection.h"
//---------------------------------------------------------------------------

PUBLIC Object ObjectConnection;

PUBLIC void ObjectConnectionInit(void)
    {
    ObjectConnection.objectID = OBJECT_CONNECTION;
    ObjectConnection.val = 0;
    ObjectConnection.valControl = 0;
    ObjectConnection.minval = 0;
    ObjectConnection.maxval = 1;
    ObjectConnection.sendDval = 1;
    ObjectConnection.valType = pvtBool;
    ObjectConnection.precision = 0;
    ObjectConnection.readTime = OBJECT_DISABLE;
    ObjectConnection.sendTime = OBJECT_DISABLE;
    ObjectConnection.config = 0;
    ObjectConnection.quality = 0xC0;
    }
//---------------------------------------------------------------------------
PUBLIC bool SetConfigConnection(int32 config)
    {
    ObjectConnection.config = config;//
       return TRUE;
    }
//---------------------------------------------------------------------------
