#ifndef DERIVED_PROPERTIES__OVERPRESSURE_SURFACE_CALCULATOR_H
#define DERIVED_PROPERTIES__OVERPRESSURE_SURFACE_CALCULATOR_H

#include "SurfacePropertyCalculator.h"

namespace DerivedProperties {

   class OverpressureSurfaceCalculator : public SurfacePropertyCalculator {

   public :

      OverpressureSurfaceCalculator ();

      virtual ~OverpressureSurfaceCalculator () {}
 
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractSurface*   surface,
                                     SurfacePropertyList&          derivedProperties ) const;

    };


}

#endif 
