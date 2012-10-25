/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxkernel.h,v 25.0 2005/07/05 08:03:46 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxkernel.h,v $
 * Revision 25.0  2005/07/05 08:03:46  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:47:55  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:27  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:10  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:42  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:22  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:21  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:33  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:42  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:10  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:00  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:12  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:41  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:16  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:02:23  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:41  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:42:05  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.3  1996/01/04  15:17:40  ibs
 * cplusplus corrections added.
 *
 * Revision 9.2  1995/10/30  09:54:14  ibs
 * Add headers.
 * */
/* ================================================================== */
#ifndef   GXKERNEL_H
#define   GXKERNEL_H

#ifdef __cplusplus
extern "C" {
#endif
    

/*
--------------------------------------------------------------------------------
 gxhernel.h   include file for Genex kernel functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        19-JUN-1992

 Description: gxkernel.h contains prototype, type and macro definitions for
              the high level Genex functions like open and close.

 History
 -------
 19-JUN-1992  P. Alphenaar  initial version
 29-JUL-1992  M.J.A. van Houtert  GxGetGeologicalConversions added
 29-JUL-1992  M.J.A. van Houtert  GxLaboratoryConversions added
 30-JUL-1992  M.J.A. van Houtert  GxGetGeologicalVanKrevelen added
 30-JUL-1992  M.J.A. van Houtert  GxGetLaboratoryVanKrevelen added
  4-AUG-1992  M.J.A. van Houtert  GxGetGeologicalAPI added
  6-AUG-1992  M.J.A. van Houtert  GxGetGeologicalGOR added
 11-Aug-1992  M.J.A. van Houtert  GxGetGeologicalMasses added
 21-AUG-1992  M.J.A. van Houtert  GxGetSubBurialTimeRange
 19-MAY-1993  M.J.A. van Houtert  GxIsSpeciesOilGas
 23-MAY-1993  M.J.A. van Houtert  GxGetGeologicalGenCumRate added
 23-MAY-1993  M.J.A. van Houtert  GxGetGeologicalGenRate added
 23-MAY-1993  M.J.A. van Houtert  GxGetLaboratoryGenCumRate added
 23-MAY-1993  M.J.A. van Houtert  GxGetLaboratoryGenRate added
 27-AUG-1993  M.J.A. van Houtert  GxFreeGeneration added
 27-AUG-1993  M.J.A. van Houtert  GxGetGeneration added
 22-DEC-1993  Th. Berkers         GxIsSpeciesMobile, GxIsSpeciesHCGas and 
                                  GxIsSpeciesOil added due to integration of
                                  Nitrogen model
                                  GxNitrogen part of model added in order to 
                                  get an integrated GENEX for both a model with
                                  and without the nitrogen element
 10-JAN-1994  Th. Berkers         GxGetLaboratoryRatios added
 20-SEP-1994  Th. Berkers         GxExpulsion and GxGenRate structure extended
                                  with mass array for species and total mass
 17-NOV-1994  Th. Berkers         GxGetGeologicalAPISelected and 
                                  GxGetGeologicalGORSelected added
 23-NOV-1994  Th. Berkers         GxGetGenerationSelected, 
                                  GxGetDownwardExpulsionSelected, 
                                  GxGetMainExpulsionSelected and 
                                  GxGetUpwardExpulsionSelected added
 14-DEC-1994  Th. Berkers         GxIsSpeciesHCWetGas added
 02-MAR-1995  Th. Berkers         Integration of Isotopes Fractionation
--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  "gxgenerl.h"
#include  "gxmodel.h"
#include  "list.h"
#include  "sortlst.h"

/* Macro definitions */
/* ----------------- */
/* -- error definitions -- */
enum {
  GXE_CANNOTOPTIMIZE = GXBE_KERNEL | GXF_APPLICATION | GXF_ERROR   | 1,
  GXE_NOCONVERGANCE ,
  GXE_MAXITERREACHED = GXBE_KERNEL | GXF_APPLICATION | GXF_ERROR   | 11,
  GXE_NOGEOMODEL, GXE_NOHORIZONS,  GXE_NOLABMODEL,   GXE_NOLITHOLOGIES,
  GXE_NOSUBAREAS, GXE_NOTASPECIES, GXE_NOTASUBMODEL, GXE_NOTPHISTDATA,
  GXE_NOTRAP,     GXE_NOTRAPHIST,  GXE_NOTROCKEVAL2
};

/* Type definitions */
/* ---------------- */
/* -- structure definitions -- */
typedef  struct GxExpulsion  GxExpulsion;
struct GxExpulsion {
   int     nspecies;
   char    **speciesname;
   double  *volume1, *volume2, *weightpct, *mass1, *mass2, total1, total2, 
           totalmass1, totalmass2;
};

typedef struct GxGenRate GxGenRate;
struct GxGenRate {
    int    nspecies;
    char   **speciesname;
    double *volume1, *volume2, *weightpct, *mass1, *mass2, total1, total2, 
           totalmass1, totalmass2;
};

typedef struct GxGasMolarogram GxGasMolarogram;
struct GxGasMolarogram {
    int      nspecies;
    char   **speciesname;
    double  *xdata;
    double  *ydata;
    GxBool  *valid;
};

typedef struct GxIsotope GxIsotope;
struct GxIsotope {
    int     nspecies;
    char  **speciesname;
    double *isovalue;
    GxBool *valid;
};

/* Function prototypes */
/* ------------------- */
GxErrorCode  GxCalculateCharge (double, GxCharge *, GxCharge *);
GxErrorCode  GxClose (void);
GxErrorCode  GxCreateModelHistory (int, double **);
GxErrorCode  GxFreeExpulsion (GxExpulsion *, GxExpulsion *, GxExpulsion *);
GxErrorCode  GxFreeGasMolarogram (GxGasMolarogram *);
GxErrorCode  GxFreeGeneration (GxGenRate *, GxGenRate *, GxGenRate *);
GxErrorCode  GxFreeIsotope (GxIsotope *, GxIsotope *, GxIsotope *);
GxErrorCode  GxFreeModelHistory (double *);
GxBool       GxGeologicalModelPresent (void);
char        *GxGetCnSpeciesName (int);
double       GxGetDefIsoFractFactor (void);
GxErrorCode  GxGetDownwardExpulsion (double, GxExpulsion **, GxExpulsion **,
                                     GxExpulsion **, double *, double *,
                                     double *);
GxErrorCode  GxGetDownwardExpulsionSelected (SortedList, SortedList, 
                                             SortedList, double, 
                                             GxExpulsion **, GxExpulsion **, 
                                             GxExpulsion **, double *, 
                                             double *, double *);
GxErrorCode  GxGetDownwardIsotopeSelected (SortedList, SortedList, SortedList, 
                                           double, GxIsotope **, GxIsotope **, 
                                           GxIsotope **);
GxErrorCode  GxGetGasMolarogramSelected (SortedList, SortedList, SortedList,
                                         double, GxUpDown, GxGasMolarogram **); 
GxErrorCode  GxGetGeneration(double, GxGenRate **, GxGenRate **, GxGenRate **);
GxErrorCode  GxGetGeologicalAPI (double, double, double *);
GxErrorCode  GxGetGeologicalAPISelected (SortedList, SortedList, SortedList,
                                         double, double, double *);
GxErrorCode  GxGetGeologicalConcentrations (char *, char *, char *, char *,
                                            double, double, double *);
GxErrorCode  GxGetGeologicalConversions (char *, char *, char *, double,
                                         double, double *, double * );
GxErrorCode  GxGetGeologicalExpulsions (char *, char *, char *, char *,
                                        GxUpDown, GxCumulative, double,
                                        double, double *);
GxErrorCode  GxGetGeologicalExpIsoHist (char *, char *, char *, char *, 
                                        GxUpDown, double, double, double *);
GxErrorCode  GxGetGeologicalExpVolume (char *, char *, char *, char *,
                                       GxUpDown, GxCumulative, double,
                                       double, double *);
GxErrorCode  GxGetGeologicalFluxes (char *, char *, char *, char *, GxUpDown,
                                    double, double, double *);
GxErrorCode  GxGetGeologicalGenCumRate  (char *, char *, char *, char *, double,
                                         double, double *);
GxErrorCode  GxGetGeologicalGenRate (char *, char *, char *, char *, double,
                                     double, double *);
GxErrorCode  GxGetGeologicalGOR (double, double, double *, double *);
GxErrorCode  GxGetGeologicalGORSelected (SortedList, SortedList, SortedList,
                                         double, double, double *, double *);
GxErrorCode  GxGetGeologicalMasses (char *, char *, char *, char *, double,
					double, double *);

GxErrorCode  GxGetGeologicalPressures (char *, char *, char *, double,
                                       double, double *);
GxErrorCode  GxGetGeologicalRetIsoHist (char *, char *, char *, char *, double,
                                        double, double *);
GxErrorCode  GxGetGeologicalTemperatures (char *, char *, char *, double,
                                          double, double *);
GxErrorCode  GxGetGeologicalTimeRange (double *, double *);
GxErrorCode  GxGetGeologicalTimes (double, double, double *);
GxErrorCode  GxGetGeologicalTimesteps (double, double, int *);
GxErrorCode  GxGetGeologicalVanKrevelen (char *, char *, char *, double, double,
                                         double *, double * );
GxErrorCode  GxGetGeologicalVRE (char *, char *, char *, double, double,
                                 double *);
GxErrorCode  GxGetLaboratoryConcentrations (char *, double, double,
                                            double *);
GxErrorCode  GxGetLaboratoryConversions ( double, double, double *, double * );
GxErrorCode  GxGetLaboratoryExpulsions (char *, GxUpDown, GxCumulative,
                                        double, double, double *);
GxErrorCode  GxGetLaboratoryExpIsoHist (char *, GxUpDown, double, double, 
                                        double *);
GxErrorCode  GxGetLaboratoryExpVolume (char *, GxUpDown, GxCumulative, double,
                                       double, double *);
GxErrorCode  GxGetLaboratoryFluxes (char *, GxUpDown, double, double,
                                    double *);
GxErrorCode  GxGetLaboratoryGenCumRate  (char *, double, double, double *);
GxErrorCode  GxGetLaboratoryGenRate (char *, double, double, double *);
GxErrorCode  GxGetLaboratoryPressures (double, double, double *);
GxErrorCode  GxGetLaboratoryRatios (double, double, double *, double *, 
                                    double *);
GxErrorCode  GxGetLaboratoryRetIsoHist (char *, double, double, double *);
GxErrorCode  GxGetLaboratoryTemperatures (double, double, double *);
GxErrorCode  GxGetLaboratoryTimeRange (double *, double *);
GxErrorCode  GxGetLaboratoryTimes (double, double, double *);
GxErrorCode  GxGetLaboratoryTimesteps (double, double, int *);
GxErrorCode  GxGetLaboratoryVanKrevelen (double, double, double *, double *);
GxErrorCode  GxGetMainExpulsion (double, GxExpulsion **, GxExpulsion **,
                                 GxExpulsion **, double *, double *,
                                 double *);
GxErrorCode  GxGetMainExpulsionSelected (SortedList, SortedList, 
                                         SortedList, double, 
                                         GxExpulsion **, GxExpulsion **, 
                                         GxExpulsion **, double *, 
                                         double *, double *);
GxErrorCode  GxGetMainIsotopeSelected (SortedList, SortedList, SortedList, 
                                       double, GxIsotope **, GxIsotope **, 
                                       GxIsotope **);
GxErrorCode  GxGetMobileSpeciesNameList (List *);
GxErrorCode  GxGetRockEval2Analysis (double *, double *, double *, double *,
                                     double *);
GxErrorCode  GxGetSpeciesNameList (List *);
GxErrorCode  GxGetSubBurialTimeRange (char *, char *, char *, double *, 
					double *);
GxErrorCode  GxGetUpwardExpulsion (double, GxExpulsion **, GxExpulsion **,
                                   GxExpulsion **, double *, double *,
                                   double *);
GxErrorCode  GxGetUpwardExpulsionSelected (SortedList, SortedList, 
                                           SortedList, double, 
                                           GxExpulsion **, GxExpulsion **, 
                                           GxExpulsion **, double *, 
                                           double *, double *);
GxErrorCode  GxGetUpwardIsotopeSelected (SortedList, SortedList, SortedList, 
                                         double, GxIsotope **, GxIsotope **, 
                                         GxIsotope **);
GxBool       GxIsSpeciesMobile        (char *);
GxBool       GxIsSpeciesHCGas         (char *);
GxBool       GxIsSpeciesHCWetGas      (char *);
GxBool       GxIsSpeciesOil           (char *);
GxBool       GxIsSpeciesOilGas        (char *);
GxBool       GxLaboratoryModelPresent (void);
GxBool       GxNitrogenPartOfModel    (void);

GxErrorCode  GxOpen (void);
GxErrorCode  GxOptimizeCharLength (char *, char *, char *, double, double *,
                                   double *, int *);
GxErrorCode  GxRunGeologicalModel (void);
GxErrorCode  GxRunLaboratoryModel (char *, char *, char *, double);
/* Constant values */
/*-----------------*/
#define GXC_UPPERCHARLENGTH	100.0
#define GXC_LOWERCHARLENGTH	7e-05
#define GXC_FINALCHARLENGTH	 10.0

#ifdef __cplusplus
}
#endif

#endif

