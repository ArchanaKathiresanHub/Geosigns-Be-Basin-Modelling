#ifndef DERIVED_PROPERTIES__PRIMARY_PROPERTY_H
#define DERIVED_PROPERTIES__PRIMARY_PROPERTY_H

#include "Interface/GridMap.h"
#include "Interface/PropertyValue.h"
#include "Property.h"

namespace DerivedProperties {

   /// \brief Contains gridMap information 

   class PrimaryProperty : virtual public Property {

   public:

      PrimaryProperty ( const DataAccess::Interface::GridMap* aMap );
      
      ///  \brief Retrieve data in-or excluding ghost points according to specification
      bool retrieveData ( bool withGhosts = false ) const;
	    
      ///  \brief Restore data according to specifications
      bool restoreData ( bool save = true, bool withGhosts = false ) const;

      ///  \brief return the value used for undefined values
      double getUndefinedValue (void) const;

   protected :

      const DataAccess::Interface::GridMap* m_gridMap;
      
   };


  
}
#endif
