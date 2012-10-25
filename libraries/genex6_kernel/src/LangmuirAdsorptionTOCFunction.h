#ifndef _GENEX6_KERNEL__LANGMUIR_ADSORPTION_TOC_FNUCTION_H_
#define _GENEX6_KERNEL__LANGMUIR_ADSORPTION_TOC_FNUCTION_H_

#include <string>


#include "Interface/GridMap.h"
#include "Interface/ProjectHandle.h"

#include "AdsorptionFunction.h"
#include "ComponentManager.h"

#include "LangmuirAdsorptionIsothermInterval.h"

// class AdsorptionProjectHandle;

namespace Genex6 {

   /// \brief Identifier needed to determine whith adsorption function is to be allocated in the factory.
   const std::string LangmuirAdsorptionTOCFunctionId = "LangmuirAdsorptionTOCFunction";

   /// \brief Allocates an adsorption function.
   AdsorptionFunction* allocateLangmuirAdsorptionTOCFunction ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                               const std::string&                         functionName );

   /// A Langmuir adsorption function for C1 that depends on TOC.
   class LangmuirAdsorptionTOCFunction : public AdsorptionFunction {

   public :

      LangmuirAdsorptionTOCFunction ( DataAccess::Interface::ProjectHandle* projectHandle,
                                      const std::string&                         functionName );

      ~LangmuirAdsorptionTOCFunction ();

      /// Compute the adsorption capacity.
      ///
      /// Temperature in Celcius, pressure in MPa.
      double compute ( const unsigned int i,
                       const unsigned int j,
                       const double       temperature,
                       const double       porePressure,
                       const double       toc,
                       const CBMGenerics::ComponentManager::SpeciesNamesId species ) const;

      /// Compute the VL.
      double computeVL ( const unsigned int i,
                         const unsigned int j,
                         const double       temperature,
                         const double       toc,
                         const CBMGenerics::ComponentManager::SpeciesNamesId species ) const;

      double getReferenceTemperature () const;

      void initialise ();

   private :

      DataAccess::Interface::ProjectHandle* m_projectHandle;

      const std::string m_functionName;

      const DataAccess::Interface::GridMap* m_tocMap;

      double m_referenceTemperature;
      double m_temperatureGradient;
      double m_coeffA;
      double m_coeffB;
      double m_langmuirPressure;

   };

}

#endif // _GENEX6_KERNEL__LANGMUIR_ADSORPTION_TOC_FNUCTION_H_
