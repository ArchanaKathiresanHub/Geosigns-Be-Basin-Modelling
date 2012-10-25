/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxmodel.c,v 25.0 2005/07/05 08:03:47 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxmodel.c,v $
 * Revision 25.0  2005/07/05 08:03:47  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:42:58  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:28  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:11  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:43  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:26  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:23  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:37  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:47  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:14  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:06  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:16  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:47  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:26  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:03:11  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:07  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:23  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:53:28  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 GxModel.c    Genex model functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        25-JUN-1992

 Description: GxModel.c contains the Genex functions for control of the
              modeling. The following functions are available:

                GxCalcRockEval2Results - perform RE2 analysis on model
                GxCreateModel          - create a new model
                GxDestroyModel         - destroy an existing model
		GxGetAPIHist	       - calculate the API for oil
                GxGetAPIHistSelected   - calculate API for sel. submodels
                GxGetCharge            - calculate hydrocarbon charge
                GxGetConcentrationHist - get the conc history from model
                GxGetConversionHist    - get the conv history from model
                GxGetExpIsotopeHist    - Retrieve the isotope history data of a 
                                         expelled species
                GxGetExpulsionHist     - get the expulsion history from model
                GxGetFluxHist          - get the flux history from model
                GxGetGenerationHist    - Retrieve the generation data
                GxGetGORHist	       - get the GOR
                GxGetGORHistSelected   - get the GOR for sel. submodels
                GxGetIsotopeHist       - Retrieve the isotope history data
		GxGetMassesHist        - get the retention Masses
                GxGetNumberOfTimes     - get the number of times from model
                GxGetRatios            - get the HC, OC and NC values
                GxGetPressureHist      - get the pressure history from model
                GxGetRetIsotopeHist    - Retrieve the isotope history data of a 
                                         retained species
                GxGetTemperatureHist   - get the temp history from model
                GxGetTimeHist          - get the time history from the model
                GxGetTimeRange         - get the time range from the model
                GxGetTotDownExpulsion  - get the total downwards expulsion
                GxGetTotDownExpulsionSelected
                                       - get the total downwards expulsion for
                GxGetTotDownIsotopeSelected
                                       - get the total downwards isotopes for
                                         selected submodels
                GxGetTotGeneration     - Get the total generation
                GxGetTotGenerationSelected  
                                       - Get the total generation for selected
                                         submodels
                GxGetTotMainExpulsion  - get the total expulsion into main sms
                GxGetTotMainExpulsionSelected
                                       - get the total expulsion into main sms
                                         for selected submodels
                GxGetTotMainIsotopeSelected
                                       - get the total isotopes into main sms
                                         for selected submodels
                GxGetTotUpExpulsion    - get the total upwards expulsion
                GxGetTotUpExpulsionSelected
                                       - get the total upwards expulsion for
                                         selected submodels
                GxGetTotUpIsotopeSelected
                                       - get the total upwards isotopes for
                                         selected submodels
                GxGetVanKrevelen       - get the Van Krevelen data from model
                GxGetVREHist           - get the VRE history from the model
                GxInitializeModel      - initialize model to a particular time
                GxRunModel             - run a complete model
                GxSampleSubModel       - sample properties in a submodel
                GxStepModel            - step the model to a particular time

              To support these functions this file contains a number of
              local functions that can not be used outside this file.

 Usage notes: - In order to use the functions in this source file the header
                file gxmodel.h should be included.
              - All functions return an error code. If an error is detected
                the error handler function is called and an error code unequal
                to GXE_NOERROR is returned to the caller.


 History
 -------
 25-JUN-1992  P. Alphenaar   initial version
 28-JUL-1992  M. van Houtert kerogen & asphaltene conversions added to
                             - GxAllocSubModelArrays
                             - GxInitializeSubModel
                             - GxFreeSubModelArrays
                             - GxStepSubModel
 29-JUL-1992  M. van Houtert GxGetConversionHist created
 30-JUL-1992  M. van Houtert GxGetVanKrevelen created
  5-AUG-1992  M. van Houtert GxCalcOilMassVolume added
  5-AUG-1992  M. van Houtert GxGetAPIHist added
  6-AUG-1992  M. van Houtert GxCalcGasMassVolume added
  6-AUG-1992  M. van Houtert GxGetGORHist added
 10-AUG-1992  M. van Houtert GxGetMassesHist added
 28-MAY-1993  M. van Houtert Array generation added in GxAllocSubModelArrays
                             and GxFreeSubModelArrays
 28-MAY-1993  M. van Houtert Argument in GxGeneration and GxGenerationDiffusion
                             call added in GxStepSubModel.
  1-JUN-1993  M. van Houtert Retention bug fixed !!!
 18-AUG-1993  Th. Berkers    Net Generation implemented in submodel using 
                             notes from J.G. Stainforth, 6/93.
                             Changes have been implemented in: 
                                 GxAllocSubModelArrays
                                 GxFreeSubModelArrays
                                 GxStepSubModel                   
 27-AUG-1993  M. van Houtert GxGetTotGeneration added
 07-OCT-1993  Th. Berkers    GxGetVanKrevelen, immobile species and asphaltene
                             taken into account during calculations
 21-DEC-1993  Th. Berkers    When retrieving the cumulative generation the delta
                             time was not taken in account. 
 04-JAN-1994  Th. Berkers    Nitrogen model implemented. 
                             n2mass and n2volume taken into account in function:
                                GxGetCharge and
                                GxGetGORHist
                             Function GxCalcN2MassVolume added
 10-JAN-1994  Th. Berkers    Implementation of Nitrogen model
                                GcGetRatios added
 23-SEP-1994  Th. Berkers    Masses added in GxGetCharge
 17-NOV-1994  Th. Berkers    GxGetAPIHistSelected and GxGetGORHistSelected
                             added
 23-NOV-1994  Th. Berkers    GxGetTotGenerationSelected, 
                             GxGetTotDownExpulsionSelected, 
                             GxGetTotMainExpulsionSelected, 
                             GxGetTotUpExpulsionSelected added
 27-Dec-1994  Th. Berkers    GxGetBeginEndStep added
 16-Jan-1994  Th. Berkers    GxGetBeginEndStep used for all calculations
 02-MAR-1995  Th. Berkers    Integration of Isotopes Fractionation
 31-MAR-1995  P.J. Huysse    Added const def in GxRunModel 2nd arg.
--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <stdio.h>
#include  <assert.h>
#include  <float.h>
#include  <math.h>
#include  <stdlib.h>
#include  <string.h>
#include  "gxconsts.h"
#include  "gxerror.h"
#include  "gxmodel.h"
#include  "gxnumer.h"
#include  "gxphchem.h"
#include  "gxutils.h"
#include  "gxgenerl.h"
#include  "gxlithol.h"

/* Macro definitions */
/* ----------------- */
#define  GX_DEFAULTOILDENSITY  800.0
#define  GX_DEFAULTGASDENSITY  0.8
#define  GX_DEFAULTN2DENSITY   1.25
#define  GX_RE2CUTTEMPERATURE  (305.0+GXC_ZEROCELCIUS)
/* -- constants for calculations -- */
#define  GXC_FAILURECRITERION  0.8
/* -- error text definitions -- */
#define  GXT_NOTINITIALIZED  "The model is not initialized."
#define  GXT_NOTPRESENT      "The requested property is not present."
#define  GXT_STEPSEXCEEDED   "The number of time steps exceeds the number " \
                             "specified when the model was created.\n"
#define  GXT_SUBMODELNOTRUN  "The specified submodel was not calculated " \
                             "because its net thickness is zero.\n"

/* Function prototypes */
/* ------------------- */
static GxErrorCode GxAllocSubModelArrays (GxModel *, unsigned long);
static void        GxCalcGasMassVolume (GxModel *, int, int, int, double *,
				        double, int, double *, double *);
static void        GxCalcOilMassVolume (GxModel *, int, int, int, double *, 
					double, int, double *, double *);
static void        GxCalcN2MassVolume (GxModel *, int, int, int, double *, 
				       double, int, double *, double *);
static void        GxFreeSubModelArrays (GxModel *);
static double      GxGetPressure (GxTPHistory *, double);
static double      GxGetTemperature (GxTPHistory *, double);
static double      GxGetTime (double, double, int, int);
static void        GxInitializeSubModel (GxSubModel *, int, int, double,
                                         double, double, double *);
static int         GxSliceNumber (double, int);
static GxErrorCode GxStepLithologies (GxSubModel *, const GxIsoFract *,
                                      const GxIsoFactor *, int, int, int, int,
                                      double, double, double, GxSpecies **,
                                      GxBool, double ***, double *, double *,
                                      double *, double *, double *,
                                      GxTPHistory *, int, int, double, double,
                                      double *, double **, double **);
static void        GxStepSubModel (GxSubModel *, const GxIsoFract *,
                                   GxIsoFactor *, Lithology *, int, int, int, 
                                   double,
                                   double, double, GxSpecies *, GxBool,
                                   double **, double, double, double, double,
                                   double, int, int, double, double, double,
                                   double *, double **, double **);





/*
--------------------------------------------------------------------------------
 GxAllocSubModelArrays - allocate the arrays for all submodels
--------------------------------------------------------------------------------

 GxAllocSubModelArrays allocates the arrays that are needed for running the
 submodels.

 Arguments
 ---------
 model        - model for which the submodel arrays should be allocated
 historyflags - flags identifying the properties to be saved through time

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocSubModelArrays (GxModel *model,
                                          unsigned long historyflags)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, j, k, l, m;
   GxSubModel   *submodel;

   assert (model);
   assert (model->nspecies > 0);
   assert (model->nlocations > 0);
   assert (model->nhorizons > 0);
   assert (model->nlithologies > 0);
   assert (model->nslices > 1);
   assert (model->ntimesteps > 1);

/* Allocate the array for the submodels */
   model->submodel = NULL;
   rc = GxAllocArray (&model->submodel, sizeof (GxSubModel),
                      3, model->nlocations, model->nhorizons,
                      model->nlithologies);
   if (!rc) {

/* -- Initialize the pointers in the submodels */
      for (i=0; i<model->nlocations; i++) {
         for (j=0; j<model->nhorizons; j++) {
            for (k=0; k<model->nlithologies; k++) {
               model->submodel[i][j][k].concentration        = NULL;
               model->submodel[i][j][k].temperaturehist      = NULL;
               model->submodel[i][j][k].pressurehist         = NULL;
               model->submodel[i][j][k].upwardflux           = NULL;
               model->submodel[i][j][k].downwardflux         = NULL;
               model->submodel[i][j][k].concentrationhist    = NULL;
               model->submodel[i][j][k].kerogenconversion    = NULL;
	       model->submodel[i][j][k].asphalteneconversion = NULL;
               model->submodel[i][j][k].negativegenrate      = NULL;
               model->submodel[i][j][k].positivegenrate      = NULL;
               model->submodel[i][j][k].generationhist       = NULL;
               model->submodel[i][j][k].isotopehist          = NULL;
            }
         }
      }
      for (i=0; (!rc)&&(i<model->nlocations); i++) {
         for (j=0; (!rc)&&(j<model->nhorizons); j++) {
            for (k=0; (!rc)&&(k<model->nlithologies); k++) {
               submodel = &model->submodel[i][j][k];

/* ----------- Allocate the array for the concentration */
               rc = GxAllocArray (&submodel->concentration, sizeof (double),
                                  2, model->nslices, model->nspecies);
               if (!rc) {
                  for (l=0; l<model->nslices; l++) {
                     for (m=0; m<model->nspecies; m++) {
                        submodel->concentration[l][m] = 0.0;
                     }
                  }
               }

/* ----------- Allocate the array for the negative generation rate */
               rc = GxAllocArray (&submodel->negativegenrate, sizeof (double),
                                  2, model->nslices, model->nspecies);
               if (!rc) {
                  for (l=0; l<model->nslices; l++) {
                     for (m=0; m<model->nspecies; m++) {
                        submodel->negativegenrate[l][m] = 0.0;
                     }
                  }
               }

/* ----------- Allocate the array for the positive generation rate */
               rc = GxAllocArray (&submodel->positivegenrate, sizeof (double),
                                  2, model->nslices, model->nspecies);
               if (!rc) {
                  for (l=0; l<model->nslices; l++) {
                     for (m=0; m<model->nspecies; m++) {
                        submodel->positivegenrate[l][m] = 0.0;
                     }
                  }
               }

/* ----------- Allocate the array for the temperature history */
               if (!rc && (historyflags & GXP_TEMPERATURE))
                  rc = GxAllocArray (&submodel->temperaturehist,
                                     sizeof (double), 1, model->ntimesteps+1);
               if (submodel->temperaturehist) {
                  for (l=0; l<model->ntimesteps+1; l++) {
                     submodel->temperaturehist[l] = GXC_ZEROCELCIUS;
                  }
               }
                
/* ----------- Allocate the array for the pressure history */
               if (!rc && (historyflags & GXP_PRESSURE))
                  rc = GxAllocArray (&submodel->pressurehist, sizeof (double),
                                     1, model->ntimesteps+1);
               if (submodel->pressurehist) {
                  for (l=0; l<model->ntimesteps+1; l++) {
                     submodel->pressurehist[l] = 0.0;
                  }
               }
/* ----------- Allocate the arrays for the expulsion histories */
               if (!rc && (historyflags & GXP_EXPULSIONFLUX))
                  rc = GxAllocArray (&submodel->upwardflux, sizeof (double),
                                     2, model->ntimesteps+1, model->nspecies);
               if (submodel->upwardflux) {
                  for (l=0; l<model->ntimesteps+1; l++) {
                     for (m=0; m<model->nspecies; m++) {
                        submodel->upwardflux[l][m] = 0.0;
                     }
                  }
               }
               if (!rc && (historyflags & GXP_EXPULSIONFLUX))
                  rc = GxAllocArray (&submodel->downwardflux, sizeof (double),
                                     2, model->ntimesteps+1, model->nspecies);
               if (submodel->downwardflux) {
                  for (l=0; l<model->ntimesteps+1; l++) {
                     for (m=0; m<model->nspecies; m++) {
                        submodel->downwardflux[l][m] = 0.0;
                     }
                  }
               }

/* ----------- Allocate the array for the concentration history */
               if (!rc && (historyflags & GXP_CONCENTRATION))
                  rc = GxAllocArray (&submodel->concentrationhist,
                                     sizeof (double), 2, model->ntimesteps+1,
                                     model->nspecies);
               if (submodel->concentrationhist) {
                  for (l=0; l<model->ntimesteps+1; l++) {
                     for (m=0; m<model->nspecies; m++) {
                        submodel->concentrationhist[l][m] = 0.0;
                     }
                  }
               }

/* ----------- Allocate the array for the conversion history   */
	       if (!rc && (historyflags & GXP_KEROGENCONVERSION))
		  rc = GxAllocArray (&submodel->kerogenconversion,
				     sizeof (double), 1, model->ntimesteps+1);
               if (submodel->kerogenconversion) {
                  for (l=0; l<model->ntimesteps+1; l++) {
                     submodel->kerogenconversion[l] = 0.0;
                  }
               }

/* ----------- Allocate the array for the conversion history   */
	       if (!rc && (historyflags & GXP_ASPHALTENECONVERSION))
		  rc = GxAllocArray (&submodel->asphalteneconversion,
				     sizeof (double), 1, model->ntimesteps+1);
               if (submodel->asphalteneconversion) {
                  for (l=0; l<model->ntimesteps+1; l++) {
                     submodel->asphalteneconversion[l] = 0.0;
                  }
               }

/* ----------- Allocate the array for the generation history */
               if (!rc && (historyflags & GXP_GENERATION))
                  rc = GxAllocArray (&submodel->generationhist, 
                                     sizeof (double), 2, model->ntimesteps+1,
                                     model->nspecies);
               if (submodel->generationhist) {
                  for (l=0; l<model->ntimesteps+1; l++) {
                     for (m=0; m<model->nspecies; m++) {
                        submodel->generationhist[l][m] = 0.0;
                     }
                  }
               }
                                 
/* ----------- Allocate the array for the isotope fractionation history */
               if (!rc && (historyflags & GXP_ISOTOPE))
                  rc = GxAllocArray (&submodel->isotopehist, 
                                     sizeof (double), 2, model->ntimesteps+1,
                                     model->nspecies);
               if (submodel->isotopehist) {
                  for (l=0; l<model->ntimesteps+1; l++) {
                     for (m=0; m<model->nspecies; m++) {
                        submodel->isotopehist[l][m] = 0.0;
                     }
                  }
               }
            }
         }
      }
   }
   if (rc) GxFreeSubModelArrays (model);
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxCalcGasMassVolume - calculate the volume and the mass of the Gas
--------------------------------------------------------------------------------

 This routine calculates the volume and the mass expelled from a submodel
 at a given time. These values are required for the API and GOR
 history calculations

 Arguments
 ---------

 model             - the lab or geological model
 locationnr        - # of drainage subarea
 horizonnr         - # of horizon
 lithologynr       - # of lithology
 areas             - the sizes of all the drainage subareas
 deltatime         - duration of a time step
 timestep          - the number of the timestep itself
 gasmass           - array for storing the mass data
 gasvolume         - array for storing the volume data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered


--------------------------------------------------------------------------------
*/


static void  GxCalcGasMassVolume (GxModel *model, int locationnr,
                           int horizonnr,  int lithologynr,
                           double *areas,  double deltatime,
                           int timestep,   double *gasmass,
                           double *gasvolume)
{
   GxSubModel   *submodel;
   double        expulsion;
   int           i;

   assert (model);
   assert (areas);
   assert (gasmass);
   assert (gasvolume);
   assert (locationnr  >= 0);
   assert (horizonnr   >= 0);
   assert (lithologynr >= 0);
   assert (deltatime   >= 0.0);
   assert (timestep    >= 0);

   submodel = &model->submodel[locationnr][horizonnr][lithologynr];

   *gasmass   = 0.0;
   *gasvolume = 0.0;

   if (submodel->netthickness > 0.0) {

      for (i=0; i<model->nspecies; i++) {

         if (GxIsHCGas (&model->species[lithologynr][i])) {

            expulsion  = 0.0;
            if (model->upperbiot[lithologynr] >= 1.0) {
               expulsion = areas[locationnr] * deltatime *
                                     submodel->upwardflux[timestep][i];
            }
            if (model->lowerbiot[lithologynr] >= 1.0) {
               expulsion += areas[locationnr] * deltatime *
                                        submodel->downwardflux[timestep][i];
            }

            *gasmass   += expulsion;
            *gasvolume += expulsion/model->species[lithologynr][i].density;
         }
      }
   }
}


/*
--------------------------------------------------------------------------------
 GxCalcOilMassVolume - calculate the volume and the mass of the oil
--------------------------------------------------------------------------------

 This routine calculates the volume and the mass expelled from a submodel
 at a given time. These values are required for the API-history calculations

 Arguments
 ---------

 model             - the lab or geological model
 locationnr        - # of drainage subarea
 horizonnr         - # of horizon
 lithologynr       - # of lithology
 areas             - the sizes of all the drainage subareas
 deltatime         - duration of a time step
 timestep          - the number of the timestep itself
 oilmass           - array for storing the mass data
 oilvolume         - array for storing the volume data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered


--------------------------------------------------------------------------------
*/

static void  GxCalcOilMassVolume (GxModel *model, int locationnr,
                           int horizonnr,  int lithologynr,
                           double *areas,  double deltatime,
                           int timestep,   double *oilmass,
                           double *oilvolume)
{
   GxSubModel   *submodel;
   double        expulsion;
   int           i;

   assert (model);
   assert (areas);
   assert (oilmass);
   assert (oilvolume);
   assert (locationnr  >= 0);
   assert (horizonnr   >= 0);
   assert (lithologynr >= 0);
   assert (deltatime   >= 0.0);
   assert (timestep    >= 0);

   submodel = &model->submodel[locationnr][horizonnr][lithologynr];

   *oilmass   = 0.0;

   *oilvolume = 0.0;

   if (submodel->netthickness > 0.0) {

      for (i=0; i<model->nspecies; i++) {

         if (model->species[lithologynr][i].mobile
          && GxHasOilComposition (&model->species[lithologynr][i])) {

            expulsion  = 0.0;
            if (model->upperbiot[lithologynr] >= 1.0) {
               expulsion = areas[locationnr] * deltatime *
                                     submodel->upwardflux[timestep][i];
            }
            if (model->lowerbiot[lithologynr] >= 1.0) {
               expulsion += areas[locationnr] * deltatime *
                                        submodel->downwardflux[timestep][i];
            }

            *oilmass   += expulsion;
            *oilvolume += expulsion/model->species[lithologynr][i].density;
	 }
      }
   }
}


/*
--------------------------------------------------------------------------------
 GxCalcN2MassVolume - calculate the volume and the mass of the Nitrogen
--------------------------------------------------------------------------------

 This routine calculates the volume and the mass expelled from a submodel
 at a given time. These values are required for the GOR-history calculations

 Arguments
 ---------

 model             - the lab or geological model
 locationnr        - # of drainage subarea
 horizonnr         - # of horizon
 lithologynr       - # of lithology
 areas             - the sizes of all the drainage subareas
 deltatime         - duration of a time step
 timestep          - the number of the timestep itself
 n2mass            - array for storing the mass data
 n2volume          - array for storing the volume data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered


--------------------------------------------------------------------------------
*/

static void  GxCalcN2MassVolume (GxModel *model, int locationnr,
                           int horizonnr,  int lithologynr,
                           double *areas,  double deltatime,
                           int timestep,   double *n2mass,
                           double *n2volume)
{
   GxSubModel   *submodel;
   double        expulsion;
   int           i;

   assert (model);
   assert (areas);
   assert (n2mass);
   assert (n2volume);
   assert (locationnr  >= 0);
   assert (horizonnr   >= 0);
   assert (lithologynr >= 0);
   assert (deltatime   >= 0.0);
   assert (timestep    >= 0);

   submodel = &model->submodel[locationnr][horizonnr][lithologynr];

   *n2mass   = 0.0;

   *n2volume = 0.0;

   if (submodel->netthickness > 0.0) {

      for (i=0; i<model->nspecies; i++) {

         if (GxIsN2Gas (&model->species[lithologynr][i])) {

            expulsion  = 0.0;
            if (model->upperbiot[lithologynr] >= 1.0) {
               expulsion = areas[locationnr] * deltatime *
                                     submodel->upwardflux[timestep][i];
            }
            if (model->lowerbiot[lithologynr] >= 1.0) {
               expulsion += areas[locationnr] * deltatime *
                                        submodel->downwardflux[timestep][i];
            }

            *n2mass   += expulsion;
            *n2volume += expulsion/model->species[lithologynr][i].density;
	 }
      }
   }
}


/*
--------------------------------------------------------------------------------
 GxCalcRockEval2Results - simulate Rock-Eval II experiment on model
--------------------------------------------------------------------------------

 GxCalcRockEval2Results performs an analysis of the model that is a
 simulation of the Rock-Eval II experiment performed in the laboratory. It is
 assumed that the model was run with the parameters of the Rock-Eval II
 laboratory experiment.

 Arguments
 ---------
 model       - model for which the Rock-Eval II results should be calculated
 subareanr   - number of subarea for which to sample concentrations
 horizonnr   - number of horizon for which to sample concentrations
 lithologynr - number of lithology for which to sample concentrations
 density     - the density of the rock
 toc         - the total organic carbon
 s1ptr       - pointer to the variable for the S1
 s2ptr       - pointer to the variable for the S2
 s1tocptr    - pointer to the variable for the S1 over TOC
 hiptr       - pointer to the variable for the HI
 tmaxptr     - pointer to the variable for the Tmax

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCalcRockEval2Results (GxModel *model, int subareanr,
                                    int horizonnr, int lithologynr,
                                    double density, double toc,
                                    double *s1ptr, double *s2ptr,
                                    double *s1tocptr, double *hiptr,
                                    double *tmaxptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxSubModel   *submodel;
   int          i, j, ninclude, *incspecies;
   double       s1, s2, tmax, deltatime, totalflux, peakflux, apparenttmax;

   if (!model || (subareanr < 0) || (subareanr >= model->nlocations)
      || (horizonnr < 0) || (horizonnr >= model->nhorizons)
      || (lithologynr < 0) || (lithologynr >= model->nlithologies)
      || (density <= 0.0) || (toc < 0.0) || (toc > 1.0)
      || !s1ptr || !s2ptr || !s1tocptr || !hiptr || !tmaxptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags
            & (GXP_TIME | GXP_TEMPERATURE | GXP_EXPULSIONFLUX))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the model has been run */
            submodel = &model->submodel[subareanr][horizonnr][lithologynr];
            if (submodel->netthickness <= 0.0) {
               rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
            } else {

/* ----------- Allocate a temporary array that identifies the species
 * ----------- that should be include in the analysis
 */
               rc = GxAllocArray (&incspecies, sizeof (int), 1,
                                  model->nspecies);
               if (!rc) {

/* -------------- Determine which species to include in the analysis */
                  ninclude = 0;
                  for (i=0; i<model->nspecies; i++) {
                     if (model->species[lithologynr][i].mobile
                       && GxIsHydrocarbon (&model->species[lithologynr][i])) {
                       incspecies[ninclude++] = i;
                     }
                  }

/* -------------- Initialize the running totals */
                  s1       = 0.0;
                  s2       = 0.0;
                  peakflux = 0.0;
                  tmax     = 0.0;

/* -------------- Perform the analysis */
                  for (i=1; i<=model->timestepnr; i++) {
                     deltatime = ABS (model->timehist[i] -
                                      model->timehist[i-1]);

/* ----------------- Calculate the total flux at this time step */
                     totalflux = 0.0;
                     for (j=0; j<ninclude; j++) {
                        totalflux += (submodel->upwardflux[i][incspecies[j]] +
                                    submodel->downwardflux[i][incspecies[j]]);
                     }

/* ----------------- Depending on temperature add expulsion to s1 or s2 */
                     if (submodel->temperaturehist[i] <=
                         GX_RE2CUTTEMPERATURE) {
                        s1 += totalflux * deltatime;
                     } else {
                        s2 += totalflux * deltatime;

/* -------------------- Determine peakflux temperature */
                        if (totalflux > peakflux) {
                           peakflux = totalflux;
                           tmax     = submodel->temperaturehist[i];
                        }
                     }
                  }

/* -------------- Free temporary array */
                  GxFreeArray (incspecies, 1, model->nspecies);

/* -------------- Make correction for apparent Tmax */
                  apparenttmax = GXC_ZEROCELCIUS +
                                 (tmax + 14.0 - GXC_ZEROCELCIUS) / 1.114;
                  if (!rc) {

/* ----------------- Calculate important parameters */
                     s1 /= submodel->netthickness * density / 1000.0;
                     s2 /= submodel->netthickness * density / 1000.0;
                     *s1ptr    = s1;
                     *s2ptr    = s2;
                     *s1tocptr = s1 / toc;
                     *hiptr    = s2 / toc;
                     *tmaxptr  = apparenttmax;
                  }
               }
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCreateModel - create a new model
--------------------------------------------------------------------------------

 GxCreateModel allocates memory for a new Genex model.

 Arguments
 ---------
 nelements     - number of elements in model
 nspecies      - number of different species in model
 nreactions    - the number of reactions in the reactions scheme
 nlocations    - number of locations in model
 nhorizons     - number of horizons to model for each location
 nlithologies  - number of lithologies to model for each horizon
 nnodes        - number of nodes in the model
 ntimesteps    - number of time steps for the model
 historyflags  - flags identifying the properties to be saved through time
 modelptr      - pointer for the model

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCreateModel (int nelements, int nspecies, int nreactions,
                           int nlocations, int nhorizons, int nlithologies,
                           int nnodes, int ntimesteps,
                           unsigned long historyflags, GxModel **modelptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxModel      *model=NULL;

   if ((nelements <= 0) || (nspecies <= 0) || (nlocations <= 0)
      || (nreactions <= 0) || (nhorizons <= 0) || (nlithologies <= 0)
      || (nnodes <= 0) || (ntimesteps <= 0) || !modelptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Allocate the model structure */
      model = ( GxModel * ) malloc (sizeof (GxModel));
      if (!model) {
         rc = GxError (GXE_MEMORY, GXT_MEMORY);
      } else {

/* ----- Initialize the fields of the model structure */
         model->nelements    = nelements;
         model->nspecies     = nspecies;
         model->nreactions   = nreactions;
         model->nlocations   = nlocations;
         model->nhorizons    = nhorizons;
         model->nlithologies = nlithologies;
         model->nslices      = nnodes + 1;
         model->ntimesteps   = ntimesteps;
         model->historyflags = historyflags;
         model->timestepnr   = -1;
         model->aromchain    = NULL;
         model->tphistories  = NULL;
         model->timehist     = NULL;
         model->crackingrate = NULL;
         model->equations    = NULL;
         model->conclumped   = NULL;

/* ----- Allocate array to hold the aromaticity of chains for lithologies */
         rc = GxAllocArray (&model->aromchain, sizeof (double),
                            1, nlithologies);

/* ----- Allocate the array for the time history */
         if (!rc && (historyflags & GXP_TIME))
            rc = GxAllocArray (&model->timehist, sizeof (double),
                               1, ntimesteps+1);

/* ----- Allocate the memory for the submodel data */
         if (!rc) rc = GxAllocSubModelArrays (model, historyflags);

/* ----- Allocate temporary arrays used during the modelling run */
         if (!rc) rc = GxAllocArray (&model->crackingrate, sizeof (double),
                                     1, nspecies);
         if (!rc) rc = GxAllocArray (&model->equations, sizeof (double), 2,
                                     5, model->nslices);
         if (!rc) rc = GxAllocArray (&model->conclumped, sizeof (double),
                                     2, model->nslices, GXN_LUMPED);
      }
   }
   if (rc) {
      if (model) GxDestroyModel (model);
   } else {
      *modelptr = model;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxDestroyModel - destroy an existing model
--------------------------------------------------------------------------------

 GxDestroyModel frees the memory that was allocated for a model.

 Arguments
 ---------
 model - model to be destroyed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxDestroyModel (GxModel *model)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!model) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Free the memory allocated for the temporary modelling arrays */
      if (model->crackingrate)
         GxFreeArray (model->crackingrate, 1, model->nspecies);
      if (model->equations)
         GxFreeArray (model->equations, 2, 5, model->nslices);
      if (model->conclumped)
         GxFreeArray (model->conclumped, 2, model->nslices, GXN_LUMPED);

/* -- Free the array for the time history */
      if (model->timehist)
         GxFreeArray (model->timehist, 1, model->ntimesteps+1);

/* -- Free the array for the aromaticity of chains for the lithologies */
      if (model->aromchain)
         GxFreeArray (model->aromchain, 1, model->nlithologies);

/* -- Free the memory allocated for the submodels */
      GxFreeSubModelArrays (model);

/* -- Free the model itself */
      free (model);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeSubModelArrays - free the arrays for all submodels
--------------------------------------------------------------------------------

 GxFreeSubModelArrays frees the arrays that were allocated for the
 submodels.

 Arguments
 ---------
 model - model for which the submodel arrays should be freed

--------------------------------------------------------------------------------
*/

static void GxFreeSubModelArrays (GxModel *model)
{
   int         i, j, k;
   GxSubModel  *submodel;

   assert (model);

   if (model->submodel && (model->nspecies > 0) && (model->nlocations > 0)
      && (model->nhorizons > 0) && (model->nlithologies > 0)
      && (model->nslices > 1)) {

/* -- Loop over all the submodels */
      for (i=0; i<model->nlocations; i++) {
         for (j=0; j<model->nhorizons; j++) {
            for (k=0; k<model->nlithologies; k++) {
               submodel = &model->submodel[i][j][k];

/* ----------- Free the array for the concentrations */
               if (submodel->concentration)
                  GxFreeArray (submodel->concentration, 2, model->nslices,
                               model->nspecies);

/* ----------- Free the array for the negative rate of generation */
               if (submodel->negativegenrate) 
                  GxFreeArray (submodel->negativegenrate, 2, model->nslices,
                               model->nspecies);

/* ----------- Free the array for the positive rate of generation */
               if (submodel->positivegenrate) 
                  GxFreeArray (submodel->positivegenrate, 2, model->nslices,
                               model->nspecies);

/* ----------- Free the array for the temperature history */
               if (submodel->temperaturehist)
                  GxFreeArray (submodel->temperaturehist, 1,
                               model->ntimesteps+1);

/* ----------- Free the array for the pressure history */
               if (submodel->pressurehist)
                  GxFreeArray (submodel->pressurehist, 1,
                               model->ntimesteps+1);

/* ----------- Free the arrays for the flux histories */
               if (submodel->upwardflux)
                  GxFreeArray (submodel->upwardflux, 2,
                               model->ntimesteps+1, model->nspecies);
               if (submodel->downwardflux)
                  GxFreeArray (submodel->downwardflux, 2,
                               model->ntimesteps+1, model->nspecies);

/* ----------- Free the array for the concentration history */
               if (submodel->concentrationhist)
                  GxFreeArray (submodel->concentrationhist, 2,
                               model->ntimesteps+1, model->nspecies);

/* ----------- Free the array for the kerogen conversion    */
	       if (submodel->kerogenconversion)
		  GxFreeArray (submodel->kerogenconversion, 1,
			       model->ntimesteps+1);

/* ----------- Free the array for the asphaltene conversion */
	       if (submodel->asphalteneconversion)
		  GxFreeArray (submodel->asphalteneconversion, 1,
			       model->ntimesteps+1);

/* ----------- Free the array for the generation history */
               if (submodel->generationhist)
                  GxFreeArray (submodel->generationhist, 2,
                               model->ntimesteps+1, model->nspecies);

/* ----------- Free the array for the isotope fractionation history */
               if (submodel->isotopehist)
                  GxFreeArray (submodel->isotopehist, 2,
                               model->ntimesteps+1, model->nspecies);
            }
         }
      }

/* -- Free the array for the submodels themselves */
      GxFreeArray (model->submodel, 3, model->nlocations, model->nhorizons,
                   model->nlithologies);
      model->submodel = NULL;
   }
}

/*
--------------------------------------------------------------------------------
 GxGetAPIHist - get the API gravity
--------------------------------------------------------------------------------

 GxGetAPIHist gets the data by using the volume and the mass of the expelled
 oil of the complete model.
 The charge can only be calculated if the expulsion fluxes are stored during
 the running of the model. This is defined in the call to the function
 GxCreateModel.

 Arguments
 ---------
 model           - the model for which the charge should be calculated
 begintime       - start time for retrieving data
 endtime         - end time
 areas		 - size of the drainage subareas
 api		 - array for storing api data


 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetAPIHist (GxModel *model, double begintime, double endtime,
                      double *areas, double *api )
{
   GxErrorCode  rc = GXE_NOERROR;
   double      *oilmass, *oilvolume;
   double       submass, subvolume, oildensity, deltatime;
   int          i, j, k, l, begintimestep, endtimestep, size;
   if (!model || !areas || !api) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

      if (!(model->historyflags & (GXP_TIME | GXP_EXPULSIONFLUX))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

            rc = GxGetNumberOfTimes (model, begintime, endtime, &size);
            if (!rc) rc = GxAllocArray (&oilmass, sizeof (double), 1, size);
            if (!rc) rc = GxAllocArray (&oilvolume, sizeof(double), 1, size);


            if (!rc) {
               for (i=0; i<size; i++) {
                  oilmass[i]   = 0.0;
                  oilvolume[i] = 0.0;
               }

/* ----------- Determine the begin and end time step number */
               rc = GxGetBeginEndStep (model, begintime, endtime, 
                                       &begintimestep, &endtimestep);


/* ------------------- Deal with the first timestep */
               deltatime = ABS(model->timehist[begintimestep]-begintime);
               for (j=0; j<model->nlocations; j++) {
                  for (k=0; k<model->nhorizons; k++) {
                     for (l=0; l<model->nlithologies; l++) {
                        GxCalcOilMassVolume (model, j, k, l, areas, deltatime,
                                           begintimestep, &submass, &subvolume);
                        oilmass[0]   += submass;
                        oilvolume[0] += subvolume;
                     }
                  }
               }
/* -------- Deal with the remaining time steps */
               for (i=begintimestep+1; i<=endtimestep; i++) {
		  oilmass[i-begintimestep]   = oilmass[i-begintimestep-1];
		  oilvolume[i-begintimestep] = oilvolume[i-begintimestep-1];
                  deltatime = ABS(model->timehist[i]  - model->timehist[i-1]);
                  for (j=0; j<model->nlocations; j++) {
                     for (k=0; k<model->nhorizons; k++) {
                        for (l=0; l<model->nlithologies; l++) {
                          GxCalcOilMassVolume (model, j, k, l, areas, deltatime,
                                                    i, &submass, &subvolume);
                          oilmass[i-begintimestep]   += submass;
                          oilvolume[i-begintimestep] += subvolume;
                        }
                     }
                  }
               }
               for (i=0; i<size; i++) {
		  if (oilvolume[i] > FLT_MIN) {
		     oildensity = oilmass[i] / oilvolume[i];
                     api[i] = GxAPIGravity (oildensity);
		  } else {
		     api[i] = 0.0;
		  }
               }
            }
            if (oilmass)
               GxFreeArray (oilmass, 1, size);
            if (oilvolume)
               GxFreeArray (oilvolume, 1, size);
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetAPIHistSelected - get the API gravity of selected submodels
--------------------------------------------------------------------------------

 GxGetAPIHistSelected gets the data by using the volume and the mass of the 
 expelled oil of the complete model.
 The charge can only be calculated if the expulsion fluxes are stored during
 the running of the model. This is defined in the call to the function
 GxCreateModel.

 Arguments
 ---------
 model           - the model for which the charge should be calculated
 submodelsel     - the selected submodels to be used
 begintime       - start time for retrieving data
 endtime         - end time
 areas		 - size of the drainage subareas
 api		 - array for storing api data


 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetAPIHistSelected (GxModel *model, List submodelsel,
                                  double begintime, double endtime,
                                  double *areas, double *api )
{
   GxErrorCode   rc = GXE_NOERROR;
   double       *oilmass, *oilvolume;
   double        submass, subvolume, oildensity, deltatime;
   GxSubModelId *submodelId;
   int           i, begintimestep, endtimestep, size;
   GxBool        submodelAvailable = GxFalse;
   GxSubModel   *submodel;

   if (!model || !submodelsel || !areas || !api) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

      if (!(model->historyflags & (GXP_TIME | GXP_EXPULSIONFLUX))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

            rc = GxGetNumberOfTimes (model, begintime, endtime, &size);
            if (!rc) rc = GxAllocArray (&oilmass, sizeof (double), 1, size);
            if (!rc) rc = GxAllocArray (&oilvolume, sizeof(double), 1, size);


            if (!rc) {
               for (i=0; i<size; i++) {
                  oilmass[i]   = 0.0;
                  oilvolume[i] = 0.0;
               }

/* ----------- Determine the begin and end time step number */
               rc = GxGetBeginEndStep (model, begintime, endtime, 
                                       &begintimestep, &endtimestep);


/* ------------------- Deal with the first timestep */
               deltatime = ABS(model->timehist[begintimestep]-begintime);
               submodelId = GetFirstFromList (submodelsel);
               while (submodelId) {
/* --------------- Check that the model has been run */
                   submodel = &model->submodel[submodelId->subareanr]
                                              [submodelId->horizonnr]
                                              [submodelId->lithologynr];
                   if (submodel->netthickness > 0.0) {
                       submodelAvailable = GxTrue;
                       GxCalcOilMassVolume (model, submodelId->subareanr,
                                            submodelId->horizonnr,
                                            submodelId->lithologynr, areas, 
                                            deltatime, begintimestep, 
                                            &submass, &subvolume);
                       oilmass[0]   += submass;
                       oilvolume[0] += subvolume;
                   }
                   submodelId = GetNextFromList (submodelsel);
               }

/* -------- Deal with the remaining time steps */
               for (i=begintimestep+1; i<=endtimestep; i++) {
		  oilmass[i-begintimestep]   = oilmass[i-begintimestep-1];
		  oilvolume[i-begintimestep] = oilvolume[i-begintimestep-1];
                  deltatime = ABS(model->timehist[i]  - model->timehist[i-1]);
                  submodelId = GetFirstFromList (submodelsel);
                  while (submodelId) {
/* ------------------ Check that the model has been run */
                      submodel = &model->submodel[submodelId->subareanr]
                                                 [submodelId->horizonnr]
                                                 [submodelId->lithologynr];
                      if (submodel->netthickness > 0.0) {
                          submodelAvailable = GxTrue;
                          GxCalcOilMassVolume (model, submodelId->subareanr,
                                               submodelId->horizonnr,
                                               submodelId->lithologynr, areas, 
                                               deltatime, i, &submass, 
                                               &subvolume);
                          oilmass[i-begintimestep]   += submass;
                          oilvolume[i-begintimestep] += subvolume;
                      }
                      submodelId = GetNextFromList (submodelsel);
                  }
               }
               for (i=0; i<size; i++) {
		  if (oilvolume[i] > FLT_MIN) {
		     oildensity = oilmass[i] / oilvolume[i];
                     api[i] = GxAPIGravity (oildensity);
		  } else {
		     api[i] = 0.0;
		  }
               }

               if (!submodelAvailable) {
                  rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
               }
            }
            if (oilmass)
               GxFreeArray (oilmass, 1, size);
            if (oilvolume)
               GxFreeArray (oilvolume, 1, size);
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetBeginEndStep - get the begin and end step index for specified range
--------------------------------------------------------------------------------

 GxGetBeginEndStep retrieves the begin and end step for the specified begin- 
 and endtime. The stepnumbers retrieved ensure that the specified range will
 be covered completely. 

 Arguments
 ---------
 model           - the model for which the charge should be calculated
 begintime       - the start time for which step number ust be retrieved
 endtime         - the end time for which step number ust be retrieved
 begintimestep   - retrieved begin time step index 
 endtimestep     - retrieved end time step index 

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetBeginEndStep (GxModel *model, double begintime, 
                               double endtime, int *begintimestep,
                               int *endtimestep)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!model || !begintimestep || !endtimestep) { 
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
/*--- Determine the begin and end time step number */
      *begintimestep = MAX (0, GxSearchDouble (model->timestepnr+1,
                            model->timehist, begintime));
      *endtimestep   = MIN (model->timestepnr,
                            GxSearchDouble (model->timestepnr+1,
                                            model->timehist, endtime));

/*--- Time specified by begintimestep must be equal or earlier than begintime*/
      if ( model->timehist[*begintimestep] < begintime ) {
          *begintimestep = MAX (0, *begintimestep - 1);
      }

/*--- Time specified by endtimestep must be equal or later than endtime   */
      if ( model->timehist[*endtimestep] > endtime ) {
          *endtimestep = MIN (model->timestepnr, *endtimestep + 1);
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetCharge - get the charge produced from the model
--------------------------------------------------------------------------------

 GxGetCharge returns the hydrocarbon charge information for the perfectly
 open and perfectly closed case since the time specified by <starttime>.
 The charge can only be calculated if the expulsion fluxes are stored during
 the running of the model. This is defined in the call to the function
 GxCreateModel.

 Arguments
 ---------
 model           - the model for which the charge should be calculated
 starttime       - the start time for the expulsion calculation
 areas           - array with the areas
 grossthickness  - array with the gross thickness
 porosity        - array with the porosity
 nettogross      - array with the net to gross
 traptemperature - temperature for the trap
 trappressure    - pressure for the trap
 trapdepth       - top depth of the trap
 salinity        - salinityof the water in the trap
 open            - structure for the open case charge data
 closed          - structure for the closed case charge data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetCharge (GxModel *model, double starttime, double *areas,
                         double *grossthickness, double *porosity,
                         double *nettogross, double traptemperature,
                         double trappressure, double trapdepth,
                         double salinity, GxCharge *open, GxCharge *closed)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, j;
   double       **expulsions=NULL, watervolume, waterdensity, waterweight,
                oilmass, oilvolume, gasmass, gasvolume, oildensity,
                gasdensity;
   double       n2mass, n2volume, n2density;

   if (!model || !areas || !grossthickness || !porosity
      || !nettogross || (traptemperature < 0.0) || (trappressure < 0.0)
      || (trapdepth < 0.0) || (salinity < 0.0) || (salinity > 1.0) || !open
      || !closed) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_EXPULSIONFLUX))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the start time is within modelled time range */
            if (!BETWEEN (starttime, model->timehist[0],
                          model->timehist[model->timestepnr])) {
               rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
            } else {

/* ----------- Allocate and fill an array with expulsion data */
               rc = GxAllocArray (&expulsions, sizeof (double), 2,
                                  model->nlithologies, model->nspecies);


               if (!rc) rc = GxGetTotMainExpulsion (model, starttime,
                                                    areas, expulsions);
               if (!rc) {

/* -------------- Initialize running totals for oil and gas and n2 */
                  oilmass   = 0.0;
                  oilvolume = 0.0;
                  gasmass   = 0.0;
                  gasvolume = 0.0;
                  n2mass    = 0.0;
                  n2volume  = 0.0;

/* -------------- Loop over all lithologies and species */
                  for (i=0; i<model->nlithologies; i++) {
                     for (j=0; j<model->nspecies; j++) {

/* -------------------- Add to oil or gas depending on composition/mobility */
                        if (model->species[i][j].mobile
                           && GxHasOilComposition (&model->species[i][j])) {
                           oilmass   += expulsions[i][j];
                           oilvolume += expulsions[i][j] /
                                        model->species[i][j].density;
                        } else if (GxIsHCGas (&model->species[i][j])) { 
                           gasmass   += expulsions[i][j];
                           gasvolume += expulsions[i][j] /
                                        model->species[i][j].density;
                        } else if ((GxNitrogenPartOfModel ()) &&
                                   (GxIsN2Gas (&model->species[i][j]))) {
                           n2mass    += expulsions[i][j];
                           n2volume  += expulsions[i][j] /
                                        model->species[i][j].density;
                        }
                     }
                  }
               }

/* ----------- Free the temporary expulsion array */
               if (expulsions) GxFreeArray (expulsions, 2,
                                  model->nlithologies, model->nspecies);

/* ----------- Derive the densities of oil, HC gas and N2 */
               oildensity = (oilvolume > FLT_MIN) ?
                            oilmass / oilvolume : GX_DEFAULTOILDENSITY;
               open->apigravity   = GxAPIGravity (oildensity);
               closed->apigravity = open->apigravity;
               gasdensity = (gasvolume > FLT_MIN) ?
                            gasmass / gasvolume : GX_DEFAULTGASDENSITY;
               n2density  = (n2volume > FLT_MIN) ?
                            n2mass / n2volume : GX_DEFAULTN2DENSITY;

/* ----------- Calculate the parameters for the water in the trap */
               watervolume = 0.0;
               for (i=0; i<model->nlocations; i++)
                  watervolume += grossthickness[i] * areas[i] * porosity[i] *
                                 nettogross[i];
               waterdensity = GxWaterDensity (traptemperature, trappressure,
                                              salinity);
               waterweight = watervolume * waterdensity;

/* ----------- Determine the pressures */
               open->fluidpressure = trapdepth * (waterdensity - 20.0) *
                                     GXC_GRAVITY;
               open->overpressure = 0.0;
               closed->overpressure = GxOverPressure (oilmass, gasmass, n2mass,
                                                      waterweight);
               closed->fluidpressure = open->fluidpressure +
                                       closed->overpressure;
               open->lithostaticpressure = trapdepth;
               open->lithostaticpressure *= model->overburdendensity;
               open->lithostaticpressure *= GXC_GRAVITY;

/* ----------- The next statement is added to solve the compiler problem */
/*             on the RS_6000. Npo problems occured with the previous    */
/*             release of AIX. But with this version 3.2.2 Genex crashes */
/*             On other machines like HP, SUN and Silicon Graphics, the  */
/*             program runs fine. I hope this is not a scary stack       */
/*             problem!!!                                                */
               printf ("", trapdepth); 

               closed->lithostaticpressure = open->lithostaticpressure;
               open->failpressure = GxFailPressure (
                                       (double)GXC_FAILURECRITERION,
                                       open->lithostaticpressure);
               closed->failpressure = open->failpressure;
               if (closed->fluidpressure >= closed->failpressure) {
                  closed->fluidpressure = closed->failpressure;
                  closed->overpressure = closed->fluidpressure -
                                         open->fluidpressure;
               }
/* ----------- Calculate GOR and CGR values */
               open->saturationgor = GxBubblePointGOR (oildensity, gasdensity,
                  traptemperature, open->fluidpressure);
               closed->saturationgor = GxBubblePointGOR (oildensity,
                  gasdensity, traptemperature, closed->fluidpressure);
               open->saturationcgr = GxSaturationCGR (traptemperature,
                  open->fluidpressure);
               closed->saturationcgr = GxSaturationCGR (traptemperature,
                  closed->fluidpressure);

/* ----------- Determine the amount of gas dissolved in water */
               open->gasinwater = GxGasInWater (open->saturationgor,
                  open->fluidpressure, salinity, gasvolume, oilvolume,
                  watervolume);
               closed->gasinwater = GxGasInWater (closed->saturationgor,
                  closed->fluidpressure, salinity, gasvolume, oilvolume,
                  watervolume);

/* ----------- Determine the amount of N2 dissolved in water */
               if ( GxNitrogenPartOfModel () ) {
                   open->n2inwater = GxN2InWater (open->fluidpressure, salinity,
                                                  n2volume, watervolume);
                   closed->n2inwater = GxN2InWater (closed->fluidpressure, 
                                                    salinity, n2volume, 
                                                    watervolume);
               }

/* ----------- Calculate the amount of gas left over */
               open->gasvolume1   = gasvolume - open->gasinwater;
               closed->gasvolume1 = gasvolume - closed->gasinwater;
               if (oilvolume > FLT_MIN) {
                  open->gor1   = open->gasvolume1 / oilvolume;
                  closed->gor1 = closed->gasvolume1 / oilvolume;
               } else {
                  open->gor1   = 0.0;
                  closed->gor1 = 0.0;
               }

/* ----------- Calculate the amount of N2 left over */
               open->n2volume1   = n2volume - open->n2inwater;
               closed->n2volume1 = n2volume - closed->n2inwater;

/* ----------- Now, determine the field type */
               open->fieldtype = GxDetermineFieldType (oilvolume,
                  open->gasvolume1, watervolume, open->gor1,
                  open->saturationgor, open->saturationcgr);
               closed->fieldtype = GxDetermineFieldType (oilvolume,
                  closed->gasvolume1, watervolume, closed->gor1,
                  closed->saturationgor, closed->saturationcgr);

/* ----------- Finally, fill in the remaining fields in the charge struct */
               open->watervolume      = watervolume;
               open->watermass        = waterweight;
               closed->watervolume    = watervolume;
               closed->watermass      = waterweight;
               open->gasmassinwater   = open->gasinwater * gasdensity;
               closed->gasmassinwater = closed->gasinwater * gasdensity;
               open->n2massinwater    = open->n2inwater * n2density;
               closed->n2massinwater  = closed->n2inwater * n2density;
               open->oilvolume1       = oilvolume;
               open->oilmass1         = oilmass;
               closed->oilvolume1     = oilvolume;
               closed->oilmass1       = oilmass;
               open->oilvolume2       = oilvolume * GXC_SM3TOMB;
               open->oilmass2         = oilmass * GXC_KGTOIMP;
               closed->oilvolume2     = oilvolume * GXC_SM3TOMB;
               closed->oilmass2       = oilmass * GXC_KGTOIMP;
               open->gasmass1         = open->gasvolume1 * gasdensity;
               open->gasmass2         = open->gasmass1 * GXC_KGTOIMP;
               open->gasvolume2       = open->gasvolume1 * GXC_SM3TOBCF;
               closed->gasmass1       = closed->gasvolume1 * gasdensity;
               closed->gasmass2       = closed->gasmass1 * GXC_KGTOIMP;
               closed->gasvolume2     = closed->gasvolume1 * GXC_SM3TOBCF;
               open->n2mass1          = open->n2volume1 * n2density;
               open->n2mass2          = open->n2mass1 * GXC_KGTOIMP;
               open->n2volume2        = open->n2volume1 * GXC_SM3TOBCF;
               closed->n2mass1        = closed->n2volume1 * n2density;
               closed->n2mass2        = closed->n2mass1 * GXC_KGTOIMP;
               closed->n2volume2      = closed->n2volume1 * GXC_SM3TOBCF;
               open->gor2             = open->gor1 * GXC_GORCONVERSION;
               closed->gor2           = closed->gor1 * GXC_GORCONVERSION;
               open->cgr              = 1000000.0 / open->gor1;
               closed->cgr            = 1000000.0 / closed->gor1;
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetConcentrationHist - get the concentration history from a submodel
--------------------------------------------------------------------------------

 GxGetConcentrationHist returns the concentration data that has been
 calculated for a species in a submodel. The concentration history can only
 be returned if it is stored during the running of the model. This is defined
 in the call to the function GxCreateModel.

 Arguments
 ---------
 model       - the model from which concentration data should be retrieved
 subareanr   - number of subarea from which to retrieve concentration data
 horizonnr   - number of horizon from which to retrieve concentration data
 lithologynr - number of lithology from which to retrieve concentration data
 speciesnr   - number of the species for which to retrieve concentrations
 begintime   - the begin time for the history
 endtime     - the end time for the history
 conchist    - array for the concentration history data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetConcentrationHist (GxModel *model, int subareanr,
                                    int horizonnr, int lithologynr,
                                    int speciesnr, double begintime,
                                    double endtime, double *conchist)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxSubModel   *submodel;
   int          i, begintimestep, endtimestep;

   if (!model || (subareanr < 0) || (subareanr >= model->nlocations)
      || (horizonnr < 0) || (horizonnr >= model->nhorizons)
      || (lithologynr < 0) || (lithologynr >= model->nlithologies)
      || (speciesnr < 0) || (speciesnr >= model->nspecies) || !conchist) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_CONCENTRATION))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the model has been run */
            submodel = &model->submodel[subareanr][horizonnr][lithologynr];
            if (submodel->netthickness <= 0.0) {
               rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
            } else {

/* ----------- Determine the begin and end time step number */
               rc = GxGetBeginEndStep (model, begintime, endtime, 
                                       &begintimestep, &endtimestep);

/* ----------- Copy the concentration for all time steps in the range */
               for (i=begintimestep; i<=endtimestep; i++)
                  conchist[i-begintimestep] =
                     submodel->concentrationhist[i][speciesnr];
            }
         }
      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxGetConversionHist - get the conversion history from a submodel
--------------------------------------------------------------------------------

GxGetConversionHist return the conversion data that has been
calculated in a submodel. The conversion history can only
be returned if it is stored during the running of the model. This
is defined in the call to the function GxCreateModel.

Arguments
---------
model          - the model from which conversion data should be retrieved
subareanr      - number of subarea from which to retrieve conversion data
horizonnr      - number of horizon from which to retrieve conversion data
lithologynr    - number of lithology from which to retrieve conversion data
begintime      - the begin time for the history
endtime        - the end time for the history
kerogenhist    - array for the kerogen conversion history data
asphaltenehist - array for the asphaltene conversion history data

Return value
------------
GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetConversionHist (GxModel *model, int subareanr,
                                 int horizonnr, int lithologynr,
                                 double begintime, double endtime,
				 double *kerogenhist, double *asphaltenehist)
{
    GxErrorCode  rc=GXE_NOERROR;
    GxSubModel   *submodel;
    int          i, begintimestep, endtimestep;

    if (!model || subareanr < 0 || (subareanr >= model->nlocations)
       || (horizonnr < 0) || (horizonnr >= model->nhorizons)
       || (lithologynr < 0) || (lithologynr >= model->nlithologies)
       || !kerogenhist || !asphaltenehist ) {
       rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
    } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_KEROGENCONVERSION |
	   			              GXP_ASPHALTENECONVERSION))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ---- Check that the model has been initialised */
	if (model->timestepnr < 0) {
	   rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
        } else {

/* -------- Check that the model has been run */
	    submodel = &model->submodel[subareanr][horizonnr][lithologynr];
	    if (submodel->netthickness <= 0.0) {
	       rc = GxError ( GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
            } else {

/* ----------- Determine the begin and time step number */
               rc = GxGetBeginEndStep (model, begintime, endtime, 
                                       &begintimestep, &endtimestep);

/* ----------- Copy the conversion for all time steps in the range */
	       for ( i=begintimestep; i<=endtimestep; i++) {
		  kerogenhist[i-begintimestep] = submodel->kerogenconversion[i];
                  asphaltenehist[i-begintimestep] =
		      submodel->asphalteneconversion[i];
               }
	    }
         }
      }
   }
   return (rc);
}					    


/*
--------------------------------------------------------------------------------
 GxGetExpIsotopeHist - get the expelled isotope history from a submodel
--------------------------------------------------------------------------------

 GxGetExpIsotopeHist returns the expelled isotope history data that has been
 calculated for a species in a submodel. The isotope history can only
 be returned if it is stored during the running of the model. This is defined
 in the call to the function GxCreateModel.

 Arguments
 ---------
 model       - the model from which generation data should be retrieved
 subareanr   - number of subarea from which to retrieve generation data
 horizonnr   - number of horizon from which to retrieve generation data
 lithologynr - number of lithology from which to retrieve generation data
 speciesnr   - number of the species for which to retrieve generations
 updown      - specifies expulsion data (up, down or both) to be returned
 begintime   - the begin time for the history
 endtime     - the end time for the history
 isohist     - array for the isotope history data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetExpIsotopeHist (GxModel *model, int subareanr, int horizonnr, 
                                 int lithologynr, int speciesnr, 
                                 GxUpDown updown,
                                 double begintime, double endtime, 
                                 double *isohist)
{
   GxErrorCode   rc=GXE_NOERROR;
   GxSubModel   *submodel;
   int           i, begintimestep, endtimestep;
   double        totalflux, weightedIsotope;

   if (!model || (subareanr < 0) || (subareanr >= model->nlocations)
      || (horizonnr < 0) || (horizonnr >= model->nhorizons)
      || (lithologynr < 0) || (lithologynr >= model->nlithologies)
      || (speciesnr < 0) || (speciesnr >= model->nspecies) || !isohist) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_ISOTOPE))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the model has been run */
            submodel = &model->submodel[subareanr][horizonnr][lithologynr];
            if (submodel->netthickness <= 0.0) {
               rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
            } else {
               isohist[0] = 0.0;
/* ----------- Determine the begin and end time step number */
               rc = GxGetBeginEndStep (model, begintime, endtime, 
                                       &begintimestep, &endtimestep);
       
               if (!rc) {
/* -------------- Copy the isotope history for all time steps in the range */
                  totalflux = 0.0;
                  weightedIsotope = 0.0;
                  switch (updown) {
                     case GX_UP:
/* ------------------- Get upward expulsion for all time steps in the range */
                       for (i=MAX (1,begintimestep); i<=endtimestep; i++) {
                          totalflux += submodel->upwardflux[i][speciesnr];
                          weightedIsotope += 
                                     submodel->upwardflux[i][speciesnr] *
                                     submodel->isotopehist[i][speciesnr];
                          if (totalflux == 0.0) {
                             isohist[i-begintimestep] = 0.0;
                          } else {
                             isohist[i-begintimestep] = weightedIsotope /
                                                        totalflux;
                          }
                       }
                       break;

                     case GX_DOWN:
/* ------------------- Get downward expulsion for all time steps in range */
                       for (i=MAX (1,begintimestep); i<=endtimestep; i++) {
                          totalflux += submodel->downwardflux[i][speciesnr];
                          weightedIsotope += 
                                     submodel->downwardflux[i][speciesnr] *
                                     submodel->isotopehist[i][speciesnr];
                          if (totalflux == 0.0) {
                             isohist[i-begintimestep] = 0.0;
                          } else {
                             isohist[i-begintimestep] = weightedIsotope /
                                                        totalflux;
                          }
                       }
                       break;

                     case GX_UPANDDOWN:
/* ------------------- Get total expulsion for all time steps in the range */
                       for (i=MAX (1,begintimestep); i<=endtimestep; i++) {
                          totalflux += submodel->upwardflux[i][speciesnr] +
                                       submodel->downwardflux[i][speciesnr];
                          weightedIsotope += 
                                     (submodel->upwardflux[i][speciesnr] +
                                      submodel->downwardflux[i][speciesnr]) *
                                     submodel->isotopehist[i][speciesnr];
                          if (totalflux == 0.0) {
                             isohist[i-begintimestep] = 0.0;
                          } else {
                             isohist[i-begintimestep] = weightedIsotope /
                                                        totalflux;
                          }
                       }
                       break;

                     default:
                       rc = GxError (GXE_INCONSIST, GXT_INCONSIST,
                                     __FILE__, __LINE__);
                       break;
                  }
               }
            }
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetExpulsionHist - get the expulsion history from a submodel
--------------------------------------------------------------------------------

 GxGetExpulsionHist returns the expulsion data that has been calculated for
 a species in a submodel. The expulsion history can only be returned if flux
 data is stored during the running of the model. This is defined in the call
 to the function GxCreateModel.

 Arguments
 ---------
 model         - the model from which the expulsion data should be retrieved
 subareanr     - number of subarea from which to retrieve expulsion data
 horizonnr     - number of horizon from which to retrieve expulsion data
 lithologynr   - number of lithology from which to retrieve expulsion data
 speciesnr     - number of the species for which to retrieve expulsion data
 updown        - specifies expulsion data (up, down or both) to be returned
 cumulative    - specifies whether history should be cumulative
 begintime     - the begin time for the history
 endtime       - the end time for the history
 expulsionhist - array for the expulsion history data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetExpulsionHist (GxModel *model, int subareanr, int horizonnr,
                                int lithologynr, int speciesnr,
                                GxUpDown updown, GxCumulative cumulative,
                                double begintime, double endtime,
                                double *expulsionhist)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxSubModel   *submodel;
   int          i, begintimestep, endtimestep;

   if (!model || (subareanr < 0) || (subareanr >= model->nlocations)
      || (horizonnr < 0) || (horizonnr >= model->nhorizons)
      || (lithologynr < 0) || (lithologynr >= model->nlithologies)
      || (speciesnr < 0) || (speciesnr >= model->nspecies)
      || !expulsionhist) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_EXPULSIONFLUX))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the model has been run */
            submodel = &model->submodel[subareanr][horizonnr][lithologynr];
            if (submodel->netthickness <= 0.0) {
               rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
            } else {

/* ----------- Determine the begin and end time step number */
               rc = GxGetBeginEndStep (model, begintime, endtime, 
                                       &begintimestep, &endtimestep);
               expulsionhist[0] = 0.0;

               switch (updown) {
                  case GX_UP:

/* ------------------- Get upward expulsion for all time steps in the range */
                       for (i=MAX (1,begintimestep); i<=endtimestep; i++)
                          expulsionhist[i-begintimestep] =
                             submodel->upwardflux[i][speciesnr] *
                             ABS (model->timehist[i] - model->timehist[i-1]);
                       break;

                  case GX_DOWN:

/* ------------------- Get downward expulsion for all time steps in range */
                       for (i=MAX (1,begintimestep); i<=endtimestep; i++)
                          expulsionhist[i-begintimestep] =
                             submodel->downwardflux[i][speciesnr] *
                             ABS (model->timehist[i] - model->timehist[i-1]);
                       break;

                  case GX_UPANDDOWN:

/* ------------------- Get total expulsion for all time steps in the range */
                       for (i=MAX (1,begintimestep); i<=endtimestep; i++)
                          expulsionhist[i-begintimestep] =
                             (submodel->upwardflux[i][speciesnr] +
                             submodel->downwardflux[i][speciesnr]) *
                             ABS (model->timehist[i] - model->timehist[i-1]);
                       break;

                  default:
                       rc = GxError (GXE_INCONSIST, GXT_INCONSIST,
                                     __FILE__, __LINE__);
                       break;
               }

/* ----------- If requested, make the expulsion history cumulative */
               if (!rc && (cumulative == GX_CUMULATIVE))
                  for (i=1; i<endtimestep-begintimestep+1; i++)
                     expulsionhist[i] += expulsionhist[i-1];
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetFluxHist - get the expulsion flux history from a submodel
--------------------------------------------------------------------------------

 GxGetFluxHist returns the expulsion flux data that has been calculated for
 a species in a submodel. The expulsion flux history can only be returned
 if it is stored during the running of the model. This is defined in the
 call to the function GxCreateModel.

 Arguments
 ---------
 model       - the model from which the expulsion flux should be retrieved
 subareanr   - number of subarea from which to retrieve expulsion flux
 horizonnr   - number of horizon from which to retrieve expulsion flux
 lithologynr - number of lithology from which to retrieve expulsion flux
 speciesnr   - number of the species for which to retrieve expulsion flux
 updown      - specifies expulsion flux (up, down or both) to be returned
 begintime   - the begin time for the history
 endtime     - the end time for the history
 fluxhist    - array for the expulsion flux history

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetFluxHist (GxModel *model, int subareanr, int horizonnr,
                           int lithologynr, int speciesnr, GxUpDown updown,
                           double begintime, double endtime,
                           double *fluxhist)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxSubModel   *submodel;
   int          i, begintimestep, endtimestep;

   if (!model || (subareanr < 0) || (subareanr >= model->nlocations)
      || (horizonnr < 0) || (horizonnr >= model->nhorizons)
      || (lithologynr < 0) || (lithologynr >= model->nlithologies)
      || (speciesnr < 0) || (speciesnr >= model->nspecies)
      || !fluxhist) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_EXPULSIONFLUX))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the model has been run */
            submodel = &model->submodel[subareanr][horizonnr][lithologynr];
            if (submodel->netthickness <= 0.0) {
               rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
            } else {

/* ----------- Determine the begin and end time step number */
               rc = GxGetBeginEndStep (model, begintime, endtime, 
                                       &begintimestep, &endtimestep);
               fluxhist[0] = 0.0;

               switch (updown) {
                  case GX_UP:

/* ------------------- Get the upward flux for all time steps in the range */
                       for (i=MAX (1,begintimestep); i<=endtimestep; i++)
                          fluxhist[i-begintimestep] =
                             submodel->upwardflux[i][speciesnr];
                       break;

                  case GX_DOWN:

/* ------------------- Get downward flux for all time steps in the range */
                       for (i=MAX (1,begintimestep); i<=endtimestep; i++)
                          fluxhist[i-begintimestep] =
                             submodel->downwardflux[i][speciesnr];
                       break;

                  case GX_UPANDDOWN:

/* ------------------- Get the total flux for all time steps in the range */
                       for (i=MAX (1,begintimestep); i<=endtimestep; i++)
                          fluxhist[i-begintimestep] =
                             (submodel->upwardflux[i][speciesnr] +
                              submodel->downwardflux[i][speciesnr]);
                       break;

                  default:
                       rc = GxError (GXE_INCONSIST, GXT_INCONSIST,
                                     __FILE__, __LINE__);
                       break;
               }
            }
         }
      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxGetGenerationHist - get the generation history from a submodel
--------------------------------------------------------------------------------

 GxGetGenerationHist returns the generation data that has been
 calculated for a species in a submodel. The generation history can only
 be returned if it is stored during the running of the model. This is defined
 in the call to the function GxCreateModel.

 Arguments
 ---------
 model       - the model from which generation data should be retrieved
 subareanr   - number of subarea from which to retrieve generation data
 horizonnr   - number of horizon from which to retrieve generation data
 lithologynr - number of lithology from which to retrieve generation data
 speciesnr   - number of the species for which to retrieve generations
 begintime   - the begin time for the history
 endtime     - the end time for the history
 genhist     - array for the generation history data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGenerationHist (GxModel *model, int subareanr,
                                 int horizonnr, int lithologynr,
                                 int speciesnr, GxCumulative cumulative,
                                 double begintime, double endtime, 
                                 double *genhist)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxSubModel   *submodel;
   int          i, begintimestep, endtimestep;

   if (!model || (subareanr < 0) || (subareanr >= model->nlocations)
      || (horizonnr < 0) || (horizonnr >= model->nhorizons)
      || (lithologynr < 0) || (lithologynr >= model->nlithologies)
      || (speciesnr < 0) || (speciesnr >= model->nspecies) || !genhist) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_GENERATION))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the model has been run */
            submodel = &model->submodel[subareanr][horizonnr][lithologynr];
            if (submodel->netthickness <= 0.0) {
               rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
            } else {
               genhist[0] = 0.0;
/* ----------- Determine the begin and end time step number */
               rc = GxGetBeginEndStep (model, begintime, endtime, 
                                       &begintimestep, &endtimestep);
       
               if (!rc) {
/* -------------- Copy the generation for all time steps in the range */
                  for (i=MAX (1,begintimestep); i<=endtimestep; i++) {
                     genhist[i-begintimestep] =
                        submodel->generationhist[i][speciesnr];
                     if (cumulative == GX_CUMULATIVE) {
                        genhist[i-begintimestep] *=
                           ABS (model->timehist[i] - model->timehist[i-1]);
                     }
                  }
                  if (cumulative == GX_CUMULATIVE) {
                     for (i=1; i<endtimestep-begintimestep+1; i++) 
                        genhist[i] += genhist[i-1];
                  }
               }
            }
         }
      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxGetGORHist - get the Gas Oil Ratio
--------------------------------------------------------------------------------

 GxGetGORHist gets the data by using the volume and the mass of the expelled
 oil and gas of the complete model. The gas volume will be corrected for
 the amount of gas that will solve in water.
 The charge can only be calculated if the expulsion fluxes are stored during
 the running of the model. This is defined in the call to the function
 GxCreateModel.

 Arguments
 ---------
 model           - the model for which the charge should be calculated
 begintime       - start time for retrieving data
 endtime         - end time
 areas		 - sizes of the drainage subareas
 grossthickness  - thickness 
 porosity        - porosity of the layer
 nettogross      - % of water in porose material
 temperature     - temperature of trap
 pressure        - pressure of trap
 depth           - topdepth of trap
 salinity        - salinity
 opengor         - gas oil ratio for open system
 closegor        - gas oil ratio for closed system


 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGORHist (GxModel *model, double begintime, double endtime,
                      double *areas, double *grossthickness, 
		      double *porosity, double *nettogross,
		      double temperature, double pressure,
                      double depth, double salinity, 
		      double *opengor, double *closegor )
{
   GxErrorCode  rc = GXE_NOERROR;
   double      *oilmass, *oilvolume, *gasmass, *gasvolume;
   double      *n2mass, *n2volume;
   double       submass, subvolume, oildensity, gasdensity,
                openfluidpressure, closefluidpressure, failpressure,
		waterdensity, waterweight, watervolume,
                openvolume, closevolume, deltatime;
   int          i, j, k, l, begintimestep, endtimestep, size;

   if (!model || !areas || !grossthickness || !porosity || !nettogross
      || (temperature < 0.0) || (pressure < 0.0) || (depth < 0.0) 
      || (salinity < 0.0)    || !opengor || !closegor ) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

      if (!(model->historyflags & (GXP_TIME | GXP_EXPULSIONFLUX))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

            watervolume = 0.0;
            for (i=0; i<model->nlocations; i++)
   	       watervolume += grossthickness[i] * areas[i] * 
					porosity[i] * nettogross[i];

            waterdensity = GxWaterDensity (temperature, pressure, salinity);
            waterweight  = watervolume * waterdensity;
            failpressure = GxFailPressure (GXC_FAILURECRITERION,
                               depth * model->overburdendensity * GXC_GRAVITY);

            openfluidpressure = depth * (waterdensity - 20.0) * GXC_GRAVITY;


            rc = GxGetNumberOfTimes (model, begintime, endtime, &size);
            if (!rc) rc = GxAllocArray (&oilmass,   sizeof(double), 1, size);
            if (!rc) rc = GxAllocArray (&gasmass,   sizeof(double), 1, size);
            if (!rc) rc = GxAllocArray (&n2mass,    sizeof(double), 1, size);
            if (!rc) rc = GxAllocArray (&oilvolume, sizeof(double), 1, size);
            if (!rc) rc = GxAllocArray (&gasvolume, sizeof(double), 1, size);
            if (!rc) rc = GxAllocArray (&n2volume,  sizeof(double), 1, size);

            if (!rc) {
               for (i=0; i<size; i++) {
                  oilmass[i]   = 0.0;
                  gasmass[i]   = 0.0;
                  n2mass[i]    = 0.0;
                  oilvolume[i] = 0.0;
                  gasvolume[i] = 0.0;
                  n2volume[i]  = 0.0;
               }

/* ----------- Determine the begin and end time step number */
               rc = GxGetBeginEndStep (model, begintime, endtime, 
                                       &begintimestep, &endtimestep);


/* ------------------- Deal with the first timestep */
               deltatime = ABS(model->timehist[begintimestep]-begintime);
               for (j=0; j<model->nlocations; j++) {
                  for (k=0; k<model->nhorizons; k++) {
                     for (l=0; l<model->nlithologies; l++) {
                        GxCalcOilMassVolume (model, j, k, l, areas, deltatime,
                                           begintimestep, &submass, &subvolume);
                        oilmass[0]   += submass;
                        oilvolume[0] += subvolume;
			GxCalcGasMassVolume (model, j, k, l, areas, deltatime,
					   begintimestep, &submass, &subvolume);
			gasmass[0]   += submass;
			gasvolume[0] += subvolume;
                        if ( GxNitrogenPartOfModel () ) {
                            GxCalcN2MassVolume (model, j, k, l, areas, 
                                                deltatime, begintimestep,
                                                &submass, &subvolume);
                            n2mass[0]   += submass;
                            n2volume[0] += subvolume;
                        }
                     }
                  }
               }
/* -------- Deal with the remaining time steps */
               for (i=begintimestep+1; i<=endtimestep; i++) {

                  oilmass[i-begintimestep] = oilmass[i-begintimestep-1];
                  gasmass[i-begintimestep] = gasmass[i-begintimestep-1];
                  n2mass[i-begintimestep]  = n2mass[i-begintimestep-1];
                  oilvolume[i-begintimestep] = oilvolume[i-begintimestep-1];
                  gasvolume[i-begintimestep] = gasvolume[i-begintimestep-1];
                  n2volume[i-begintimestep]  = n2volume[i-begintimestep-1];

                  deltatime = ABS(model->timehist[i]  - model->timehist[i-1]);
                  for (j=0; j<model->nlocations; j++) {
                     for (k=0; k<model->nhorizons; k++) {
                        for (l=0; l<model->nlithologies; l++) {
                          GxCalcOilMassVolume (model, j, k, l, areas, deltatime,
                                                    i, &submass, &subvolume);
                          oilmass[i-begintimestep]   += submass;
                          oilvolume[i-begintimestep] += subvolume;
                          GxCalcGasMassVolume (model, j, k, l, areas, deltatime,
                                                    i, &submass, &subvolume);
                          gasmass[i-begintimestep]   += submass;
                          gasvolume[i-begintimestep] += subvolume;
                          if ( GxNitrogenPartOfModel () ) {
                              GxCalcN2MassVolume (model, j, k, l, areas, 
                                                  deltatime, i, &submass, 
                                                  &subvolume);
                              n2mass[i-begintimestep]   += submass;
                              n2volume[i-begintimestep] += subvolume;
                          }
                        }
                     }
                  }
               }
               for (i=0; i<size; i++) {
                   oildensity = (oilvolume[i] > FLT_MIN) ? oilmass[i] / 
				 oilvolume[i] : GX_DEFAULTOILDENSITY;
                   gasdensity = (gasvolume[i] > FLT_MIN) ? gasmass[i] / 
                                 gasvolume[i] : GX_DEFAULTGASDENSITY;

                   openvolume = GxCalcFreeGas (oilvolume[i], oildensity, 
                                  gasvolume[i], gasdensity, watervolume, 
                                  temperature, salinity, openfluidpressure );

                   closefluidpressure = openfluidpressure + GxOverPressure (
                                        oilmass[i], gasmass[i], n2mass[i], 
                                        waterweight);
                   if (closefluidpressure >= failpressure) {
                      closefluidpressure = failpressure;
                   }
                   closevolume = GxCalcFreeGas (oilvolume[i], oildensity,  
                                    gasvolume[i], gasdensity, watervolume, 
                                    temperature, salinity, closefluidpressure );
                   if (oilvolume[i] > FLT_MIN) {
                      opengor[i]   = openvolume  / oilvolume[i];
                      closegor[i]  = closevolume / oilvolume[i];
                   } else {
		      if (i == 0) {
			 opengor[0]  = 0.0;
     			 closegor[0] = 0.0;
		      } else {
                         opengor[i]   = 1.0E+07;
                         closegor[i]  = 1.0E+07;
		      }
                   }
               }
            }
            if (oilmass)
               GxFreeArray (oilmass, 1, size);
            if (oilvolume)
               GxFreeArray (oilvolume, 1, size);
            if (gasmass)
               GxFreeArray (gasmass, 1, size);
            if (gasvolume)
               GxFreeArray (gasvolume, 1, size);
            if (n2mass)
               GxFreeArray (n2mass, 1, size);
            if (n2volume)
               GxFreeArray (n2volume, 1, size);
         }
      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxGetGORHistSelected - get the Gas Oil Ratio for selected submodels
--------------------------------------------------------------------------------

 GxGetGORHistSelected gets the data by using the volume and the mass of the 
 expelled oil and gas of the complete model. The gas volume will be corrected 
 for the amount of gas that will solve in water.
 The charge can only be calculated if the expulsion fluxes are stored during
 the running of the model. This is defined in the call to the function
 GxCreateModel.

 Arguments
 ---------
 model           - the model for which the charge should be calculated
 submodelsel     - the selected submodels to be used
 begintime       - start time for retrieving data
 endtime         - end time
 areas		 - sizes of the drainage subareas
 grossthickness  - thickness 
 porosity        - porosity of the layer
 nettogross      - % of water in porose material
 temperature     - temperature of trap
 pressure        - pressure of trap
 depth           - topdepth of trap
 salinity        - salinity
 opengor         - gas oil ratio for open system
 closegor        - gas oil ratio for closed system


 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGORHistSelected (GxModel *model, List locationsel,
                                  List submodelsel, double begintime, 
                                  double endtime, double *areas, 
                                  double *grossthickness, double *porosity, 
                                  double *nettogross, double temperature, 
                                  double pressure, double depth, 
                                  double salinity, double *opengor, 
                                  double *closegor )
{
   GxErrorCode   rc = GXE_NOERROR;
   double       *oilmass, *oilvolume, *gasmass, *gasvolume;
   double       *n2mass, *n2volume;
   double        submass, subvolume, oildensity, gasdensity,
                 openfluidpressure, closefluidpressure, failpressure,
                 waterdensity, waterweight, watervolume,
                 openvolume, closevolume, deltatime;
   int           i, begintimestep, endtimestep, size;
   GxSubModelId *submodelId;
   int          *locnr;
   GxBool        submodelAvailable = GxFalse;
   GxSubModel   *submodel;

   if (!model || !locationsel || !submodelsel || !areas || !grossthickness 
      || !porosity || !nettogross || (temperature < 0.0) || (pressure < 0.0) 
      || (depth < 0.0) || (salinity < 0.0)    || !opengor || !closegor ) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

      if (!(model->historyflags & (GXP_TIME | GXP_EXPULSIONFLUX))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

            watervolume = 0.0;
            locnr = GetFirstFromList (locationsel);
            while (locnr) {
   	       watervolume += grossthickness[*locnr] * areas[*locnr] * 
					porosity[*locnr] * nettogross[*locnr];
               locnr = GetNextFromList (locationsel);
            }

            waterdensity = GxWaterDensity (temperature, pressure, salinity);
            waterweight  = watervolume * waterdensity;
            failpressure = GxFailPressure (GXC_FAILURECRITERION,
                               depth * model->overburdendensity * GXC_GRAVITY);

            openfluidpressure = depth * (waterdensity - 20.0) * GXC_GRAVITY;


            rc = GxGetNumberOfTimes (model, begintime, endtime, &size);
            if (!rc) rc = GxAllocArray (&oilmass,   sizeof(double), 1, size);
            if (!rc) rc = GxAllocArray (&gasmass,   sizeof(double), 1, size);
            if (!rc) rc = GxAllocArray (&n2mass,    sizeof(double), 1, size);
            if (!rc) rc = GxAllocArray (&oilvolume, sizeof(double), 1, size);
            if (!rc) rc = GxAllocArray (&gasvolume, sizeof(double), 1, size);
            if (!rc) rc = GxAllocArray (&n2volume,  sizeof(double), 1, size);

            if (!rc) {
               for (i=0; i<size; i++) {
                  oilmass[i]   = 0.0;
                  gasmass[i]   = 0.0;
                  n2mass[i]    = 0.0;
                  oilvolume[i] = 0.0;
                  gasvolume[i] = 0.0;
                  n2volume[i]  = 0.0;
               }

/* ----------- Determine the begin and end time step number */
               rc = GxGetBeginEndStep (model, begintime, endtime, 
                                       &begintimestep, &endtimestep);


/* ------------------- Deal with the first timestep */
               deltatime = ABS(model->timehist[begintimestep]-begintime);
               submodelId = GetFirstFromList (submodelsel);
               while (submodelId) {
/* --------------- Check that the model has been run */
                   submodel = &model->submodel[submodelId->subareanr]
                                              [submodelId->horizonnr]
                                              [submodelId->lithologynr];
                   if (submodel->netthickness > 0.0) {
                       submodelAvailable = GxTrue;
                       GxCalcOilMassVolume (model, submodelId->subareanr, 
                                            submodelId->horizonnr, 
                                            submodelId->lithologynr, areas, 
                                            deltatime, begintimestep, &submass,
                                            &subvolume);
                       oilmass[0]   += submass;
                       oilvolume[0] += subvolume;
                       GxCalcGasMassVolume (model, submodelId->subareanr, 
                                            submodelId->horizonnr, 
                                            submodelId->lithologynr, areas, 
                                            deltatime, begintimestep, &submass,
                                            &subvolume);
                       gasmass[0]   += submass;
                       gasvolume[0] += subvolume;
                       if ( GxNitrogenPartOfModel () ) {
                           GxCalcN2MassVolume (model, submodelId->subareanr, 
                                               submodelId->horizonnr, 
                                               submodelId->lithologynr, areas, 
                                               deltatime, begintimestep,
                                               &submass, &subvolume);
                           n2mass[0]   += submass;
                           n2volume[0] += subvolume;
                       }
                   }
                   submodelId = GetNextFromList (submodelsel);
               }

/* -------- Deal with the remaining time steps */
               for (i=begintimestep+1; i<=endtimestep; i++) {

                  oilmass[i-begintimestep] = oilmass[i-begintimestep-1];
                  gasmass[i-begintimestep] = gasmass[i-begintimestep-1];
                  n2mass[i-begintimestep]  = n2mass[i-begintimestep-1];
                  oilvolume[i-begintimestep] = oilvolume[i-begintimestep-1];
                  gasvolume[i-begintimestep] = gasvolume[i-begintimestep-1];
                  n2volume[i-begintimestep]  = n2volume[i-begintimestep-1];

                  deltatime = ABS(model->timehist[i]  - model->timehist[i-1]);
                  submodelId = GetFirstFromList (submodelsel);
                  while (submodelId) {
/* ------------------ Check that the model has been run */
                      submodel = &model->submodel[submodelId->subareanr]
                                                 [submodelId->horizonnr]
                                                 [submodelId->lithologynr];
                      if (submodel->netthickness > 0.0) {
                          submodelAvailable = GxTrue;
                          GxCalcOilMassVolume (model, submodelId->subareanr,
                                               submodelId->horizonnr,
                                               submodelId->lithologynr,
                                               areas, deltatime, i, &submass, 
                                               &subvolume);
                          oilmass[i-begintimestep]   += submass;
                          oilvolume[i-begintimestep] += subvolume;
                          GxCalcGasMassVolume (model, submodelId->subareanr, 
                                               submodelId->horizonnr, 
                                               submodelId->lithologynr, 
                                               areas, deltatime, i, &submass, 
                                               &subvolume);
                          gasmass[i-begintimestep]   += submass;
                          gasvolume[i-begintimestep] += subvolume;
                          if ( GxNitrogenPartOfModel () ) {
                              GxCalcN2MassVolume (model, submodelId->subareanr,
                                                  submodelId->horizonnr,
                                                  submodelId->lithologynr,
                                                  areas, deltatime, i, 
                                                  &submass, &subvolume);
                              n2mass[i-begintimestep]   += submass;
                              n2volume[i-begintimestep] += subvolume;
                          }
                      }
                      submodelId = GetNextFromList (submodelsel);
                  }
               }
               for (i=0; i<size; i++) {
                   oildensity = (oilvolume[i] > FLT_MIN) ? oilmass[i] / 
				 oilvolume[i] : GX_DEFAULTOILDENSITY;
                   gasdensity = (gasvolume[i] > FLT_MIN) ? gasmass[i] / 
                                 gasvolume[i] : GX_DEFAULTGASDENSITY;

                   openvolume = GxCalcFreeGas (oilvolume[i], oildensity, 
                                  gasvolume[i], gasdensity, watervolume, 
                                  temperature, salinity, openfluidpressure );

                   closefluidpressure = openfluidpressure + GxOverPressure (
                                        oilmass[i], gasmass[i], n2mass[i], 
                                        waterweight);
                   if (closefluidpressure >= failpressure) {
                      closefluidpressure = failpressure;
                   }
                   closevolume = GxCalcFreeGas (oilvolume[i], oildensity,  
                                    gasvolume[i], gasdensity, watervolume, 
                                    temperature, salinity, closefluidpressure );
                   if (oilvolume[i] > FLT_MIN) {
                      opengor[i]   = openvolume  / oilvolume[i];
                      closegor[i]  = closevolume / oilvolume[i];
                   } else {
                      if (openvolume > FLT_MIN) {
		          if (i == 0) {
			     opengor[0]  = 0.0;
		          } else {
                             opengor[i]   = 1.0E+07;
		          }
                      } else {
                          opengor[i] = 0.0;
                      }
                      if (closevolume > FLT_MIN) {
		          if (i == 0) {
			     closegor[0]  = 0.0;
		          } else {
                             closegor[i]   = 1.0E+07;
		          }
                      } else {
                          closegor[i] = 0.0;
                      }
                   }
               }
               if (!submodelAvailable) {
                  rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
               }
            }
            if (oilmass)
               GxFreeArray (oilmass, 1, size);
            if (oilvolume)
               GxFreeArray (oilvolume, 1, size);
            if (gasmass)
               GxFreeArray (gasmass, 1, size);
            if (gasvolume)
               GxFreeArray (gasvolume, 1, size);
            if (n2mass)
               GxFreeArray (n2mass, 1, size);
            if (n2volume)
               GxFreeArray (n2volume, 1, size);
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetIsotopeHist - get the isotope history from a submodel
--------------------------------------------------------------------------------

 GxGetIsotopeHist returns the isotope history data that has been
 calculated for a species in a submodel. The isotope history can only
 be returned if it is stored during the running of the model. This is defined
 in the call to the function GxCreateModel.

 Arguments
 ---------
 model       - the model from which generation data should be retrieved
 subareanr   - number of subarea from which to retrieve generation data
 horizonnr   - number of horizon from which to retrieve generation data
 lithologynr - number of lithology from which to retrieve generation data
 speciesnr   - number of the species for which to retrieve generations
 begintime   - the begin time for the history
 endtime     - the end time for the history
 isohist     - array for the isotope history data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetIsotopeHist (GxModel *model, int subareanr, int horizonnr, 
                              int lithologynr, int speciesnr, double begintime,
                              double endtime, double *isohist)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxSubModel   *submodel;
   int          i, begintimestep, endtimestep;

   if (!model || (subareanr < 0) || (subareanr >= model->nlocations)
      || (horizonnr < 0) || (horizonnr >= model->nhorizons)
      || (lithologynr < 0) || (lithologynr >= model->nlithologies)
      || (speciesnr < 0) || (speciesnr >= model->nspecies) || !isohist) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_ISOTOPE))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the model has been run */
            submodel = &model->submodel[subareanr][horizonnr][lithologynr];
            if (submodel->netthickness <= 0.0) {
               rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
            } else {
               isohist[0] = 0.0;
/* ----------- Determine the begin and end time step number */
               rc = GxGetBeginEndStep (model, begintime, endtime, 
                                       &begintimestep, &endtimestep);
       
               if (!rc) {
/* -------------- Copy the isotope history for all time steps in the range */
                  for (i=MAX (1,begintimestep); i<=endtimestep; i++) {
                     isohist[i-begintimestep] =
                        submodel->isotopehist[i][speciesnr];
                  }
               }
            }
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetMassesHist - get the masses history from a submodel
--------------------------------------------------------------------------------

 GxGetMassesHist returns the masses that have been calculated for a
 submodel. The masses history can only be returned if the concentrations are
 stored during the running of the model. This is defined in the call to 
 the function GxCreateModel.

 Arguments
 ---------
 model        - the model from which the pressures should be retrieved
 subareanr    - number of subarea from which to retrieve pressures
 horizonnr    - number of horizon from which to retrieve pressures
 lithologynr  - number of lithology from which to retrieve pressures
 speciesnr    - Number of the specy
 begintime    - the begin time for the history
 endtime      - the end time for the history
 masses       - array for the masses history data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetMassesHist (GxModel *model, int subareanr, int horizonnr,
                             int lithologynr, int speciesnr,
                             double begintime, double endtime, double area,
                             double *masses )
{
   GxErrorCode  rc=GXE_NOERROR;
   GxSubModel  *submodel;
   int          i, begintimestep, endtimestep;

   if (!model || (subareanr < 0) || (subareanr >= model->nlocations)
      || (horizonnr < 0) || (horizonnr >= model->nhorizons)
      || (lithologynr < 0) || (lithologynr >= model->nlithologies)
      || (speciesnr < 0) || (speciesnr >= model->nspecies)
      || !masses) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_CONCENTRATION))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the model has been run */
            submodel = &model->submodel[subareanr][horizonnr][lithologynr];
            if (submodel->netthickness <= 0.0) {
            rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
            } else {

/* ----------- Determine the begin and end step number */
               rc = GxGetBeginEndStep (model, begintime, endtime, 
                                       &begintimestep, &endtimestep);

/* ----------- Copy the concentration for all time steps in the range */
               for (i=begintimestep; i<=endtimestep; i++)
                  masses[i-begintimestep] = submodel->concentrationhist[i][speciesnr] * submodel->netthickness;
            }
         }
      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxGetNumberOfTimes - get the the number of times from the model
--------------------------------------------------------------------------------

 GxGetNumberOfTimes returns the number of times at which the model properties
 have been calculated between the two times specified. The calculated number
 is used in the allocation of arrays for property histories.

 Arguments
 ---------
 model     - the model from which the number of times should be retrieved
 begintime - the begin time for the history
 endtime   - the end time for the history
 ntimesptr - pointer for number of times

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetNumberOfTimes (GxModel *model, double begintime,
                                double endtime, int *ntimesptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          begintimestep, endtimestep;

   if (!model || !ntimesptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & GXP_TIME)) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Determine the begin and end time step number */
         rc = GxGetBeginEndStep (model, begintime, endtime, &begintimestep,
                            &endtimestep);
         if (!rc) {
            *ntimesptr = MAX (0, endtimestep-begintimestep+1);
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetPressure - determine the pressure at a particular time
--------------------------------------------------------------------------------

 GxGetPressure determines the pressure at a particular time by linear
 interpolation of the time in the temperature/pressure history.

 Arguments
 ---------
 tphistory - temperature and pressure history
 time      - time for which pressure should be determined

 Return value
 ------------
 the pressure that was determined for the specified time

--------------------------------------------------------------------------------
*/

static double GxGetPressure (GxTPHistory *tphistory, double time)
{
   int     index;
   double  pressure;

   assert (tphistory);
   assert (tphistory->ntimes >= 2);

/* Search for the time in the temperature and pressure history */
   index = GxSearchDouble (tphistory->ntimes, tphistory->time, time);

/* Use linear interpolation to calculate the pressure */
   if (index <= 0) {
      pressure = tphistory->pressure[0];
   } else if (index > tphistory->ntimes-1) {
      pressure = tphistory->pressure[tphistory->ntimes-1];
   } else {
      pressure = GxLinearInterpolate (time, tphistory->time[index-1],
                                      tphistory->time[index],
                                      tphistory->pressure[index-1],
                                      tphistory->pressure[index]);
   }

/* Pressure should be at least the atmospheric pressure */
   return (MAX (GXC_ATMOSPHERICPRESSURE, pressure));
}



/*
--------------------------------------------------------------------------------
 GxGetPressureHist - get the pressure history from a submodel
--------------------------------------------------------------------------------

 GxGetPressureHist returns the pressures that have been calculated for a
 submodel. The pressure history can only be returned if it is stored during
 the running of the model. This is defined in the call to the function
 GxCreateModel.

 Arguments
 ---------
 model        - the model from which the pressures should be retrieved
 subareanr    - number of subarea from which to retrieve pressures
 horizonnr    - number of horizon from which to retrieve pressures
 lithologynr  - number of lithology from which to retrieve pressures
 begintime    - the begin time for the history
 endtime      - the end time for the history
 pressurehist - array for the pressure history data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetPressureHist (GxModel *model, int subareanr, int horizonnr,
                               int lithologynr, double begintime,
                               double endtime, double *pressurehist)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxSubModel   *submodel;
   int          begintimestep, endtimestep;

   if (!model || (subareanr < 0) || (subareanr >= model->nlocations)
      || (horizonnr < 0) || (horizonnr >= model->nhorizons)
      || (lithologynr < 0) || (lithologynr >= model->nlithologies)
      || !pressurehist) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_PRESSURE))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the model has been run */
            submodel = &model->submodel[subareanr][horizonnr][lithologynr];
            if (submodel->netthickness <= 0.0) {
               rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
            } else {

/* ----------- Determine the begin and end time step number */
               rc = GxGetBeginEndStep (model, begintime, endtime, 
                                       &begintimestep, &endtimestep);
               if (endtimestep-begintimestep+1 > 0)

/* -------------- Copy the pressure history with a memory copy */
                  memcpy (pressurehist,
                          &submodel->pressurehist[begintimestep],
                          (endtimestep-begintimestep+1) * sizeof (double));
            }
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetRatios - get the HC, OC and NC data from a submodel
--------------------------------------------------------------------------------

 GxGetRatios returns the HC, OC and NC data for a submodel. These values
 are calculated from the concentration of the species in the different time 
 steps.

 Arguments
 ---------
 model       - the model for which the Van Krevelen should be calculated
 subareanr   - number of subarea for which to calculate Van Krevelen
 horizonnr   - number of horizon for which to calculate Van Krevelen
 lithologynr - number of lithology for which to calculate Van Krevelen
 begintime   - the begin time for the history
 endtime     - the end time for the history
 hc          - Hydrogen over Carbon
 oc          - Oxygen over Carbon
 nc          - Nitrogen over Carbon

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetRatios ( GxModel *model, int subareanr, int horizonnr,
                          int lithologynr, double begintime, double endtime, 
                          double *hc, double *oc, double *nc )
{
   GxErrorCode  rc=GXE_NOERROR;
   GxSubModel  *submodel;
   double      *concentration, *atomich, *atomicc, *atomico, *atomicn;
   int          i, size;

int j;

   if (!model || (subareanr < 0) || (subareanr >= model->nlocations)
      || (horizonnr < 0) || (horizonnr >= model->nhorizons )
      || (lithologynr < 0) || (lithologynr >= model->nlithologies)
      || !hc || !oc || !nc) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the model has been run */
      submodel = &model->submodel[subareanr][horizonnr][lithologynr];

      if (submodel->netthickness <= 0.0) {
         rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
      } else {

/* ----- Get the size of the concentration history array */
         rc = GxGetNumberOfTimes (model, begintime, endtime, &size);
         if (!rc) {

/* --------- Allocate memory for concentration history */
             rc = GxAllocArray ( &concentration, sizeof (double), 1, size);
             if (!rc) 
                rc = GxAllocArray (&atomich, sizeof(double), 1, size );
             if (!rc) 
                rc = GxAllocArray (&atomicc, sizeof(double), 1, size );
             if (!rc) 
                rc = GxAllocArray (&atomico, sizeof(double), 1, size );
             if (!rc) 
                rc = GxAllocArray (&atomicn, sizeof(double), 1, size );
             if (!rc) {
                for (i = 0; i < size; i++) {
                   atomich[i] = 0.0;
                   atomicc[i] = 0.0;
                   atomico[i] = 0.0;
                   atomicn[i] = 0.0;
                }


/* ------------ Check all species and calculate HC, OC & NC if immobile or
                species is asphaltene */
                i = 0;
                while (!rc && i < model->nspecies ) {
 
                   if ((!model->species[lithologynr][i].mobile) || 
                       (GxIsAsphaltene ( &(model->species[lithologynr][i]) ))) {
/* ------------------ Get the concentration history for species i */
                      rc = GxGetConcentrationHist ( model, subareanr,
                                           horizonnr, lithologynr, i,
                                           begintime, endtime, concentration );

                      if (!rc) {
                          GxCalcRatios (&model->species[lithologynr][i],
                                        size, concentration, atomich, 
                                        atomicc, atomico, atomicn );
                      }
                   }
		   i++;
                }
                for (i = 0; i < size; i++) {
                   hc[i] = (atomicc[i] > FLT_MIN) ? 
                               atomich[i] / atomicc[i] : 0.0;
                   oc[i] = (atomicc[i] > FLT_MIN) ? 
                               atomico[i] / atomicc[i] : 0.0;
                   nc[i] = (atomicc[i] > FLT_MIN) ? 
                               atomicn[i] / atomicc[i] : 0.0;
                }
             }
             if (atomich)
                GxFreeArray (atomich, 1, size);
             if (atomicc)
                GxFreeArray (atomicc, 1, size);
             if (atomico)
                GxFreeArray (atomico, 1, size);
             if (atomicn)
                GxFreeArray (atomicn, 1, size);
             if (concentration)
                GxFreeArray (concentration, 1, size);
          }
       }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetRetIsotopeHist - get the retainded isotope history from a submodel
--------------------------------------------------------------------------------

 GxGetRetIsotopeHist returns the retained isotope history data that has been
 calculated for a species in a submodel. The isotope history can only
 be returned if it is stored during the running of the model. This is defined
 in the call to the function GxCreateModel.

 Arguments
 ---------
 model       - the model from which generation data should be retrieved
 subareanr   - number of subarea from which to retrieve generation data
 horizonnr   - number of horizon from which to retrieve generation data
 lithologynr - number of lithology from which to retrieve generation data
 speciesnr   - number of the species for which to retrieve generations
 begintime   - the begin time for the history
 endtime     - the end time for the history
 isohist     - array for the isotope history data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetRetIsotopeHist (GxModel *model, int subareanr, int horizonnr, 
                              int lithologynr, int speciesnr, double begintime,
                              double endtime, double *isohist)
{
   GxErrorCode   rc=GXE_NOERROR;
   GxSubModel   *submodel;
   int           i, begintimestep, endtimestep;

   if (!model || (subareanr < 0) || (subareanr >= model->nlocations)
      || (horizonnr < 0) || (horizonnr >= model->nhorizons)
      || (lithologynr < 0) || (lithologynr >= model->nlithologies)
      || (speciesnr < 0) || (speciesnr >= model->nspecies) || !isohist) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_ISOTOPE))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the model has been run */
            submodel = &model->submodel[subareanr][horizonnr][lithologynr];
            if (submodel->netthickness <= 0.0) {
               rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
            } else {
               isohist[0] = 0.0;
/* ----------- Determine the begin and end time step number */
               rc = GxGetBeginEndStep (model, begintime, endtime, 
                                       &begintimestep, &endtimestep);
       
               if (!rc) {
/* -------------- Copy the isotope history for all time steps in the range */
                  for (i=MAX (1,begintimestep); i<=endtimestep; i++) {
                     isohist[i-begintimestep] =
                        submodel->isotopehist[i][speciesnr];
                  }
               }
            }
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetTemperature - determine the temperature at a particular time
--------------------------------------------------------------------------------

 GxGetTemperature determines the temperature at a particular time by linear
 interpolation of the time in the temperature/pressure history.

 Arguments
 ---------
 tphistory - temperature and pressure history
 time      - time for which temperature should be determined

 Return value
 ------------
 the temperature that was determined for the specified time

--------------------------------------------------------------------------------
*/

static double GxGetTemperature (GxTPHistory *tphistory, double time)
{
   int     index;
   double  temperature;

   assert (tphistory);
   assert (tphistory->ntimes >= 2);

/* Search for the time in the temperature and pressure history */
   index = GxSearchDouble (tphistory->ntimes, tphistory->time, time);

/* Use linear interpolation to calculate the temperature */
   if (index <= 0) {
      temperature = tphistory->temperature[0];
   } else if (index > tphistory->ntimes-1) {
      temperature = tphistory->temperature[tphistory->ntimes-1];
   } else {
      temperature = GxLinearInterpolate (time, tphistory->time[index-1],
                                         tphistory->time[index],
                                         tphistory->temperature[index-1],
                                         tphistory->temperature[index]);
   }
   return (temperature);
}



/*
--------------------------------------------------------------------------------
 GxGetTemperatureHist - get the temperature history from a submodel
--------------------------------------------------------------------------------

 GxGetTemperatureHist returns the temperatures that have been calculated for
 a submodel. The temperature history can only be returned if it is stored
 during the running of the model. This is defined in the call to the function
 GxCreateModel.

 Arguments
 ---------
 model           - the model from which temperatures should be retrieved
 subareanr       - number of subarea from which to retrieve temperatures
 horizonnr       - number of horizon from which to retrieve temperatures
 lithologynr     - number of lithology from which to retrieve temperatures
 begintime       - the begin time for the history
 endtime         - the end time for the history
 temperaturehist - array for the temperature history data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetTemperatureHist (GxModel *model, int subareanr,
                                  int horizonnr, int lithologynr,
                                  double begintime, double endtime,
                                  double *temperaturehist)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxSubModel   *submodel;
   int          begintimestep, endtimestep;

   if (!model || (subareanr < 0) || (subareanr >= model->nlocations)
      || (horizonnr < 0) || (horizonnr >= model->nhorizons)
      || (lithologynr < 0) || (lithologynr >= model->nlithologies)
      || !temperaturehist) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_TEMPERATURE))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the model has been run */
            submodel = &model->submodel[subareanr][horizonnr][lithologynr];
            if (submodel->netthickness <= 0.0) {
               rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
            } else {

/* ----------- Determine the begin and end time step number */
               rc = GxGetBeginEndStep (model, begintime, endtime, 
                                       &begintimestep, &endtimestep);
               if (endtimestep-begintimestep+1 > 0)

/* -------------- Copy the temperature history with a memory copy */
                  memcpy (temperaturehist,
                          &submodel->temperaturehist[begintimestep],
                          (endtimestep-begintimestep+1) * sizeof (double));
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetTime - determine the next time for the model
--------------------------------------------------------------------------------

 GxGetTime determines the next time the model should step to from the
 begin and end time, the number of time steps that should be performed,
 and the number of the current time step.

 Arguments
 ---------
 begintime  - the begin time
 endtime    - the end time
 ntimesteps - number of time steps to be performed
 timestepnr - number of the current time step

 Return value
 ------------
 the time the model should step to

--------------------------------------------------------------------------------
*/

static double GxGetTime (double begintime, double endtime, int ntimesteps,
                         int timestepnr)
{
   assert (ntimesteps > 0);
   assert ((timestepnr >= 0) && (timestepnr <= ntimesteps));

   return (GxLinearInterpolate ((double) timestepnr, 0.0, (double) ntimesteps,
                                begintime, endtime));
}



/*
--------------------------------------------------------------------------------
 GxGetTimeHist - get the time history from the model
--------------------------------------------------------------------------------

 GxGetTimeHist returns the times at which the model properties have been
 calculated. The time history can only be returned if it is stored during the
 running of the model. This is defined in the call to the function
 GxCreateModel.

 Arguments
 ---------
 model     - the model from which the times should be retrieved
 begintime - the begin time for the history
 endtime   - the end time for the history
 timehist  - array for the time history data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetTimeHist (GxModel *model, double begintime, double endtime,
                           double *timehist)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          begintimestep, endtimestep;

   if (!model || !timehist) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & GXP_TIME)) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Determine the begin and end time step number */
            rc = GxGetBeginEndStep (model, begintime, endtime, &begintimestep,
                                    &endtimestep);

            if (!rc && (endtimestep-begintimestep+1 > 0)) 

/* ----------- Copy the time history with a memory copy */
               memcpy (timehist, &model->timehist[begintimestep],
                       (endtimestep-begintimestep+1) * sizeof (double));
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetTimeRange - get the time range from the model
--------------------------------------------------------------------------------

 GxGetTimeRange returns the begin and end times at which the model properties
 have been calculated. The time range can only be returned if it the time
 history is stored during the running of the model. This is defined in the
 call to the function GxCreateModel.

 Arguments
 ---------
 model        - the model from which the time range should be retrieved
 begintimeptr - pointer for the begin time
 endtimeptr   - pointer for the end time

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetTimeRange (GxModel *model, double *begintimeptr,
                            double *endtimeptr)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!model || !begintimeptr || !endtimeptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & GXP_TIME)) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {
            *begintimeptr = model->timehist[0];
            *endtimeptr   = model->timehist[model->timestepnr];
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetTotDownExpulsion - get the total downwards expulsion
--------------------------------------------------------------------------------

 GxGetTotDownExpulsion returns the downwards expulsion for the complete
 model after the <starttime>. The expulsion can only be returned if the
 expulsion fluxes are stored during the running of the model. This is defined
 in the call to the function GxCreateModel.

 Arguments
 ---------
 model     - the model for which the expulsion should be calculated
 starttime - the start time for the expulsion calculation
 areas     - array with the areas for the subarea
 expulsion - array for the expulsion data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetTotDownExpulsion (GxModel *model, double starttime,
                                   double *areas, double **expulsion)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, j, k, l, m, mstart;
   double       deltatime;
   GxSubModel   *submodel;

   if (!model || !areas || !expulsion) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_EXPULSIONFLUX))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the start time is within modelled time range */
            if (!BETWEEN (starttime, model->timehist[0],
                          model->timehist[model->timestepnr])) {
               rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
            } else {

/* ----------- Initialize the expulsion array */
               for (i=0; i<model->nlithologies; i++) {
                  for (j=0; j<model->nspecies; j++) {
                     expulsion[i][j] = 0.0;
                  }
               }

/* ----------- Determine the start time step */
               mstart = GxSearchDouble (model->timestepnr+1, model->timehist,
                                        starttime);

/* ----------- Loop over all submodel */
               for (i=0; i<model->nlocations; i++) {
                  for (j=0; j<model->nhorizons; j++) {
                     for (k=0; k<model->nlithologies; k++) {

/* -------------------- Take only submodels that have been run into account */
                        submodel = &model->submodel[i][j][k];
                        if (submodel->netthickness > 0.0) {

/* ----------------------- Deal with the first (partial) time step */
                           deltatime = ABS (model->timehist[mstart] -
                                            starttime);
                           for (l=0; l<model->nspecies; l++) {
                              expulsion[k][l] += areas[i] * deltatime *
                                 submodel->downwardflux[mstart][l];
                           }

/* ----------------------- Deal with the remaining time steps */
                           for (m=mstart+1; m<=model->timestepnr; m++) {
                              deltatime = ABS (model->timehist[m] -
                                               model->timehist[m-1]);
                              for (l=0; l<model->nspecies; l++) {
                                 expulsion[k][l] += areas[i] * deltatime *
                                    submodel->downwardflux[m][l];
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetTotDownExpulsionSelected - get the total downwards expulsion for the
                                 selected submodels
--------------------------------------------------------------------------------

 GxGetTotDownExpulsionSelected returns the downwards expulsion for the 
 selected submodels after the <starttime>. The expulsion can only be returned 
 if the expulsion fluxes are stored during the running of the model. This is 
 defined in the call to the function GxCreateModel.

 Arguments
 ---------
 model       - the model for which the expulsion should be calculated
 submodelsel - the selected submodels to be used
 starttime   - the start time for the expulsion calculation
 areas       - array with the areas for the subarea
 expulsion   - array for the expulsion data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetTotDownExpulsionSelected (GxModel *model, List submodelsel,
                                           double starttime, double *areas, 
                                           double **expulsion)
{
   GxErrorCode   rc=GXE_NOERROR;
   int           i, j, m, mstart;
   double        deltatime;
   GxSubModel   *submodel;
   GxSubModelId *submodelId;
   GxBool        submodelAvailable = GxFalse;

   if (!model || !submodelsel || !areas || !expulsion) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_EXPULSIONFLUX))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the start time is within modelled time range */
            if (!BETWEEN (starttime, model->timehist[0],
                          model->timehist[model->timestepnr])) {
               rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
            } else {

/* ----------- Initialize the expulsion array */
               for (i=0; i<model->nlithologies; i++) {
                  for (j=0; j<model->nspecies; j++) {
                     expulsion[i][j] = 0.0;
                  }
               }

/* ----------- Determine the start time step */
               mstart = GxSearchDouble (model->timestepnr+1, model->timehist,
                                        starttime);

/* ----------- Loop over all selected submodels */
               submodelId = GetFirstFromList (submodelsel);
               while (submodelId) {
/* --------------- Take only submodels that have been run into account */
                   submodel = &model->submodel[submodelId->subareanr]
                                              [submodelId->horizonnr]
                                              [submodelId->lithologynr];
                   if (submodel->netthickness > 0.0) {
                       submodelAvailable = GxTrue;
/* ------------------- Deal with the first (partial) time step */
                       deltatime = ABS (model->timehist[mstart] - starttime);
                       for (i=0; i<model->nspecies; i++) {
                           expulsion[submodelId->lithologynr][i] += 
                               areas[submodelId->subareanr] * deltatime *
                                 submodel->downwardflux[mstart][i];
                       }

/* ------------------- Deal with the remaining time steps */
                       for (m=mstart+1; m<=model->timestepnr; m++) {
                           deltatime = ABS (model->timehist[m] -
                                               model->timehist[m-1]);
                           for (i=0; i<model->nspecies; i++) {
                               expulsion[submodelId->lithologynr][i] += 
                                   areas[submodelId->subareanr] * deltatime *
                                    submodel->downwardflux[m][i];
                           }
                       }
                   }
                   submodelId = GetNextFromList (submodelsel);
               }
               if (!submodelAvailable) {
                  rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
               }
            }
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetTotDownIsotopesSelected - get the total downwards isotopes for the
                                selected submodels
--------------------------------------------------------------------------------

 GxGetTotDownIsotopesSelected returns the downwards expulsion for the 
 selected submodels after the <starttime>. The isotopes can only be returned 
 if the expulsion fluxes and isotope history are stored during the running of 
 the model. This is defined in the call to the function GxCreateModel.

 Arguments
 ---------
 model       - the model for which the expulsion should be calculated
 submodelsel - the selected submodels to be used
 starttime   - the start time for the expulsion calculation
 areas       - array with the areas for the subarea
 isotope     - array for the isotope data
 expulsion   - array for the expulsion data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetTotDownIsotopeSelected (GxModel *model, List submodelsel,
                                         double starttime, double *areas, 
                                         double **isotope, double **expulsion)
{
   GxErrorCode   rc=GXE_NOERROR;
   int           i, j, m, mstart;
   double        deltatime;
   double        deltaexp;
   double        totalexp;
   double        totaliso;
   GxSubModel   *submodel;
   GxSubModelId *submodelId;
   GxBool        submodelAvailable = GxFalse;

   if (!model || !submodelsel || !areas || !expulsion) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_EXPULSIONFLUX))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the start time is within modelled time range */
            if (!BETWEEN (starttime, model->timehist[0],
                          model->timehist[model->timestepnr])) {
               rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
            } else {

/* ----------- Initialize the isotope array */
               for (i=0; i<model->nlithologies; i++) {
                  for (j=0; j<model->nspecies; j++) {
                     isotope[i][j]   = 0.0;
                     expulsion[i][j] = 0.0;
                  }
               }

/* ----------- Determine the start time step */
               mstart = GxSearchDouble (model->timestepnr+1, model->timehist,
                                        starttime);

/* ----------- Loop over all selected submodels */
               submodelId = GetFirstFromList (submodelsel);
               while (submodelId) {
/* --------------- Take only submodels that have been run into account */
                   submodel = &model->submodel[submodelId->subareanr]
                                              [submodelId->horizonnr]
                                              [submodelId->lithologynr];
                   if (submodel->netthickness > 0.0) {
                       submodelAvailable = GxTrue;
/* ------------------- Deal with the first (partial) time step */
                       deltatime = ABS (model->timehist[mstart] - starttime);
                       for (i=0; i<model->nspecies; i++) {
                           totalexp = expulsion[submodelId->lithologynr][i];
                           deltaexp = areas[submodelId->subareanr] * deltatime 
                                      * submodel->downwardflux[mstart][i];
                           totaliso = isotope[submodelId->lithologynr][i];
                           expulsion[submodelId->lithologynr][i] += deltaexp; 
                           if (expulsion[submodelId->lithologynr][i] != 0.0) {
                              isotope[submodelId->lithologynr][i] =
                                 (submodel->isotopehist[mstart][i] * deltaexp +
                                  totaliso * totalexp) / 
                                  expulsion[submodelId->lithologynr][i];
                           }
                       }

/* ------------------- Deal with the remaining time steps */
                       for (m=mstart+1; m<=model->timestepnr; m++) {
                           deltatime = ABS (model->timehist[m] -
                                               model->timehist[m-1]);
                           for (i=0; i<model->nspecies; i++) {
                              totalexp = expulsion[submodelId->lithologynr][i];
                              deltaexp = areas[submodelId->subareanr] * 
                                         deltatime * 
                                         submodel->downwardflux[m][i];
                              totaliso = isotope[submodelId->lithologynr][i];
                              expulsion[submodelId->lithologynr][i] += 
                                                                      deltaexp; 
                              if (expulsion[submodelId->lithologynr][i] 
                                   != 0.0) {
                                 isotope[submodelId->lithologynr][i] =
                                        (submodel->isotopehist[m][i] * 
                                         deltaexp + totaliso * totalexp) / 
                                         expulsion[submodelId->lithologynr][i];
                              }
                           }
                       }
                   }
                   submodelId = GetNextFromList (submodelsel);
               }
               if (!submodelAvailable) {
                  rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
               }
            }
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetTotGeneration - get the total generation 
--------------------------------------------------------------------------------

 GxGetTotGeneration returns the generation for the complete model after the 
 <starttime>. The generation can only be returned if the generation history 
 is stored during the running of the model. This is defined in the call to 
 the function GxCreateModel.

 Arguments
 ---------
 model      - the model for which the generation should be calculated
 starttime  - the start time for the generation calculation
 areas      - array with the areas for the subarea
 generation - array for the generation data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetTotGeneration (GxModel *model, double starttime,
                                double *areas, double **generation)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, j, k, l, m, mstart;
   double       deltatime;
   GxSubModel   *submodel;

   if (!model || !areas || !generation) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_GENERATION))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the start time is within modelled time range */
            if (!BETWEEN (starttime, model->timehist[0],
                          model->timehist[model->timestepnr])) {
               rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
            } else {

/* ----------- Initialize the generation array */
               for (i=0; i<model->nlithologies; i++) {
                  for (j=0; j<model->nspecies; j++) {
                     generation[i][j] = 0.0;
                  }
               }

/* ----------- Determine the start time step */
               mstart = GxSearchDouble (model->timestepnr+1, model->timehist,
                                        starttime);

/* ----------- Loop over all submodel */
               for (i=0; i<model->nlocations; i++) {
                  for (j=0; j<model->nhorizons; j++) {
                     for (k=0; k<model->nlithologies; k++) {

/* -------------------- Take only submodels that have been run into account */
                        submodel = &model->submodel[i][j][k];
                        if (submodel->netthickness > 0.0) {

/* ----------------------- Deal with the first (partial) time step */
                           deltatime = ABS (model->timehist[mstart] -
                                            starttime);
                           for (l=0; l<model->nspecies; l++) {
                              generation[k][l] += areas[i] * deltatime *
                                    submodel->generationhist[mstart][l];
                           }

/* ----------------------- Deal with the remaining time steps */
                           for (m=mstart+1; m<=model->timestepnr; m++) {
                              deltatime = ABS (model->timehist[m] -
                                               model->timehist[m-1]);
                              for (l=0; l<model->nspecies; l++) {
                                 generation[k][l] += areas[i] * deltatime *
                                     submodel->generationhist[m][l];
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetTotGenerationSelected - get the total generation of selected submodels 
--------------------------------------------------------------------------------

 GxGetTotGenerationSelected returns the generation for the all selected 
 submodel after the <starttime>. The generation can only be returned if the 
 generation history is stored during the running of the model. This is defined 
 in the call to the function GxCreateModel.

 Arguments
 ---------
 model       - the model for which the generation should be calculated
 submodelsel - the selected submodels to be used
 starttime   - the start time for the generation calculation
 areas       - array with the areas for the subarea
 generation  - array for the generation data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetTotGenerationSelected (GxModel *model, List submodelsel, 
                                        double starttime, double *areas, 
                                        double **generation)
{
   GxErrorCode   rc=GXE_NOERROR;
   int           i, j, m, mstart;
   double        deltatime;
   GxSubModel   *submodel;
   GxSubModelId *submodelId;
   GxBool        submodelAvailable = GxFalse;

   if (!model || !submodelsel ||!submodelsel || !areas || !generation) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_GENERATION))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the start time is within modelled time range */
            if (!BETWEEN (starttime, model->timehist[0],
                          model->timehist[model->timestepnr])) {
               rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
            } else {

/* ----------- Initialize the generation array */
               for (i=0; i<model->nlithologies; i++) {
                  for (j=0; j<model->nspecies; j++) {
                     generation[i][j] = 0.0;
                  }
               }

/* ----------- Determine the start time step */
               mstart = GxSearchDouble (model->timestepnr+1, model->timehist,
                                        starttime);

/* ----------- Loop over all selected submodel */
               submodelId = GetFirstFromList (submodelsel);
               while (submodelId) {
/* --------------- Take only submodels that have been run into account */
                   submodel = &model->submodel[submodelId->subareanr]
                                              [submodelId->horizonnr]
                                              [submodelId->lithologynr];
                   if (submodel->netthickness > 0.0) {
                       submodelAvailable = GxTrue;
/* ------------------- Deal with the first (partial) time step */
                       deltatime = ABS (model->timehist[mstart] -
                                        starttime);
                       for (i=0; i<model->nspecies; i++) {
                           generation[submodelId->lithologynr][i] += 
                               areas[submodelId->subareanr] * deltatime *
                               submodel->generationhist[mstart][i];
                       }

/* ------------------- Deal with the remaining time steps */
                       for (m=mstart+1; m<=model->timestepnr; m++) {
                           deltatime = ABS (model->timehist[m] -
                                               model->timehist[m-1]);
                           for (i=0; i<model->nspecies; i++) {
                               generation[submodelId->lithologynr][i] +=
                                   areas[submodelId->subareanr] * deltatime *
                                   submodel->generationhist[m][i];
                           }
                       }
                   }
                   submodelId = GetNextFromList (submodelsel);
               }
               if (!submodelAvailable) {
                  rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
               }
            }
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetTotMainExpulsion - get the total expulsion into the main sms
--------------------------------------------------------------------------------

 GxGetTotMainExpulsion returns the expulsion into the main secondary migration
 system for the complete model after the <starttime>. The expulsion can only
 be returned if the expulsion fluxes are stored during the running of the
 model. This is defined in the call to the function GxCreateModel.

 Arguments
 ---------
 model     - the model for which the expulsion should be calculated
 starttime - the start time for the expulsion calculation
 areas     - array with the areas for the subarea
 expulsion - array for the expulsion data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetTotMainExpulsion (GxModel *model, double starttime,
                                   double *areas, double **expulsion)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, j, k, l, m, mstart;
   double       deltatime;
   GxSubModel   *submodel;

   if (!model || !areas || !expulsion) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_EXPULSIONFLUX))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the start time is within modelled time range */
            if (!BETWEEN (starttime, model->timehist[0],
                          model->timehist[model->timestepnr])) {
               rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
            } else {

/* ----------- Initialize the expulsion array */
               for (i=0; i<model->nlithologies; i++) {
                  for (j=0; j<model->nspecies; j++) {
                     expulsion[i][j] = 0.0;
                  }
               }

/* ----------- Determine the start time step */
               mstart = GxSearchDouble (model->timestepnr+1, model->timehist,
                                        starttime);

/* ----------- Loop over all submodel */
               for (i=0; i<model->nlocations; i++) {
                  for (j=0; j<model->nhorizons; j++) {
                     for (k=0; k<model->nlithologies; k++) {

/* -------------------- Take only submodels that have been run into account */
                        submodel = &model->submodel[i][j][k];
                        if (submodel->netthickness > 0.0) {

/* ----------------------- Deal with the first (partial) time step */
                           deltatime = ABS (model->timehist[mstart] -
                                            starttime);
                           for (l=0; l<model->nspecies; l++) {
                              if (model->upperbiot[k] >= 1.0)
                                 expulsion[k][l] += areas[i] * deltatime *
                                    submodel->upwardflux[mstart][l];
                              if (model->lowerbiot[k] >= 1.0)
                                 expulsion[k][l] += areas[i] * deltatime *
                                    submodel->downwardflux[mstart][l];
                           }

/* ----------------------- Deal with the remaining time steps */
                           for (m=mstart+1; m<=model->timestepnr; m++) {
                              deltatime = ABS (model->timehist[m] -
                                               model->timehist[m-1]);
                              for (l=0; l<model->nspecies; l++) {
                                 if (model->upperbiot[k] >= 1.0)
                                    expulsion[k][l] += areas[i] *
                                       deltatime * submodel->upwardflux[m][l];
                                 if (model->lowerbiot[k] >= 1.0)
                                    expulsion[k][l] += areas[i] *
                                     deltatime * submodel->downwardflux[m][l];
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetTotMainExpulsionSelected - get the total expulsion of selected submodels
                                 into the main sms
--------------------------------------------------------------------------------

 GxGetTotMainExpulsionSelected returns the expulsion into the main secondary 
 migration system for the selected submodels after the <starttime>. The 
 expulsion can only be returned if the expulsion fluxes are stored during the 
 running of the model. This is defined in the call to the function 
 GxCreateModel.

 Arguments
 ---------
 model       - the model for which the expulsion should be calculated
 submodelsel - the selected submodels to be used
 starttime   - the start time for the expulsion calculation
 areas       - array with the areas for the subarea
 expulsion   - array for the expulsion data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetTotMainExpulsionSelected (GxModel *model, List submodelsel,
                                           double starttime, double *areas, 
                                           double **expulsion)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, j, m, mstart;
   double       deltatime;
   GxSubModel   *submodel;
   GxSubModelId *submodelId;
   GxBool        submodelAvailable = GxFalse;

   if (!model || !submodelsel || !areas || !expulsion) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_EXPULSIONFLUX))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the start time is within modelled time range */
            if (!BETWEEN (starttime, model->timehist[0],
                          model->timehist[model->timestepnr])) {
               rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
            } else {

/* ----------- Initialize the expulsion array */
               for (i=0; i<model->nlithologies; i++) {
                  for (j=0; j<model->nspecies; j++) {
                     expulsion[i][j] = 0.0;
                  }
               }

/* ----------- Determine the start time step */
               mstart = GxSearchDouble (model->timestepnr+1, model->timehist,
                                        starttime);

/* ----------- Loop over all selected submodel */
               submodelId = GetFirstFromList (submodelsel);
               while (submodelId) {
/* --------------- Take only submodels that have been run into account */
                   submodel = &model->submodel[submodelId->subareanr]
                                              [submodelId->horizonnr]
                                              [submodelId->lithologynr];
                   if (submodel->netthickness > 0.0) {
                       submodelAvailable = GxTrue;
/* ------------------- Deal with the first (partial) time step */
                       deltatime = ABS (model->timehist[mstart] - starttime);
                       for (i=0; i<model->nspecies; i++) {
                           if (model->upperbiot[submodelId->lithologynr] 
                               >= 1.0) {
                               expulsion[submodelId->lithologynr][i] += 
                                   areas[submodelId->subareanr] * deltatime *
                                   submodel->upwardflux[mstart][i];
                           }
                           if (model->lowerbiot[submodelId->lithologynr] 
                               >= 1.0) {
                               expulsion[submodelId->lithologynr][i] += 
                                   areas[submodelId->subareanr] * deltatime *
                                   submodel->downwardflux[mstart][i];
                           }
                       }

/* ------------------- Deal with the remaining time steps */
                       for (m=mstart+1; m<=model->timestepnr; m++) {
                           deltatime = ABS (model->timehist[m] -
                                               model->timehist[m-1]);
                           for (i=0; i<model->nspecies; i++) {
                               if (model->upperbiot[submodelId->lithologynr] 
                                   >= 1.0) {
                                   expulsion[submodelId->lithologynr][i] += 
                                       areas[submodelId->subareanr] * 
                                       deltatime * 
                                       submodel->upwardflux[m][i];
                               }
                               if (model->lowerbiot[submodelId->lithologynr] 
                                   >= 1.0) {
                                   expulsion[submodelId->lithologynr][i] += 
                                       areas[submodelId->subareanr] * 
                                       deltatime * 
                                       submodel->downwardflux[m][i];
                               }
                           }
                       }
                   }
                   submodelId = GetNextFromList (submodelsel);
               }
               if (!submodelAvailable) {
                  rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
               }
            }
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetTotMainIsotopeSelected - get the total isotopes of selected submodels
                               into the main sms
--------------------------------------------------------------------------------

 GxGetTotMainIsotopeSelected returns the isotopes into the main secondary 
 migration system for the selected submodels after the <starttime>. The 
 isotopes can only be returned if the expulsion and isotopes history are stored
 during the running of the model. This is defined in the call to the function 
 GxCreateModel.

 Arguments
 ---------
 model       - the model for which the expulsion should be calculated
 submodelsel - the selected submodels to be used
 starttime   - the start time for the expulsion calculation
 areas       - array with the areas for the subarea
 isotope     - array for the isotope data
 expulsion   - array for the expulsion data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetTotMainIsotopeSelected (GxModel *model, List submodelsel,
                                         double starttime, double *areas, 
                                         double **isotope, double **expulsion)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, j, m, mstart;
   double       deltatime;
   double       deltaexp;
   double       totalexp;
   double       totaliso;
   GxSubModel   *submodel;
   GxSubModelId *submodelId;
   GxBool        submodelAvailable = GxFalse;

   if (!model || !submodelsel || !areas || !expulsion) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_EXPULSIONFLUX))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the start time is within modelled time range */
            if (!BETWEEN (starttime, model->timehist[0],
                          model->timehist[model->timestepnr])) {
               rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
            } else {

/* ----------- Initialize the expulsion and isotope array */
               for (i=0; i<model->nlithologies; i++) {
                  for (j=0; j<model->nspecies; j++) {
                     expulsion[i][j] = 0.0;
                     isotope[i][j]   = 0.0;
                  }
               }

/* ----------- Determine the start time step */
               mstart = GxSearchDouble (model->timestepnr+1, model->timehist,
                                        starttime);

/* ----------- Loop over all selected submodel */
               submodelId = GetFirstFromList (submodelsel);
               while (submodelId) {
/* --------------- Take only submodels that have been run into account */
                   submodel = &model->submodel[submodelId->subareanr]
                                              [submodelId->horizonnr]
                                              [submodelId->lithologynr];
                   if (submodel->netthickness > 0.0) {
                       submodelAvailable = GxTrue;
/* ------------------- Deal with the first (partial) time step */
                       deltatime = ABS (model->timehist[mstart] - starttime);
                       for (i=0; i<model->nspecies; i++) {
                           totalexp = expulsion[submodelId->lithologynr][i];
                           totaliso = isotope[submodelId->lithologynr][i];
                           deltaexp = 0.0;
                           if (model->upperbiot[submodelId->lithologynr] 
                               >= 1.0) {
                               deltaexp += 
                                   areas[submodelId->subareanr] * deltatime *
                                   submodel->upwardflux[mstart][i];
                           }
                           if (model->lowerbiot[submodelId->lithologynr] 
                               >= 1.0) {
                               deltaexp += 
                                   areas[submodelId->subareanr] * deltatime *
                                   submodel->downwardflux[mstart][i];
                           }
                           expulsion[submodelId->lithologynr][i] += deltaexp; 
                           if (expulsion[submodelId->lithologynr][i] != 0.0) {
                              isotope[submodelId->lithologynr][i] =
                                 (submodel->isotopehist[mstart][i] * deltaexp +
                                  totaliso * totalexp) / 
                                 expulsion[submodelId->lithologynr][i]; 
                           }
                       }

/* ------------------- Deal with the remaining time steps */
                       for (m=mstart+1; m<=model->timestepnr; m++) {
                           deltatime = ABS (model->timehist[m] -
                                               model->timehist[m-1]);
                           for (i=0; i<model->nspecies; i++) {
                               totalexp = expulsion[submodelId->lithologynr][i];
                               totaliso = isotope[submodelId->lithologynr][i];
                               deltaexp = 0.0;
                               if (model->upperbiot[submodelId->lithologynr] 
                                   >= 1.0) {
                                   deltaexp += 
                                       areas[submodelId->subareanr] * 
                                       deltatime * 
                                       submodel->upwardflux[m][i];
                               }
                               if (model->lowerbiot[submodelId->lithologynr] 
                                   >= 1.0) {
                                   deltaexp += 
                                       areas[submodelId->subareanr] * 
                                       deltatime * 
                                       submodel->downwardflux[m][i];
                               }
                               expulsion[submodelId->lithologynr][i] += 
                                                                     deltaexp; 
                               if (expulsion[submodelId->lithologynr][i] 
                                   != 0.0) {
                                  isotope[submodelId->lithologynr][i] =
                                     (submodel->isotopehist[m][i] * deltaexp +
                                      totaliso * totalexp) / 
                                     expulsion[submodelId->lithologynr][i]; 
                               }
                           }
                       }
                   }
                   submodelId = GetNextFromList (submodelsel);
               }
               if (!submodelAvailable) {
                  rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
               }
            }
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetTotUpExpulsion - get the total upwards expulsion
--------------------------------------------------------------------------------

 GxGetTotUpExpulsion returns the upwards expulsion for the complete
 model after the <starttime>. The expulsion can only be returned if the
 expulsion fluxes are stored during the running of the model. This is defined
 in the call to the function GxCreateModel.

 Arguments
 ---------
 model     - the model for which the expulsion should be calculated
 starttime - the start time for the expulsion calculation
 areas     - array with the areas for the subarea
 expulsion - array for the expulsion data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetTotUpExpulsion (GxModel *model, double starttime,
                                 double *areas, double **expulsion)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, j, k, l, m, mstart;
   double       deltatime;
   GxSubModel   *submodel;

   if (!model || !areas || !expulsion) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_EXPULSIONFLUX))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the start time is within modelled time range */
            if (!BETWEEN (starttime, model->timehist[0],
                          model->timehist[model->timestepnr])) {
               rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
            } else {

/* ----------- Initialize the expulsion array */
               for (i=0; i<model->nlithologies; i++) {
                  for (j=0; j<model->nspecies; j++) {
                     expulsion[i][j] = 0.0;
                  }
               }

/* ----------- Determine the start time step */
               mstart = GxSearchDouble (model->timestepnr+1, model->timehist,
                                        starttime);

/* ----------- Loop over all submodel */
               for (i=0; i<model->nlocations; i++) {
                  for (j=0; j<model->nhorizons; j++) {
                     for (k=0; k<model->nlithologies; k++) {

/* -------------------- Take only submodels that have been run into account */
                        submodel = &model->submodel[i][j][k];
                        if (submodel->netthickness > 0.0) {

/* ----------------------- Deal with the first (partial) time step */
                           deltatime = ABS (model->timehist[mstart] -
                                            starttime);
                           for (l=0; l<model->nspecies; l++) {
                              expulsion[k][l] += areas[i] * deltatime *
                                 submodel->upwardflux[mstart][l];
                           }

/* ----------------------- Deal with the remaining time steps */
                           for (m=mstart+1; m<=model->timestepnr; m++) {
                              deltatime = ABS (model->timehist[m] -
                                               model->timehist[m-1]);
                              for (l=0; l<model->nspecies; l++) {
                                 expulsion[k][l] += areas[i] * deltatime *
                                    submodel->upwardflux[m][l];
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetTotUpExpulsionSelected - get the total upwards expulsion for selected
                               submodels
--------------------------------------------------------------------------------

 GxGetTotUpExpulsionSelected returns the upwards expulsion for the selected
 submodels after the <starttime>. The expulsion can only be returned if the
 expulsion fluxes are stored during the running of the model. This is defined
 in the call to the function GxCreateModel.

 Arguments
 ---------
 model       - the model for which the expulsion should be calculated
 submodelsel - the selected submodels to be used
 starttime   - the start time for the expulsion calculation
 areas       - array with the areas for the subarea
 expulsion   - array for the expulsion data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetTotUpExpulsionSelected (GxModel *model, List submodelsel,
                                         double starttime, double *areas, 
                                         double **expulsion)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, j, m, mstart;
   double       deltatime;
   GxSubModel   *submodel;
   GxSubModelId *submodelId;
   GxBool        submodelAvailable = GxFalse;

   if (!model || !submodelsel || !areas || !expulsion) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_EXPULSIONFLUX))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the start time is within modelled time range */
            if (!BETWEEN (starttime, model->timehist[0],
                          model->timehist[model->timestepnr])) {
               rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
            } else {

/* ----------- Initialize the expulsion array */
               for (i=0; i<model->nlithologies; i++) {
                  for (j=0; j<model->nspecies; j++) {
                     expulsion[i][j] = 0.0;
                  }
               }

/* ----------- Determine the start time step */
               mstart = GxSearchDouble (model->timestepnr+1, model->timehist,
                                        starttime);

/* ----------- Loop over all selected submodel */
               submodelId = GetFirstFromList (submodelsel);
               while (submodelId) {
/* --------------- Take only submodels that have been run into account */
                   submodel = &model->submodel[submodelId->subareanr]
                                              [submodelId->horizonnr]
                                              [submodelId->lithologynr];
                   if (submodel->netthickness > 0.0) {
                       submodelAvailable = GxTrue;
/* ------------------- Deal with the first (partial) time step */
                       deltatime = ABS (model->timehist[mstart] - starttime);
                       for (i=0; i<model->nspecies; i++) {
                           expulsion[submodelId->lithologynr][i] += 
                               areas[submodelId->subareanr] * deltatime *
                               submodel->upwardflux[mstart][i];
                       }

/* ------------------- Deal with the remaining time steps */
                       for (m=mstart+1; m<=model->timestepnr; m++) {
                           deltatime = ABS (model->timehist[m] -
                                               model->timehist[m-1]);
                           for (i=0; i<model->nspecies; i++) {
                               expulsion[submodelId->lithologynr][i] += 
                                   areas[submodelId->subareanr] * deltatime *
                                   submodel->upwardflux[m][i];
                           }
                       }
                   }
                   submodelId = GetNextFromList (submodelsel);
               }
               if (!submodelAvailable) {
                  rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
               }
            }
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetTotUpIsotopeSelected - get the total upwards isotopes for selected
                             submodels
--------------------------------------------------------------------------------

 GxGetTotUpIsotopesSelected returns the upwards isotopes for the selected
 submodels after the <starttime>. The isotopes can only be returned if the
 expulsion fluxes and isotope history are stored during the running of the 
 model. This is defined in the call to the function GxCreateModel.

 Arguments
 ---------
 model       - the model for which the expulsion should be calculated
 submodelsel - the selected submodels to be used
 starttime   - the start time for the expulsion calculation
 areas       - array with the areas for the subarea
 isotope     - array for the isotope data
 expulsion   - array for the expulsion data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetTotUpIsotopeSelected (GxModel *model, List submodelsel,
                                       double starttime, double *areas, 
                                       double **isotope, double **expulsion)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, j, m, mstart;
   double       deltatime;
   double       deltaexp;
   double       totalexp;
   double       totaliso;
   GxSubModel   *submodel;
   GxSubModelId *submodelId;
   GxBool        submodelAvailable = GxFalse;

   if (!model || !submodelsel || !areas || !expulsion) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_EXPULSIONFLUX))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the start time is within modelled time range */
            if (!BETWEEN (starttime, model->timehist[0],
                          model->timehist[model->timestepnr])) {
               rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
            } else {

/* ----------- Initialize the isotope and expulsion array */
               for (i=0; i<model->nlithologies; i++) {
                  for (j=0; j<model->nspecies; j++) {
                     expulsion[i][j] = 0.0;
                     isotope[i][j]   = 0.0;
                  }
               }

/* ----------- Determine the start time step */
               mstart = GxSearchDouble (model->timestepnr+1, model->timehist,
                                        starttime);

/* ----------- Loop over all selected submodel */
               submodelId = GetFirstFromList (submodelsel);
               while (submodelId) {
/* --------------- Take only submodels that have been run into account */
                   submodel = &model->submodel[submodelId->subareanr]
                                              [submodelId->horizonnr]
                                              [submodelId->lithologynr];
                   if (submodel->netthickness > 0.0) {
                       submodelAvailable = GxTrue;
/* ------------------- Deal with the first (partial) time step */
                       deltatime = ABS (model->timehist[mstart] - starttime);
                       for (i=0; i<model->nspecies; i++) {
                           totalexp = expulsion[submodelId->lithologynr][i];
                           deltaexp = areas[submodelId->subareanr] * deltatime 
                                      * submodel->downwardflux[mstart][i];
                           totaliso = isotope[submodelId->lithologynr][i];
                           expulsion[submodelId->lithologynr][i] += deltaexp; 
                           if (expulsion[submodelId->lithologynr][i] != 0.0) {
                              isotope[submodelId->lithologynr][i] =
                                 (submodel->isotopehist[mstart][i] * deltaexp +
                                  totaliso * totalexp) / 
                                  expulsion[submodelId->lithologynr][i];
                           }
                       }

/* ------------------- Deal with the remaining time steps */
                       for (m=mstart+1; m<=model->timestepnr; m++) {
                           deltatime = ABS (model->timehist[m] -
                                               model->timehist[m-1]);
                           for (i=0; i<model->nspecies; i++) {
                              totalexp = expulsion[submodelId->lithologynr][i];
                              deltaexp = areas[submodelId->subareanr] * 
                                         deltatime * 
                                         submodel->downwardflux[m][i];
                              totaliso = isotope[submodelId->lithologynr][i];
                              expulsion[submodelId->lithologynr][i] += 
                                                                      deltaexp; 
                              if (expulsion[submodelId->lithologynr][i] 
                                   != 0.0) {
                                 isotope[submodelId->lithologynr][i] =
                                        (submodel->isotopehist[m][i] * 
                                         deltaexp + totaliso * totalexp) / 
                                         expulsion[submodelId->lithologynr][i];
                              }
                           }
                       }
                   }
                   submodelId = GetNextFromList (submodelsel);
               }
               if (!submodelAvailable) {
                  rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
               }
            }
         }
      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxGetVanKrevelen - get the Van Krevelen data from a submodel
--------------------------------------------------------------------------------

 GxGetVanKrevelen returns the Van Krevelen data for a submodel. These values
 are calculated from the concentration of the species in the different time 
 steps.

 Arguments
 ---------
 model       - the model for which the Van Krevelen should be calculated
 subareanr   - number of subarea for which to calculate Van Krevelen
 horizonnr   - number of horizon for which to calculate Van Krevelen
 lithologynr - number of lithology for which to calculate Van Krevelen
 begintime   - the begin time for the history
 endtime     - the end time for the history
 hc          - Hydrogen over Carbon
 oc          - Oxygen over Carbon

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetVanKrevelen ( GxModel *model, int subareanr, int horizonnr,
                               int lithologynr, double begintime,
                               double endtime, double *hc, double *oc )
{
   GxErrorCode  rc=GXE_NOERROR;
   GxSubModel  *submodel;
   double      *concentration, *atomich, *atomicc, *atomico;
   int          i, size;

int j;

   if (!model || (subareanr < 0) || (subareanr >= model->nlocations)
      || (horizonnr < 0) || (horizonnr >= model->nhorizons )
      || (lithologynr < 0) || (lithologynr >= model->nlithologies)
      || !hc || !oc) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the model has been run */
      submodel = &model->submodel[subareanr][horizonnr][lithologynr];

      if (submodel->netthickness <= 0.0) {
         rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
      } else {

/* ----- Get the size of the concentration history array */
         rc = GxGetNumberOfTimes (model, begintime, endtime, &size);
         if (!rc) {

/* --------- Allocate memory for concentration history */
             rc = GxAllocArray ( &concentration, sizeof (double), 1, size);
             if (!rc) 
                rc = GxAllocArray (&atomich, sizeof(double), 1, size );
             if (!rc) 
                rc = GxAllocArray (&atomicc, sizeof(double), 1, size );
             if (!rc) 
                rc = GxAllocArray (&atomico, sizeof(double), 1, size );
             if (!rc) {
                for (i = 0; i < size; i++) {
                   atomich[i] = 0.0;
                   atomicc[i] = 0.0;
                   atomico[i] = 0.0;
                }


/* ------------ Check all species and calculate HC & OC if immobile or
                species is asphaltene */
                i = 0;
                while (!rc && i < model->nspecies ) {
 
                   if ((!model->species[lithologynr][i].mobile) || 
                       (GxIsAsphaltene ( &(model->species[lithologynr][i]) ))) {
/* ------------------ Get the concentration history for species i */
                      rc = GxGetConcentrationHist ( model, subareanr,
                                           horizonnr, lithologynr, i,
                                           begintime, endtime, concentration );

                      if (!rc) {
                          GxCalcVanKrevelen (&model->species[lithologynr][i],
                                                size, concentration, atomich, 
                                                atomicc, atomico );
                      }
                   }
		   i++;
                }
                for (i = 0; i < size; i++) {
                   hc[i] = (atomicc[i] > FLT_MIN) ? atomich[i] / atomicc[i] : 0.0;
                   oc[i] = (atomicc[i] > FLT_MIN) ? atomico[i] / atomicc[i] : 0.0;
                }
             }
             if (atomich)
                GxFreeArray (atomich, 1, size);
             if (atomicc)
                GxFreeArray (atomicc, 1, size);
             if (atomico)
                GxFreeArray (atomico, 1, size);
             if (concentration)
                GxFreeArray (concentration, 1, size);
          }
       }
   }
   return (rc);
}



 

/*
--------------------------------------------------------------------------------
 GxGetVREHist - get the VRE history from a submodel
--------------------------------------------------------------------------------

 GxGetVREHist returns the VRE values for a submodel. These VRE values are
 calculated from the time and temperature history for the submodel using
 the function and starting value passed in the argument list. The VRE history
 can only be calculated if both the time and the temperature have been stored
 during the running of the model. This is defined in the call to the function
 GxCreateModel.

 Arguments
 ---------
 model       - the model for which the VRE should be calculated
 subareanr   - number of subarea for which to calculate VRE
 horizonnr   - number of horizon for which to calculate VRE
 lithologynr - number of lithology for which to calculate VRE
 vrefunc     - pointer to function for calculating VRE
 initialparm - initial value for parameter to function vrefunc
 begintime   - the begin time for the history
 endtime     - the end time for the history
 vrehist     - array for the VRE history data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetVREHist (GxModel *model, int subareanr, int horizonnr,
                          int lithologynr,
                          double (*vrefunc) (double, double, double *),
                          double initialparm, double begintime,
                          double endtime, double *vrehist)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxSubModel   *submodel;
   double       parm=initialparm;
   int          i, begintimestep, endtimestep;

   if (!model || (subareanr < 0) || (subareanr >= model->nlocations)
      || (horizonnr < 0) || (horizonnr >= model->nhorizons)
      || (lithologynr < 0) || (lithologynr >= model->nlithologies)
      || !vrefunc || !vrehist) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the necessary data is available in the model */
      if (!(model->historyflags & (GXP_TIME | GXP_TEMPERATURE))) {
         rc = GxError (GXE_NOTPRESENT, GXT_NOTPRESENT);
      } else {

/* ----- Check that the model has been initialized */
         if (model->timestepnr < 0) {
            rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
         } else {

/* -------- Check that the model has been run */
            submodel = &model->submodel[subareanr][horizonnr][lithologynr];
            if (submodel->netthickness <= 0.0) {
               rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
            } else {

/* ----------- Determine the begin and end time step number */
               rc = GxGetBeginEndStep (model, begintime, endtime, 
                                       &begintimestep, &endtimestep);

/* ----------- Run VRE algorithm for time steps before begintimestep */
               if (begintimestep == 0) {
                  vrehist[0] = vrefunc (0.0, submodel->temperaturehist[0],
                                        &parm);
               } else {
                  vrefunc (0.0, submodel->temperaturehist[0], &parm);
                  for (i=1; i<begintimestep; i++)
                     vrefunc (ABS (model->timehist[i]-model->timehist[i-1]),
                              submodel->temperaturehist[i], &parm);
               }

/* ----------- Continu VRE calculations in the time step range requested */
               for (i=MAX (1, begintimestep); i<=endtimestep; i++)
                  vrehist[i-begintimestep] =
                     vrefunc (ABS (model->timehist[i]-model->timehist[i-1]),
                              submodel->temperaturehist[i], &parm);
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxInitializeModel - initialize the model to a particular time
--------------------------------------------------------------------------------

 GxInitializeModel initializes the time for the model data structure to
 the time passed in the argument list.

 Arguments
 ---------
 model             - model to be initialized
 overburdendensity - the density of the overburden
 inorganicdensity  - the density of the inorganic matter
 surfacetemp       - the surface temperature
 tempgradient      - the temperature gradient
 expansivity       - the expansivity
 compressibility   - the compressibility
 timeunitfactor    - number of seconds per time unit
 species           - the data for the species in the model
 stoichiometry     - the stoich factors for the reaction scheme
 tphistories       - temperature and pressure histories
 charlength        - the characteristic lengths for each lithology
 upperbiot         - the upper Biot numbers for each lithology
 lowerbiot         - the lower Biot numbers for each lithology
 netthickness      - the net thickness for each submodel
 referenceposition - the reference position relative to the char length
 time              - time to model should be initialized to
 initialconc       - initial concentration for each species for each lithology
 initkerogenconc   - initial concentration of kerogen for each lithology

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxInitializeModel (GxModel *model, double overburdendensity,
                               double inorganicdensity, double surfacetemp,
                               double tempgradient, double expansivity,
                               double compressibility, double timeunitfactor,
                               GxSpecies **species, double ***stoichiometry,
                               GxTPHistory **tphistories, double *charlength,
                               double *upperbiot, double *lowerbiot,
                               double ***netthickness,
                               double referenceposition, double time,
                               double **initialconc, double *initkerogenconc)
{
   GxErrorCode  rc=GXE_NOERROR;
   double       temperature, pressure;
   int          i, j, k;

   if (!model || (overburdendensity <= 0.0) || (inorganicdensity <= 0.0)
      || (surfacetemp < 0.0) || (tempgradient <= 0.0)
      || (expansivity < 0.0) || (compressibility < 0.0)
      || (timeunitfactor <= 0.0) || !species || !stoichiometry
      || !tphistories || !charlength || !upperbiot || !lowerbiot
      || !netthickness || (referenceposition < 0.0)
      || (referenceposition > 1.0) || !initialconc || !initkerogenconc) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Initialize the model structure with the data passed in argument list */
      model->timeunitfactor      = timeunitfactor;
      model->overburdendensity   = overburdendensity;
      model->inorganicdensity    = inorganicdensity;
      model->surfacetemperature  = surfacetemp;
      model->temperaturegradient = tempgradient;
      model->expansivity         = expansivity;
      model->compressibility     = compressibility;
      model->species             = species;
      model->stoichiometry       = stoichiometry;
      model->tphistories         = tphistories;
      model->charlength          = charlength;
      model->upperbiot           = upperbiot;
      model->lowerbiot           = lowerbiot;
      model->initkerogenconc     = initkerogenconc;

/* -- Determine if asphaltene is present in reaction scheme */
      if (GxIsAsphaltene (&species[0][GX_ASPHALTENE])) {
         model->asphaltenepresent = GxTrue;
      } else {
         model->asphaltenepresent = GxFalse;
      }

/* -- Convert reference position to reference node */
      model->referencenode =
         GxSliceNumber (referenceposition, model->nslices);

/* -- Initialize the time keeping of the model */
      model->timestepnr = 0;
      model->time       = time;

/* -- If requested, save the time of the time step */
      if (model->timehist) model->timehist[0] = time;

/* -- Determine initial temperatures and pressure for the submodels */
      for (i=0; i<model->nlocations; i++) {
         for (j=0; j<model->nhorizons; j++) {
            temperature = GxGetTemperature (&tphistories[i][j], time);
            pressure    = GxGetPressure (&tphistories[i][j], time);

/* -------- Initialize each of the submodels */
            for (k=0; k<model->nlithologies; k++) {
               GxInitializeSubModel (&model->submodel[i][j][k],
                  model->nspecies, model->nslices, netthickness[i][j][k],
                  temperature, pressure, initialconc[k]);
            }
         }
      }

/* -- Calculate the aromaticity of chains for each lithology */
      for (i=0; i<model->nlithologies; i++) {
         model->aromchain[i] = GxAromaticityOfChains (model->nspecies,
                               species[i], (const double **)stoichiometry[i]);
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxInitializeSubModel - initialize the submodel
--------------------------------------------------------------------------------

 GxInitializeSubModel initializes the submodel data structure with the data
 passed in the argument list.

 Arguments
 ---------
 submodel     - submodel to be initialized
 nspecies     - number of species to be modeled
 nslices      - number of slices to be modeled
 netthickness - the net thickness for the submodel
 temperature  - the temperature at the start of the model run
 pressure     - the pressure at the start of the model run
 initialconc  - initial concentration for each species

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static void GxInitializeSubModel (GxSubModel *submodel, int nspecies,
                                  int nslices, double netthickness,
                                  double temperature, double pressure,
                                  double *initialconc)
{
   int  i, j;

   assert (submodel);
   assert (nspecies > 0);
   assert (nslices > 0);
   assert (netthickness >= 0.0);
   assert (temperature >= 0.0);
   assert (pressure >= 0.0);
   assert (initialconc);

/* If requested, store the temperature and pressure in the history array */
   if (submodel->temperaturehist)
      submodel->temperaturehist[0] = temperature;
   if (submodel->pressurehist)
      submodel->pressurehist[0]    = pressure;

/* If requested, store the first conversion factors in the history array */
   if (submodel->kerogenconversion)
      submodel->kerogenconversion[0] = 0.0;
   if (submodel->asphalteneconversion)
      submodel->asphalteneconversion[0] = 0.0;

/* If flux history requested, set the fluxes for first time step to zero */
   if (submodel->upwardflux)
      for (i=0; i<nspecies; i++)
         submodel->upwardflux[0][i]   = 0.0;
   if (submodel->downwardflux)
      for (i=0; i<nspecies; i++)
         submodel->downwardflux[0][i] = 0.0;

/* If requested, store the initial concentration in the history array */
   if (submodel->concentrationhist)
      for (i=0; i<nspecies; i++)
         submodel->concentrationhist[0][i] = initialconc[i];

/* Store the net thickness for the submodel */
   submodel->netthickness = netthickness;

/* Initialize the concentrations for each of the species in each slice */
   for (i=0; i<nslices; i++)
      for (j=0; j<nspecies; j++)
         submodel->concentration[i][j] = initialconc[j];

}



/*
--------------------------------------------------------------------------------
 GxRunModel - run a complete model
--------------------------------------------------------------------------------

 GxRunModel performs the model calculations for all the locations in the
 model using the data passed in the argument list.

 Arguments
 ---------
 model             - model to be run
 isofract          - isotope fractionation data
 isofactor         - isotope fractionation factor data
 overburdendensity - the density of the overburden
 inorganicdensity  - the density of the inorganic matter
 surfacetemp       - the surface temperature
 tempgradient      - the temperature gradient
 expansivity       - the expansivity
 compressibility   - the compressibility
 timeunitfactor    - number of seconds per time unit
 species           - the data for the species in the model
 stoichiometry     - the stoich factors for the reaction scheme
 tphistories       - temperature and pressure histories
 charlength        - the characteristic length for each submodel
 upperbiot         - the upper Biot number for each submodel
 lowerbiot         - the lower Biot number for each submodel
 netthickness      - the net thickness for each submodel
 referenceposition - the reference position relative to the char length
 initialconc       - initial concentration for each species and each submodel
 initkerogenconc   - initial concentration of kerogen for each submodel

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxRunModel (GxModel *model, const GxIsoFract *isofract,
                        GxIsoFactor *isofactor, double overburdendensity,
                        double inorganicdensity, double surfacetemp,
                        double tempgradient, double expansivity,
                        double compressibility, double timeunitfactor,
                        GxSpecies **species, double ***stoichiometry,
                        GxTPHistory **tphistories, double *charlength,
                        double *upperbiot, double *lowerbiot,
                        double ***netthickness, double referenceposition,
                        double **initialconc, double *initkerogenconc)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, j, k, l, m, n, lasttimeindex;
   double       minbegin, maxbegin, minend, maxend, begintime, endtime, time;

   if (!model || (overburdendensity <= 0.0) || (inorganicdensity <= 0.0)
      || (surfacetemp < 0.0) || (tempgradient <= 0.0) || (expansivity < 0.0) 
      || (compressibility < 0.0) || (timeunitfactor <= 0.0) || !species 
      || !stoichiometry || !tphistories || !charlength || !upperbiot 
      || !lowerbiot || !netthickness || (referenceposition < 0.0) 
      || (referenceposition > 1.0) || !initialconc || !initkerogenconc) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Use the t&p histories to determine earliest begin and latest end time */
      minbegin =  DBL_MAX;
      maxbegin = -DBL_MAX;
      minend   =  DBL_MAX;
      maxend   = -DBL_MAX;
      for (i=0; i<model->nlocations; i++) {
         for (j=0; j<model->nhorizons; j++) {
            lasttimeindex = tphistories[i][j].ntimes - 1;
            minbegin = MIN (minbegin, tphistories[i][j].time[0]);
            maxbegin = MAX (maxbegin, tphistories[i][j].time[0]);
            minend   = MIN (minend, tphistories[i][j].time[lasttimeindex]);
            maxend   = MAX (maxend, tphistories[i][j].time[lasttimeindex]);
         }
      }

/* -- Determine begin and end time depending on the direction of time */
      if (minbegin < minend) {
         begintime = minbegin;
         endtime   = maxend;
      } else {
         begintime = maxbegin;
         endtime   = minend;
      }

/* -- Initialize the model to time step zero */
      time = GxGetTime (begintime, endtime, model->ntimesteps, 0);
      rc = GxInitializeModel (model, overburdendensity, inorganicdensity,
              surfacetemp, tempgradient, expansivity, compressibility,
              timeunitfactor, species, stoichiometry, tphistories, charlength,
              upperbiot, lowerbiot, netthickness, referenceposition, time,
              initialconc, initkerogenconc);


/* -- Loop the model over all time steps */
      for (i=0; (!rc)&&(i<model->ntimesteps); i++) {
/* ----- Initialize the negative rate of generation array and 
   ----- net rate of generation array
   ----- */
         for (j=0; j<model->nlocations; j++) {
            for (k=0; k<model->nhorizons; k++) {
               for (l=0; l<model->nlithologies; l++) {
                  for (m=0; m<model->nslices; m++) {
                     for (n=0; n<model->nspecies; n++) {
                        model->submodel[j][k][l].negativegenrate[m][n] = 0.0;
                        model->submodel[j][k][l].positivegenrate[m][n] = 0.0;
                     }
                  }
               }
            }
         }
         time = GxGetTime (begintime, endtime, model->ntimesteps, i+1);
         rc = GxStepModel (model, isofract, isofactor, time);
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxSampleSubModel - sample properties in a submodel
--------------------------------------------------------------------------------

 GxSampleSubModel returns a number of properties of the specified submodel
 and position.

 Arguments
 ---------
 model           - model to be sampled
 subareanr       - number of subarea for which to sample properties
 horizonnr       - number of horizon for which to sample properties
 lithologynr     - number of lithology for which to sample properties
 sampleposition  - the sample position
 concentration   - concentrations for each of the species
 initkerogenconc - the initial concentration of kerogen in the submodel
 density         - the density of the rock in the submodel
 toc             - the total organic carbon of the organic matter

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxSampleSubModel (GxModel *model, int subareanr, int horizonnr,
                              int lithologynr, double sampleposition,
                              double *concentration, double *initkerogenconc,
                              double *density, double *toc)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxSubModel   *submodel;
   int          i, sampleslice;
   double       vor, vrat, atomicc, hoverc, ooverc;

   if (!model || (subareanr < 0) || (subareanr >= model->nlocations)
      || (horizonnr < 0) || (horizonnr >= model->nhorizons)
      || (lithologynr < 0) || (lithologynr >= model->nlithologies)
      || (sampleposition < 0.0) || (sampleposition > 1.0)
      || !concentration || !initkerogenconc || !density || !toc) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the model has been initialized */
      if (model->timestepnr < 0) {
         rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
      } else {

/* ----- Check that the model has been run */
         submodel    = &model->submodel[subareanr][horizonnr][lithologynr];
         if (submodel->netthickness <= 0.0) {
            rc = GxError (GXE_SUBMODELNOTRUN, GXT_SUBMODELNOTRUN);
         } else {

/* -------- Convert the sample position to a slice number */
            sampleslice = GxSliceNumber (sampleposition, model->nslices);

/* -------- Determine the compaction ratio */
            vor  = GxVolumeFractionOM (model->nspecies,
                      model->species[lithologynr],
                      submodel->concentration[sampleslice]);
            vrat = vor + (1.0 - model->initkerogenconc[lithologynr] /
                      model->species[lithologynr][GX_KEROGEN].density);

/* -------- Get the concentrations of the sample, correcting for compaction */
            for (i=0; i<model->nspecies; i++)
               concentration[i] = submodel->concentration[sampleslice][i] /
                                  vrat;
            *initkerogenconc = model->initkerogenconc[lithologynr] / vrat;

/* -------- Calculate the total organic carbon in the sample */
            GxConcLumped (model->nspecies, model->nslices,
                          model->species[lithologynr],
                          (const double **)submodel->concentration, 
			  (double **)model->conclumped);
            *density = model->conclumped[sampleslice][GX_TOTALOM] / vrat +
                         (1.0 - vor) * model->inorganicdensity;
            GxAtomicRatios (model->nspecies, model->species[lithologynr],
                            concentration, &atomicc, &hoverc, &ooverc);
            *toc = GxTotalOrganicCarbon (atomicc, *density);
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxSliceNumber - determine the number of a slice from a position
--------------------------------------------------------------------------------

 GxSliceNumber determines the number of the slice that corresponds to a
 position relative to the characteristic length.

 Arguments
 ---------
 position - the position relative to the characteristic length
 nslices  - the number of slices in the model

 Return value
 ------------
 the number of the slice that contains the position specified

--------------------------------------------------------------------------------
*/

static int GxSliceNumber (double position, int nslices)
{
   assert ((position >= 0.0) && (position <= 1.0));
   assert (nslices > 0);

   return (0.5 + position * (nslices - 1));
}



/*
--------------------------------------------------------------------------------
 GxStepLithologies - perform one time step for one location and one horizon
--------------------------------------------------------------------------------

 GxStepLithologies performs the calculations needed to advance the models
 for one location and one horizon in time.

 Arguments
 ---------
 submodel          - submodels for location and horizon
 isofract          - isotope fractionation data
 isofactor         - isotope fractionation factor data
 nspecies          - number of species to be modeled
 nreactions        - number of reactions to be modeled
 nlithologies      - the number of lithologies to be modeled
 nslices           - number of nslices to be modeled
 expansivity       - the expansivity
 compressibility   - the compressibility
 timeunitfactor    - number of seconds per time unit
 species           - the data for each of the species
 asphaltenepresent - GxTrue if asphaltene is present in reaction scheme
 stoichiometry     - the stoich factors for the reactions to be performed
 aromchain         - aromaticity of aliphatic chains
 charlength        - the characteristic length for each lithology
 upperbiot         - the upper Biot number for each lithology
 lowerbiot         - the lower Biot number for each lithology
 initkerogenconc   - the initial concentration of kerogen for each lithology
 tphistory         - temperature and pressure history
 referencenode     - the reference node for the model
 timestepnr        - the number of the current time step
 time              - the time the submodels should be stepped to
 deltatime         - size of the time step to be performed
 crackingrate      - array for storing cracking rates
 equations         - array for storing linear equations
 conclumped        - array for storing concentrations of lumped species

--------------------------------------------------------------------------------
*/

static GxErrorCode GxStepLithologies (GxSubModel *submodel, 
                               const GxIsoFract *isofract, 
                               const GxIsoFactor *isofactor, 
			       int nspecies,
                               int nreactions, int nlithologies, int nslices,
                               double expansivity, double compressibility,
                               double timeunitfactor, GxSpecies **species,
                               GxBool asphaltenepresent,
                               double ***stoichiometry,
                               double *aromchain, double *charlength,
                               double *upperbiot, double *lowerbiot,
                               double *initkerogenconc,
                               GxTPHistory *tphistory, int referencenode,
                               int timestepnr, double time, double deltatime,
                               double *crackingrate, double **equations,
                               double **conclumped)
{
   double        temperature, pressure;
   int           i;
   SortedList    lithologylist=NULL;
   Lithology    *lithology;
   char         *lithologyname=NULL;
   GxIsoFactor  *ptrisofactor=NULL;
   GxErrorCode   rc=GXE_NOERROR, rc2;

   assert (submodel);
   assert (nspecies > 0);
   assert (nreactions > 0);
   assert (nlithologies > 0);
   assert (nslices > 0);
   assert (expansivity >= 0.0);
   assert (compressibility >= 0.0);
   assert (timeunitfactor > 0.0);
   assert (species);
   assert (stoichiometry);
   assert (aromchain);
   assert (charlength);
   assert (upperbiot);
   assert (lowerbiot);
   assert (initkerogenconc);
   assert (tphistory);
   assert ((referencenode >= 0) && (referencenode < nslices));
   assert (timestepnr > 0);
   assert (deltatime > 0.0);
   assert (crackingrate);
   assert (equations);
   assert (conclumped);

/* Create temporary data structures for retrieval of data from project */
   rc = GxCreateLithology (&lithology);

   if (!rc) {
/* -- Check that the time is within modelled time range */
      if (BETWEEN (time, tphistory->time[0],
                   tphistory->time[tphistory->ntimes-1])) {

/* ----- Determine the temperature and the pressure for this time */
         temperature = GxGetTemperature (tphistory, time);
         pressure    = GxGetPressure (tphistory, time);

/* ----- If isofract data is passed, get first lithology in list */
         if (isofract) {
            rc  = GxGetLithologyNameList (&lithologylist);
            if (!rc) {
               lithologyname = GetFirstFromSortedList (lithologylist);
            }
         }
          
/* ----- Loop over the lithologies, stepping all the submodels */
         for (i=0; i<nlithologies && !rc; i++) {
/* -------- Only when lithology data is passed, retrieve the lithology data */
            if (isofract && lithologyname) {
               rc = GxGetLithology (lithologyname, lithology);
               if (!rc) {
                  submodel->isotopehist[0][0] = lithology->initfractionation;
               }
               ptrisofactor = (GxIsoFactor *)(isofactor + i);
            }
            if (!rc) {
               GxStepSubModel (&submodel[i], isofract, ptrisofactor, 
                            lithology, nspecies, nreactions, nslices, 
                            expansivity, compressibility, timeunitfactor, 
                            species[i], asphaltenepresent, stoichiometry[i], 
                            aromchain[i], charlength[i], upperbiot[i], 
                            lowerbiot[i], initkerogenconc[i], referencenode, 
                            timestepnr, temperature, pressure, deltatime, 
                            crackingrate, equations, conclumped);

               if (isofract) {
                  lithologyname = GetNextFromSortedList (lithologylist);
               }
            }
         }

         if (lithologylist) {
            DeleteSortedList (lithologylist);
         }
      }

      if (lithology) {
         rc2 = GxFreeLithology (lithology);
         if (!rc) rc = rc2;
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxStepModel - step the model to a particular time
--------------------------------------------------------------------------------

 GxStepModel performs the calculations needed to advance the model to the
 specified time.

 Arguments
 ---------
 model             - model to be run
 isofract          - isotope fractionation data
 isofactor         - isotope fractionation factor data
 time              - the time the model should be advanced to

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxStepModel (GxModel *model, const GxIsoFract *isofract, 
                         const GxIsoFactor *isofactor, double time)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, j;
   double       deltatime;

   if (!model) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Make sure we don't make more time steps than we have array elements */
      if (model->timestepnr >= model->ntimesteps) {
         rc = GxError (GXE_STEPSEXCEEDED, GXT_STEPSEXCEEDED);

/* -- Check that the model has been initialized */
      } else if (model->timestepnr < 0) {
         rc = GxError (GXE_NOTINITIALIZED, GXT_NOTINITIALIZED);
      } else {

/* ----- Do some time keeping */
         model->timestepnr++;
         deltatime   = ABS (time - model->time);
         model->time = time;

/* ----- If requested, store the time for time step in the history array */
         if (model->timehist) model->timehist[model->timestepnr] = time;

/* ----- Loop over all locations and all horizons */
         for (i=0; i<model->nlocations && !rc; i++)
            for (j=0; j<model->nhorizons && !rc; j++) 
               rc = GxStepLithologies (model->submodel[i][j], isofract,
                                       isofactor, model->nspecies,
                                       model->nreactions, model->nlithologies,
                                       model->nslices, model->expansivity,
                                       model->compressibility,
                                       model->timeunitfactor, model->species,
                                       model->asphaltenepresent,
                                       model->stoichiometry, model->aromchain,
                                       model->charlength, model->upperbiot,
                                       model->lowerbiot, model->initkerogenconc,
                                       &model->tphistories[i][j],
                                       model->referencenode, model->timestepnr,
                                       time, deltatime, model->crackingrate,
                                       model->equations, model->conclumped);
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxstepSubModel - perform one time step for one submodel
--------------------------------------------------------------------------------

 GxStepSubModel performs the calculations needed to advance one submodel
 in time.

 Arguments
 ---------
 submodel          - the submodel that should be stepped in time
 isofract          - isotope fractionation data
 isofactor         - isotope fractionation factor data
 lithology         - structure with lithology data
 nspecies          - number of species to be modeled
 nreactions        - number of reactions to be modeled
 nslices           - number of nslices to be modeled
 expansivity       - the expansivity
 compressibility   - the compressibility
 timeunitfactor    - number of seconds per time unit
 species           - the data for each of the species
 asphaltenepresent - GxTrue if asphaltene is present in reaction scheme
 stoichiometry     - the stoich factors for the reactions to be performed
 aromchain         - aromaticity of aliphatic chains
 charlength        - the characteristic length for the submodel
 upperbiot         - the upper Biot number for the submodel
 lowerbiot         - the lower Biot number for the submodel
 initkerogenconc   - the initial concentration of kerogen
 referencenode     - the reference node for the summodel
 timestepnr        - the number of the current time step
 temperature       - the temperature at the end of the time step
 pressure          - the pressure at the end of the time step
 deltatime         - size of the time step to be performed
 crackingrate      - array for storing cracking rates
 equations         - array for storing linear equations
 conclumped        - array for storing concentrations of lumped species
--------------------------------------------------------------------------------
*/

static void GxStepSubModel (GxSubModel *submodel, const GxIsoFract *isofract, 
                            GxIsoFactor *isofactor, Lithology *lithology,  
                            int nspecies,
                            int nreactions, int nslices, double expansivity,
                            double compressibility, double timeunitfactor,
                            GxSpecies *species, GxBool asphaltenepresent,
                            double **stoichiometry, double aromchain,
                            double charlength, double upperbiot,
                            double lowerbiot, double initkerogenconc,
                            int referencenode, int timestepnr,
                            double temperature, double pressure,
                            double deltatime, double *crackingrate,
                            double **equations, double **conclumped)
{
   double  fouriernumber, lowerconv, upperconv, kerogentransratio,
           asphaltenetransratio, upwardflux, downwardflux;
   int     i, j;
   double  newgen, newneggen; 
   double  oldconc;
   
   assert (submodel);
   assert (nspecies > 0);
   assert (nreactions > 0);
   assert (nslices > 0);
   assert (expansivity >= 0.0);
   assert (compressibility >= 0.0);
   assert (timeunitfactor > 0.0);
   assert (species);
   assert (stoichiometry);
   assert ((aromchain >= 0.0) && (aromchain <= 1.0));
   assert (charlength > 0.0);
   assert (upperbiot > 0.0);
   assert (lowerbiot > 0.0);
   assert (initkerogenconc > 0.0);
   assert ((referencenode >= 0) && (referencenode < nslices));
   assert (timestepnr > 0);
   assert (temperature >= 0.0);
   assert (pressure >= 0.0);
   assert (deltatime > 0.0);
   assert (crackingrate);
   assert (equations);
   assert (conclumped);

/* Check that model has a positive thickness, if not it is disabled */
   if (submodel->netthickness > 0.0) {

/* -- If requested, store the temperature and pressure in history arrays */
      if (submodel->temperaturehist)
         submodel->temperaturehist[timestepnr] = temperature;
      if (submodel->pressurehist)
         submodel->pressurehist[timestepnr]    = pressure;

/* -- Calculate the transformation ratios for kerogen and asphaltene */
      if (asphaltenepresent) {
         kerogentransratio = GxKerogenTransf (initkerogenconc,
            submodel->concentration[referencenode][GX_KEROGEN],
            submodel->concentration[referencenode][GX_ASPHALTENE]);
         asphaltenetransratio = GxAsphalteneTransf (initkerogenconc,
               submodel->concentration[referencenode][GX_KEROGEN],
               submodel->concentration[referencenode][GX_ASPHALTENE],
               stoichiometry[GX_KEROGEN][GX_ASPHALTENE]);
	 if (submodel->kerogenconversion) 
	    submodel->kerogenconversion[timestepnr] = kerogentransratio;
         if (submodel->asphalteneconversion)
	    submodel->asphalteneconversion[timestepnr] = asphaltenetransratio;
      } else {
         kerogentransratio = GxKerogenTransf (initkerogenconc,
            submodel->concentration[referencenode][GX_KEROGEN], 0.0);
         if (submodel->kerogenconversion) 
           submodel->kerogenconversion[timestepnr] = kerogentransratio;
         if (submodel->asphalteneconversion)
           submodel->asphalteneconversion[timestepnr] = 0.0;
      }

/* -- Calculate the cracking rate for the species at the current conditions */
      for (i=0; i<nspecies; i++) {
        if ((i == GX_KEROGEN) || !asphaltenepresent) {
            crackingrate[i] = GxCrackingRate (&species[i], temperature,
                                              pressure, timeunitfactor,
                                              kerogentransratio);
         } else {
            crackingrate[i] = GxCrackingRate (&species[i], temperature,
                                              pressure, timeunitfactor,
                                              asphaltenetransratio);
         }
      }

/* -- Calculate the concentrations of the lumped species */
      GxConcLumped (nspecies, nslices, species, 
		   (const double **)submodel->concentration, 
		   (double **)conclumped);

/* -- Loop over all the species */
      for (i=0; i<nspecies; i++) {
         if (species[i].mobile) {

/* -------- The species is mobile, perform both generation and diffusion */
            fouriernumber = GxFourierNumber (&species[i], aromchain,
                               temperature, pressure, deltatime,
                               charlength/(nslices-1), expansivity,
                               compressibility, timeunitfactor);
            lowerconv = GxConvMassTransfer (lowerbiot,
                           species[i].referencediffusivity, charlength);
            upperconv = GxConvMassTransfer (upperbiot,
                           species[i].referencediffusivity, charlength);
            GxGenerationDiffusion ((GxIsoFract *)isofract, isofactor, lithology,
                                   nslices, nreactions, i, timestepnr, 
				   (const double **)stoichiometry,
                                   crackingrate, deltatime,
                                   submodel->asphalteneconversion[timestepnr],
                                   submodel->concentration, 
                                   submodel->negativegenrate,
                                   submodel->positivegenrate,
                                   submodel->isotopehist,
				   (const double **)conclumped,
                                   fouriernumber, lowerconv, upperconv,
                                   charlength/(nslices-1), equations, 
                                   temperature); 

/* -------- Calculate the current fluxes for this species */
            upwardflux   = upperconv * submodel->concentration[0][i] *
                           submodel->netthickness / charlength;
            downwardflux = lowerconv * submodel->concentration[nslices-1][i] *
                           submodel->netthickness / charlength;
         } else {

/* -------- The species is immobile, only generation taken into account */
            GxGeneration ((GxIsoFract *)isofract, isofactor, lithology, 
                          nslices, nreactions, i, timestepnr, 
                          (const double **)stoichiometry, crackingrate, 
                          deltatime, 
                          submodel->asphalteneconversion[timestepnr], 
                          submodel->concentration, submodel->negativegenrate, 
                          submodel->positivegenrate, submodel->isotopehist,
                          temperature);

/* -------- The fluxes for immobile species are zero */
            upwardflux   = 0.0;
            downwardflux = 0.0;
         }

/* ----- If requested, store fluxes and concentrations in history arrays */
         if (submodel->upwardflux)
            submodel->upwardflux[timestepnr][i]   = upwardflux;
         if (submodel->downwardflux)
            submodel->downwardflux[timestepnr][i] = downwardflux;
         if (submodel->concentrationhist) {
	    submodel->concentrationhist[timestepnr][i] =
               submodel->concentration[referencenode][i];
         }

         if (submodel->generationhist) {
            submodel->generationhist[timestepnr][i] = 0.0;
            for (j=1;j<nslices-1;j++) {
                submodel->generationhist[timestepnr][i] += 
                       (submodel->negativegenrate[j][i] + 
                        submodel->positivegenrate[j][i]) *
                       (submodel->netthickness / nslices);
            }
/*
Alternative solution: to be examined for next GENEX release...

            submodel->generationhist[timestepnr][i] += 
                       (submodel->negativegenrate[0][i] + 
                        submodel->positivegenrate[0][i] +
                        submodel->positivegenrate[nslices-1][i] +
                        submodel->negativegenrate[nslices-1][i]) *
                       ((submodel->netthickness / nslices) / 2.0);
*/
            submodel->generationhist[timestepnr][i] += 
                       (submodel->negativegenrate[0][i] + 
                        submodel->positivegenrate[0][i] +
                        submodel->positivegenrate[nslices-1][i] +
                        submodel->negativegenrate[nslices-1][i]) *
                       ((submodel->netthickness / nslices));
         }

/* ----- At this point the isotope fractionation can be calculated */
         if (submodel->isotopehist &&
             submodel->generationhist &&
             submodel->upwardflux &&
             submodel->downwardflux) {

            newgen = 0.0;
            for (j=0;j<nslices;j++) {
               newgen += submodel->positivegenrate[j][i] * deltatime;
            }
            
            newneggen = 0.0;
            for (j=0;j<nslices;j++) {
               newneggen -= submodel->negativegenrate[j][i] * deltatime; 
            }

            oldconc = submodel->concentrationhist[timestepnr - 1][i] * 
                      submodel->netthickness; 

/* -------- Timestep 0 isotope history will be same as the generated */
/* -------- in timestep 1                                            */
            if ((newgen != 0.0) && (timestepnr == 1)) {
               submodel->isotopehist[0][i] = 
                     submodel->isotopehist[timestepnr][i] * deltatime / newgen; 
            }

            if ((newgen != 0.0) && ((oldconc + newgen) != 0.0)) {
               if (i == GX_ASPHALTENE) {
                  submodel->isotopehist[timestepnr][i] = 
                        GxCalcIsotopeFractSR (
                            submodel->asphalteneconversion[timestepnr],
                            lithology->initfractionation, 
                            submodel->isotopehist[timestepnr - 1]
                                                 [GX_ASPHALTENE],
                            temperature, isofactor);
               } else {
                  submodel->isotopehist[timestepnr][i] =
                      (submodel->isotopehist[timestepnr][i] * deltatime + 
                       oldconc * submodel->isotopehist[timestepnr-1][i] ) /
                      (oldconc + newgen);
               }
            } else {
               if ((newneggen > 0.0) && (i == GX_ASPHALTENE)) {
                  submodel->isotopehist[timestepnr][i] =
                       GxCalcIsotopeFractSR (
                            submodel->asphalteneconversion[timestepnr],
                            lithology->initfractionation, 
                            submodel->isotopehist[timestepnr - 1]
                                                 [GX_ASPHALTENE],
                            temperature, isofactor);
               } else {
                  submodel->isotopehist[timestepnr][i] = 
                      submodel->isotopehist[timestepnr - 1][i];
               }
            }
         }
      }
   }
}

