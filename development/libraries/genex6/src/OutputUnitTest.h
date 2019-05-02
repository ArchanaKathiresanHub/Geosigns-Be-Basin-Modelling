//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#ifndef GENEX6__OUTPUT_UNIT_TEST_H
#define GENEX6__OUTPUT_UNIT_TEST_H

// std library
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

// genex6 library
#include "ConstantsGenex.h"
#include "Input.h"

// utilities library
#include "ConstantsMathematics.h"
using Utilities::Maths::CelciusToKelvin;

namespace Genex6
{
   class OutputUnitTest: public Input
{
   friend class SimulatorStateAdsorption;

public:
   OutputUnitTest(const double in_currentTime, const double in_temperature, 
         const double in_pressure, const double thicknessScaleFactor = 1.0);

   virtual ~OutputUnitTest(){}

   void   PrintAdsorptionState( ofstream &outputfile) const ;
private:
   // Genex7
   double m_solidVol;
   double m_poreVol;
   double m_poreVolp;
   double m_nPores;
   double m_organoPorosity;
   double m_molesGas;
   double m_molarVolGas;
   double m_PRa;
   double m_gasPressurePR;
   double m_gasPressureRealGas;
   double m_gasPressureVDW;
   double m_phalfLangmuir;
   double m_phalfPrangmuir;
   double m_pressurePrangmuir;
   double m_pressureLangmuir;
   double m_pressureJGS;
   double m_pressureVirial;
};


}
#endif // GENEX6__OUTPUT_UNIT_TEST_H
