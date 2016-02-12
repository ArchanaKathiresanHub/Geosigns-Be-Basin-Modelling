#include "SurfacePropertyOffsetCalculator.h"

#include "AbstractFormation.h"
#include "FormationProperty.h"
#include "FormationPropertyAtSurface.h"


DerivedProperties::SurfacePropertyOffsetCalculator::SurfacePropertyOffsetCalculator ( const DataModel::AbstractProperty* property,
                                                                                      const std::vector<std::string>&    dependentPropertyNames ) :
   m_property ( property )
{

   if ( m_property != 0 ) {
      addPropertyName ( m_property->getName ());
   }

   for ( size_t i = 0; i < dependentPropertyNames.size (); ++i ) {
      addDependentPropertyName ( dependentPropertyNames [ i ]);
   }

}

const DataModel::AbstractFormation* DerivedProperties::SurfacePropertyOffsetCalculator::getAdjacentFormation ( const DataModel::AbstractSurface* surface, const bool useBottom ) const {

   if ( surface == 0 ) {
      return 0;
   }

   const DataModel::AbstractFormation* formationAbove = surface->getTopFormation ();
   const DataModel::AbstractFormation* formationBelow = surface->getBottomFormation ();

   if( useBottom ) {
      if ( formationBelow != 0 ) {
         return formationBelow;
      } else if ( formationAbove != 0 ) {
         return formationAbove;
      } else {
         return 0;
      }
   } else {
      if ( formationAbove != 0 and ( formationBelow == 0 or formationBelow->getName () == "Crust" )) {
         return formationAbove;
      } else if ( formationBelow != 0 ) {
         return formationBelow;
      } else {
         return 0;
      }
   }
}

void DerivedProperties::SurfacePropertyOffsetCalculator::calculate ( AbstractPropertyManager&           propManager,
                                                                     const DataModel::AbstractSnapshot* snapshot,
                                                                     const DataModel::AbstractSurface*  surface,
                                                                           SurfacePropertyList&         derivedProperties ) const {

   const DataModel::AbstractFormation* formation = getAdjacentFormation ( surface, false );

   FormationPropertyPtr formationProperty;

   if ( formation != 0 ) {
      SurfacePropertyPtr result;

      formationProperty = propManager.getFormationProperty ( m_property, snapshot, formation );

      if ( formationProperty != 0 ) {
         result = SurfacePropertyPtr ( new FormationPropertyAtSurface ( formationProperty, surface ));
         derivedProperties.push_back ( result );
      } else {
         // try the formation below
         formation = getAdjacentFormation ( surface, true );
         formationProperty = propManager.getFormationProperty ( m_property, snapshot, formation );

         if ( formationProperty != 0 ) {
            result = SurfacePropertyPtr ( new FormationPropertyAtSurface ( formationProperty, surface ));
            derivedProperties.push_back ( result );
         }
      }
   }
}

bool DerivedProperties::SurfacePropertyOffsetCalculator::isComputable ( const AbstractPropertyManager&      propManager,
                                                                        const DataModel::AbstractSnapshot*  snapshot,
                                                                        const DataModel::AbstractSurface*   surface ) const {

   if( surface == 0 ) {
      return true;
   }
   const DataModel::AbstractFormation* formation = getAdjacentFormation ( surface, false );

   if ( formation != 0 ) {
      const bool isComputable = propManager.formationPropertyIsComputable ( m_property, snapshot, formation );
      if( not isComputable ) {
         // check the formation below
         formation = getAdjacentFormation ( surface, true );
         return propManager.formationPropertyIsComputable ( m_property, snapshot, formation );
      } else {
         return true;
      }
   } else {
      return false;
   }

}
