#include "LangmuirAdsorptionTOCFunction.h"

#include <algorithm>
#include <iostream>
using namespace std;

#include "Interface/Formation.h"
#include "Interface/SourceRock.h"
#include "Interface/GridMap.h"

#include "NumericFunctions.h"

Genex5::AdsorptionFunction* allocateLangmuirAdsorptionTOCFunction ( AdsorptionProjectHandle* projectHandle,
                                                                    const std::string& formationName ) {
   return new LangmuirAdsorptionTOCFunction ( projectHandle, formationName );
}



LangmuirAdsorptionTOCFunction::LangmuirAdsorptionTOCFunction ( AdsorptionProjectHandle* adsorptionProjectHandle,
                                                               const std::string& formationName ) :
   m_adsorptionProjectHandle ( adsorptionProjectHandle ),
   m_formationName ( formationName )
{

//    const Interface::Formation* formation;
   const LangmuirAdsorptionTOCEntry* adsorptionEntry = adsorptionProjectHandle->getLangmuirAdsorptionTOCEntry ( formationName );

//    formation = m_adsorptionProjectHandle->getDALProjectHandle ()->findFormation ( formationName );

//    if ( formation != 0 and formation->getSourceRock () != 0 ) {
//       m_tocMap = formation->getSourceRock ()->getMap ( Interface::TocIni );
//       m_tocMap->retrieveData ();
//    } else {
//       m_tocMap = 0;
//       // What to do here? Error?
//    } 

   if ( adsorptionEntry != 0 ) {
      m_referenceTemperature = adsorptionEntry->getReferenceTemperature ();
      m_temperatureGradient = adsorptionEntry->getTemperatureGradient ();
      m_coeffA = adsorptionEntry->getCoeffA ();
      m_coeffB = adsorptionEntry->getCoeffB ();
      m_langmuirPressure = adsorptionEntry->getLangmuirPressure ();
   } else {
      std::cout << "Error : entry in LangmuirAdsorptionTOCIsothermIoTbl for formation " << formationName << " was not found." << endl;
      throw formationName;
      // What to do?
   }

}

LangmuirAdsorptionTOCFunction::~LangmuirAdsorptionTOCFunction () {

//    if ( m_tocMap != 0 ) {
//       m_tocMap->restoreData ();
//    } 

}

double LangmuirAdsorptionTOCFunction::compute ( const unsigned int i,
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


double LangmuirAdsorptionTOCFunction::computeVL ( const unsigned int i,
                                                  const unsigned int j,
                                                  const double       temperature,
                                                  const double       toc,
                                                  const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   if ( species != CBMGenerics::ComponentManager::C1 ) {
      return 0.0;
   }

   double vl;

   vl = (( m_coeffA * toc + m_coeffB ) + m_temperatureGradient * ( temperature - m_referenceTemperature ));

   return NumericFunctions::Maximum ( 0.0, vl );
}

double LangmuirAdsorptionTOCFunction::getReferenceTemperature () const {
   return m_referenceTemperature;
} 
