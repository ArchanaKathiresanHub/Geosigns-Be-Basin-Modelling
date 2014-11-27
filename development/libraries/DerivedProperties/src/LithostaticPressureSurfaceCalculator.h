#ifndef DERIVED_PROPERTIES__LITHOSTATIC_PRESSURE_SURFACE_CALCULATOR_H
#define DERIVED_PROPERTIES__LITHOSTATIC_PRESSURE_SURFACE_CALCULATOR_H

#include "SurfacePropertyCalculator.h"

namespace DerivedProperties {

   class  LithostaticPressureSurfaceCalculator : public SurfacePropertyCalculator {

   public :

      LithostaticPressureSurfaceCalculator ( const GeoPhysics::ProjectHandle* projectHandle );

      virtual ~ LithostaticPressureSurfaceCalculator () {}
 
      virtual void calculate ( AbstractPropertyManager&            propManager,
                               const DataModel::AbstractSnapshot*  snapshot,
                               const DataModel::AbstractSurface*   surface,
                                     SurfacePropertyList&          derivedProperties ) const;

      /// \brief Get a list of the property names that will be calculated by the calculator.
      virtual const std::vector<std::string>& getPropertyNames () const;

   private :

      std::vector<std::string> m_propertyNames;
    };


}

#endif 
