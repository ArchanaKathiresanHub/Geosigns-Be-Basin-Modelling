#include "IrreducibleWaterSaturationFunction.h"

Genex6::IrreducibleWaterSaturationFunction::IrreducibleWaterSaturationFunction (DataAccess::Interface::ProjectHandle& projectHandle ) {

   const DataAccess::Interface::IrreducibleWaterSaturationSample* iws = projectHandle.getIrreducibleWaterSaturationSample ();

   if ( iws != 0 ) {
      m_coeffA = iws->getCoefficientA ();
      m_coeffB = iws->getCoefficientB ();
   } else {
      m_coeffA = -0.187;
      m_coeffB = 1.18;
   }

}
