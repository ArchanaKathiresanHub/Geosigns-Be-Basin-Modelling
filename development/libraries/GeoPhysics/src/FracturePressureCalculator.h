#ifndef _GEOPHYSICS__FRACTURE_PRESSURE_CALCULATOR_H_
#define _GEOPHYSICS__FRACTURE_PRESSURE_CALCULATOR_H_

#include "Interface/ProjectHandle.h"
#include "Interface/FracturePressureFunctionParameters.h"

#include "CompoundLithology.h"
#include "GeoPhysicsFluidType.h"

namespace GeoPhysics {

   /// Provides functionality for calculating the fracture pressure.
   class FracturePressureCalculator {

   public :

      FracturePressureCalculator ( DataAccess::Interface::ProjectHandle* projectHandle );

      virtual ~FracturePressureCalculator ();


      /// Compute the fracture pressure.
      double fracturePressure ( const CompoundLithology* lithology,
                                const double             seaBottomDepth,
                                const double             depth,
                                const double             hydrostaticPressure,
                                const double             lithostaticPressure,
                                const double             pressureAtSeaBottom ) const;

      /// Compute the fracture pressure.
      double fracturePressure ( const CompoundLithology* lithology,
                                const FluidType*         fluid,
                                const double             seaBottomTemperature,
                                const double             seaBottomDepth,
                                const double             depth,
                                const double             hydrostaticPressure,
                                const double             lithostaticPressure ) const;

      /// Determine whether or not the pore-pressure exceeds the fracture-pressure.
      bool hasFractured ( const CompoundLithology* lithology,
                          const double             seaBottomDepth,
                          const double             depth,
                          const double             hydrostaticPressure,
                          const double             lithostaticPressure,
                          const double             pressureAtSeaBottom,
                          const double             porePressure ) const;
      
      /// Compute the fracture pressure.
      bool hasFractured ( const CompoundLithology* lithology,
                          const FluidType*         fluid,
                          const double             seaBottomTemperature,
                          const double             seaBottomDepth,
                          const double             depth,
                          const double             hydrostaticPressure,
                          const double             lithostaticPressure,
                          const double             porePressure ) const;

      const DataAccess::Interface::FracturePressureFunctionParameters& getFracturePressureFunctionParameters () const;

   private :

      /// Depth function coefficients.
      /// There are initialised with CAULDRONIBSNULLVALUE and are only set if there
      /// is a depth function selected in the project file.
      /// pf = a + b z + c z^2 + d z^3.
      double m_a;
      double m_b;
      double m_c;
      double m_d;

      /// The fracture pressure function that has been selected.
      DataAccess::Interface::FracturePressureFunctionType m_selectedFunction;

      const DataAccess::Interface::FracturePressureFunctionParameters* m_fracturePressureFunctionParameters;

   };

}

#endif // _GEOPHYSICS__FRACTURE_PRESSURE_CALCULATOR_H_
