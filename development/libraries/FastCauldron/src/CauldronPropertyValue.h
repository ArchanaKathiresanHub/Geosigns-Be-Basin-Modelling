//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef _FASTCAULDRON_PROPERTY_VALUE_H_
#define _FASTCAULDRON_PROPERTY_VALUE_H_

#include "PropertyValue.h"
#include "Interface.h"


namespace DataAccess
{
   namespace Interface
   {
      class ProjectHandle;
   }
}
using namespace DataAccess;

class CauldronPropertyValue : public Interface::PropertyValue {

public :

   CauldronPropertyValue ( Interface::ProjectHandle& projectHandle,
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
   bool saveMapToFile ( Interface::MapWriter & mapWriter, const bool saveAsPrimary = false );

   /// save a 3D PropertyValue to file
   bool saveVolumeToFile ( Interface::MapWriter & mapWriter, const bool saveAsPrimary = false );

   database::Record * createTimeIoRecord (database::Table * timeIoTbl);


private :

   bool m_allowOutput;

};



#endif // _FASTCAULDRON_PROPERTY_VALUE_H_
