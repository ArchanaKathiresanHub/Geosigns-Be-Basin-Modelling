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

class Property : public DataAccess::Interface::Property {

public :

   Property ( DataAccess::Interface::ProjectHandle& projectHandle,
              database::Record * record,
              const std::string & userName, const std::string & cauldronName,
              const std::string & unit, const DataAccess::Interface::PropertyType type,
              const DataModel::PropertyAttribute attr, const DataModel::PropertyOutputAttribute attrOut );


   DataAccess::Interface::PropertyOutputOption getOption () const;

   const DataAccess::Interface::OutputProperty* getOutputProperty () const;

   void setOutputProperty ( const DataAccess::Interface::OutputProperty* outputProperty );

   void asString ( std::string& str ) const;

private :

   /// From time-filter table.
   const DataAccess::Interface::OutputProperty* m_outputProperty;

};


#endif // _FASTCAULDRON_PROPERTY_VALUE_H_
