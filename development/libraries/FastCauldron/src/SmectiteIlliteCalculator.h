//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef SMECTITEILLITE_CALCULATOR_H_
#define SMECTITEILLITE_CALCULATOR_H_

#include "ConstantsFastcauldron.h"

class AppCtx;
class PETSC_2D_Array;

namespace Calibration
{
	class SmectiteIlliteSimulator;
	class BiomarkersSimulator;
}
class SmectiteIlliteCalculator
{
 public:
  SmectiteIlliteCalculator( AppCtx* Application_Context );
  ~SmectiteIlliteCalculator();

  bool computeSnapShotSmectiteIllite( const double time, const Boolean2DArray& validNeedleSet );

  bool computeSmectiteIlliteIncrement( double time, double timeStep );
 
 private:

  AppCtx*     m_Basin_Model;
  Calibration::SmectiteIlliteSimulator *m_SmectiteIlliteSimulator;
};

#endif 
