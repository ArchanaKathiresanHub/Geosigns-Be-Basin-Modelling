//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "HCDensity.h"

// std library
#include <cmath>
#include <assert.h>
#include <string.h>
#include <sstream>

// utilities library
#include "NumericFunctions.h"

// Eospack library
#include "EosPack.h"
#include "EosApplication.h"
#include "EosPvtTable.h"

// CBMGenerics library
#include "ComponentManager.h"
typedef CBMGenerics::ComponentManager::SpeciesNamesId ComponentId;

#define OIL 1
#define GAS 0

GeoPhysics::HCDensity::HCDensity() {

  m_isComputed = false;
  
  m_startTempValue = 273.0; //K
  m_startPressValue = 1e5;  //Pa
  m_stepTempValue = 10.0;
  m_stepPressValue = 1e6; 
}

GeoPhysics::HCDensity::~HCDensity() {
}

double GeoPhysics::HCDensity::densityFromTable ( unsigned int phaseId, 
                                                 const double temperature, 
                                                 const double pressure ) const {

  double densityValue = -1.0;
  if( phaseId == GAS )      densityValue = m_vapourDensityTbl.compute (  pressure, temperature );
  else if( phaseId == OIL ) densityValue = m_liquidDensityTbl.compute (  pressure, temperature );
  
  return densityValue;
}

void GeoPhysics::HCDensity::createPropertyTables ()
{
   if (m_isComputed)
      return;

   cout << "Start create hydrocarbons density tables" << endl;

   double pressure = m_startPressValue; //[Pa]               
   double temperature = m_startTempValue; //[K]=[Celsius+273.15]

   bool isGormPrescribed = false;
   double compMasses[ComponentId::NUMBER_OF_SPECIES];

   double phaseCompMasses[pvtFlash::numberOfPhases][ComponentId::NUMBER_OF_SPECIES];
   double phaseDensity[pvtFlash::numberOfPhases];
   double phaseViscosity[pvtFlash::numberOfPhases];

   memset (compMasses, 0, sizeof (double) * ComponentId::NUMBER_OF_SPECIES);
   memset (phaseCompMasses, 0, sizeof (double) * ComponentId::NUMBER_OF_SPECIES * pvtFlash::numberOfPhases);
   memset (phaseDensity, 0, sizeof (double) * pvtFlash::numberOfPhases);
   memset (phaseViscosity, 0, sizeof (double) * pvtFlash::numberOfPhases);

   pvtFlash::EosPack & eosPack = pvtFlash::EosPack::getInstance ();

   cout << "Comp.\t T\t P\t D[G]\t D[O]" << endl;
   for (int j = 0; j < TEMP_NUM; ++j)
   {
      pressure = m_startPressValue;
      for (int k = 0; k < PRESSURE_NUM; ++k)
      {
         compMasses[ComponentId::C1] = 1.0;
         compMasses[ComponentId::C6_MINUS_14SAT] = 0.0;
         eosPack.computeWithLumping (temperature,
                                     pressure, compMasses, phaseCompMasses, phaseDensity, phaseViscosity, isGormPrescribed, 0.0);

         cout << "C1\t " << temperature << "\t " << pressure << "\t " << phaseDensity[GAS] << "\t " << phaseDensity[OIL] << endl;

         if (phaseDensity[GAS] != EOS_DEFAULT_DENSITY)
            m_vapourDensityTbl.addPoint (pressure, temperature, phaseDensity[GAS]);
         else
            m_vapourDensityTbl.addPoint (pressure, temperature, phaseDensity[OIL]);


         compMasses[ComponentId::C1] = 0.0;
         compMasses[ComponentId::C6_MINUS_14SAT] = 1.0;
         eosPack.computeWithLumping (temperature,
                                     pressure, compMasses, phaseCompMasses, phaseDensity, phaseViscosity, isGormPrescribed, 0.0);

         cout << "C6\t " << temperature << "\t " << pressure << "\t " << phaseDensity[GAS] << "\t " << phaseDensity[OIL] << endl;

         if (phaseDensity[OIL] != EOS_DEFAULT_DENSITY)
            m_liquidDensityTbl.addPoint (pressure, temperature, phaseDensity[OIL]);
         else
            m_liquidDensityTbl.addPoint (pressure, temperature, phaseDensity[GAS]);

         pressure += m_stepPressValue;
      }
      temperature += m_stepTempValue;
   }
   cout << "Create hydrocarbons density tables - done" << endl;
#if 1
   cerr << "Done HCDensity::createPropertyTables( T=" << m_startTempValue <<
         " P=" << m_startPressValue << "/ T=" << temperature - m_stepTempValue <<
         " P=" << pressure - m_stepPressValue << " )" << endl;
#endif
   m_isComputed = true;
}
