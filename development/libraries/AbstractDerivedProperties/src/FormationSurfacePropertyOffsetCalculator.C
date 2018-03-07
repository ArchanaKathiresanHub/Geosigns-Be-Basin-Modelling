//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FormationSurfacePropertyOffsetCalculator.h"

#include "AbstractFormation.h"
#include "FormationProperty.h"
#include "FormationSurfacePropertyAtSurface.h"


AbstractDerivedProperties::FormationSurfacePropertyOffsetCalculator::FormationSurfacePropertyOffsetCalculator ( const DataModel::AbstractProperty* property,
                                                                                                                const std::vector<std::string>&    dependentPropertyNames ) :
   m_property ( property )
{

   if ( m_property != nullptr ) {
      addPropertyName ( m_property->getName ());
   }

   for ( size_t i = 0; i < dependentPropertyNames.size (); ++i ) {
      addDependentPropertyName ( dependentPropertyNames [ i ]);
   }

}


void AbstractDerivedProperties::FormationSurfacePropertyOffsetCalculator::calculate ( AbstractPropertyManager&      propManager,
                                                                                const DataModel::AbstractSnapshot*  snapshot,
                                                                                const DataModel::AbstractFormation* formation,
                                                                                const DataModel::AbstractSurface*   surface,
                                                                                      FormationSurfacePropertyList& derivedProperties ) const {

   const DataModel::AbstractFormation* formationAbove = surface->getTopFormation ();
   const DataModel::AbstractFormation* formationBelow = surface->getBottomFormation ();

   if ( formation == nullptr or ( formationAbove != formation and formationBelow != formation )) {
      // No property can be calculated.
      return;
   }

   FormationPropertyPtr formationProperty = propManager.getFormationProperty ( m_property, snapshot, formation );

   if ( formationProperty != nullptr ) {
      FormationSurfacePropertyPtr result;

      result = FormationSurfacePropertyPtr ( new FormationSurfacePropertyAtSurface ( formationProperty, surface ));
      derivedProperties.push_back ( result );
   }

}

bool AbstractDerivedProperties::FormationSurfacePropertyOffsetCalculator::isComputable ( const AbstractPropertyManager&      propManager,
                                                                                         const DataModel::AbstractSnapshot*  snapshot,
                                                                                         const DataModel::AbstractFormation* formation,
                                                                                         const DataModel::AbstractSurface*   surface ) const {
   (void) surface;
   return propManager.formationPropertyIsComputable ( m_property, snapshot, formation );
}
