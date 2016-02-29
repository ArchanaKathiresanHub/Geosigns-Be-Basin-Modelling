//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _FILTERWIZARD_H_
#define _FILTERWIZARD_H_

#include "timefilter.h"

class Filterwizard
{
public:
  Filterwizard(TimeFilter* tf);
  ~Filterwizard();

  TimeFilter* timefilter;
  
  bool    SetDerivedProperties;
  bool    bBulkDensities;
  bool    bVelocity;
  bool    bSonic;
  bool    bReflectivity;
  bool    bIsoStaticMass;
  bool    bIsoStaticDeflection;
  bool    bIsoStaticWaterBottom;

// member functions
private:
public:
  bool IsBiomarkersCalculationNeeded();
  bool IsBulkThCondCalculationNeeded();
  bool IsBulkPermeabilityCalculationNeeded();
  bool IsDiffusivityCalculationNeeded();
  bool IsFissionTrackCalculationNeeded();
  bool IsHeatFlowCalculationNeeded();
  bool IsHeatFlow_CalculationNeeded();
  bool IsMassFluxCalculationNeeded();
  bool IsMassFlux_CalculationNeeded();
  bool IsPressureCalculationNeeded();
  bool IsSmectiteIlliteCalculationNeeded();
  bool IsTemperatureCalculationNeeded();
  bool IsVreCalculationNeeded();
  bool IsXmigCalculationNeeded();
  bool InitDerivedCalculationsNeeded();
  bool fluidVelocityIsNeeded () const;
  bool thicknessCalculationNeeded () const;

};

#endif /* _FILTERWIZARD_H_ */
