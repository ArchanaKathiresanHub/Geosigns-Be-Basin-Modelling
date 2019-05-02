#include "LangmuirAdsorptionIsothermInterval.h"

#include <iostream>
#include <sstream>

#include "Interface/Interface.h"

Genex6::LangmuirAdsorptionIsothermInterval::LangmuirAdsorptionIsothermInterval ( const DataAccess::Interface::LangmuirAdsorptionIsothermSample* lowerBound,
                                                                                 const DataAccess::Interface::LangmuirAdsorptionIsothermSample* upperBound ) :
   m_lowerBound ( lowerBound ),
   m_upperBound ( upperBound ),

   m_temperatureLow ( lowerBound->getLangmuirTemperature ()),
   m_vlLow ( lowerBound->getLangmuirVolume ()),
   m_plLow ( lowerBound->getLangmuirPressure ()),

   m_temperatureHigh ( upperBound->getLangmuirTemperature ()),
   m_vlHigh ( upperBound->getLangmuirVolume ()),
   m_plHigh ( upperBound->getLangmuirPressure ()),

   m_temperatureDiff ( 1.0 / ( m_temperatureHigh - m_temperatureLow )),

   m_aCoeffVl (( m_vlHigh - m_vlLow ) / ( m_temperatureHigh - m_temperatureLow )),
   m_bCoeffVl (( m_vlLow * m_temperatureHigh - m_vlHigh * m_temperatureLow ) / ( m_temperatureHigh - m_temperatureLow )),
   m_aCoeffPl (( m_plHigh - m_plLow ) / ( m_temperatureHigh - m_temperatureLow )),
   m_bCoeffPl (( m_plLow * m_temperatureHigh - m_plHigh * m_temperatureLow ) / ( m_temperatureHigh - m_temperatureLow ))
{
}

bool Genex6::LangmuirAdsorptionIsothermInterval::temperatureInRange ( const double temperature ) const {
   return m_temperatureLow <= temperature and temperature <= m_temperatureHigh;
}

double Genex6::LangmuirAdsorptionIsothermInterval::compute ( const double temperature,
                                                             const double pressure ) const {

   double adsorped = 0.0;

#if 0
   // Crop the temperature to be in the range [ low .. high ].
   double temperatureUsed = ( temperature > m_temperatureHigh ? m_temperatureHigh : ( temperature < m_temperatureLow ? m_temperatureLow : temperature ));

   double vl = ( m_vlLow * ( m_temperatureHigh - temperatureUsed ) + m_vlHigh * ( temperatureUsed - m_temperatureLow  )) * m_temperatureDiff;
   double pl = ( m_plLow * ( m_temperatureHigh - temperatureUsed ) + m_plHigh * ( temperatureUsed - m_temperatureLow  )) * m_temperatureDiff;
#endif


#if 1
   double vl = computeVL ( temperature );
   double pl = ( m_plLow * ( m_temperatureHigh - temperature ) + m_plHigh * ( temperature - m_temperatureLow  )) * m_temperatureDiff;

   adsorped = vl * pressure / ( pl + pressure );
#endif

   //
   // Possibility of optimisation here.
   // 
   // A = ( vlHhigh - vlLow ) / ( tempHigh - tempLow )
   //
   // B = ( vlLow * tempHigh - vlHigh * tempLow ) / ( tempHigh - tempLow )
   //
   // vl = A * temperature + B;
   //
   // Same for the pl.

#if 0
   double vl = m_aCoeffVl * temperature + m_bCoeffVl;
   double pl = m_aCoeffPl * temperature + m_bCoeffPl;

   adsorped = vl * pressure / ( pl + pressure );
#endif

#if 0
   double lowVol = m_vlLow * pressure / ( m_plLow + pressure );
   double highVol = m_vlHigh * pressure / ( m_plHigh + pressure );

   adsorped = ( lowVol * ( m_temperatureHigh - temperature ) + highVol * ( temperature - m_temperatureLow  )) * m_temperatureDiff;
#endif

   return adsorped;

}



double Genex6::LangmuirAdsorptionIsothermInterval::computeVL ( const double temperature ) const {

   double vl = ( m_vlLow * ( m_temperatureHigh - temperature ) + m_vlHigh * ( temperature - m_temperatureLow  )) * m_temperatureDiff;

   return vl;
}

double Genex6::LangmuirAdsorptionIsothermInterval::computePL ( const double temperature ) const {

   double pl = ( m_plLow * ( m_temperatureHigh - temperature ) + m_plHigh * ( temperature - m_temperatureLow  )) * m_temperatureDiff;

   return pl;
}


std::string Genex6::LangmuirAdsorptionIsothermInterval::image () const {

   std::stringstream buffer;

   buffer << std::endl;
   buffer << " Lower bound: " << m_lowerBound->getLangmuirTemperature () << "  "
          << m_lowerBound->getLangmuirPressure () << "  "
          << m_lowerBound->getLangmuirVolume () << "  "
          << m_temperatureLow << "  "
          << m_plLow << "  " 
          << m_vlLow << "  "
          << std::endl;
   buffer << " Upper bound: " << m_upperBound->getLangmuirTemperature () << "  " << m_upperBound->getLangmuirPressure () << "  "
          << m_upperBound->getLangmuirVolume () << "  "
          << m_temperatureHigh << "  "
          << m_plHigh << "  " 
          << m_vlHigh << "  "
          << std::endl;

   return buffer.str ();
}


bool Genex6::LangmuirAdsorptionIsothermInterval::isValid () const {
   return m_temperatureLow != DataAccess::Interface::DefaultUndefinedScalarValue and 
          m_vlLow != DataAccess::Interface::DefaultUndefinedScalarValue and
          m_plLow != DataAccess::Interface::DefaultUndefinedScalarValue and
          m_temperatureHigh != DataAccess::Interface::DefaultUndefinedScalarValue and
          m_vlHigh != DataAccess::Interface::DefaultUndefinedScalarValue and
          m_plHigh != DataAccess::Interface::DefaultUndefinedScalarValue;
}


std::string Genex6::LangmuirAdsorptionIsothermInterval::getErrorMessage () const {

   std::stringstream buffer;

   if ( m_temperatureLow == DataAccess::Interface::DefaultUndefinedScalarValue ) {
      buffer << " Interval lower bound temperature is invalid. " << std::endl;
   }

   if ( m_vlLow == DataAccess::Interface::DefaultUndefinedScalarValue ) {
      buffer << " Interval lower bound Langmuir volume is invalid. " << std::endl;
   }

   if ( m_plLow == DataAccess::Interface::DefaultUndefinedScalarValue ) {
      buffer << " Interval lower bound Langmuir pressure is invalid. " << std::endl;
   }

   if ( m_temperatureHigh == DataAccess::Interface::DefaultUndefinedScalarValue ) {
      buffer << " Interval upper bound temperature is invalid. " << std::endl;
   }

   if ( m_vlHigh == DataAccess::Interface::DefaultUndefinedScalarValue ) {
      buffer << " Interval upper bound Langmuir volume is invalid. " << std::endl;
   }

   if ( m_plHigh == DataAccess::Interface::DefaultUndefinedScalarValue ) {
      buffer << " Interval upper bound Langmuir pressure is invalid. " << std::endl;
   }

   return buffer.str ();
}
