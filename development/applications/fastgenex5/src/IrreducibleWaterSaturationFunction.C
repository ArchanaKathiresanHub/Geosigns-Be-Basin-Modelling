#include "IrreducibleWaterSaturationFunction.h"

IrreducibleWaterSaturationFunction::IrreducibleWaterSaturationFunction ( AdsorptionProjectHandle* adsorptionProjectHandle ) :
   m_adsorptionProjectHandle ( adsorptionProjectHandle )
{

   const IrreducibleWaterSaturationSample* iws = adsorptionProjectHandle->getIrreducibleWaterSaturationSample ();

   if ( iws != 0 ) {
      m_coeffA = iws->getCoefficientA ();
      m_coeffB = iws->getCoefficientB ();
   } else {
      m_coeffA = -0.187;
      m_coeffB = 1.18;
   }

}
