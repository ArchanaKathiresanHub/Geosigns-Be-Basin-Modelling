#ifndef _THERMALINDICATOR_H_
#define _THERMALINDICATOR_H_

double Lopatin (double timeStep, double prevTemp, double currTemp, 
	       double fFactor, double refTemp, double tempInc);


double TtiToVr (double tti, double p, double q, double initialVr);

void   GetSweeneyData (int *naes, double *arr, double **fe, double **en);
void   GetLarterData (int *naes, double *arr, double **fe, double **en);


double VreKinetic (double timeStep, double prevTemp, double currTemp,
		  double arr, double *fe, double *en, double *gint, 
		  int naes);

double VreLarter (double fConv);

double VreSweeney (double fConv);

#endif /* _THERMALINDICATOR_H_ */


