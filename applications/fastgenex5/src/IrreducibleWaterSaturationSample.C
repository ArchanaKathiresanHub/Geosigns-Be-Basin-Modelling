#include "IrreducibleWaterSaturationSample.h"
#include "cauldronschemafuncs.h"

IrreducibleWaterSaturationSample::IrreducibleWaterSaturationSample ( AdsorptionProjectHandle* adsorptionProjectHandle,
                                                                     database::Record*        record ) {

   m_coeffA = database::getCoefficientA ( record );
   m_coeffB = database::getCoefficientB ( record );
}

double IrreducibleWaterSaturationSample::getCoefficientA () const {
   return m_coeffA;
}

double IrreducibleWaterSaturationSample::getCoefficientB () const {
   return m_coeffB;
}
