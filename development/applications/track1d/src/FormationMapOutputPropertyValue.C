//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "FormationMapOutputPropertyValue.h"

FormationMapOutputPropertyValue::FormationMapOutputPropertyValue ( AbstractDerivedProperties::AbstractPropertyManager& propertyManager,
                                                                   const DataModel::AbstractProperty*                  property,
                                                                   const DataModel::AbstractSnapshot*                  snapshot,
                                                                   const DataModel::AbstractFormation*                 formation ) {

   if ( property != 0 and snapshot != 0 and formation != 0 ) {
      m_formationMapProperty = propertyManager.getFormationMapProperty ( property, snapshot, formation );
   }

}

double FormationMapOutputPropertyValue::getValue ( const double i, const double j, const double k ) const {

   if ( m_formationMapProperty != 0 ) {
      return m_formationMapProperty->interpolate ( i, j );
   } else {
      return DataAccess::Interface::DefaultUndefinedMapValue;
   }

}

unsigned int FormationMapOutputPropertyValue::getDepth () const {

   if ( m_formationMapProperty != 0 ) {
      return 1;
   } else {
      return 0;
   }
   
}

bool FormationMapOutputPropertyValue::isPrimary() const {
   
   if ( m_formationMapProperty != 0 ) {
      return m_formationMapProperty->isPrimary();
   } else {
      return false;
   }
}
