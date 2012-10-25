#ifndef BIOMARKERS_CALCULATOR_H_
#define BIOMARKERS_CALCULATOR_H_

//#include <string>

class AppCtx;
class PETSC_2D_Array;

#include "globaldefs.h"

namespace Calibration
{
	class BiomarkersSimulator;
}
class BiomarkersCalculator
{
 public:
  BiomarkersCalculator( AppCtx* Application_Context );
  ~BiomarkersCalculator();

  bool computeSnapShotBiomarkers( const double time, const Boolean2DArray& validNeedleSet );

  bool computeBiomarkersIncrement( double time, double timeStep );
 
 private:

  AppCtx*     m_Basin_Model;
  Calibration::BiomarkersSimulator *m_BiomarkersSimulator;

};

#endif 
