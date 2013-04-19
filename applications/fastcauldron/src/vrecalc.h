#ifndef _VRECALC_H_
#define _VRECALC_H_

#include <string>

#include "propinterface.h"

using namespace std;

enum VreAlgorithm{LOPATINMETHOD=1, LARTERMETHOD, SWEENEYMETHOD};

class VreCalc
{
 public:
  VreCalc( const AppCtx* Application_Context );
  ~VreCalc();

  bool CalcSnaptimeVr( const double    time,
		       const Boolean2DArray& dataPresent );

  bool CalcStep( double time, double timeStep );
  

  void initialiseVectors ();

 private:

  VreAlgorithm          ALGORITHMTYPE;
  double                FFACTOR;
  double                REFTEMP;
  double                TEMPINC;
  double                P;
  double                Q;
  int                   NAES;
  double                ARR;
  double*               FE;
  double*               EN;

  AppCtx*     Basin_Model;
  bool InitializeGINT ();
  bool SetAlgorithm( const string& algorithmName );

};

#endif /* _VRECALC_H_ */
