#include "SurfacePropertyOffsetCalculator.h"

#include "AbstractFormation.h"
#include "FormationProperty.h"
#include "FormationPropertyAtSurface.h"


DerivedProperties::SurfacePropertyOffsetCalculator::SurfacePropertyOffsetCalculator ( const DataModel::AbstractProperty* property ) : m_property ( property ) {

   if ( m_property != 0 ) {
      addPropertyName ( m_property->getName ());
   }

}


void DerivedProperties::SurfacePropertyOffsetCalculator::calculate ( AbstractPropertyManager&           propManager,
                                                                     const DataModel::AbstractSnapshot* snapshot,
                                                                     const DataModel::AbstractSurface*  surface,
                                                                          SurfacePropertyList&         derivedProperties ) const {

   const DataModel::AbstractFormation* formationAbove = surface->getTopFormation ();
   const DataModel::AbstractFormation* formationBelow = surface->getBottomFormation ();

   FormationPropertyPtr formationProperty;

   if ( formationAbove != 0 and ( formationBelow == 0 or formationBelow->getName () == "Crust" )) {
      formationProperty = propManager.getFormationProperty ( m_property, snapshot, formationAbove );
   } else {
      formationProperty = propManager.getFormationProperty ( m_property, snapshot, formationBelow );
   }

   if ( formationProperty != 0 ) {
      SurfacePropertyPtr result;

      result = SurfacePropertyPtr ( new FormationPropertyAtSurface ( formationProperty, surface ));
      derivedProperties.push_back ( result );
   }

}
