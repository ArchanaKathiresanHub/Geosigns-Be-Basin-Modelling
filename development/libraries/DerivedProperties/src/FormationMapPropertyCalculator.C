#include "FormationMapPropertyCalculator.h"
#include "AbstractPropertyManager.h"

bool DerivedProperties::FormationMapPropertyCalculator::isComputable ( const AbstractPropertyManager&      propManager,
                                                                       const DataModel::AbstractSnapshot*  snapshot,
                                                                       const DataModel::AbstractFormation* formation ) const {

   const std::vector<std::string>& dependentProperties = getDependentPropertyNames ();
   bool propertyIsComputable = true;

   // Determine if the required properties are computable.
   for ( size_t i = 0; i < dependentProperties.size () and propertyIsComputable; ++i ) {
      const DataModel::AbstractProperty* property = propManager.getProperty ( dependentProperties [ i ]);

      if ( property == 0 ) {
         propertyIsComputable = false;
      } else {
         propertyIsComputable = propertyIsComputable and propManager.formationMapPropertyIsComputable ( property, snapshot, formation );
      }

   }

   return propertyIsComputable;
}
