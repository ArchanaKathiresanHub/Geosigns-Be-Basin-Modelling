//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "ThicknessFormationMapCalculator.h"

#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedFormationMapProperty.h"
#include "DerivedPropertyManager.h"

#include "GeoPhysicsFormation.h"

#include "PropertyRetriever.h"

using namespace AbstractDerivedProperties;

DerivedProperties::ThicknessFormationMapCalculator::ThicknessFormationMapCalculator () {
   addPropertyName ( "Thickness" );

   addDependentPropertyName ( "Depth" );
}

double DerivedProperties::ThicknessFormationMapCalculator::calculateThickness(const double bottomDepth, const double topDepth) const
{
  return bottomDepth - topDepth;
}

void DerivedProperties::ThicknessFormationMapCalculator::calculate ( AbstractPropertyManager&            propertyManager,
                                                                     const DataModel::AbstractSnapshot*  snapshot,
                                                                     const DataModel::AbstractFormation* formation,
                                                                           FormationMapPropertyList&     derivedProperties ) const {

   const DataModel::AbstractProperty* thicknessProperty = propertyManager.getProperty ( "Thickness" );
   const DataModel::AbstractProperty* depthProperty = propertyManager.getProperty ( "Depth" );
   const FormationPropertyPtr depth = propertyManager.getFormationProperty ( depthProperty, snapshot, formation );

   PropertyRetriever depthRetriever ( depth );

   derivedProperties.clear ();

   if( depth != 0  ) {
      DerivedFormationMapPropertyPtr thickness = DerivedFormationMapPropertyPtr ( new DerivedProperties::DerivedFormationMapProperty ( thicknessProperty,
                                                                                                                                       snapshot,
                                                                                                                                       formation,
                                                                                                                                       propertyManager.getMapGrid ()));
      unsigned int nodeCount = depth->lastK ();
      double undefinedValue = depth->getUndefinedValue ();

      for ( unsigned int i = depth->firstI ( true ); i <= depth->lastI ( true ); ++i ) {

         for ( unsigned int j = depth->firstJ ( true ); j <= depth->lastJ ( true ); ++j ) {

            if ( propertyManager.getNodeIsValid ( i, j )) {

               thickness->set ( i, j, calculateThickness(depth->get(i,j,0), depth->get(i,j,nodeCount)));
            } else {
               thickness->set ( i, j, undefinedValue );
            }

         }

      }

      derivedProperties.push_back ( thickness );
   }

}


bool DerivedProperties::ThicknessFormationMapCalculator::isComputable ( const AbstractPropertyManager&      propManager,
                                                                        const DataModel::AbstractSnapshot*  snapshot,
                                                                        const DataModel::AbstractFormation* formation ) const {

   const DataModel::AbstractProperty* depth   = propManager.getProperty ( "Depth" );
   return propManager.formationPropertyIsComputable ( depth, snapshot, formation );
}
