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
      DataAccess::Interface::GridMap * produceDerivedTopSurfaceGridMap ( DerivedProperties::FormationPropertyPtr aProperty );


   private:
      vector <DataAccess::Interface::GridMap *> m_derivedMaps;

      DerivedProperties::FormationPropertyPtr m_formationPropertyPtr;
      DerivedProperties::SurfacePropertyPtr   m_surfacePropertyPtr;
   };

   double getTopValue ( DerivedProperties::SurfacePropertyPtr aSurfaceProperty, DerivedProperties::FormationPropertyPtr aFormationProperty, unsigned int i, unsigned j );
   double getBottomValue ( DerivedProperties::SurfacePropertyPtr aSurfaceProperty, DerivedProperties::FormationPropertyPtr aFormationProperty, unsigned int i, unsigned j );
   void retrieveData ( DerivedProperties::SurfacePropertyPtr aSurfaceProperty, DerivedProperties::FormationPropertyPtr aFormationProperty );
   void restoreData ( DerivedProperties::SurfacePropertyPtr aSurfaceProperty, DerivedProperties::FormationPropertyPtr aFormationProperty );
}


#endif // FASTMIG__MIGRATION_PROPERTY_MANAGER_H
