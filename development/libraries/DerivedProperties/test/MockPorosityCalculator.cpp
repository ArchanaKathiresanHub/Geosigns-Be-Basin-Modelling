//                                                                      
// Copyright (C) 2017-2018 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "MockPorosityCalculator.h"
#include "AbstractProperty.h"

#include "DerivedFormationProperty.h"

namespace DataModel{

   MockPorosityCalculator::MockPorosityCalculator () {
      addPropertyName ( "Porosity" );
   }
   
   void MockPorosityCalculator::calculate (   AbstractDerivedProperties::AbstractPropertyManager& propertyManager,
                                        const DataModel::AbstractSnapshot*                        snapshot,
                                        const DataModel::AbstractFormation*                       formation,
                                              AbstractDerivedProperties::FormationPropertyList&   derivedProperties ) const {
   
      const DataModel::AbstractProperty* porosity = propertyManager.getProperty ( "Porosity" );
   
      DerivedProperties::DerivedFormationPropertyPtr porosityProp = DerivedProperties::DerivedFormationPropertyPtr( new DerivedProperties::DerivedFormationProperty ( porosity,
                                                                                                                                snapshot,
                                                                                                                                formation,
                                                                                                                                propertyManager.getMapGrid (),
                                                                                                                                numberOfGridNodes ));
   
      const double  maxN= static_cast<double> ( (porosityProp->lastI ( true ) - porosityProp->firstI ( true ) +1)
                       * (porosityProp->lastJ ( true ) - porosityProp->firstJ ( true ) +1)
                       * (porosityProp->lastK ()       - porosityProp->firstK ()       +1) );
      double el_counter=0;
   
      for ( unsigned int i = porosityProp->firstI ( true ); i <= porosityProp->lastI ( true ); ++i ) {
   
         for ( unsigned int j = porosityProp->firstJ ( true ); j <= porosityProp->lastJ ( true ); ++j ) {
            for ( unsigned int k = porosityProp->firstK (); k <= porosityProp->lastK (); ++k ) {
               porosityProp->set ( i, j, porosityProp->lastK()-k, el_counter/maxN*100.0);
               el_counter+=1.0;
            }
         }
   
      }
   
      derivedProperties.push_back ( porosityProp );
   }


}
