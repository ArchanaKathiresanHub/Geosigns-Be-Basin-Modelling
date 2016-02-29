//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "SurfacePropertyOffsetCalculator.h"

#include "AbstractFormation.h"
#include "FormationProperty.h"
#include "FormationPropertyAtSurface.h"

// utilitites library
#include "LogHandler.h"

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

   bool isComputable = propManager.formationPropertyIsComputable ( m_property, snapshot, formation );
   if (not isComputable) {
      // check the formation below
      formation = getAdjacentFormation( surface, true );
      isComputable = propManager.formationPropertyIsComputable( m_property, snapshot, formation );
   }
   if (not isComputable) {
      LogHandler( LogHandler::DEBUG_SEVERITY ) << "Derived surface offset property '" << m_property->getName() << "' is not computable.";
   }
   return isComputable;

}
