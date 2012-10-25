#include "LangmuirAdsorptionFunction.h"

#include <algorithm>

Genex5::AdsorptionFunction* allocateLangmuirAdsorptionFunction ( AdsorptionProjectHandle* projectHandle,
                                                                 const std::string& langmuirName ) {
   return new LangmuirAdsorptionFunction ( projectHandle, langmuirName );
}



LangmuirAdsorptionFunction::LangmuirAdsorptionFunction ( AdsorptionProjectHandle* adsorptionProjectHandle,
                                                         const std::string& langmuirName ) :
   m_adsorptionProjectHandle ( adsorptionProjectHandle ),
   m_langmuirName ( langmuirName )
{
   ConstLangmuirAdsorptionIsothermSampleList* samples = m_adsorptionProjectHandle->getLangmuirAdsorptionIsothermSampleList ( langmuirName );

   if ( samples->size () < 2 ) {
      cout << " Formation: " << langmuirName << " does not have enough langmuir isotherm samples" << endl;
      assert ( samples->size () >= 2 );
   }

   ConstLangmuirAdsorptionIsothermSampleList::const_iterator sampleIter = samples->begin ();
   ConstLangmuirAdsorptionIsothermSampleList::const_iterator endSamples = samples->end ();

   const LangmuirAdsorptionIsothermSample* previous = *sampleIter;
   ++sampleIter;

   m_temperatureLowerBound = previous->getLangmuirTemperature ();

   for ( ; sampleIter != endSamples; ++sampleIter ) {
      m_isothermIntervals.push_back ( new LangmuirAdsorptionIsothermInterval ( previous, *sampleIter ));
      previous = *sampleIter;
   }

   m_temperatureUpperBound = previous->getLangmuirTemperature ();

   std::sort ( m_isothermIntervals.begin (), m_isothermIntervals.end (), LangmuirAdsorptionIsothermLowerBoundOrder ());
   delete samples;
}


double LangmuirAdsorptionFunction::compute ( const unsigned int i,
                                             const unsigned int j,
                                             const double       temperature,
                                             const double       porePressure,
                                             const double       toc,
                                             const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   if ( species != CBMGenerics::ComponentManager::C1 ) {
      return 0.0;
   }

//    if ( temperature < m_temperatureLowerBound or m_temperatureUpperBound < temperature  ) {
//       return 0.0;
//    }

//    m_temperatureInRange.setTemperature ( 50 );
   m_temperatureInRange.setTemperature ( temperature );
   LangmuirAdsorptionIsothermIntervalVector::const_iterator interval = std::find_if ( m_isothermIntervals.begin (), m_isothermIntervals.end (), m_temperatureInRange );

   if ( interval != m_isothermIntervals.end ()) {
      return (*interval)->compute ( temperature, porePressure );
   } else {
      return 0.0;
   }

}


double LangmuirAdsorptionFunction::computeVL ( const unsigned int i,
                                               const unsigned int j,
                                               const double       temperature,
                                               const double       toc,
                                               const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   if ( species != CBMGenerics::ComponentManager::C1 ) {
      return 0.0;
   }

   m_temperatureInRange.setTemperature ( temperature );
   LangmuirAdsorptionIsothermIntervalVector::const_iterator interval = std::find_if ( m_isothermIntervals.begin (), m_isothermIntervals.end (), m_temperatureInRange );

   if ( interval != m_isothermIntervals.end ()) {
      return (*interval)->computeVL ( temperature );
   } else {
      return 0.0;
   }

}


void LangmuirAdsorptionFunction::initialise () {

}


bool LangmuirAdsorptionFunction::LangmuirAdsorptionIsothermLowerBoundOrder::operator ()( const LangmuirAdsorptionIsothermInterval* i1,
                                                                                         const LangmuirAdsorptionIsothermInterval* i2 ) const {


   return i1->getLowerBound ()->getLangmuirTemperature () < i2->getLowerBound ()->getLangmuirTemperature ();
}


LangmuirAdsorptionFunction::LangmuirAdsorptionIsothermInRange::LangmuirAdsorptionIsothermInRange () {
   m_temperature = 0.0;
}

void LangmuirAdsorptionFunction::LangmuirAdsorptionIsothermInRange::setTemperature ( const double temperature ) {
   m_temperature = temperature;
}

bool LangmuirAdsorptionFunction::LangmuirAdsorptionIsothermInRange::operator ()( const LangmuirAdsorptionIsothermInterval* interval ) const {
   return interval->getLowerBound ()->getLangmuirTemperature () <= m_temperature and
          m_temperature <= interval->getUpperBound ()->getLangmuirTemperature ();
}

double LangmuirAdsorptionFunction::getReferenceTemperature () const {
   return m_temperatureLowerBound;
} 
