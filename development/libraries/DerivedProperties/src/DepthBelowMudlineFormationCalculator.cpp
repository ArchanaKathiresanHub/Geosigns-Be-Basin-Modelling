//
// Copyright (C) 2021-2021 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by CGI.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedPropertyManager.h"

#include "Snapshot.h"

#include "DepthBelowMudlineFormationCalculator.h"
#include "PropertyRetriever.h"

using namespace AbstractDerivedProperties;

DerivedProperties::DepthBelowMudlineFormationCalculator::DepthBelowMudlineFormationCalculator( const GeoPhysics::ProjectHandle& projectHandle ) : m_projectHandle ( projectHandle ) 
{
   addPropertyName ( "DepthBelowMudline" );
   addDependentPropertyName( "Depth" );
}

void DerivedProperties::DepthBelowMudlineFormationCalculator::calculate (       AbstractPropertyManager&      propertyManager,
                                                                            const DataModel::AbstractSnapshot*  snapshot,
                                                                            const DataModel::AbstractFormation* formation,
                                                                                  FormationPropertyList&        derivedProperties ) const 
{
    const DataModel::AbstractProperty* adepthBelowMudline = propertyManager.getProperty("DepthBelowMudline");
    const DataModel::AbstractProperty* adepthProperty = propertyManager.getProperty("Depth");
    const FormationPropertyPtr depth = propertyManager.getFormationProperty(adepthProperty, snapshot, formation);

    PropertyRetriever depthRetriever(depth);

    derivedProperties.clear();

   if( depth ) 
   {
      auto depthBelowMudline = std::make_shared<DerivedFormationProperty>(adepthBelowMudline, snapshot, formation,
                                                                             propertyManager.getMapGrid(), depth->lengthK());
      double undefinedValue = depth->getUndefinedValue ();

      for ( unsigned int i = depth->firstI ( true ); i <= depth->lastI ( true ); ++i )
      {
         for ( unsigned int j = depth->firstJ ( true ); j <= depth->lastJ ( true ); ++j )
         {
               for ( unsigned int k = depth->firstK (); k <= depth->lastK (); ++k )
               {
                   // Calculte the 'DepthBelowMudline' property value (DbML) at any (i,j,k)
                   auto DbML = depth->get(i, j, k) - m_projectHandle.getSeaBottomDepth(i, j, snapshot->getTime());
                   // Set the property value at the node depending upon the validity of the node
                   // The value is set to the calculated value (DbML) if it is a valid node else it is set to 'undefinedValue'
                   depthBelowMudline->set(i, j, k,
                       DbML * m_projectHandle.getNodeIsValid(i, j) +
                       undefinedValue * (!m_projectHandle.getNodeIsValid(i, j)));
               }
         }
      }
      derivedProperties.push_back (depthBelowMudline);
   }
}
//------------------------------------------------------------//

bool DerivedProperties::DepthBelowMudlineFormationCalculator::isComputable ( const AbstractPropertyManager&      propManager,
                                                                               const DataModel::AbstractSnapshot*  snapshot,
                                                                               const DataModel::AbstractFormation* formation ) const 
{
    const DataModel::AbstractProperty* depth = propManager.getProperty("Depth");
    return propManager.formationPropertyIsComputable(depth, snapshot, formation);
}
