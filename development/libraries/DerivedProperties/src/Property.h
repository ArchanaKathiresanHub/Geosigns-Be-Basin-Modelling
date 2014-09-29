#ifndef DERIVED_PROPERTIES__PROPERTY_H
#define DERIVED_PROPERTIES__PROPERTY_H

#include "Interface/Interface.h"

namespace DerivedProperties {

   /// \brief Contains virtual methods 

   class Property {
   public:

      Property () {};
      virtual ~Property () {};
      
      ///  \brief Retrieve data in-or excluding ghost points according to specification
      virtual bool retrieveData ( bool withGhosts = false ) const;
      
      ///  \brief Restore data according to specifications
      virtual bool restoreData ( bool save = true, bool withGhosts = false ) const;
      
      ///  \brief return the value used for undefined values
      virtual double getUndefinedValue (void) const;
      
   };

}
#endif

