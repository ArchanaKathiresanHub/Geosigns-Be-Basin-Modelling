#include "Interface/IrreducibleWaterSaturationSample.h"
#include "cauldronschemafuncs.h"

#include "database.h"

DataAccess::Interface::IrreducibleWaterSaturationSample::IrreducibleWaterSaturationSample ( ProjectHandle*    projectHandle,
                                                                                                 database::Record* record ) : DAObject ( projectHandle, record ) {

   m_coeffA = database::getCoefficientA ( record );
   m_coeffB = database::getCoefficientB ( record );
}


DataAccess::Interface::IrreducibleWaterSaturationSample::~IrreducibleWaterSaturationSample ()
{
}

double DataAccess::Interface::IrreducibleWaterSaturationSample::getCoefficientA () const {
   return m_coeffA;
}

double DataAccess::Interface::IrreducibleWaterSaturationSample::getCoefficientB () const {
   return m_coeffB;
}
