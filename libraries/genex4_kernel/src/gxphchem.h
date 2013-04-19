/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxphchem.h,v 25.0 2005/07/05 08:03:49 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxphchem.h,v $
 * Revision 25.0  2005/07/05 08:03:49  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.1  2004/10/04 14:53:31  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:12  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:44  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:30  ibs
 * RS6000 relkease
 *
 * Revision 21.1  2001/06/07 09:45:31  ibs
 * moved the Log keyword to keep RCS happy.
 *
 * Revision 21.0  2001/05/02 08:59:26  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:41  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:52  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:17  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:13  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:19  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:53  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:38  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:03:52  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:49  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:42:15  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.3  1996/01/04  15:17:57  ibs
 * cplusplus corrections added.
 *
 * Revision 9.2  1995/10/30  09:52:49  ibs
 * Add headers.
 * */
/* ================================================================== */
#ifndef   GXPHCHEM_H
#define   GXPHCHEM_H

#ifdef __cplusplus
extern "C" {
#endif
    

/*
--------------------------------------------------------------------------------
 gxphchem.h   include file for Genex physics and chemistry functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        19-JUN-1992

 Description: gxphchem.h contains prototype, type and macro definitions for
              the physics and chemistry functions like stoichiometry.

 History
 -------
 19-JUN-1992  P. Alphenaar  initial version
 29-JUL-1992  M.J.A. van Houtert GxCalcVanKrevelen
  6-AUG-1992  M.J.A. van Houtert GxCalcFreeGas added
 18-AUG-1993  Th. Berkers        Net Generation implemented in submodel using 
                                 notes from J.G. Stainforth, 6/93.
                                 Changes have been implemented in parameterlist
                                 for functions: 
                                      GxGeneration
                                      GxGenerationDiffusion 
 22-DEC-1993  Th. Berkers        Implementing Nitrogen model: To Parameter-list
                                 of GxCalcSpecies n/c index added as double
                                 GxIsN2Gas added
 04-JAN-1994  Th. Berkers        Implementing Nitrogen model: To parameter-list
                                 of GxOverPressure n2mass added as double
                                 GxN2InWater added
 10-JAN-1994  Th. Berkers        Implementing Nitrogen model: 
                                      GxCalcRatios added
 23-SEP-1994  Th. Berkers        Conversion factor for metric kg to imperial 
                                 added
 14-DEC-1994  Th. Berkers        GxIsHCWetGas added
 02-MAR-1995  Th. Berkers        Integration of Isotopes Fractionation
--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  "gxgenerl.h"
#include  "gxlithol.h"

/* Macro definitions */
/* ----------------- */
#define  GXC_GORCONVERSION       5.551931378
#define  GXC_SM3TOBCF            3.5315E-08
#define  GXC_SM3TOMB             6.2898E-06
#define  GX_INITIALGEOLTAU       1.7185E-06
#define  GXC_KGTOIMP             1.0
/* -- enumeration types -- */
#define  GXN_LUMPED              3
enum {
   GX_BITUMEN, GX_SOLID, GX_TOTALOM
};
/* -- error definitions -- */
enum {
  GXE_INVREACTIONSCHEME = GXBE_PHCHEM | GXF_APPLICATION | GXF_ERROR   | 1
};

/* Function prototypes */
/* ------------------- */
double       GxAPIGravity (double);
double       GxAromaticity (const GxSpecies *);
double       GxAromaticityOfChains (int, const GxSpecies *, const double **);
void         GxAsphalteneComp (int, const double *, double *);
double       GxAsphalteneTransf (double, double, double, double);
void         GxAtomicRatios (int, GxSpecies *, double *, double *, double *,
                             double *);
double       GxBubblePointGOR (double, double, double, double);
double       GxCalcFreeGas ( double, double, double, double, double, double,
			     double, double );
GxErrorCode  GxCalcInitialConc (int, GxSpecies *, double, double, double *);
void         GxCalcIsoFactorConst (GxIsoFactor *);
double       GxCalcIsotopeFractSR (double, double, double, double, 
                                   GxIsoFactor *);
void         GxCalcRatios (GxSpecies *, int, double *, double *, 
                           double *, double *, double *);
GxErrorCode  GxCalcSpecies (int, int, GxSpecies *, double, double, double,
                            double, double, double, double);
GxErrorCode  GxCalcStoichiometry (int, int, const GxSpecies *,
                                  const GxReactionScheme *, double **);
void         GxCalcVanKrevelen (GxSpecies *, int, double *, double *, 
                                double *, double *);

void         GxConcLumped (int, int, const GxSpecies *, const double **,
                           double **);
double       GxConvMassTransfer (double, double, double);
double       GxCrackingRate (const GxSpecies *, double, double, double,
                             double);
double       GxDepth (double, double, double);
GxFieldType  GxDetermineFieldType (double, double, double, double, double,
                                   double);
double       GxFailPressure (double, double);
double       GxFormulaWeight (int, const double *);
double       GxFourierNumber (const GxSpecies *, double, double, double,
                              double, double, double, double, double);
double       GxGasInWater (double, double, double, double, double, double);
void         GxGeneration (GxIsoFract *, GxIsoFactor *, Lithology *, int, int, 
                           int, int, const double **, const double *, double, 
                           double, double **, double **, double **, double**,
                           double);
void         GxGenerationDiffusion (GxIsoFract *, GxIsoFactor *, Lithology *, 
                                    int, int, int, int, const double **, 
                                    const double *, double, double, double **,
                                    double **, double **, double **,
                                    const double **, double, double, double, 
                                    double, double **, double);
double       GxGeologicalVRE (double, double, double *);
GxBool       GxHasOilComposition (const GxSpecies *);
double       GxImmobileDensity (const GxSpecies *);
GxBool       GxIsAsphaltene (const GxSpecies *);
GxBool       GxIsHCGas (const GxSpecies *);
GxBool       GxIsHCWetGas (const GxSpecies *);
GxBool       GxIsHydrocarbon (const GxSpecies *);
GxBool       GxIsN2Gas (const GxSpecies *);
double       GxKerogenTransf (double, double, double);
double       GxLithostaticPressure (double, double);
double       GxMassStoichFactor (const GxSpecies *, const GxSpecies *,
                                 double);
double       GxN2InWater (double, double, double, double);
double       GxOverPressure (double, double, double, double);
double       GxReferenceDiffusivity (const GxSpecies *, int, double, double,
                                     double, double, double, double,
                                     double);
double       GxSaturationCGR (double, double);
double       GxTotalOrganicCarbon (double, double);
double       GxVolumeFractionOM (int, const GxSpecies *, const double *);
double       GxWaterDensity (double, double, double);


#ifdef __cplusplus
}
#endif

#endif

