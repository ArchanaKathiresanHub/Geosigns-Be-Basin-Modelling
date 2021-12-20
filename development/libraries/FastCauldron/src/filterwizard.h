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
  bool IsBulkThCondCalculationNeeded();
  bool IsDiffusivityCalculationNeeded();
  bool IsHeatFlow_CalculationNeeded();
  bool InitDerivedCalculationsNeeded();
  bool thicknessCalculationNeeded () const;
};

#endif /* _FILTERWIZARD_H_ */
