#ifndef _GENEX5_ADSORPTION_FUNCTION_H_
#define _GENEX5_ADSORPTION_FUNCTION_H_

#include "SourceRockNodeInput.h"
#include "SourceRockNodeOutput.h"
#include "ComponentManager.h"

namespace Genex5 {

   class AdsorptionFunction {

   public :

      virtual ~AdsorptionFunction ();

      /// Compute the adsorption capacity.
      ///
      /// Temperature in Celcius, pressure MPa.
      /// Adsorption units are cc/g.
      virtual double compute ( const unsigned int i,
                               const unsigned int j,
                               const double       temperature,
                               const double       porePressure,
                               const double       toc,
                               const CBMGenerics::ComponentManager::SpeciesNamesId species ) const = 0;

      /// Compute the VL.
      virtual double computeVL ( const unsigned int i,
                                 const unsigned int j,
                                 const double       temperature,
                                 const double       toc,
                                 const CBMGenerics::ComponentManager::SpeciesNamesId species ) const = 0;

      virtual double getReferenceTemperature () const = 0;

   };


}


#endif // _GENEX5_ADSORPTION_FUNCTION_H_
