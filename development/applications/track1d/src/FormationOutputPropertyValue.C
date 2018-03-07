//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FormationOutputPropertyValue.h"

FormationOutputPropertyValue::FormationOutputPropertyValue ( AbstractDerivedProperties::AbstractPropertyManager& propertyManager,
                                                             const DataModel::AbstractProperty*                  property,
                                                             const DataModel::AbstractSnapshot*                  snapshot,
                                                             const DataModel::AbstractFormation*                 formation ) {

   if ( property != 0 and snapshot != 0 and formation != 0 ) {
      m_formationProperty = propertyManager.getFormationProperty ( property, snapshot, formation );
   }

}

FormationOutputPropertyValue::FormationOutputPropertyValue ( AbstractDerivedProperties::FormationPropertyPtr& formationProperty ) : m_formationProperty ( formationProperty )
{
}


double FormationOutputPropertyValue::getValue ( const double i, const double j, const double k ) const {

   if ( m_formationProperty != 0 ) {
      return m_formationProperty->interpolate ( i, j, k );
   } else {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }

}

unsigned int FormationOutputPropertyValue::getDepth () const {

   if ( m_formationProperty != 0 ) {
      return m_formationProperty->lengthK ();
   } else {
      return 0;
   }
   
}

bool FormationOutputPropertyValue::isPrimary() const {
   
   if ( m_formationProperty != 0 ) {
      return m_formationProperty->isPrimary();
   } else {
      return false;
   }
}
