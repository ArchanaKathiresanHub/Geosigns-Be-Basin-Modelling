#include "LangmuirAdsorptionIsotherm.h"

LangmuirAdsorptionIsotherm::LangmuirAdsorptionIsotherm ( const LangmuirAdsorptionIsothermSample& sample ) :
   m_sample ( sample ),
   m_temperature ( sample.getLangmuirTemperature ()),
   m_pressure ( sample.getLangmuirPressure ()),
   m_volume ( sample.getLangmuirVolume ())
{}

double LangmuirAdsorptionIsotherm::compute ( const double temperature, 
                                             const double pressure ) const {
   return 1.0;
}
