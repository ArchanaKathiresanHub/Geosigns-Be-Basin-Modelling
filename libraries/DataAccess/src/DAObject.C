#include "database.h"
#include "dataschema.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/AttributeValue.h"
#include "Interface/ProjectHandle.h"
#include "Interface/DAObject.h"

using namespace DataAccess;
using namespace Interface;

DAObject::DAObject (ProjectHandle * projectHandle, Record * record) :
      m_projectHandle (projectHandle), m_record (record)
{
}

DAObject::~DAObject (void)
{
}

Record * DAObject::getRecord (void) const
{
   return m_record;
}

void DAObject::setRecord (Record * record)
{
   m_record = record;
}

ProjectHandle * DAObject::getProjectHandle (void) const
{
   return m_projectHandle;
}

ObjectFactory * DAObject::getFactory (void) const
{
   return getProjectHandle ()->getFactory ();
}

AttributeValue & DAObject::getAttributeValue (const string & attributeName, unsigned int indexOffset) const
{
   int attrIndex = getAttributeIndex (attributeName) + indexOffset;

   assert (attrIndex >= -1);

   if (!getChild (attrIndex + 1))
   {
      switch (getAttributeType (attributeName))
      {
         case Bool:
            (void *) new AttributeValue (this, attrIndex + 1, m_record->getValue (attributeName, (bool *) 0));
            break;
         case Int:
            (void *) new AttributeValue (this, attrIndex + 1, m_record->getValue (attributeName, (int *) 0));
            break;
         case Long:
            (void *) new AttributeValue (this, attrIndex + 1, m_record->getValue (attributeName, (long *) 0));
            break;
         case Float:
            (void *) new AttributeValue (this, attrIndex + 1, m_record->getValue (attributeName, (float *) 0));
            break;
         case Double:
            (void *) new AttributeValue (this, attrIndex + 1, m_record->getValue (attributeName, (double *) 0));
            break;
         case String:
            (void *) new AttributeValue (this, attrIndex + 1, m_record->getValue (attributeName, (string *) 0));
            break;
         case NoAttributeType:
         default:
	    (void *) new AttributeValue (this, attrIndex + 1);
            break;
      }
   }
   return ((AttributeValue &) (* getChild (attrIndex + 1)));
}

AttributeType DAObject::getAttributeType (const string & attributeName) const
{
   const TableDefinition & tb = m_record->getTableDefinition ();

   int fdIndex = tb.getIndex (attributeName);
   if (fdIndex < 0) return NoAttributeType;

   const FieldDefinition * fd = tb.getFieldDefinition (fdIndex);

   return (AttributeType) fd->dataType ();

}

int DAObject::getAttributeIndex (const string & attributeName) const
{
   const TableDefinition & tb = m_record->getTableDefinition ();
   return tb.getIndex (attributeName);
}

