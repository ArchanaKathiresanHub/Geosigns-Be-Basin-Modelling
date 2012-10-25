/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxmodel.h,v 25.0 2005/07/05 08:03:48 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxmodel.h,v $
 * Revision 25.0  2005/07/05 08:03:48  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:47:55  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:29  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:11  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:43  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:27  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:24  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:38  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:47  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:14  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:07  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:16  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:48  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:29  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:03:18  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:45  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:42:11  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.3  1996/01/04  15:17:51  ibs
 * cplusplus corrections added.
 *
 * Revision 9.2  1995/10/30  09:53:43  ibs
 * Add headers.
 * */
/* ================================================================== */
#ifndef   GXMODEL_H
#define   GXMODEL_H

#ifdef __cplusplus
extern "C" {
#endif
    

/*
--------------------------------------------------------------------------------
 gxmodel.h    include file for Genex model functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        19-JUN-1992

 Description: gxmodel.h contains prototype, type and macro definitions for
              the model functions like initialization and running.

 History
 -------
 19-JUN-1992  P. Alphenaar        initial version
 27-JUL-1992  M.J.A. van Houtert  kerogen and asphaltene conversion added to
                                  GxSubModel
 29-JUL-1992  M.J.A. van Houtert  GxGetConversionHist routine added
 30-JUL-1992  M.J.A. van Houtert  GxGetVanKrevelen routine added
  5-AUG-1992  M.J.A. van Houtert  GxGetAPIHist routine added
  6-AUG-1992  M.J.A. van Houtert  GxGetGORHist routine added
 10-AUG-1992  M.J.A. van Houtert  GxGetMassesHist routine added
 28-MAY-1993  M.J.A. van Houtert  generation added to submodel
 18-AUG-1993  Th. Berkers         Net Generation implemented in submodel using 
                                  notes from J.G. Stainforth, 6/93.
                                  Changes have been implemented in: 
                                      struct GxSubModel
 10-JAN-1994  Th. Berkers         Implementation of Nitrogen model
                                      GxGetRatios added 
 17-NOV-1994  Th. Berkers         GxGetAPIHistSelected and GxGetGORHistSelected
                                  added
 23-NOV-1994  Th. Berkers         GxGetTotGenerationSelected, 
                                  GxGetTotDownExpulsionSelected, 
                                  GxGetTotMainExpulsionSelected, 
                                  GxGetTotUpExpulsionSelected added
 27-DEC-1994  Th. Berkers         GxGetBeginEndStep added
 02-MAR-1995  Th. Berkers         Integration of Isotopes Fractionation
--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  "gxgenerl.h"
#include  "list.h"

/* Macro definitions */
/* ----------------- */
/* -- error definitions -- */
enum {
  GXE_NOTINITIALIZED = GXBE_MODEL | GXF_APPLICATION | GXF_ERROR   | 1,
  GXE_NOTPRESENT, GXE_STEPSEXCEEDED, GXE_SUBMODELNOTRUN
};
/* -- model property history flags -- */
#define  GXP_TIME                  (1L<<1)
#define  GXP_TEMPERATURE           (1L<<2)
#define  GXP_PRESSURE              (1L<<3)
#define  GXP_EXPULSIONFLUX         (1L<<4)
#define  GXP_CONCENTRATION         (1L<<5)
#define  GXP_KEROGENCONVERSION     (1L<<6)
#define  GXP_ASPHALTENECONVERSION  (1L<<7)
#define  GXP_GENERATION            (1L<<8)
#define  GXP_ISOTOPE               (1L<<9)

/* Type definitions */
/* ---------------- */
/* -- structure definitions -- */
typedef  struct GxModel     GxModel;
typedef  struct GxSubModel  GxSubModel;
typedef  struct GxSubModelId  GxSubModelId;
struct GxModel {
   GxSubModel     ***submodel;
   int            nelements, nspecies, nreactions, nlocations, nhorizons,
                  nlithologies, nslices, ntimesteps;
   double         overburdendensity, inorganicdensity, surfacetemperature,
                  temperaturegradient, expansivity, compressibility;
   GxSpecies      **species;
   GxBool         asphaltenepresent;
   double         ***stoichiometry, *aromchain;
   GxTPHistory    **tphistories;
   double         *charlength, *upperbiot, *lowerbiot, *initkerogenconc,
                  timeunitfactor;
   int            referencenode;
   unsigned long  historyflags;
   double         *timehist;
   int            timestepnr;
   double         time, *crackingrate, **equations, **conclumped;
};
struct GxSubModel {
   double  netthickness, **concentration, *temperaturehist, *pressurehist,
           **upwardflux, **downwardflux, **concentrationhist, 
           *kerogenconversion, *asphalteneconversion, **negativegenrate,
           **positivegenrate, **generationhist, **isotopehist;
};
struct GxSubModelId {
   int     subareanr, horizonnr, lithologynr; 
};



/* Function prototypes */
/* ------------------- */
GxErrorCode  GxCalcRockEval2Results (GxModel *, int, int, int, double, double,
                                     double *, double *, double *, double *,
                                     double *);
GxErrorCode  GxCreateModel (int, int, int, int, int, int, int, int,
                            unsigned long, GxModel **);
GxErrorCode  GxDestroyModel (GxModel *);
GxErrorCode  GxGetAPIHist (GxModel *, double, double, double *, double *);
GxErrorCode  GxGetAPIHistSelected (GxModel *, List, double, double, 
                                   double *, double *);
GxErrorCode  GxGetBeginEndStep (GxModel *, double, double, int *, int *);
GxErrorCode  GxGetCharge (GxModel *, double, double *, double *, double *,
                          double *, double, double, double, double,
                          GxCharge *, GxCharge *);
GxErrorCode  GxGetConcentrationHist (GxModel *, int, int, int, int, double,
                                     double, double *);
GxErrorCode  GxGetConversionHist (GxModel *, int, int, int, double, double,
                                  double *, double * );
GxErrorCode  GxGetExpIsotopeHist (GxModel *, int, int, int, int, GxUpDown,
                                  double, double, double *);
GxErrorCode  GxGetExpulsionHist (GxModel *, int, int, int, int, GxUpDown,
                                 GxCumulative, double, double, double *);
GxErrorCode  GxGetFluxHist (GxModel *, int, int, int, int, GxUpDown,
                            double, double, double *);
GxErrorCode  GxGetGenerationHist (GxModel *, int, int, int, int, GxCumulative,
                                  double, double, double *);
GxErrorCode  GxGetGORHist (GxModel *, double, double, double *, 
			double *, double *, double *, double, double, 
			double, double, double *, double *);
GxErrorCode  GxGetGORHistSelected (GxModel *, List, List, double, double, 
                        double *, double *, double *, double *, double, double, 
			double, double, double *, double *);
GxErrorCode  GxGetIsotopeHist (GxModel *, int, int, int, int, double, double, 
                               double *);
GxErrorCode  GxGetMassesHist (GxModel *, int, int, int, int, double, double,
			      double, double *);
GxErrorCode  GxGetNumberOfTimes (GxModel *, double, double, int *);
GxErrorCode  GxGetPressureHist (GxModel *, int, int, int, double, double,
                                double *);
GxErrorCode  GxGetRatios (GxModel *, int, int, int, double, double,
                          double *, double *, double *);
GxErrorCode  GxGetRetIsotopeHist (GxModel *, int, int, int, int, double, 
                                  double, double *);
GxErrorCode  GxGetTemperatureHist (GxModel *, int, int, int, double, double,
                                   double *);
GxErrorCode  GxGetTimeHist (GxModel *, double, double, double *);
GxErrorCode  GxGetTimeRange (GxModel *, double *, double *);
GxErrorCode  GxGetTotDownExpulsion (GxModel *, double, double *, double **);
GxErrorCode  GxGetTotDownExpulsionSelected (GxModel *, List, double, double *, 
                                            double **);
GxErrorCode  GxGetTotDownIsotopeSelected (GxModel *, List, double, double *, 
                                          double **, double **);
GxErrorCode  GxGetTotGeneration (GxModel *, double, double *, double **);
GxErrorCode  GxGetTotGenerationSelected (GxModel *, List, double, double *, 
                                         double **);
GxErrorCode  GxGetTotMainExpulsion (GxModel *, double, double *, double **);
GxErrorCode  GxGetTotMainExpulsionSelected (GxModel *, List, double, double *, 
                                            double **);
GxErrorCode  GxGetTotMainIsotopeSelected (GxModel *, List, double, double *, 
                                          double **, double **);
GxErrorCode  GxGetTotUpExpulsion (GxModel *, double, double *, double **);
GxErrorCode  GxGetTotUpExpulsionSelected (GxModel *, List, double, double *, 
                                          double **);
GxErrorCode  GxGetTotUpIsotopeSelected (GxModel *, List, double, double *, 
                                        double **, double **);
GxErrorCode  GxGetVanKrevelen (GxModel *, int, int, int, double, double,
                               double *, double * );
GxErrorCode  GxGetVREHist (GxModel *, int, int, int,
                           double (*) (double, double, double *), double,
                           double, double, double *);
GxErrorCode  GxInitializeModel (GxModel *, double, double, double, double,
                                double, double, double, GxSpecies **,
                                double ***, GxTPHistory **, double *,
                                double *, double *, double ***, double,
                                double, double **, double *);
GxErrorCode  GxRunModel (GxModel *, const GxIsoFract *, GxIsoFactor *, double, 
                         double, double, double, double,
                         double, double, GxSpecies **, double ***,
                         GxTPHistory **, double *, double *, double *,
                         double ***, double, double **, double *); 
GxErrorCode  GxSampleSubModel (GxModel *, int, int, int, double, double *,
                               double *, double *, double *);
GxErrorCode  GxStepModel (GxModel *, const GxIsoFract *, const GxIsoFactor *,
                          double);


#ifdef __cplusplus
}
#endif

#endif
