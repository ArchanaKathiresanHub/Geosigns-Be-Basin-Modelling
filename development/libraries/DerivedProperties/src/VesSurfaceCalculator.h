#ifndef DERIVED_PROPERTIES__VES_SURFACE_CALCULATOR_H
#define DERIVED_PROPERTIES__VES_SURFACE_CALCULATOR_H

#include "SurfacePropertyCalculator.h"

namespace DerivedProperties {

   class VesSurfaceCalculator : public SurfacePropertyCalculator {

   public :

      VesSurfaceCalculator ();

      virtual ~VesSurfaceCalculator () {}
 
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractSurface*   surface,
                                     SurfacePropertyList&          derivedProperties ) const;

    };


}

#endif 
