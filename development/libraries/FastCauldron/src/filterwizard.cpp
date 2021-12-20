//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include   "filterwizard.h"

#include <string>
using std::string;

Filterwizard::Filterwizard(TimeFilter* tf){
  
  SetDerivedProperties = false;
   
  bBulkDensities = false;
  bVelocity      = false;
  bSonic         = false;
  bReflectivity  = false;
  bIsoStaticMass = false;

  bIsoStaticDeflection = false;
  bIsoStaticWaterBottom = false;

  timefilter = tf;

}

Filterwizard::~Filterwizard(){};

bool Filterwizard::IsBulkThCondCalculationNeeded()
{

   bool bCalculationNeeded = false;

   bCalculationNeeded =
     ( timefilter->PropertyOutputOption[THCONDVEC] ||
       IsDiffusivityCalculationNeeded() ||
       IsHeatFlow_CalculationNeeded() );

   return (bCalculationNeeded);

}

bool Filterwizard::IsDiffusivityCalculationNeeded()
{

   bool bCalculationNeeded = false;

   bCalculationNeeded = timefilter->PropertyOutputOption[DIFFUSIVITYVEC];

   return (bCalculationNeeded);

}

bool Filterwizard::IsHeatFlow_CalculationNeeded()
{

   bool bCalculationNeeded = false;

   bCalculationNeeded = timefilter->PropertyOutputOption[HEAT_FLOW_];

   return (bCalculationNeeded);

}

bool Filterwizard::InitDerivedCalculationsNeeded()
{

  bool bCalculationNeeded = false;
  
  if (timefilter->PropertyOutputOption[REFLECTIVITYVEC])
    {
      bReflectivity = true;
      bVelocity = true;
      bBulkDensities = true;
      bCalculationNeeded = true;
    }
  
  if (timefilter->PropertyOutputOption[SONICVEC])
    {
      bSonic = true;
      bVelocity = true;
      bBulkDensities = true;
      bCalculationNeeded = true;
    }
  
  if (timefilter->PropertyOutputOption[VELOCITYVEC])
    {
      bVelocity = true;
      bBulkDensities = true;
      bCalculationNeeded = true;
    }
  
  if (timefilter->PropertyOutputOption[BULKDENSITYVEC])
    {
      bBulkDensities = true;
      bCalculationNeeded = true;
    }
  
  if (timefilter->PropertyOutputOption[ISOSTATICMASS])
    {
      bIsoStaticMass = true;
      bBulkDensities = true;
      bCalculationNeeded = true;
    }
  if (timefilter->PropertyOutputOption[ISOSTATICDEFLECTION])
    {
      bIsoStaticDeflection = true;
      bIsoStaticMass = true;
      bBulkDensities = true;
      bCalculationNeeded = true;
    }
  if (timefilter->PropertyOutputOption[ISOSTATICWB])
    {
      bIsoStaticWaterBottom = true;
      bIsoStaticDeflection = true;
      bIsoStaticMass = true;
      bBulkDensities = true;
      bCalculationNeeded = true;
    }

  if (bCalculationNeeded) SetDerivedProperties = true;

  return (bCalculationNeeded);
  
}

bool Filterwizard::thicknessCalculationNeeded () const {
  return timefilter->PropertyOutputOption[THICKNESS] > NOOUTPUT;
}
