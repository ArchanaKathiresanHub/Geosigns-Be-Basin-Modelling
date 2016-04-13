//                                                                      
// Copyright (C) 2016-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Derived property library
#include "AbstractPropertyManager.h"
#include "DerivedFormationProperty.h"
#include "DerivedPropertyManager.h"
#include "PropertyRetriever.h"

// Geophysics library
#include "GeoPhysicsFormation.h"
#include "GeoPhysicsProjectHandle.h"

// Utility library
#include "FormattingException.h"

#include "SonicFormationCalculator.h"

typedef formattingexception::GeneralException SonicException;

DerivedProperties::SonicFormationCalculator::SonicFormationCalculator() {
   addPropertyName ( "SonicSlowness" );
   addDependentPropertyName ( "Velocity" );
}

void DerivedProperties::SonicFormationCalculator::calculate( DerivedProperties::AbstractPropertyManager& propertyManager,
                                                                 const DataModel::AbstractSnapshot*  snapshot,
                                                                 const DataModel::AbstractFormation* formation,
                                                                       FormationPropertyList&        derivedProperties ) const {

   DataModel::AbstractProperty const * const velocityProperty = propertyManager.getProperty( "Velocity" );

   DataModel::AbstractProperty const * const sonicProperty = propertyManager.getProperty ( "SonicSlowness" );
   
   const FormationPropertyPtr velocity = propertyManager.getFormationProperty( velocityProperty, snapshot, formation );

   GeoPhysics::Formation const * const geophysicsFormation = dynamic_cast<const GeoPhysics::Formation*>(formation);
   GeoPhysics::ProjectHandle const * const projectHandle = dynamic_cast<const GeoPhysics::ProjectHandle*>(geophysicsFormation->getProjectHandle());
   
   derivedProperties.clear ();

   if (velocity != 0) {

      PropertyRetriever porosityRetriever( velocity );

      DerivedFormationPropertyPtr sonic = DerivedFormationPropertyPtr( new DerivedProperties::DerivedFormationProperty( sonicProperty,
                                                                                                                        snapshot,
                                                                                                                        formation, 
                                                                                                                        propertyManager.getMapGrid (),
                                                                                                                        geophysicsFormation->getMaximumNumberOfElements() + 1 ));

      double undefinedValue = sonic->getUndefinedValue ();
      double sonicValue;

      for (unsigned int i = sonic->firstI( true ); i <= sonic->lastI( true ); ++i) {

         for (unsigned int j = sonic->firstJ( true ); j <= sonic->lastJ( true ); ++j) {

            if ( projectHandle->getNodeIsValid ( i, j )) {

               for (unsigned int k = sonic->firstK(); k <= sonic->lastK(); ++k) {

                  sonicValue = 1.0e6 / velocity->getA( i, j, k );

                  sonic->set( i, j, k, sonicValue );
               }

            } else {

               for (unsigned int k = sonic->firstK(); k <= sonic->lastK(); ++k) {
                  sonic->set( i, j, k, undefinedValue );
               }

            }

         }

      }

      derivedProperties.push_back ( sonic );
   }

   else{
      throw SonicException() << "Cannot get sonic slowness dependent property 'velocity'.";
   }

}
