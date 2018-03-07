//
// Copyright (C) 2013-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef TRACK1D__FORMATION_MAP_OUTPUT_PROPERTY_VALUE__H
#define TRACK1D__FORMATION_MAP_OUTPUT_PROPERTY_VALUE__H

#include "OutputPropertyValue.h"

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"

#include "AbstractPropertyManager.h"
#include "FormationMapProperty.h"

class FormationMapOutputPropertyValue : public OutputPropertyValue {

public :

   FormationMapOutputPropertyValue ( AbstractDerivedProperties::AbstractPropertyManager& propertyManager,
                                     const DataModel::AbstractProperty*                  property,
                                     const DataModel::AbstractSnapshot*                  snapshot,
                                     const DataModel::AbstractFormation*                 formation );

   FormationMapOutputPropertyValue ( AbstractDerivedProperties::FormationMapPropertyPtr& formationMapProperty );

   virtual double getValue ( const double i, const double j, const double k ) const;

   unsigned int getDepth () const;

   bool isPrimary() const;

private :

   AbstractDerivedProperties::FormationMapPropertyPtr m_formationMapProperty;

};


#endif // TRACK1D__FORMATION_MAP_OUTPUT_PROPERTY_VALUE__H
