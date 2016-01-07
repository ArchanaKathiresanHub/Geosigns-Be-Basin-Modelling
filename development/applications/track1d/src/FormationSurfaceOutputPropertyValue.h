#ifndef TRACK1D__FORMATION_SURFACE_OUTPUT_PROPERTY_VALUE__H
#define TRACK1D__FORMATION_SURFACE_OUTPUT_PROPERTY_VALUE__H

#include "OutputPropertyValue.h"

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractFormation.h"

#include "AbstractPropertyManager.h"
#include "FormationProperty.h"



class FormationSurfaceOutputPropertyValue : public OutputPropertyValue {

public :

   FormationSurfaceOutputPropertyValue ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                         const DataModel::AbstractProperty*          property,
                                         const DataModel::AbstractSnapshot*          snapshot,
                                         const DataModel::AbstractFormation*         formation,
                                         const DataModel::AbstractSurface*           surface );

   FormationSurfaceOutputPropertyValue ( DerivedProperties::FormationSurfacePropertyPtr& formationSurfaceProperty );

   virtual double getValue ( const double i, const double j, const double k ) const;

   unsigned int getDepth () const;

   bool isPrimary() const;

private :

   DerivedProperties::FormationSurfacePropertyPtr m_formationSurfaceProperty;

};


#endif // TRACK1D__FORMATION_OUTPUT_PROPERTY_VALUE__H
