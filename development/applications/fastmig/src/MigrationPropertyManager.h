#ifndef FASTMIG__MIGRATION_PROPERTY_MANAGER_H
#define FASTMIG__MIGRATION_PROPERTY_MANAGER_H


#include "DerivedPropertyManager.h"

namespace DerivedProperties
{
   class DerivedPropertyManager;
}

namespace migration
{

   /// \brief Handles derived properties and their calculation for fastmig.
   class MigrationPropertyManager : public DerivedProperties::DerivedPropertyManager {

   public :

      /// \brief Constructor.
      MigrationPropertyManager ( GeoPhysics::ProjectHandle* projectHandle ); 
      
      /// \brief Destructor.
      ~MigrationPropertyManager();

      DataAccess::Interface::GridMap * produceDerivedGridMap ( DerivedProperties::FormationPropertyPtr aProperty );
      DataAccess::Interface::GridMap * produceDerivedGridMap ( DerivedProperties::FormationSurfacePropertyPtr aProperty );
      DataAccess::Interface::GridMap * produceDerivedGridMap ( DerivedProperties::SurfacePropertyPtr aProperty );


   private:
      vector <DataAccess::Interface::GridMap *> m_derivedMaps;

   };

}


#endif // FASTMIG__MIGRATION_PROPERTY_MANAGER_H
