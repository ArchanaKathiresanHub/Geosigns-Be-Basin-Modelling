
#ifndef FASTPROPERTIES_FORMATION_OUTPUT_PROPERTY_VALUE__H
#define FASTPROPERTIES_FORMATION_OUTPUT_PROPERTY_VALUE__H

#include "OutputPropertyValue.h"

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"

#include "AbstractPropertyManager.h"
#include "FormationProperty.h"



class FormationOutputPropertyValue : public OutputPropertyValue {

public :

   FormationOutputPropertyValue ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                  const DataModel::AbstractProperty*          property,
                                  const DataModel::AbstractSnapshot*          snapshot,
                                  const DataModel::AbstractFormation*         formation );

   FormationOutputPropertyValue ( DerivedProperties::FormationPropertyPtr&    formationProperty );

   virtual double getValue ( const double i, const double j, const double k ) const;

   unsigned int getDepth () const;

   const string & getName() const { return  m_formationProperty->getProperty()->getName(); }

private :

   DerivedProperties::FormationPropertyPtr m_formationProperty;

};


#endif // FASTPROPERTIES_FORMATION_OUTPUT_PROPERTY_VALUE__H
