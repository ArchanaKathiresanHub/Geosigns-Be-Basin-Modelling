#ifndef FASTMIG__MIGRATION_PROPERTY_MANAGER_H
#define FASTMIG__MIGRATION_PROPERTY_MANAGER_H

#include "DerivedPropertyManager.h"

namespace migration
{

   /// \brief Handles derived properties and their calculation for fastmig.
   class MigrationPropertyManager : public DerivedProperties::DerivedPropertyManager {

   public :

      /// \brief Constructor.
      MigrationPropertyManager ( GeoPhysics::ProjectHandle* projectHandle ); 
      
      /// \brief Destructor.
      ~MigrationPropertyManager() {}

   };

}


#endif // FASTMIG__MIGRATION_PROPERTY_MANAGER_H
