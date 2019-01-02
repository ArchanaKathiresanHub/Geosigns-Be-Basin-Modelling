#include "LangmuirAdsorptionTOCFunction.h"

#include <algorithm>
#include <iostream>
#include <sstream>
using namespace std;

#include "Interface/Formation.h"
#include "Interface/SourceRock.h"
#include "Interface/GridMap.h"
#include "Interface/LangmuirAdsorptionTOCEntry.h"

#include "NumericFunctions.h"

Genex6::AdsorptionFunction* Genex6::allocateLangmuirAdsorptionTOCFunction ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                                            const std::string&                         functionName ) {
   return new LangmuirAdsorptionTOCFunction ( projectHandle, functionName );
}



Genex6::LangmuirAdsorptionTOCFunction::LangmuirAdsorptionTOCFunction ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                                       const std::string&                         functionName ) :
   m_projectHandle ( projectHandle ),
   m_functionName ( functionName )
{

   const DataAccess::Interface::LangmuirAdsorptionTOCEntry* adsorptionEntry = projectHandle->getLangmuirAdsorptionTOCEntry ( functionName );

   if ( adsorptionEntry != 0 ) {
      m_referenceTemperature = adsorptionEntry->getReferenceTemperature ();
      m_temperatureGradient = adsorptionEntry->getTemperatureGradient ();
      m_coeffA = adsorptionEntry->getCoeffA ();
      m_coeffB = adsorptionEntry->getCoeffB ();
      m_langmuirPressure = adsorptionEntry->getLangmuirPressure ();
   } else {
      std::cout << "Basin_Error: entry in LangmuirAdsorptionTOCIsothermIoTbl for function named " << functionName << " was not found." << endl;
      throw functionName;
      // What to do?
   }

}

Genex6::LangmuirAdsorptionTOCFunction::~LangmuirAdsorptionTOCFunction () {

//    if ( m_tocMap != 0 ) {
//       m_tocMap->restoreData ();
//    } 

}

double Genex6::LangmuirAdsorptionTOCFunction::compute ( const unsigned int i,
                                                        const unsigned int j,
                                                        const double       temperature,
                                                        const double       porePressure,
                                                        const double       toc,
                                                        const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   if ( species != CBMGenerics::ComponentManager::C1 ) {
      return 0.0;
   }

   double adsorptionVolume = 0.0;

   adsorptionVolume = computeVL ( i, j, temperature, toc, species ) * porePressure / ( m_langmuirPressure + porePressure );

   return adsorptionVolume;
}


double Genex6::LangmuirAdsorptionTOCFunction::computeVL ( const unsigned int i,
                                                          const unsigned int j,
                                                          const double       temperature,
                                                          const double       toc,
                                                          const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   if ( species != CBMGenerics::ComponentManager::C1 ) {
      return 0.0;
   }
   // Added to prevent a compiler warning about unused parameter.
   (void) i;
   (void) j;

   double vl;

   vl = (( m_coeffA * toc + m_coeffB ) + m_temperatureGradient * ( temperature - m_referenceTemperature ));

   return NumericFunctions::Maximum ( 0.0, vl );
}

double Genex6::LangmuirAdsorptionTOCFunction::getReferenceTemperature () const {
   return m_referenceTemperature;
} 

bool Genex6::LangmuirAdsorptionTOCFunction::isValid () const {
   return m_referenceTemperature != DataAccess::Interface::DefaultUndefinedScalarValue and
          m_temperatureGradient != DataAccess::Interface::DefaultUndefinedScalarValue and
          m_coeffA != DataAccess::Interface::DefaultUndefinedScalarValue and
          m_coeffB != DataAccess::Interface::DefaultUndefinedScalarValue and
          m_langmuirPressure != DataAccess::Interface::DefaultUndefinedScalarValue;
}

std::string Genex6::LangmuirAdsorptionTOCFunction::getErrorMessage () const {

   std::stringstream buffer;

   if ( m_referenceTemperature == DataAccess::Interface::DefaultUndefinedScalarValue ) {
      buffer << " Reference temperature is invalid " << std::endl;
   }

   if ( m_temperatureGradient == DataAccess::Interface::DefaultUndefinedScalarValue  ) {
      buffer << " Temperature gradient is invalid  " << std::endl;
   }

   if ( m_coeffA == DataAccess::Interface::DefaultUndefinedScalarValue  ) {
      buffer << " CoeffA is invalid  " << std::endl;
   }

   if ( m_coeffB == DataAccess::Interface::DefaultUndefinedScalarValue  ) {
      buffer << " CoeffB is invalid  " << std::endl;
   }

   if ( m_langmuirPressure == DataAccess::Interface::DefaultUndefinedScalarValue ) {
      buffer << " Lamgmuir pressure is invalid  " << std::endl;
   }

   return buffer.str ();
}
