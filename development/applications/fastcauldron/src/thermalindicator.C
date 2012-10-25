#include "thermalindicator.h"

#include <assert.h>
#include <math.h>

#include "globaldefs.h"
#include "utils.h"

#include "NumericFunctions.h"

#define AFTERLAST -2
#define BEFOREFIRST -1
#define TRUE 1
#define FALSE 0

/* Data used for the kinetics vitrinite calculations */
static int   naesSweeney   = 20;
static double arrSweeney    = 1.0e13;
static double feSweeney[] =
{ 
  0.03, 0.03, 0.04, 0.04, 0.05, 
  0.05, 0.06, 0.04, 0.04, 0.07, 
  0.06, 0.06, 0.06, 0.05, 0.05, 
  0.04, 0.03, 0.02, 0.02, 0.01 
};
static double enSweeney[]   = 
{ 
  34000.0, 36000.0, 38000.0, 40000.0, 42000.0, 
  44000.0, 46000.0, 48000.0, 50000.0, 52000.0, 
  54000.0, 56000.0, 58000.0, 60000.0, 62000.0, 
  64000.0, 66000.0, 68000.0, 70000.0, 72000.0 
};

static int   naesLarter = 13;
static double arrLarter  = 7.4e8;
static double feLarter[] = 
{
  0.0020, 0.0088, 0.0270, 0.0648, 0.1210,
  0.1760, 0.1995, 0.1760, 0.1210, 0.0648, 
  0.0270, 0.0088, 0.0020
};
static double enLarter[] = 
{     
  33119., 36675., 38230., 39786., 41342.,
  42898., 44454., 46010., 47566., 49122.,
  50678., 52233., 53789.
};

double Lopatin (double timeStep, double prevTemp, double currTemp, 
	       double  fFactor, double  refTemp, double  tempInc)
{
   double retval;
   double a, dTemp, htrate, rt1, rt2;
   a = log ((double)fFactor) / tempInc;
   
   dTemp = currTemp - prevTemp;
   htrate = dTemp / timeStep;
   
   /* ====  Reaction Rates at T1 and T2 */
   rt1 = exp ((double)a * (double)(prevTemp - refTemp));
   rt2 = exp ((double)a * (double)(currTemp - refTemp));
   
   /* ====  Determine increase in tti between t1 and t2 */
   if (htrate == 0.) 
      retval = (rt1 * timeStep);
   else 
      retval = ((rt2 - rt1) / (htrate * a));

//     if ( isnan (retval )) {
//       PetscEnd ();
//     }

   assert (/* finite (retval) && */ !isnan (retval));

//  #define PETSC_ASSERT(status) {if(!(status)) {PetscEnd (); assert(status);}}

//     PETSC_ASSERT (/* finite (retval) && */ !isnan (retval));
   return retval;
}


double  TtiToVr (double tau, double p, double q, double initialVr)
{
  double Vr, tti;
  double ttibeg = exp((log((double)initialVr) + (double)q) / (double)p);

  tti = ttibeg + tau;
  if (tti > .0)
  {
    Vr = exp ((double)p * log ((double)tti) - (double)q);
  }
  else
  { 
    Vr = initialVr;
  }

  PETSC_ASSERT (/* finite (Vr) && */ !isnan (Vr));
  return Vr;
}





void GetLarterData (int *naes, double *arr, double **fe, double **en)
{
  *naes = naesLarter;
  *arr  = arrLarter;
  *fe   = feLarter;
  *en   = enLarter;
}


void GetSweeneyData (int *naes, double *arr, double **fe, double **en)
{
  *naes = naesSweeney;
  *arr  = arrSweeney;
  *fe   = feSweeney;
  *en   = enSweeney;
}


double VreKinetic (double timeStep, double prevTemp, double currTemp,
			  double arr, double *fe, double *en, double *gint, 
			  int naes)
{
  int n;
  //double dtem;
  double delt, vmat;
  double ratio, a1, a2, b1, b2, t1, t2, ed, et;
  double ed1, et1;

  /* ==== Initialise variables */
  vmat = 0.;

  //dtem = currTemp - prevTemp;
  t1 = prevTemp + 273.15;
  t2 = currTemp + 273.15;
  /* ==== Constant temperature leg not allowed so check */

  if ( NumericFunctions::isEqual ( t1, t2, 0.001 )) {

     if ( t2 > t1 ) {
        t2 = t2 + 0.001;
     } else {
        t1 = t1 + 0.001;
     }

  }

  a1 = 2.334733;
  a2 = 0.250621;
  b1 = 3.330657;
  b2 = 1.681534;
  
  /* ==== Loop over the number of activation energies */
  for (n = 0; n < naes; ++n) 
  {
    et = en[n] / (t2 * 1.987);
    et1 = en[n] / (t1 * 1.987);
    ed = 1 - (et * et + a1 * et + a2) / (et * et + b1 * et +  b2);
    ed1 = 1 - (et1 * et1 + a1 * et1 + a2) / (et1 * et1 + b1 * et1 + b2);
    ratio = timeStep / (t2 - t1);
    delt = arr * 3.15576e13 * ratio;
    gint[n] += delt * t2 * ed * exp(-(double)et) -
      delt * t1 * ed1 * exp(-(double)et1);
    vmat += fe[n] * (1 - exp(-(double)gint[n]));
  }

  PETSC_ASSERT (/* finite (vmat) && */ !isnan (vmat));
  return vmat;
}

double VreLarter (double fConv)
{
  return (1.58 - (1. - fConv) * 1.13);
}

double VreSweeney (double fConv)
{
  return (double)(exp(log((double)(INITIALVRE)) + (double)(fConv * 3.7)));
}

