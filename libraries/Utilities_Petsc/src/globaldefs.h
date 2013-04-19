#ifndef _GLOBALDEFS_H_
#define _GLOBALDEFS_H_

#include <string>
using std::string;

#include <assert.h>

const bool   NO_ERROR              = true;

const string BLANKQUOTE            = "";

const int    MAXLINESIZE            = 512;
const int    Null                   = 0;

const double IBSNULLVALUE           = -9999;
const double CAULDRONIBSNULLVALUE   = 99999;
const double GRAVITY                = 9.81;
const double W_IN_mW                = 0.000001;    /* Number of Watts in a microWatt */
const double W_IN_MW                = 0.001;       /* Number of Watts in a milliWatt */
const double Pa_To_MPa              = 0.000001;    /* Number of Pascal in a MPascal */
const double Secs_IN_MA             = 3.15576E13;  /* Number of Seconds in a Million Year */
const double MILLIDARCYTOM2         = 0.9869233E-15; /* mDarcy to meter square conversion */

const double PRESSURE_AT_SEA_TOP    = 0.1;    /* value in MPa */
const double DENSITY_FOR_STANDARD_WATER = 1000.0;

const double LOPATINREFTEMP         = 105.0;
const double LOPATINTEMPINC         = 10.0;
const double LOPATINFACTOR          = 2.0;
const double INITIALVRE             = 0.2;
const double MODIFIEDLOPATINFACTOR  = 5.1;
const double MODIFIEDLOPATINP       = 0.0821;
const double MODIFIEDLOPATINQ       = 0.6250;
const double LOPATINGOFFP           = 0.20799;
const double LOPATINGOFFQ           = 1.08680;
const double LOPATINHOODP           = 0.17340;
const double LOPATINHOODQ           = 0.98875;
const double LOPATINISSLERP         = 0.16171;
const double LOPATINISSLERQ         = 0.98882;
const double LOPATINROYDENP         = 0.17300;
const double LOPATINROYDENQ         = 0.80360;
const double LOPATINWAPPLESP        = 0.24300;
const double LOPATINWAPPLESQ        = 1.01772;
const double LOPATINDYKSTRAP        = 0.23697;
const double LOPATINDYKSTRAQ        = 1.04431;

const string VRALGBURNHAMSWEENEY    = "Burnham & Sweeney";
const string VRALGLOPATINMODIFIED   = "Modified Lopatin";
const string VRALGLARTER            = "Larter";
const string VRALGLOPATINDYKSTRA    = "Lopatin & Dykstra";
const string VRALGLOPATINWAPPLES    = "Lopatin & Wapples";
const string VRALGLOPATINROYDEN     = "Lopatin & Royden";
const string VRALGLOPATINISSLER     = "Lopatin & Issler";
const string VRALGLOPATINHOOD       = "Lopatin & Hood";
const string VRALGLOPATINGOFF       = "Lopatin & Goff";

const double Zero                   = 0.0;
const double NegOne                 = -1.0;
const double One                    = 1.0;
const double Hundred                = 100.0;
const double EPS1                   = 0.1;
const double TempNLSolverTolerance  = 1.0E-06;
const double TempNLSolverMaxIter    = 20;
const double PressNLSolverTolerance = 1.0E-06;
const double Log_10                 = 2.3025850929940459;
const double Present_Day            = 0.0;

#define IBSASSERT(t) assert(t);
#define PETSC_ASSERT(status) {if(!status)PetscFinalize(); assert(status);}

typedef enum {PIECEWISE_LINEAR, CUBIC_SPLINE} Interpolation_Method;
typedef enum {EXPONENTIAL, SOIL_MECHANICS} PorosityModelType;
typedef enum {CONST = 1, CALC, TABLE, UNSPECIFIED} ModelType;
typedef enum {HEATFLOW , FIXED_TEMPERATURE} BottomBoundaryType;
typedef enum {SECONDS, MINUTES, HOURS} Time;
enum Surface{TOPSURFACE, BOTTOMSURFACE};
enum OutputFormat{MAP,VOLUME,MAPandVOLUME};
enum Resolution{HIGHRES,LOWRES};
enum ModelArea{ON,OFF};

#endif // GLOBALDEFS
