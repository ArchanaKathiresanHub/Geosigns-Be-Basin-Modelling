#ifndef FASTPROPERTIES_FORMATION_MAP_OUTPUT_PROPERTY_VALUE__H
#define FASTPROPERTIES_FORMATION_MAP_OUTPUT_PROPERTY_VALUE__H

#include "OutputPropertyValue.h"

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"

#include "AbstractPropertyManager.h"
#include "FormationMapProperty.h"

class FormationMapOutputPropertyValue : public OutputPropertyValue {

public :

   FormationMapOutputPropertyValue ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                     const DataModel::AbstractProperty*          property,
                                     const DataModel::AbstractSnapshot*          snapshot,
                                     const DataModel::AbstractFormation*         formation );

   FormationMapOutputPropertyValue ( DerivedProperties::FormationMapPropertyPtr& formationMapProperty );

   virtual double getValue ( const double i, const double j, const double k ) const;

   unsigned int getDepth () const;

   const string & getName() const { return  m_formationMapProperty->getProperty()->getName(); }

   bool hasMap() const;

 private :

   DerivedProperties::FormationMapPropertyPtr m_formationMapProperty;

};


#endif // FASTPROPERTIES_FORMATION_MAP_OUTPUT_PROPERTY_VALUE__H
