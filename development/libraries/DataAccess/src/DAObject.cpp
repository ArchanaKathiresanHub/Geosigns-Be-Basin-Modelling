//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "database.h"
#include "dataschema.h"
#include "cauldronschemafuncs.h"

using namespace database;
using namespace std;

#include "AttributeValue.h"
#include "ProjectHandle.h"
#include "DAObject.h"

using namespace DataAccess;
using namespace Interface;

DAObject::DAObject (ProjectHandle& projectHandle, Record * record) :
      m_projectHandle (projectHandle), m_record (record)
{
}

DAObject::~DAObject (void)
{
}

DAObject::DAObject(const DAObject& object) :
  m_projectHandle(object.m_projectHandle),
  m_record(object.m_record)
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

ProjectHandle& DAObject::getProjectHandle(void) const
{
   return m_projectHandle;
}

const ObjectFactory * DAObject::getFactory (void) const
{
   return m_projectHandle.getFactory();
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
            (void *) new AttributeValue (this, attrIndex + 1, m_record->getValue<bool>(attributeName));
            break;
         case Int:
            (void *) new AttributeValue (this, attrIndex + 1, m_record->getValue<int>(attributeName));
            break;
         case Long:
            (void *) new AttributeValue (this, attrIndex + 1, m_record->getValue<long>(attributeName));
            break;
         case Float:
            (void *) new AttributeValue (this, attrIndex + 1, m_record->getValue<float>(attributeName));
            break;
         case Double:
            (void *) new AttributeValue (this, attrIndex + 1, m_record->getValue<double>(attributeName));
            break;
         case String:
            (void *) new AttributeValue (this, attrIndex + 1, m_record->getValue<std::string>(attributeName));
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

