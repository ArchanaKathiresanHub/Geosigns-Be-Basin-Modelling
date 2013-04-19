#ifndef _GENEX5_SIMULATOR__LANGMUIR_ADSORPTION_TOC_FNUCTION_H_
#define _GENEX5_SIMULATOR__LANGMUIR_ADSORPTION_TOC_FNUCTION_H_

#include <string>


// #include "database.h"

#include "Interface/GridMap.h"

#include "AdsorptionFunction.h"
#include "ComponentManager.h"

#include "AdsorptionProjectHandle.h"
#include "LangmuirAdsorptionIsothermInterval.h"

// class AdsorptionProjectHandle;


const std::string LangmuirAdsorptionTOCFunctionId = "LangmuirAdsorptionTOCFunction";

Genex5::AdsorptionFunction* allocateLangmuirAdsorptionTOCFunction ( AdsorptionProjectHandle* projectHandle,
                                                                    const std::string&       formationName );

/// A Langmuir adsorption function for C1 that depends on TOC.
class LangmuirAdsorptionTOCFunction : public Genex5::AdsorptionFunction {

public :

   LangmuirAdsorptionTOCFunction ( AdsorptionProjectHandle* adsorptionProjectHandle,
                                   const std::string& formationName );

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

   AdsorptionProjectHandle* m_adsorptionProjectHandle;

   const std::string m_formationName;

   const Interface::GridMap* m_tocMap;

   double m_referenceTemperature;
   double m_temperatureGradient;
   double m_coeffA;
   double m_coeffB;
   double m_langmuirPressure;

};

#endif // _GENEX5_SIMULATOR__LANGMUIR_ADSORPTION_TOC_FNUCTION_H_
