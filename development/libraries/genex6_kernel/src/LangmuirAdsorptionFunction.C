#include "LangmuirAdsorptionFunction.h"

#include "Interface/Interface.h"
#include "Interface/LangmuirAdsorptionIsothermSample.h"

#include <algorithm>
#include <iostream>

#include "LangmuirAdsorptionIsothermInterval.h"

Genex6::AdsorptionFunction* Genex6::allocateLangmuirAdsorptionFunction ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                                         const std::string&                         langmuirName ) {
   return new LangmuirAdsorptionFunction ( projectHandle, langmuirName );
}



Genex6::LangmuirAdsorptionFunction::LangmuirAdsorptionFunction ( DataAccess::Interface::ProjectHandle* projectHandle,
                                                                 const std::string&                         langmuirName ) :
   m_projectHandle ( projectHandle ),
   m_langmuirName ( langmuirName )
{
   DataAccess::Interface::LangmuirAdsorptionIsothermSampleList* samples = m_projectHandle->getLangmuirAdsorptionIsothermSampleList ( langmuirName );

   if ( samples->size () < 2 ) {
      std::cout << " Formation: " << langmuirName << " does not have enough langmuir isotherm samples" << std::endl;
      assert ( samples->size () >= 2 );
   }

   DataAccess::Interface::LangmuirAdsorptionIsothermSampleList::const_iterator sampleIter = samples->begin ();
   DataAccess::Interface::LangmuirAdsorptionIsothermSampleList::const_iterator endSamples = samples->end ();

   const DataAccess::Interface::LangmuirAdsorptionIsothermSample* previous = *sampleIter;
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


double Genex6::LangmuirAdsorptionFunction::compute ( const unsigned int i,
                                                     const unsigned int j,
                                                     const double       temperature,
                                                     const double       porePressure,
                                                     const double       toc,
                                                     const CBMGenerics::ComponentManager::SpeciesNamesId species ) const {

   if ( species != CBMGenerics::ComponentManager::C1 ) {
      return 0.0;
   }

   m_temperatureInRange.setTemperature ( temperature );
   LangmuirAdsorptionIsothermIntervalVector::const_iterator interval = std::find_if ( m_isothermIntervals.begin (), m_isothermIntervals.end (), m_temperatureInRange );

   if ( interval != m_isothermIntervals.end ()) {
      return (*interval)->compute ( temperature, porePressure );
   } else {
      return 0.0;
   }

}


double Genex6::LangmuirAdsorptionFunction::computeVL ( const unsigned int i,
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


void Genex6::LangmuirAdsorptionFunction::initialise () {

}


bool Genex6::LangmuirAdsorptionFunction::LangmuirAdsorptionIsothermLowerBoundOrder::operator ()( const LangmuirAdsorptionIsothermInterval* i1,
                                                                                                 const LangmuirAdsorptionIsothermInterval* i2 ) const {


   return i1->getLowerBound ()->getLangmuirTemperature () < i2->getLowerBound ()->getLangmuirTemperature ();
}


Genex6::LangmuirAdsorptionFunction::LangmuirAdsorptionIsothermInRange::LangmuirAdsorptionIsothermInRange () {
   m_temperature = 0.0;
}

void Genex6::LangmuirAdsorptionFunction::LangmuirAdsorptionIsothermInRange::setTemperature ( const double temperature ) {
   m_temperature = temperature;
}

bool Genex6::LangmuirAdsorptionFunction::LangmuirAdsorptionIsothermInRange::operator ()( const LangmuirAdsorptionIsothermInterval* interval ) const {
   return interval->getLowerBound ()->getLangmuirTemperature () <= m_temperature and
          m_temperature <= interval->getUpperBound ()->getLangmuirTemperature ();
}

double Genex6::LangmuirAdsorptionFunction::getReferenceTemperature () const {
   return m_temperatureLowerBound;
} 
