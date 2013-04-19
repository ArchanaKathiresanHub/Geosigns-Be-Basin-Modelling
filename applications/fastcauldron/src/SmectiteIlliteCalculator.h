#ifndef SMECTITEILLITE_CALCULATOR_H_
#define SMECTITEILLITE_CALCULATOR_H_

//#include <string>
#include "globaldefs.h"

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
