//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "OutputUnitTest.h"
#include "ComponentManager.h"

// utilities library
#include "ConstantsNumerical.h"
#include "NumericFunctions.h"
using Utilities::Numerical::CauldronNoDataValue;

namespace Genex6
{
OutputUnitTest::OutputUnitTest(const double in_currentTime, const double in_temperature, 
                             const double in_pressure, const double thicknessScaleFactor) : Input( in_currentTime, in_temperature, in_pressure, thicknessScaleFactor )
{
   m_solidVol = CauldronNoDataValue;
   m_poreVol = CauldronNoDataValue;
   m_poreVolp = CauldronNoDataValue;
   m_nPores = CauldronNoDataValue;
   m_organoPorosity = CauldronNoDataValue;
   m_molesGas = CauldronNoDataValue;
   m_molarVolGas = CauldronNoDataValue;
   m_PRa = CauldronNoDataValue;
   m_gasPressurePR = CauldronNoDataValue;
   m_gasPressureRealGas = CauldronNoDataValue;
   m_gasPressureVDW = CauldronNoDataValue;
   m_phalfLangmuir = CauldronNoDataValue;
   m_phalfPrangmuir = CauldronNoDataValue;
   m_pressurePrangmuir = CauldronNoDataValue;
   m_pressureLangmuir = CauldronNoDataValue;
   m_pressureJGS = CauldronNoDataValue;
   m_pressureVirial = CauldronNoDataValue;

}
void OutputUnitTest::PrintAdsorptionState( ofstream &outputfile) const {

   outputfile << scientific <<  m_carrierBedPorosity << "," <<
      m_carrierBedPermeability << "," <<
      m_solidVol << "," <<
      m_poreVol << "," <<
      m_poreVolp << "," <<
      m_organoPorosity << "," <<
      m_molesGas << "," <<
      m_molarVolGas << "," <<
      m_PRa << "," <<
      m_gasPressurePR << "," <<
      m_gasPressureRealGas << "," <<
      m_gasPressureVDW << "," <<
      m_phalfLangmuir << "," <<
      m_phalfPrangmuir << "," <<
      m_pressurePrangmuir << "," <<
      m_pressureLangmuir << "," <<
      m_pressureJGS << "," <<
      m_pressureVirial;
}

    
}
