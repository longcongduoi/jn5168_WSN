#include "ObjectChildren1.h"

//---------------------------------------------------------------------------

PUBLIC Object ObjectChildren1;

PUBLIC void ObjectChildren1Init(void)
    {
    ObjectChildren1.objectID = OBJECT_CHILDREN1;
    ObjectChildren1.val = 0;
    ObjectChildren1.valControl = 0;
    ObjectChildren1.minval = 0;
    ObjectChildren1.maxval = 100;
    ObjectChildren1.sendDval = 1;
    ObjectChildren1.valType = pvtUint8;
    ObjectChildren1.precision = 0;

    ObjectChildren1.readTime = OBJECT_DISABLE;
    ObjectChildren1.sendTime = OBJECT_DISABLE;
    ObjectChildren1.config = 0;
    ObjectChildren1.quality = 0xC0;
    }

//---------------------------------------------------------------------------

