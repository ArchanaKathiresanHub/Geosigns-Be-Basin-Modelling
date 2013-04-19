#ifndef _GENEX5_LANGMUIR_ADSORPTION_H_
#define _GENEX5_LANGMUIR_ADSORPTION_H_

namespace Genex5 {

   class LangmuirAdsorption {

   public :

      LangmuirAdsorption ( const double lowTemperature,  const double vlLow,  const double plLow,
                           const double highTemperature, const double vlHigh, const double plHigh );

      ~LangmuirAdsorption () {}

      double operator ()( const double pressure,
                          const double temperature ) const;

   private :

      const double m_temperatureLow;
      const double m_vlLow;
      const double m_plLow;

      const double m_temperatureHigh;
      const double m_vlHigh;
      const double m_plHigh;

      const double m_temperatureDiff;

   };

//------------------------------------------------------------//

Genex5::LangmuirAdsorption::LangmuirAdsorption ( const double lowTemperature,  const double vlLow,  const double plLow,
                                                 const double highTemperature, const double vlHigh, const double plHigh ) :
   m_temperatureLow ( lowTemperature ),
   m_vlLow ( vlLow ),
   m_plLow ( plLow ),
   m_temperatureHigh ( highTemperature ),
   m_vlHigh ( vlHigh ),
   m_plHigh ( plHigh )
   m_temperatureDiff ( 1.0 / ( highTemperature - lowTemperature ))
{
}

//------------------------------------------------------------//

inline double Genex5::LangmuirAdsorption::operator ()( const double pressure,
                                                       const double temperature ) const {

   //
   // Possibility of optimisation here.
   // 
   // A = ( vlHhigh - vlLow ) / ( tempHigh - tempLow )
   //
   // B = ( vlLow * tempHigh - vlHigh * tempLow ) / ( tempHigh - tempLow )
   //
   // vl = A * temperature + B;
   //
   double vl = ( m_vlLow * ( m_temperatureHigh - temperature ) + m_vlhigh * ( temperature - m_temperatureLow  )) * m_temperatureDiff;
   double pl = ( m_plLow * ( m_temperatureHigh - temperature ) + m_plhigh * ( temperature - m_temperatureLow  )) * m_temperatureDiff;

   return vl * pressure / ( pl + pressure );
}


#endif // _GENEX5_LANGMUIR_ADSORPTION_H_
