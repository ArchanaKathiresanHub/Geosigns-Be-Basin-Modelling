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

void DerivedProperties::SurfacePropertyOffsetCalculator::calculate ( AbstractPropertyManager&           propManager,
                                                                     const DataModel::AbstractSnapshot* snapshot,
                                                                     const DataModel::AbstractSurface*  surface,
                                                                          SurfacePropertyList&         derivedProperties ) const {

   const DataModel::AbstractFormation* formationAbove = surface->getTopFormation ();
   const DataModel::AbstractFormation* formationBelow = surface->getBottomFormation ();

   FormationPropertyPtr formationProperty;

   if ( formationAbove != 0 and ( formationBelow == 0 or formationBelow->getName () == "Crust" )) {
      formationProperty = propManager.getFormationProperty ( m_property, snapshot, formationAbove );
   } else if ( formationBelow != 0 ) {
      formationProperty = propManager.getFormationProperty ( m_property, snapshot, formationBelow );
   }

   if ( formationProperty != 0 ) {
      SurfacePropertyPtr result;

      result = SurfacePropertyPtr ( new FormationPropertyAtSurface ( formationProperty, surface ));
      derivedProperties.push_back ( result );
   }

}

bool DerivedProperties::SurfacePropertyOffsetCalculator::isComputable ( const AbstractPropertyManager&      propManager,
                                                                        const DataModel::AbstractSnapshot*  snapshot,
                                                                        const DataModel::AbstractSurface*   surface ) const {

   const DataModel::AbstractFormation* formationAbove = surface->getTopFormation ();
   const DataModel::AbstractFormation* formationBelow = surface->getBottomFormation ();

   FormationPropertyPtr formationProperty;

   if ( formationAbove != 0 and ( formationBelow == 0 or formationBelow->getName () == "Crust" )) {
      return propManager.formationPropertyIsComputable ( m_property, snapshot, formationAbove );
   } else if ( formationBelow != 0 ) {
      return propManager.formationPropertyIsComputable ( m_property, snapshot, formationBelow );
   } else {
      return false;
   }

}
