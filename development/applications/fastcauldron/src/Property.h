#ifndef _FASTCAULDRON_PROPERTY_H_
#define _FASTCAULDRON_PROPERTY_H_

#include "PropertyAttribute.h"

#include "Property.h"
#include "OutputProperty.h"

#include "Interface.h"


namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
      class OutputProperty;
   }
}
using namespace DataAccess;

class Property : public Interface::Property {

public :

   Property ( Interface::ProjectHandle * projectHandle,
              database::Record * record,
              const string & userName, const string & cauldronName,
              const string & unit, const Interface::PropertyType type,
              const DataModel::PropertyAttribute attr, const DataModel::PropertyOutputAttribute attrOut );


   Interface::PropertyOutputOption getOption () const;

   const Interface::OutputProperty* getOutputProperty () const;

   void setOutputProperty ( const Interface::OutputProperty* outputProperty );

   void asString ( string& str ) const;

private :

   /// From time-filter table.
   const Interface::OutputProperty* m_outputProperty;

};


#endif // _FASTCAULDRON_PROPERTY_VALUE_H_
