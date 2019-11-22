//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef _FASTCTC_PROPERTYVALUE_H_
#define _FASTCTC_PROPERTYVALUE_H_

// DataAccessLibrary
#include "PropertyValue.h"
#include "Interface.h"

// TableIo library
#include "database.h"

using namespace DataAccess;

namespace Ctc {
   /// @class PropertyValue The property value used by fastctc
   class CTCPropertyValue : public Interface::PropertyValue {

   public:

      CTCPropertyValue(Interface::ProjectHandle& projectHandle,
                     database::Record * record,
                     const string & name,
                     const Interface::Property * property,
                     const Interface::Snapshot * snapshot,
                     const Interface::Reservoir * reservoir,
                     const Interface::Formation * formation,
                     const Interface::Surface * surface,
                     const Interface::PropertyStorage storage );

      bool toBeSaved() const;

      /// @brief Allow or prevent a property from being output.
      /// @details Allows repression of output what-ever the output option is selected.
      void allowOutput( const bool output );

   private:

      bool m_allowOutput; ///< Says if the property has to be output
   };

}
#endif
