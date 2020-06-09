//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FormationMapPropertyCalculator.h"
#include "AbstractPropertyManager.h"

bool AbstractDerivedProperties::FormationMapPropertyCalculator::isComputable ( const AbstractPropertyManager&      propManager,
                                                                               const DataModel::AbstractSnapshot*  snapshot,
                                                                               const DataModel::AbstractFormation* formation ) const {

   const std::vector<std::string>& dependentProperties = getDependentPropertyNames ();
   bool propertyIsComputable = true;

   // Determine if the required properties are computable.
   for ( size_t i = 0; i < dependentProperties.size () and propertyIsComputable; ++i ) {
      const DataModel::AbstractProperty* property = propManager.getProperty ( dependentProperties [ i ]);

      if ( property == nullptr ) {
         propertyIsComputable = false;
      } else {
         propertyIsComputable = propertyIsComputable and propManager.formationMapPropertyIsComputable ( property, snapshot, formation );
      }

   }

   return propertyIsComputable;
}
