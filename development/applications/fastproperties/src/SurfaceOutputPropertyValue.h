#ifndef FASTPROPERTIES_SURFACE_OUTPUT_PROPERTY_VALUE__H
#define FASTPROPERTIES_SURFACE_OUTPUT_PROPERTY_VALUE__H

#include "OutputPropertyValue.h"

#include "AbstractProperty.h"
#include "AbstractSnapshot.h"
#include "AbstractSurface.h"

#include "AbstractPropertyManager.h"
#include "SurfaceProperty.h"

class SurfaceOutputPropertyValue : public OutputPropertyValue {

public :

   SurfaceOutputPropertyValue ( DerivedProperties::AbstractPropertyManager& propertyManager,
                                     const DataModel::AbstractProperty*        property,
                                     const DataModel::AbstractSnapshot*        snapshot,
                                     const DataModel::AbstractSurface*         surface );

   SurfaceOutputPropertyValue ( DerivedProperties::SurfacePropertyPtr& surfaceProperty );

   virtual double getValue ( const double i, const double j, const double k ) const;

   unsigned int getDepth () const;

   const string & getName() const { return  m_surfaceProperty->getProperty()->getName(); }

   const DataModel::AbstractSurface* getSurface() const { return  m_surfaceProperty->getSurface(); }

   bool hasMap() const { return  m_surfaceProperty != 0; }

 private :

   DerivedProperties::SurfacePropertyPtr m_surfaceProperty;

};


#endif // FASTPROPERTIES_SURFACE_OUTPUT_PROPERTY_VALUE__H
