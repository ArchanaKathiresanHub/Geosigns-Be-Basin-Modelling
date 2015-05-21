#include "FormationSurfacePropertyOffsetCalculator.h"

#include "AbstractFormation.h"
#include "FormationProperty.h"
#include "FormationSurfacePropertyAtSurface.h"


DerivedProperties::FormationSurfacePropertyOffsetCalculator::FormationSurfacePropertyOffsetCalculator ( const DataModel::AbstractProperty* property ) : m_property ( property ) {

   if ( m_property != 0 ) {
      addPropertyName ( m_property->getName ());
   }

}


void DerivedProperties::FormationSurfacePropertyOffsetCalculator::calculate ( AbstractPropertyManager&            propManager,
                                                                              const DataModel::AbstractSnapshot*  snapshot,
                                                                              const DataModel::AbstractFormation* formation,
                                                                              const DataModel::AbstractSurface*   surface,
                                                                                    FormationSurfacePropertyList& derivedProperties ) const {

   const DataModel::AbstractFormation* formationAbove = surface->getTopFormation ();
   const DataModel::AbstractFormation* formationBelow = surface->getBottomFormation ();

   if ( formation == 0 or ( formationAbove != formation and formationBelow != formation )) {
      // No property can be calculated.
      return;
   }

   FormationPropertyPtr formationProperty = propManager.getFormationProperty ( m_property, snapshot, formation );

   if ( formationProperty != 0 ) {
      FormationSurfacePropertyPtr result;

      result = FormationSurfacePropertyPtr ( new FormationSurfacePropertyAtSurface ( formationProperty, surface ));
      derivedProperties.push_back ( result );
   }

}
