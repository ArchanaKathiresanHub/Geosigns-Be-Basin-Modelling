#ifndef _GENEX6__ADSORPTION_FUNCTION_H_
#define _GENEX6__ADSORPTION_FUNCTION_H_

#include "ComponentManager.h"

namespace Genex6 {

   /// \brief Function to calculate the amount of adsorption.
   class AdsorptionFunction {

   public :

      /// \brief destructor.
      virtual ~AdsorptionFunction ();

      /// \brief Compute the adsorption capacity.
      ///
      /// Temperature in Celcius, pressure MPa.
      /// Adsorption units are cc/g.
      virtual double compute ( const unsigned int i,
                               const unsigned int j,
                               const double       temperature,
                               const double       porePressure,
                               const double       toc,
                               const CBMGenerics::ComponentManager::SpeciesNamesId species ) const = 0;

      /// \brief Compute the Langmuir volume.
      virtual double computeVL ( const unsigned int i,
                                 const unsigned int j,
                                 const double       temperature,
                                 const double       toc,
                                 const CBMGenerics::ComponentManager::SpeciesNamesId species ) const = 0;

      /// \brief The reference temperature.
      virtual double getReferenceTemperature () const = 0;

   };


}


#endif // _GENEX6__ADSORPTION_FUNCTION_H_
