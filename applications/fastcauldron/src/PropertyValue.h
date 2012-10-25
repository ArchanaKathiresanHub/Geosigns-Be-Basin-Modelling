#ifndef _FASTCAULDRON_PROPERTY_VALUE_H_
#define _FASTCAULDRON_PROPERTY_VALUE_H_

#include "Interface/PropertyValue.h"
#include "Interface/Interface.h"


namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
   }
}
using namespace DataAccess;

class PropertyValue : public Interface::PropertyValue {

public :

   PropertyValue ( Interface::ProjectHandle * projectHandle,
                   database::Record * record,
                   const string & name, 
                   const Interface::Property * property,
                   const Interface::Snapshot * snapshot,
                   const Interface::Reservoir * reservoir,
                   const Interface::Formation * formation,
                   const Interface::Surface * surface, 
                   const Interface::PropertyStorage storage );


   bool toBeSaved () const;

   /// Allow or prevent a property from being output.
   ///
   /// Allows repression of output what-ever the output option is selected.
   void allowOutput ( const bool output );

   /// Indicates whether or not this property should be output.
   bool outputIsRequested () const;

   /// save a 2D PropertyValue to file
   bool saveMapToFile ( Interface::MapWriter & mapWriter );

   /// save a 3D PropertyValue to file
   bool saveVolumeToFile ( Interface::MapWriter & mapWriter );

   database::Record * createTimeIoRecord (database::Table * timeIoTbl, Interface::ModellingMode theMode);


private :

   bool m_allowOutput;

};



#endif // _FASTCAULDRON_PROPERTY_VALUE_H_
