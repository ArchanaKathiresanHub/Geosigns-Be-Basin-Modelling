/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxkernel.c,v 25.0 2005/07/05 08:03:46 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxkernel.c,v $
 * Revision 25.0  2005/07/05 08:03:46  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:42:56  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:26  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:09  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:42  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:21  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:20  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:32  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:41  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:09  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:25:58  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:11  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:39  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:14  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:02:16  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:55:59  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:14  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:53:54  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 GxKernel.c   Genex kernel interface for standalone version
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        25-JUN-1992

 Description: GxKernel.c contains the high level functions for the
              stand-alone version of Genex. These functions will be called
              from the user interface. The following functions are available:

                GxClose                       - close Genex
                GxCalculateCharge             - calculates the charge
                GxCreateModelHistory          - alloc memory for model history
                GxFreeGasMolarogram           - free memory for gas molarogram
                                                data
                GxFreeGeneration              - free memory for generation data
                GxFreeExpulsion               - free memory for expulsion data
                GxFreeModelHistory            - free memory for model history
                GxGeologicalModelPresent      - true if geol model present
                GxGetCnSpeciesName            - Get the name of Cn species 
                GxGetDefIsoFractFactor        - Get default isotope 
                                                fractionation factor alpha
                GxGetDownwardExpulsion        - return downward expulsion data
                GxGetDownwardExpulsionSelected
                                              - return downward expulsion data
                GxGetDownwardIsotopeSelected  - return isotope data for 
                                                downward expulsion for selected
                                                submodels
                GxGetExpIsotopeWetness        - return expulsion isotope wetness
                                                data for selected submodels
                GxGetGasMolarogramSelected    - return gas molarogram data for
                                                selected submodels
                GxGetGeneration               - return the generation data
                GxGetGenerationSelected       - return the generation data for
                                                selected submodels
	        GxGetGeologicalAPI	      - return API of whole model
	        GxGetGeologicalAPISelected    - return API of selected submodels
                GxGetGeologicalConcentrations - get concs from geol model
                GxGetGeologicalConversions    - get convs from geol model
		GxGetGeologicalExpIsoHist     - get isotope history of expelled
                                                species
                GxGetGeologicalExpulsions     - get expulsions from geol model
                GxGetGeologicalExpVolume      - get expulsions volume from geol
                                                model
                GxGetGeologicalFluxes         - get fluxes from geol model
                GxGetGeologicalGenCumRate     - get cumm. generation rate
                GxGetGeologicalGenRate        - get generation rate
		GxGetGeologicalGOR            - get gas oil ratio from model
		GxGetGeologicalGORSelected    - get GOR from selected submodels
		GxGetGeologicalMasses	      - get masses of retention species
                GxGetGeologicalPressures      - get pressures from geol model
		GxGetGeologicalRetIsoHist     - get isotope history of retained
                                                species
                GxGetGeologicalTemperatures   - get temps from geol model
                GxGetGeologicalTimeRange      - get time range from geol model
                GxGetGeologicalTimes          - get times from geol model
                GxGetGeologicalTimesteps      - get timesteps from geol model
                GxGetGeologicalVanKrevelen    - get Van Krevelen from geol
                GxGetGeologicalVRE            - get VRE values from geol model
                GxGetLaboratoryConcentrations - get concs from lab model
                GxGetLaboratoryConversions    - get convs from lab model
                GxGetLaboratoryExpulsions     - get expulsions from lab model
		GxGetLaboratoryExpIsoHist     - get isotope history of expelled
                                                species
                GxGetLaboratoryExpVolume      - get expulsions volume from geol
                                                model
                GxGetLaboratoryFluxes         - get fluxes from lab model
                GxGetLaboratoryGenCumRate     - get cumm. generation rate
                GxGetLaboratoryGenRate        - get generation rate
                GxGetLaboratoryPressures      - get pressures from lab model
		GxGetLaboratoryRetIsoHist     - get isotope history of retained
                                                species
                GxGetLaboratoryTemperatures   - get temps from lab model
                GxGetLaboratoryTimeRange      - get time range from lab model
                GxGetLaboratoryTimes          - get times from lab model
                GxGetLaboratoryTimesteps      - get timesteps from lab model
                GxGetLaboratoryVanKrevelen    - get Van Krevelen from lab model
                GxGetMainExpulsion            - return main sms expulsion data
                GxGetMainExpulsionSelected    - return main sms expulsion data
                                                for selected submodels
                GxGetMainIsotopeSelected      - return isotope data for 
                                                main expulsion for selected
                                                submodels
                GxGetMobileSpeciesNameList    - returns names of mobile spec
                GxGetRockEval2Analysis        - Rock-Eval II analysis of model
                GxGetSpeciesNameList          - returns names of mobile spec
		GxGetSubBurialTimeRange	      - Get time range of submodel
                GxGetUpwardExpulsion          - return upward expulsion data
                GxGetUpwardExpulsionSelected  - return upward expulsion data 
                                                for selected submodels
                GxGetUpwardIsotopeSelected    - return isotope data for 
                                                upward expulsion for selected
                                                submodels
                GxIsSpeciesMobile             - Check if species is mobile
                GxIsSpeciesHCGas              - Check if species is gas
                GxIsSpeciesHCWetGas           - Check if species is a wet hc gas
                GxIsSpeciesOil                - Check if species is oil 
                GxIsSpeciesOilGas             - Check if species is oil or gas
                GxLaboratoryModelPresent      - true if lab model present
                GxNitrogenPartOfModel         - true if nitrogen element is used
                GxOpen                        - open Genex for modeling
                GxOptimizeCharLength          - loop to optimize char. length
                GxRunGeologicalModel          - run the geological model
                GxRunLaboratoryModel          - run the laboratory model

              To support these functions this file contains a number of
              local functions that can not be used outside this file.

 Usage notes: - In order to use the functions in this source file the header
                file gxkernel.h should be included.
              - All functions return an error code. If an error is detected
                the error handler function is called and an error code unequal
                to GXE_NOERROR is returned to the caller.


 History
 -------
 25-JUN-1992  P. Alphenaar  initial version
  6-JUL-1992  M.J.A. van Houtert  OptimizeCharlength with boundary check !!!
 29-JUL-1992  M.J.A. van Houtert  GxGetGeologicalConversions added
 29-JUL-1992  M.J.A. van Houtert  GxLaboratoryConversions added
 30-JUL-1992  M.J.A. van Houtert  GxGetGeologicalVanKrevelen added
 30-JUL-1992  M.J.A. van Houtert  GxGetLaboratoryVanKrevelen added
  5-AUG-1992  M.J.A. van Houtert  GxGetGeologicalAPI
  6-AUG-1992  M.J.A. van Houtert  GxGetGeologicalGOR
 10-AUG-1992  M.J.A. van Houtert  GxGetGeologicalMasses
 20-AUG-1992  M.J.A. van Houtert  GxGetSubBurialTimeRange
 19-MAY-1993  M.J.A. van Houtert  GxIsSpeciesOilGas added
 23-MAY-1993  M.J.A. van Houtert  GxGetGeologicalGenCumRate added
 23-MAY-1993  M.J.A. van Houtert  GxGetGeologicalGenRate added
 23-MAY-1993  M.J.A. van Houtert  GxGetLaboratoryGenCumRate added
 23-MAY-1993  M.J.A. van Houtert  GxGetLaboratoryGenRate added
 27-AUG-1993  M.J.A. van Houtert  GxGetGeneration added
 04-DEC-1993  Th. Berkers         When deleting an item form a list the next 
                                  item in this list will afterwards be the 
                                  current. When using next item function one 
                                  item in the list will be skipped which can 
                                  cause a system crash after saving and 
                                  reloading the project file 
                                  (Changes in GxAllocGeologicalRunData)
 22-DEC-1993  Th. Berkers         Implementing Nitrogen model:
                                  Call to GxCalcSpecies extended with nci of
                                  lithology in use.
                                  GxIsSpeciesMobile, GxIsSpeciesHCGas and 
                                  GxIsSpeciesOil added
                                  GxNitrogenPartOfModel added in order to 
                                  get an integrated GENEX for both a model with
                                  and without the nitrogen element
 10-JAN-1994  Th. Berkers         Implementing Nitrogen model:
                                  GxGetLaboratoryRatios added
 08-SEP-1994  Th. Berkers         In a prospect it must be able to get the
                                  charge data eventhough there is no TP data
                                  from the burial history. In this case the
                                  trap temperature and pressure has to be taken
 23-SEP-1994  Th. Berkers         Masses added to GxExpulsion and GxGenRate
                                  structure 
 17-NOV-1994  Th. Berkers         GxGetGeologicalAPISelected and 
                                  GxGetGeologicalGORSelected added
 23-NOV-1994  Th. Berkers         GxGetGenerationSelected, 
                                  GxGetDownwardExpulsionSelected, 
                                  GxGetMainExpulsionSelected and 
                                  GxGetUpwardExpulsionSelected added
 02-DEC-1994  Th. Berkers         GxGetGasMolarogramSelected and 
                                  GxFreeGasMolarogram added
 14-DEC-1994  Th. Berkers         GxIsSpeciesHCWetGas added
 02-MAR-1995  Th. Berkers         Integration of Isotopes Fractionation
--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <assert.h>
#include  <float.h>
#include  <math.h>
#include  <stdlib.h>
#include  <string.h>
#include  "gxconfig.h"
#include  "gxerror.h"
#include  "gxhist.h"
#include  "gxhorizn.h"
#include  "gxlithol.h"
#include  "gxkernel.h"
#include  "gxlabexp.h"
#include  "gxnumer.h"
#include  "gxoption.h"
#include  "gxprjct.h"
#include  "gxphchem.h"
#include  "gxressub.h"
#include  "gxrock.h"
#include  "gxsbarea.h"
#include  "gxtrap.h"
#include  "gxutils.h"

/* Macro definitions */
/* ----------------- */
/* -- time unit factors */
#define  GX_GEOLOGICALTIME   3.1536E+13
#define  GX_LABORATORYTIME   1.0

/* -- Gas molarogram definitions -- */
#define C1_SPECIES     "C1"
#define C2_SPECIES     "C2"
#define C3_SPECIES     "C3"
#define C4_SPECIES     "C4"
#define C5_SPECIES     "C5"

/* -- standalone history mask -- */
#define  GX_HISTORYMASK      (GXP_TIME|GXP_TEMPERATURE|GXP_PRESSURE| \
                              GXP_EXPULSIONFLUX|GXP_CONCENTRATION|   \
                              GXP_KEROGENCONVERSION|GXP_ASPHALTENECONVERSION| \
                              GXP_GENERATION)
/* -- error text definitions -- */
#define  GXT_CANNOTOPTIMIZE  "In this case the characteristic length can't " \
			     "be optimised.\nSee user manual section 5.4" \
			     " 'Optimising the characteristic length'" 
#define  GXT_MAXITERREACHED  "The maximum number of iterations has been " \
                             "reached without the desired accuracy.\n"
#define  GXT_NOCONVERGANCE   "The characteristic length does not converge.\n"
#define  GXT_NOGEOMODEL      "There is no geological model present.\n"
#define  GXT_NOHORIZONS      "There are no source rock horizons in " \
                             "the project - model can not be run.\n"
#define  GXT_NOLABMODEL      "There is no laboratory model present.\n"
#define  GXT_NOLITHOLOGIES   "There are no lithologies in the " \
                             "project - model can not be run.\n"
#define  GXT_NOSUBAREAS      "There are no subareas with area > 0.0 in " \
                             "the project - model can not be run.\n"
#define  GXT_NOTASPECIES     "The species %s does not exists.\n"
#define  GXT_NOTASUBMODEL    "The submodel identified by subarea %s,\n" \
                             "source rock horizon %s and lithology %s\n" \
                             "does not exists.\n"
#define  GXT_NOTPHISTDATA    "There is insufficient temperature and " \
                             "pressure data for modelling.\n"
#define  GXT_NOTRAP          "There is no trap defined in the project - " \
                             "charge can not be calculated.\n"
#define  GXT_NOTRAPHIST      "There is no temperature, pressure and " \
                             "structural volume data for the trap.\n"
#define  GXT_NOTROCKEVAL2    "The laboratory experiment was not run with\n" \
                             "the Rock-Eval II experiment type selected.\n" \
			     "\nNo Rock-Eval II table can be created.\n"

/* Type definitions */
/* ---------------- */
/* -- structure definitions -- */
typedef  struct GxGeologicalRunData  GxGeologicalRunData;
typedef  struct GxLaboratoryRunData  GxLaboratoryRunData;
typedef  struct GxOptimizeRunData    GxOptimizeRunData;
struct GxGeologicalRunData {
   int          nlocations, nhorizons, nlithologies;
   GxModel      *model;
   SortedList   subareas, horizons, lithologies;
   GxSpecies    **species;
   double       ***stoichiometry;
   GxTPHistory  **tphistory;
   double       *charlength, *upperbiot, *lowerbiot, ***netthickness,
                referenceposition, **concentration, *initkerogenconc;
   GxIsoFactor  *isofactor;
};
struct GxLaboratoryRunData {
   GxModel      *model;
   GxSpecies    **species;
   double       ***stoichiometry;
   GxTPHistory  **tphistory;
   int          experimenttype;
   double       charlength, upperbiot, lowerbiot, ***netthickness,
                referenceposition, **concentration, initkerogenconc,
                density, toc;
   GxIsoFactor  isofactor;
};
struct GxOptimizeRunData {
   GxModel      *geomodel, *labmodel;
   GxSpecies    **geospecies, **labspecies;
   double       ***stoichiometry;
   GxTPHistory  **geotphistory, **labtphistory;
   double       geocharlength, labcharlength, geoupperbiot, labupperbiot,
                geolowerbiot, lablowerbiot, ***geonetthickness,
                ***labnetthickness, referenceposition, sampleposition,
                **geoconcentration, **labconcentration, initkerogenconc,
                density, toc, s1tocm, maxerror, factor;
   int          maxiterations;
};

/* Function prototypes */
/* ------------------- */
static GxErrorCode  GxAllocExpulsion (GxExpulsion **, GxExpulsion **,
                                      GxExpulsion **);
static GxErrorCode  GxAllocGasMolarogram (GxGasMolarogram **);
static GxErrorCode  GxAllocGeneration (GxGenRate **, GxGenRate **,
                                       GxGenRate **);
static GxErrorCode  GxAllocExpulsionStruct (int, GxExpulsion **);
static GxErrorCode  GxAllocGasMolarogramStruct (int, GxGasMolarogram **);
static GxErrorCode  GxAllocGenerationStruct (int, GxGenRate **);
static GxErrorCode  GxAllocGeologicalRunData (void);
static GxErrorCode  GxAllocIsotope (GxIsotope **, GxIsotope **, GxIsotope **);
static GxErrorCode  GxAllocIsotopeStruct (int, GxIsotope **);
static GxErrorCode  GxAllocLaboratoryRunData (void);
static GxErrorCode  GxAllocOptimizeRunData (GxOptimizeRunData *);
static void         GxConvertExpulsion (double **, GxExpulsion *,
                                        GxExpulsion *, GxExpulsion *,
                                        double *, double *, double *);
static void         GxConvertGasMolarogram (GxExpulsion *, GxGasMolarogram *);
static void         GxConvertGeneration (double **, GxGenRate *,
                                        GxGenRate *, GxGenRate *);
static void         GxConvertIsotopes (double **, double **, GxIsotope *,
                                       GxIsotope *, GxIsotope *);
static GxErrorCode  GxCreateTPHistory (int, double *, double *, double *,
                                       double *, double, double, double,
                                       GxTPHistory *);
static GxErrorCode  GxGetDataForCharge (double *, double *, double *,
                                        double *, double *, double *,
                                        double *, double *);
static GxErrorCode  GxFillGeologicalRunData (void);
static GxErrorCode  GxFillLaboratoryRunData (char *, char *, char *, double);
static GxErrorCode  GxFillOptimizeRunData (char *, char *, char *,
                                           GxOptimizeRunData *);
static GxErrorCode  GxFindSpecies (char *, int *);
static GxErrorCode  GxFindSubModel (char *, char *, char *, int *, int *,
                                    int *);
static void         GxFreeGasMolarogramStruct (GxGasMolarogram *);
static void         GxFreeGenerationStruct (GxGenRate *);
static void         GxFreeExpulsionStruct  (GxExpulsion *);
static GxErrorCode  GxFreeGeologicalRunData (void);
static void         GxFreeIsotopeStruct  (GxIsotope *);
static GxErrorCode  GxFreeLaboratoryRunData (void);
static GxErrorCode  GxFreeOptimizeRunData (GxOptimizeRunData *);
static void         GxFreeTPHistory (GxTPHistory *);
static GxErrorCode  GxGetGeologicalRunData (void);
static GxErrorCode  GxGetLaboratoryRunData (char *, char *, char *, double);
static GxErrorCode  GxGetOptimizeRunData (char *, char *, char *,
                                          GxOptimizeRunData *);
static GxErrorCode  GxGetSubAreaData (double *);
static GxBool       GxHasFractionationData (int);
static GxErrorCode  GxOptimizationPass (GxOptimizeRunData *, double *);

/* Global variables */
/* ---------------- */
/* -- file scope variables -- */
static GxConfiguration      *theConfiguration=NULL;
static GxGeologicalRunData  *theGeologicalRunData=NULL;
static GxLaboratoryRunData  *theLaboratoryRunData=NULL;



/*
--------------------------------------------------------------------------------
 GxAllocExpulsion - allocate memory for expulsion data
--------------------------------------------------------------------------------

 GxAllocExpulsion allocates memory for the expulsion data from the geological
 model. The memory is freed by calling GxFreeExpulsion.

 Arguments
 ---------
 oilexpulsionptr   - pointer for the oil expulsion data
 gasexpulsionptr   - pointer for the gas expulsion data
 nonhcexpulsionptr - pointer for the non-hydrocarbon expulsion data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocExpulsion (GxExpulsion **oilexpulsionptr,
                                     GxExpulsion **gasexpulsionptr,
                                     GxExpulsion **nonhcexpulsionptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxExpulsion  *oilexpulsion=NULL, *gasexpulsion=NULL, *nonhcexpulsion=NULL;
   int          noils, ngasses, nnonhcs, i;

   assert (oilexpulsionptr);
   assert (gasexpulsionptr);
   assert (nonhcexpulsionptr);

/* Initialize the number of oil, gas and non hydrocarbon species */
   noils   = 0;
   ngasses = 0;
   nnonhcs = 0;

/* Count the number of oil, gas and non hydrocarbon species */
   for (i=0; i<theConfiguration->nspecies; i++) {
      if (i != GX_KEROGEN) {
         if (theConfiguration->species[i].mobile
            && (GxHasOilComposition (&theConfiguration->species[i])
               || ((i == GX_ASPHALTENE)
                  && GxIsAsphaltene (&theConfiguration->species[i])))) {
            noils++;
         } else if (GxIsHCGas (&theConfiguration->species[i])) {
            ngasses++;
         } else if (!GxIsHydrocarbon (&theConfiguration->species[i])) {
            nnonhcs++;
         }
      }
   }

/* Allocate and return structures for the oil, gas and non hydrocarbon data */
   rc = GxAllocExpulsionStruct (noils, &oilexpulsion);
   if (!rc) rc = GxAllocExpulsionStruct (ngasses, &gasexpulsion);
   if (!rc) rc = GxAllocExpulsionStruct (nnonhcs, &nonhcexpulsion);
   if (!rc) {
      *oilexpulsionptr   = oilexpulsion;
      *gasexpulsionptr   = gasexpulsion;
      *nonhcexpulsionptr = nonhcexpulsion;
   } else {
      GxFreeExpulsionStruct (oilexpulsion);
      GxFreeExpulsionStruct (gasexpulsion);
      GxFreeExpulsionStruct (nonhcexpulsion);
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxAllocGasMolarogram - allocate memory for gas molarogram data
--------------------------------------------------------------------------------

 GxAllocGasMolarogram allocates memory for the gas molarogram data from the 
 geological model. The memory is freed by calling GxFreeGasMolarogram.

 Arguments
 ---------
 gasmolarogramptr  - pointer for the gas molarogram data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocGasMolarogram (GxGasMolarogram **gasmolarogramptr)
{
   GxErrorCode       rc=GXE_NOERROR;
   GxGasMolarogram  *gasmolarogram=NULL;
   int               noils, 
                     ngasses,
                     nnonhcs, 
                     i;

   assert (gasmolarogramptr);

/* Initialize the number of oil, hc gas and non hc species */
   noils   = 0;
   ngasses = 0;
   nnonhcs  = 0;

/* Count the number of gas species */
   for (i=0; i<theConfiguration->nspecies; i++) {
      if (i != GX_KEROGEN) {
         if (theConfiguration->species[i].mobile
            && (GxHasOilComposition (&theConfiguration->species[i])
               || ((i == GX_ASPHALTENE)
                  && GxIsAsphaltene (&theConfiguration->species[i])))) {
            noils++;
         } else if (GxIsHCGas (&theConfiguration->species[i])) {
            ngasses++;
         } else if (!GxIsHydrocarbon (&theConfiguration->species[i])) {
            nnonhcs++;
         }
      }
   }

/* Allocate and return structures for the gas molarogram data */
   rc = GxAllocGasMolarogramStruct (ngasses, &gasmolarogram);
   if (!rc) {
      *gasmolarogramptr  = gasmolarogram;
   } else {
      GxFreeGasMolarogramStruct (gasmolarogram);
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxAllocGeneration - allocate memory for generation data
--------------------------------------------------------------------------------

 GxAllocGeneration allocates memory for the generation data from the geological
 model. The memory is freed by calling GxFreeGeneration.

 Arguments
 ---------
 oilgenerationptr   - pointer for the oil generation data
 gasgenerationptr   - pointer for the gas generation data
 nonhcgenerationptr - pointer for the non-hydrocarbon generation data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocGeneration (GxGenRate **oilgenerationptr,
                                      GxGenRate **gasgenerationptr,
                                      GxGenRate **nonhcgenerationptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxGenRate   *oilgeneration=NULL, *gasgeneration=NULL, *nonhcgeneration=NULL;
   int          noils, ngasses, nnonhcs, i;

   assert (oilgenerationptr);
   assert (gasgenerationptr);
   assert (nonhcgenerationptr);

/* Initialize the number of oil, gas and non hydrocarbon species */
   noils   = 0;
   ngasses = 0;
   nnonhcs = 0;

/* Count the number of oil, gas and non hydrocarbon species */
   for (i=0; i<theConfiguration->nspecies; i++) {
      if (i != GX_KEROGEN) {
         if (theConfiguration->species[i].mobile
            && (GxHasOilComposition (&theConfiguration->species[i])
               || ((i == GX_ASPHALTENE)
                  && GxIsAsphaltene (&theConfiguration->species[i])))) {
            noils++;
         } else if (GxIsHCGas (&theConfiguration->species[i])) {
            ngasses++;
         } else if (!GxIsHydrocarbon (&theConfiguration->species[i])) {
            nnonhcs++;
         }
      }
   }

/* Allocate and return structures for the oil, gas and non hydrocarbon data */
   rc = GxAllocGenerationStruct (noils, &oilgeneration);
   if (!rc) rc = GxAllocGenerationStruct (ngasses, &gasgeneration);
   if (!rc) rc = GxAllocGenerationStruct (nnonhcs, &nonhcgeneration);
   if (!rc) {
      *oilgenerationptr   = oilgeneration;
      *gasgenerationptr   = gasgeneration;
      *nonhcgenerationptr = nonhcgeneration;
   } else {
      GxFreeGenerationStruct (oilgeneration);
      GxFreeGenerationStruct (gasgeneration);
      GxFreeGenerationStruct (nonhcgeneration);
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxAllocGasMolarogramStruct - allocate the structure for gas molarogram data
--------------------------------------------------------------------------------

 GxAllocgasMolarogramStruct allocates memory for the gas molarogram data 
 structure.  The memory is freed by calling GxFreeGasMolarogramStruct.

 Arguments
 ---------
 nspecies               - the number of species
 gasmolarogramstructptr - pointer struct for gas molarogram data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocGasMolarogramStruct (
                           int nspecies,
                           GxGasMolarogram **gasmolarogramstructptr)
{
   GxErrorCode      rc=GXE_NOERROR;
   GxGasMolarogram *gasmolarogramstruct;

   assert (nspecies >= 0);
   assert (gasmolarogramstructptr);

/* Allocate the basic generation structure */
   gasmolarogramstruct = ( GxGasMolarogram * )malloc (
                                                     sizeof (GxGasMolarogram));
   if (!gasmolarogramstruct) {
      rc = GxError (GXE_MEMORY, GXT_MEMORY);
   } else {

/* -- Initialize the structure */
      gasmolarogramstruct->nspecies    = nspecies;
      gasmolarogramstruct->speciesname = NULL;
      gasmolarogramstruct->xdata       = NULL;
      gasmolarogramstruct->ydata       = NULL;
      gasmolarogramstruct->valid       = NULL;
      if (nspecies > 0) {

/* ----- Allocate arrays for species names, x and y data and valid indicator */
         rc = GxAllocArray (&gasmolarogramstruct->speciesname, sizeof (char),
                            2, nspecies, GXL_SPECIESNAME+1);
         if (!rc) rc = GxAllocArray (&gasmolarogramstruct->xdata,
                                     sizeof (double), 1, nspecies);
         if (!rc) rc = GxAllocArray (&gasmolarogramstruct->ydata,
                                     sizeof (double), 1, nspecies);
         if (!rc) rc = GxAllocArray (&gasmolarogramstruct->valid,
                                     sizeof (GxBool), 1, nspecies);
      }
   }
   if (rc) {
      GxFreeGasMolarogramStruct (gasmolarogramstruct);
   } else {
      *gasmolarogramstructptr = gasmolarogramstruct;
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxAllocGenerationStruct - allocate the structure for generation data
--------------------------------------------------------------------------------

 GxAllocGenerationStruct allocates memory for the generation data structure.
 The memory is freed by calling GxFreeGenerationStruct.

 Arguments
 ---------
 nspecies            - the number of species
 generationstructptr - pointer struct for generation data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocGenerationStruct (int nspecies,
                                           GxGenRate **generationstructptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxGenRate    *generationstruct;

   assert (nspecies >= 0);
   assert (generationstructptr);

/* Allocate the basic generation structure */
   generationstruct = ( GxGenRate * )malloc (sizeof (GxGenRate));
   if (!generationstruct) {
      rc = GxError (GXE_MEMORY, GXT_MEMORY);
   } else {

/* -- Initialize the structure */
      generationstruct->nspecies    = nspecies;
      generationstruct->speciesname = NULL;
      generationstruct->volume1     = NULL;
      generationstruct->volume2     = NULL;
      generationstruct->mass1       = NULL;
      generationstruct->mass2       = NULL;
      generationstruct->weightpct   = NULL;
      if (nspecies > 0) {

/* ----- Allocate arrays for species names, volumes and weight fractions */
         rc = GxAllocArray (&generationstruct->speciesname, sizeof (char),
                            2, nspecies, GXL_SPECIESNAME+1);
         if (!rc) rc = GxAllocArray (&generationstruct->volume1,
                                     sizeof (double), 1, nspecies);
         if (!rc) rc = GxAllocArray (&generationstruct->volume2,
                                     sizeof (double), 1, nspecies);
         if (!rc) rc = GxAllocArray (&generationstruct->mass1,
                                     sizeof (double), 1, nspecies);
         if (!rc) rc = GxAllocArray (&generationstruct->mass2,
                                     sizeof (double), 1, nspecies);
         if (!rc) rc = GxAllocArray (&generationstruct->weightpct,
                                     sizeof (double), 1, nspecies);
      }
   }
   if (rc) {
      GxFreeGenerationStruct (generationstruct);
   } else {
      *generationstructptr = generationstruct;
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxAllocExpulsionStruct - allocate the structure for expulsion data
--------------------------------------------------------------------------------

 GxAllocExpulsionStruct allocates memory for the expulsion data structure.
 The memory is freed by calling GxFreeExpulsionStruct.

 Arguments
 ---------
 nspecies           - the number of species
 expulsionstructptr - pointer struct for expulsion data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocExpulsionStruct (int nspecies,
                                           GxExpulsion **expulsionstructptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxExpulsion  *expulsionstruct;

   assert (nspecies >= 0);
   assert (expulsionstructptr);

/* Allocate the basic expulsion structure */
   expulsionstruct = ( GxExpulsion * )malloc (sizeof (GxExpulsion));
   if (!expulsionstruct) {
      rc = GxError (GXE_MEMORY, GXT_MEMORY);
   } else {

/* -- Initialize the structure */
      expulsionstruct->nspecies    = nspecies;
      expulsionstruct->speciesname = NULL;
      expulsionstruct->volume1     = NULL;
      expulsionstruct->volume2     = NULL;
      expulsionstruct->mass1       = NULL;
      expulsionstruct->mass2       = NULL;
      expulsionstruct->weightpct   = NULL;
      if (nspecies > 0) {

/* ----- Allocate arrays for species names, volumes and weight fractions */
         rc = GxAllocArray (&expulsionstruct->speciesname, sizeof (char),
                            2, nspecies, GXL_SPECIESNAME+1);
         if (!rc) rc = GxAllocArray (&expulsionstruct->volume1,
                                     sizeof (double), 1, nspecies);
         if (!rc) rc = GxAllocArray (&expulsionstruct->volume2,
                                     sizeof (double), 1, nspecies);
         if (!rc) rc = GxAllocArray (&expulsionstruct->mass1,
                                     sizeof (double), 1, nspecies);
         if (!rc) rc = GxAllocArray (&expulsionstruct->mass2,
                                     sizeof (double), 1, nspecies);
         if (!rc) rc = GxAllocArray (&expulsionstruct->weightpct,
                                     sizeof (double), 1, nspecies);
      }
   }
   if (rc) {
      GxFreeExpulsionStruct (expulsionstruct);
   } else {
      *expulsionstructptr = expulsionstruct;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxAllocGeologicalRunData - allocates memory for the geological run data
--------------------------------------------------------------------------------

 GxAllocGeologicalRunData allocates memory for the geological run data. The
 information needed for the allocation is retrieved from the project and
 stored in the geological run data.

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocGeologicalRunData (void)
{
   GxErrorCode  rc=GXE_NOERROR, rc2;
   SubArea      *subarea=NULL;
   char         *subareaname;
   RunOptions   *runoptions=NULL;

   assert (theConfiguration);
   assert (!theGeologicalRunData);

/* Allocate the basic structure for the geological run data */
   theGeologicalRunData = ( GxGeologicalRunData * ) 
                              malloc (sizeof (GxGeologicalRunData));
   if (!theGeologicalRunData) {
      rc = GxError (GXE_MEMORY, GXT_MEMORY);
   } else {

/* -- Initialize the geological run data structure */
      theGeologicalRunData->nlocations      = 0;
      theGeologicalRunData->nhorizons       = 0;
      theGeologicalRunData->nlithologies    = 0;
      theGeologicalRunData->model           = NULL;
      theGeologicalRunData->species         = NULL;
      theGeologicalRunData->stoichiometry   = NULL;
      theGeologicalRunData->tphistory       = NULL;
      theGeologicalRunData->charlength      = NULL;
      theGeologicalRunData->upperbiot       = NULL;
      theGeologicalRunData->lowerbiot       = NULL;
      theGeologicalRunData->netthickness    = NULL;
      theGeologicalRunData->concentration   = NULL;
      theGeologicalRunData->initkerogenconc = NULL;
      theGeologicalRunData->subareas        = NULL;
      theGeologicalRunData->horizons        = NULL;
      theGeologicalRunData->lithologies     = NULL;
      theGeologicalRunData->isofactor       = NULL;

/* -- Get the names of the subareas, horizon and lithologies in the project */
      rc = GxGetSubAreaNameList (&theGeologicalRunData->subareas);
      if (!rc) rc = GxGetSRHorizonNameList (&theGeologicalRunData->horizons);
      if (!rc)
         rc = GxGetLithologyNameList (&theGeologicalRunData->lithologies);

/* -- Remove the subareas that have an area <= 0.0 from the list */
      if (!rc) rc = GxCreateSubArea (&subarea);
      if (!rc) {
         subareaname =
            GetFirstFromSortedList (theGeologicalRunData->subareas);
         while (!rc && subareaname) {
            rc = GxGetSubArea (subareaname, subarea);
            if (subarea->area <= 0.0) {
               RemoveFromSortedList (theGeologicalRunData->subareas);
               subareaname = GetCurrentFromSortedList 
                                 (theGeologicalRunData->subareas);
            } else {
               subareaname = GetNextFromSortedList 
                                 (theGeologicalRunData->subareas);
            }
         }
      }
      if (!rc) {

/* ----- Store the number of locations, horizons and lithologies remaining */
         theGeologicalRunData->nlocations   =
            (int) CountList (theGeologicalRunData->subareas);
         theGeologicalRunData->nhorizons    =
            (int) CountList (theGeologicalRunData->horizons);
         theGeologicalRunData->nlithologies =
            (int) CountList (theGeologicalRunData->lithologies);
      }

/* -- Check that there is at least one of each of the data units */
      if (!rc && (theGeologicalRunData->nlocations == 0))
         rc = GxError (GXE_NOSUBAREAS, GXT_NOSUBAREAS);
      if (!rc && (theGeologicalRunData->nhorizons == 0))
         rc = GxError (GXE_NOHORIZONS, GXT_NOHORIZONS);
      if (!rc && (theGeologicalRunData->nlithologies == 0))
         rc = GxError (GXE_NOLITHOLOGIES, GXT_NOLITHOLOGIES);

/* -- Retrieve the run options from the project */
      if (!rc) rc = GxCreateRunOptions (&runoptions);
      if (!rc) rc = GxGetRunOptions (runoptions);
      if (!rc) {

/* ----- Store the reference position in the run data */
         theGeologicalRunData->referenceposition =
            runoptions->referenceposition;
      }

/* -- Create a model data structure using the information retrieved so far */
      if (!rc) rc = GxCreateModel (theConfiguration->nelements,
                       theConfiguration->nspecies,
                       theConfiguration->reactionscheme.nreactions,
                       theGeologicalRunData->nlocations,
                       theGeologicalRunData->nhorizons,
                       theGeologicalRunData->nlithologies,
                       runoptions->nnodes, runoptions->ntimesteps,
                       GX_HISTORYMASK | GXP_ISOTOPE, 
                       &theGeologicalRunData->model);

/* -- Allocate arrays for species data, stoichiometry, t&p histories,
 * -- characteristic lengths, Biot numbers, net thicknesses and initial
 * -- concentrations
 */
      if (!rc) rc = GxAllocArray (&theGeologicalRunData->species,
                                  sizeof (GxSpecies), 2,
                                  theGeologicalRunData->nlithologies,
                                  theConfiguration->nspecies);
      if (!rc) rc = GxAllocArray (&theGeologicalRunData->stoichiometry,
                                  sizeof (double), 3,
                                  theGeologicalRunData->nlithologies,
                                  theConfiguration->reactionscheme.nreactions,
                                  theConfiguration->nspecies);
      if (!rc) rc = GxAllocArray (&theGeologicalRunData->tphistory,
                                  sizeof (GxTPHistory), 2,
                                  theGeologicalRunData->nlocations,
                                  theGeologicalRunData->nhorizons);
      if (!rc) rc = GxAllocArray (&theGeologicalRunData->charlength,
                                  sizeof (double), 1,
                                  theGeologicalRunData->nlithologies);
      if (!rc) rc = GxAllocArray (&theGeologicalRunData->upperbiot,
                                  sizeof (double), 1,
                                  theGeologicalRunData->nlithologies);
      if (!rc) rc = GxAllocArray (&theGeologicalRunData->lowerbiot,
                                  sizeof (double), 1,
                                  theGeologicalRunData->nlithologies);
      if (!rc) rc = GxAllocArray (&theGeologicalRunData->netthickness,
                                  sizeof (double), 3,
                                  theGeologicalRunData->nlocations,
                                  theGeologicalRunData->nhorizons,
                                  theGeologicalRunData->nlithologies);
      if (!rc) rc = GxAllocArray (&theGeologicalRunData->concentration,
                                  sizeof (double), 2,
                                  theGeologicalRunData->nlithologies,
                                  theConfiguration->nspecies);
      if (!rc) rc = GxAllocArray (&theGeologicalRunData->initkerogenconc,
                                  sizeof (double), 1,
                                  theGeologicalRunData->nlithologies);
      if (!rc) rc = GxAllocArray (&theGeologicalRunData->isofactor,
                                  sizeof (GxIsoFactor), 1,
                                  theGeologicalRunData->nlithologies);
   }
   if (subarea) {
      rc2 = GxFreeSubArea (subarea);
      if (!rc) rc = rc2;
   }
   if (runoptions) {
      rc2 = GxFreeRunOptions (runoptions);
      if (!rc) rc = rc2;
   }
   if (rc) GxFreeGeologicalRunData ();
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxAllocIsotope - allocate memory for isotope data
--------------------------------------------------------------------------------

 GxAllocIsotope allocates memory for the isotope data from the geological
 model. The memory is freed by calling GxFreeIsotope.

 Arguments
 ---------
 oilisotopeptr   - pointer for the oil isotope data
 gasisotopeptr   - pointer for the gas isotope data
 nonhcisotopeptr - pointer for the non-hydrocarbon isotope data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocIsotope (GxIsotope **oilisotopeptr,
                                   GxIsotope **gasisotopeptr,
                                   GxIsotope **nonhcisotopeptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxIsotope   *oilisotope=NULL, *gasisotope=NULL, *nonhcisotope=NULL;
   int          noils, ngasses, nnonhcs, i;

   assert (oilisotopeptr);
   assert (gasisotopeptr);
   assert (nonhcisotopeptr);

/* Initialize the number of oil, gas and non hydrocarbon species */
   noils   = 0;
   ngasses = 0;
   nnonhcs = 0;

/* Count the number of oil, gas and non hydrocarbon species */
   for (i=0; i<theConfiguration->nspecies; i++) {
      if (i != GX_KEROGEN) {
         if (theConfiguration->species[i].mobile
            && (GxHasOilComposition (&theConfiguration->species[i])
               || ((i == GX_ASPHALTENE)
                  && GxIsAsphaltene (&theConfiguration->species[i])))) {
            noils++;
         } else if (GxIsHCGas (&theConfiguration->species[i])) {
            ngasses++;
         } else if (!GxIsHydrocarbon (&theConfiguration->species[i])) {
            nnonhcs++;
         }
      }
   }

/* Allocate and return structures for the oil, gas and non hydrocarbon data */
   rc = GxAllocIsotopeStruct (noils, &oilisotope);
   if (!rc) rc = GxAllocIsotopeStruct (ngasses, &gasisotope);
   if (!rc) rc = GxAllocIsotopeStruct (nnonhcs, &nonhcisotope);
   if (!rc) {
      *oilisotopeptr   = oilisotope;
      *gasisotopeptr   = gasisotope;
      *nonhcisotopeptr = nonhcisotope;
   } else {
      GxFreeIsotopeStruct (oilisotope);
      GxFreeIsotopeStruct (gasisotope);
      GxFreeIsotopeStruct (nonhcisotope);
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxAllocIsotopeStruct - allocate the structure for expulsion data
--------------------------------------------------------------------------------

 GxAllocIsotopeStruct allocates memory for the isotope data structure.
 The memory is freed by calling GxFreeIsotopeStruct.

 Arguments
 ---------
 nspecies         - the number of species
 isotopestructptr - pointer struct for isotope data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocIsotopeStruct (int nspecies,
                                           GxIsotope **isotopestructptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxIsotope   *isotopestruct;

   assert (nspecies >= 0);
   assert (isotopestructptr);

/* Allocate the basic expulsion structure */
   isotopestruct = ( GxIsotope * )malloc (sizeof (GxIsotope));
   if (!isotopestruct) {
      rc = GxError (GXE_MEMORY, GXT_MEMORY);
   } else {

/* -- Initialize the structure */
      isotopestruct->nspecies    = nspecies;
      isotopestruct->speciesname = NULL;
      isotopestruct->isovalue    = NULL;
      isotopestruct->valid       = NULL;
      if (nspecies > 0) {

/* ----- Allocate arrays for species names, volumes and weight fractions */
         rc = GxAllocArray (&isotopestruct->speciesname, sizeof (char),
                            2, nspecies, GXL_SPECIESNAME+1);
         if (!rc) rc = GxAllocArray (&isotopestruct->isovalue,
                                     sizeof (double), 1, nspecies);
         if (!rc) rc = GxAllocArray (&isotopestruct->valid,
                                     sizeof (GxBool), 1, nspecies);
      }
   }
   if (rc) {
      GxFreeIsotopeStruct (isotopestruct);
   } else {
      *isotopestructptr = isotopestruct;
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxAllocLaboratoryRunData - allocates memory for the laboratory run data
--------------------------------------------------------------------------------

 GxAllocLaboratoryRunData allocates memory for the laboratory run data. The
 information needed for the allocation is retrieved from the project and
 stored in the laboratory run data.

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocLaboratoryRunData (void)
{
   GxErrorCode  rc=GXE_NOERROR, rc2;
   RunOptions   *runoptions=NULL;

   assert (theConfiguration);
   assert (!theLaboratoryRunData);

/* Allocate the basic structure for the laboratory run data */
   theLaboratoryRunData = ( GxLaboratoryRunData * ) malloc (sizeof (GxLaboratoryRunData));
   if (!theLaboratoryRunData) {
      rc = GxError (GXE_MEMORY, GXT_MEMORY);
   } else {

/* -- Initialize the laboratory run data structure */
      theLaboratoryRunData->model         = NULL;
      theLaboratoryRunData->species       = NULL;
      theLaboratoryRunData->stoichiometry = NULL;
      theLaboratoryRunData->tphistory     = NULL;
      theLaboratoryRunData->netthickness  = NULL;
      theLaboratoryRunData->concentration = NULL;

/* -- Retrieve the run options from the project */
      rc = GxCreateRunOptions (&runoptions);
      if (!rc) rc = GxGetRunOptions (runoptions);
      if (!rc) {
         theLaboratoryRunData->referenceposition =
            runoptions->referenceposition;
      }

/* -- Create a model data structure using the information retrieved so far */
      if (!rc) rc = GxCreateModel (theConfiguration->nelements,
                       theConfiguration->nspecies,
                       theConfiguration->reactionscheme.nreactions,
                       1, 1, 1, runoptions->nnodes, runoptions->ntimesteps,
                       GX_HISTORYMASK | GXP_ISOTOPE, 
                       &theLaboratoryRunData->model);

/* -- Allocate arrays for species data, stoichiometry, t&p histories,
 * -- net thicknesses and initial concentrations
 */
      if (!rc) rc = GxAllocArray (&theLaboratoryRunData->species,
                                  sizeof (GxSpecies), 2, 1,
                                  theConfiguration->nspecies);
      if (!rc) rc = GxAllocArray (&theLaboratoryRunData->stoichiometry,
                                  sizeof (double), 3, 1,
                                  theConfiguration->reactionscheme.nreactions,
                                  theConfiguration->nspecies);
      if (!rc) rc = GxAllocArray (&theLaboratoryRunData->tphistory,
                                  sizeof (GxTPHistory), 2, 1, 1);
      if (!rc) rc = GxAllocArray (&theLaboratoryRunData->netthickness,
                                  sizeof (double), 3, 1, 1, 1);
      if (!rc) rc = GxAllocArray (&theLaboratoryRunData->concentration,
                                  sizeof (double), 2, 1,
                                  theConfiguration->nspecies);
   }
   if (runoptions) {
      rc2 = GxFreeRunOptions (runoptions);
      if (!rc) rc = rc2;
   }
   if (rc) GxFreeLaboratoryRunData ();
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxAllocOptimizeRunData - allocates memory for the optimize run data
--------------------------------------------------------------------------------

 GxAllocOptimizeRunData allocates memory for the characteristic length
 optimization loop run data. The information needed for the allocation is
 retrieved from the project and stored in the laboratory run data.

 Arguments
 ---------
 optrundata - pointer to the optimize run data struct

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocOptimizeRunData (GxOptimizeRunData *optrundata)
{
   GxErrorCode  rc=GXE_NOERROR, rc2;
   RunOptions   *runoptions=NULL;

   assert (theConfiguration);
   assert (optrundata);

/* Initialize the optimization run data structure */
   optrundata->geomodel         = NULL;
   optrundata->labmodel         = NULL;
   optrundata->geospecies       = NULL;
   optrundata->labspecies       = NULL;
   optrundata->stoichiometry    = NULL;
   optrundata->geotphistory     = NULL;
   optrundata->labtphistory     = NULL;
   optrundata->geonetthickness  = NULL;
   optrundata->labnetthickness  = NULL;
   optrundata->geoconcentration = NULL;
   optrundata->labconcentration = NULL;

/* Retrieve the run options from the project */
   rc = GxCreateRunOptions (&runoptions);
   if (!rc) rc = GxGetRunOptions (runoptions);
   if (!rc) {
      optrundata->maxiterations     = runoptions->maxchleniterations;
      optrundata->maxerror          = runoptions->maxchlenerror;
      optrundata->factor            = runoptions->chlenfactor;
      optrundata->referenceposition = runoptions->referenceposition;
   }

/* Create model data structures for the geological and laboratory run */
   if (!rc) rc = GxCreateModel (theConfiguration->nelements,
                    theConfiguration->nspecies,
                    theConfiguration->reactionscheme.nreactions,
                    1, 1, 1, runoptions->nnodes, runoptions->ntimesteps,
                    0L, &optrundata->geomodel);
   if (!rc) rc = GxCreateModel (theConfiguration->nelements,
                    theConfiguration->nspecies,
                    theConfiguration->reactionscheme.nreactions,
                    1, 1, 1, runoptions->nnodes, runoptions->ntimesteps,
                    GXP_TIME|GXP_TEMPERATURE|GXP_EXPULSIONFLUX,
                    &optrundata->labmodel);

/* Allocate arrays for species data, stoichiometry, t&p histories,
 * net thicknesses and initial concentrations
 */
   if (!rc) rc = GxAllocArray (&optrundata->geospecies, sizeof (GxSpecies),
                    2, 1, theConfiguration->nspecies);
   if (!rc) rc = GxAllocArray (&optrundata->labspecies, sizeof (GxSpecies),
                    2, 1, theConfiguration->nspecies);
   if (!rc) rc = GxAllocArray (&optrundata->stoichiometry, sizeof (double),
                    3, 1, theConfiguration->reactionscheme.nreactions,
                    theConfiguration->nspecies);
   if (!rc) rc = GxAllocArray (&optrundata->geotphistory,
                    sizeof (GxTPHistory), 2, 1, 1);
   if (!rc) rc = GxAllocArray (&optrundata->labtphistory,
                    sizeof (GxTPHistory), 2, 1, 1);
   if (!rc) rc = GxAllocArray (&optrundata->geonetthickness, sizeof (double),
                    3, 1, 1, 1);
   if (!rc) rc = GxAllocArray (&optrundata->labnetthickness, sizeof (double),
                    3, 1, 1, 1);
   if (!rc) rc = GxAllocArray (&optrundata->geoconcentration, sizeof (double),
                    2, 1, theConfiguration->nspecies);
   if (!rc) rc = GxAllocArray (&optrundata->labconcentration, sizeof (double),
                    2, 1, theConfiguration->nspecies);
   if (runoptions) {
      rc2 = GxFreeRunOptions (runoptions);
      if (!rc) rc = rc2;
   }
   if (rc) GxFreeOptimizeRunData (optrundata);
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCalculateCharge - calculate the hydrocarbon charge
--------------------------------------------------------------------------------

 GxCalculateCharge calculates the hydrocarbon charge from the model in
 both the perfectly open and perfectly closed case.

 Arguments
 ---------
 starttime    - start time for the expulsion calculations
 opencharge   - charge information in the perfectly open case
 closedcharge - charge information in the perfectly closed case

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCalculateCharge (double starttime, GxCharge *opencharge,
                               GxCharge *closedcharge)
{
   GxErrorCode  rc=GXE_NOERROR;
   double       *areas=NULL, *grossthickness=NULL, *porosity=NULL,
                *nettogross=NULL, traptemperature, trappressure, trapdepth,
                salinity;

   if (!opencharge || !closedcharge) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Allocate arrays for additional data to be retrieved from project */
         rc = GxAllocArray (&areas, sizeof (double), 1,
                            theGeologicalRunData->nlocations);
         if (!rc) rc = GxAllocArray (&grossthickness, sizeof (double), 1,
                                     theGeologicalRunData->nlocations);
         if (!rc) rc = GxAllocArray (&porosity, sizeof (double), 1,
                                     theGeologicalRunData->nlocations);
         if (!rc) rc = GxAllocArray (&nettogross, sizeof (double), 1,
                                     theGeologicalRunData->nlocations);

/* ----- Get additional data for charge calculations from project */
         if (!rc) rc = GxGetDataForCharge (areas, grossthickness, porosity,
                                           nettogross, &traptemperature,
                                           &trappressure, &trapdepth,
                                           &salinity);

/* ----- Perform the charge calculations on the model and data retrieved */
         if (!rc) {
            rc = GxGetCharge (theGeologicalRunData->model, starttime,
                                    areas, grossthickness, porosity,
                                    nettogross, traptemperature,
                                    trappressure, trapdepth, salinity,
                                    opencharge, closedcharge);
         }

/* ----- Free the arrays allocated for the retrieval of project data */
         if (areas) GxFreeArray (areas, 1, theGeologicalRunData->nlocations);
         if (grossthickness) GxFreeArray (grossthickness, 1,
                                          theGeologicalRunData->nlocations);
         if (porosity) GxFreeArray (porosity, 1,
                                    theGeologicalRunData->nlocations);
         if (nettogross) GxFreeArray (nettogross, 1,
                                      theGeologicalRunData->nlocations);
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxClose - close Genex for modeling
--------------------------------------------------------------------------------

 GxClose closes Genex for modeling by deallocating all memory allocated when
 the open function GxOpen was called.

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxClose (void)
{
   GxErrorCode  rc=GXE_NOERROR, rc2;

   if (!theConfiguration) {
      rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
   } else {

/* -- Deallocate the memory for the run data, project and configuration */
      rc2 = GxFreeGeologicalRunData ();
      if (!rc) rc = rc2;
      rc2 = GxFreeLaboratoryRunData ();
      if (!rc) rc = rc2;
      rc2 = GxFreeProject ();
      if (!rc) rc = rc2;
      rc2 = GxFreeConfiguration (theConfiguration);
      if (!rc) rc = rc2;
      theConfiguration = NULL;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxConvertExpulsion - convert expulsion data to different struct and units
--------------------------------------------------------------------------------

 GxConvertExpulsion converts the expulsion data from the model to a different
 structure and different units.

 Arguments
 ---------
 expulsion      - expulsion volumes for each of the species and lithologies
 oilexpulsion   - expulsion data for oil species
 gasexpulsion   - expulsion data for gas species
 nonhcexpulsion - expulsion data for non-hydrocarbon species
 gor1ptr        - pointer for gas oil ratio in standard units
 gor2ptr        - pointer for gas oil ratio in alternative units
 cgrptr         - pointer for condensate gas ratio

--------------------------------------------------------------------------------
*/

static void GxConvertExpulsion (double **expulsion, GxExpulsion *oilexpulsion,
                                GxExpulsion *gasexpulsion,
                                GxExpulsion *nonhcexpulsion,
                                double *gor1ptr, double *gor2ptr,
                                double *cgrptr)
{
   int     noils=0, ngasses=0, nnonhcs=0, i, j;

   assert (expulsion);
   assert (oilexpulsion);
   assert (gasexpulsion);
   assert (nonhcexpulsion);
   assert (gor1ptr);
   assert (gor2ptr);
   assert (cgrptr);

   oilexpulsion->total1       = 0.0;
   oilexpulsion->totalmass1   = 0.0;
   gasexpulsion->total1       = 0.0;
   gasexpulsion->totalmass1   = 0.0;
   nonhcexpulsion->total1     = 0.0;
   nonhcexpulsion->totalmass1 = 0.0;

/* Loop over all the species in the model */
   for (i=0; i<theConfiguration->nspecies; i++) {

/* -- Check the type of the species: oil, gas, or non hudrocarbon */
      if (i != GX_KEROGEN) {
         if (theConfiguration->species[i].mobile
            && (GxHasOilComposition (&theConfiguration->species[i])
               || ((i == GX_ASPHALTENE)
                  && GxIsAsphaltene (&theConfiguration->species[i])))) {

/* -------- Oil: copy name, calculate volume and mass and add to totals */
            strcpy (oilexpulsion->speciesname[noils],
                    theConfiguration->species[i].name);
            oilexpulsion->volume1[noils] = 0.0;
            oilexpulsion->mass1[noils]   = 0.0;
            for (j=0; j<theGeologicalRunData->nlithologies; j++) {
               oilexpulsion->volume1[noils] += expulsion[j][i] /
                  theGeologicalRunData->species[j][i].density;
               oilexpulsion->mass1[noils]   += expulsion[j][i];
            }
            oilexpulsion->volume2[noils] =
               GXC_SM3TOMB * oilexpulsion->volume1[noils];
            oilexpulsion->mass2[noils] =
               GXC_KGTOIMP * oilexpulsion->mass1[noils];
            oilexpulsion->total1     += oilexpulsion->volume1[noils];
            oilexpulsion->totalmass1 += oilexpulsion->mass1[noils];
            noils++;
         } else if (GxIsHCGas (&theConfiguration->species[i])) {

/* -------- Gas: copy name, calculate volume and mass and add to totals */
            strcpy (gasexpulsion->speciesname[ngasses],
                    theConfiguration->species[i].name);
            gasexpulsion->volume1[ngasses] = 0.0;
            gasexpulsion->mass1[ngasses]   = 0.0;
            for (j=0; j<theGeologicalRunData->nlithologies; j++) {
               gasexpulsion->volume1[ngasses] += expulsion[j][i] /
                  theGeologicalRunData->species[j][i].density;
               gasexpulsion->mass1[ngasses]   += expulsion[j][i];
            }
            gasexpulsion->volume2[ngasses] =
               GXC_SM3TOBCF * gasexpulsion->volume1[ngasses];
            gasexpulsion->mass2[ngasses] =
               GXC_KGTOIMP * gasexpulsion->mass1[ngasses];
            gasexpulsion->total1     += gasexpulsion->volume1[ngasses];
            gasexpulsion->totalmass1 += gasexpulsion->mass1[ngasses];
            ngasses++;
         } else if (!GxIsHydrocarbon (&theConfiguration->species[i])) {

/* -------- Non-hc: copy name, calculate volume and mass and add to totals */
            strcpy (nonhcexpulsion->speciesname[nnonhcs],
                    theConfiguration->species[i].name);
            nonhcexpulsion->volume1[nnonhcs] = 0.0;
            nonhcexpulsion->mass1[nnonhcs]   = 0.0;
            for (j=0; j<theGeologicalRunData->nlithologies; j++) {
               nonhcexpulsion->volume1[nnonhcs] += expulsion[j][i] /
                  theGeologicalRunData->species[j][i].density;
               nonhcexpulsion->mass1[nnonhcs]   += expulsion[j][i];
            }
            nonhcexpulsion->volume2[nnonhcs] =
               GXC_SM3TOBCF * nonhcexpulsion->volume1[nnonhcs];
            nonhcexpulsion->mass2[nnonhcs] =
               GXC_KGTOIMP * nonhcexpulsion->mass1[nnonhcs];
            nonhcexpulsion->total1     += nonhcexpulsion->volume1[nnonhcs];
            nonhcexpulsion->totalmass1 += nonhcexpulsion->mass1[nnonhcs];
            nnonhcs++;
         }
      }
   }

/* Convert totals to field units */
   oilexpulsion->total2       = GXC_SM3TOMB  * oilexpulsion->total1;
   oilexpulsion->totalmass2   = GXC_KGTOIMP  * oilexpulsion->totalmass1;
   gasexpulsion->total2       = GXC_SM3TOBCF * gasexpulsion->total1;
   gasexpulsion->totalmass2   = GXC_KGTOIMP * gasexpulsion->totalmass1;
   nonhcexpulsion->total2     = GXC_SM3TOBCF * nonhcexpulsion->total1;
   nonhcexpulsion->totalmass2 = GXC_KGTOIMP * nonhcexpulsion->totalmass1;

/* Calculate weight fractions (note that the name weightpct is misleading) */
   for (i=0; i<noils; i++) {
      if (oilexpulsion->totalmass1 > FLT_MIN) {
         oilexpulsion->weightpct[i] = oilexpulsion->mass1[i] /
                                      oilexpulsion->totalmass1;
      } else {
         oilexpulsion->weightpct[i]  = 0.0;
      }
   }
   for (i=0; i<ngasses; i++) {
      if (gasexpulsion->totalmass1 > FLT_MIN) {
         gasexpulsion->weightpct[i] = gasexpulsion->mass1[i] /
                                      gasexpulsion->totalmass1;
      } else {
         gasexpulsion->weightpct[i]  = 0.0;
      }
   }
   for (i=0; i<nnonhcs; i++) {
      if (nonhcexpulsion->totalmass1 > FLT_MIN) {
         nonhcexpulsion->weightpct[i] = nonhcexpulsion->mass1[i] /
                                        nonhcexpulsion->totalmass1;
      } else {
         nonhcexpulsion->weightpct[i]  = 0.0;
      }
   }

/* Calculate GOR and CGR in different units */
   if (oilexpulsion->total1 > FLT_MIN) {
      *gor1ptr = gasexpulsion->total1 / oilexpulsion->total1;
   } else {
      *gor1ptr = 1.0E+20;
   }
   if (*gor1ptr < 10000) {
      *gor2ptr = *gor1ptr * GXC_GORCONVERSION;
      *cgrptr  = 0.0;
   } else {
      *gor2ptr = 0.0;
      *cgrptr  = 1.0 / *gor1ptr;
   }
}


/*
--------------------------------------------------------------------------------
 GxConvertGasMolarogram - convert gas expulsion data to the gas molarogram
                          struct
--------------------------------------------------------------------------------

 GxConvertGasMolarogram converts the gas expulsion data from the selected 
 submodels to a gas molarogram structure.

 Arguments
 ---------
 gasexpulsion  - expulsion data for gas species
 gasmolarogram - gas molarogram data structure

--------------------------------------------------------------------------------
*/

static void GxConvertGasMolarogram (GxExpulsion *gasexpulsion,
                                    GxGasMolarogram *gasmolarogram)
{
   double   totalVolume,
           *volume = NULL;
   int      i;

   assert (gasexpulsion);
   assert (gasmolarogram);

/* Specify all data as being invalid --- */
   for (i=0; i<gasexpulsion->nspecies; i++) {
       gasmolarogram->valid[i] = GxFalse;
   }   

   totalVolume  = 0.0;
   volume = (double *)malloc (gasexpulsion->nspecies * sizeof (double));
  
   if (volume) { 

/* --- Reset the volume array */
       for (i=0; i<gasexpulsion->nspecies; i++) {
           volume[i] = 0.0;
       }   

/* --- Loop over all the species in the gas expulsion structure */
       for (i=0; i<gasexpulsion->nspecies; i++) {
           totalVolume += gasexpulsion->volume1[i];
      
           if (strcmp (gasexpulsion->speciesname[i], C1_SPECIES) == 0) {
               volume[4] = gasexpulsion->volume1[i];
           }

           if (strcmp (gasexpulsion->speciesname[i], C2_SPECIES) == 0) {
               volume[3] = gasexpulsion->volume1[i];
           }

           if (strcmp (gasexpulsion->speciesname[i], C3_SPECIES) == 0) {
               volume[2] = gasexpulsion->volume1[i];
           }

           if (strcmp (gasexpulsion->speciesname[i], C4_SPECIES) == 0) {
               volume[1] = gasexpulsion->volume1[i];
           } 

           if (strcmp (gasexpulsion->speciesname[i], C5_SPECIES) == 0) {
               volume[0] = gasexpulsion->volume1[i];
           } 
       }

/* --- Calculate the values for the carbon numbers */
       strcpy (gasmolarogram->speciesname[4], C1_SPECIES);
       strcpy (gasmolarogram->speciesname[3], C2_SPECIES);
       strcpy (gasmolarogram->speciesname[2], C3_SPECIES);
       strcpy (gasmolarogram->speciesname[1], C4_SPECIES);
       strcpy (gasmolarogram->speciesname[0], C5_SPECIES);
       
       for (i=0; i<gasexpulsion->nspecies; i++) {
           if (volume[i] > 0.0) {
               gasmolarogram->valid[i] = GxTrue;
               gasmolarogram->ydata[i] = volume[i] / totalVolume; 
               gasmolarogram->xdata[i] = (double)(gasexpulsion->nspecies - i);
           }
       }
       free (volume);
   }
} 


/*
--------------------------------------------------------------------------------
 GxConvertGeneration - convert generation data to different struct and units
--------------------------------------------------------------------------------

 GxConvertGeneration converts the generation data from the model to a different
 structure and different units.

 Arguments
 ---------
 generation      - generation rates for each of the species and lithologies
 oilgeneration   - generation data for oil species
 gasgeneration   - generation data for gas species
 nonhcgeneration - generation data for non-hydrocarbon species

--------------------------------------------------------------------------------
*/

static void GxConvertGeneration (double **generation, GxGenRate *oilgeneration,
                                 GxGenRate *gasgeneration,
                                 GxGenRate *nonhcgeneration)
{
   int     noils=0, ngasses=0, nnonhcs=0, i, j;

   assert (generation);
   assert (oilgeneration);
   assert (gasgeneration);
   assert (nonhcgeneration);

   oilgeneration->total1       = 0.0;
   oilgeneration->totalmass1   = 0.0;
   gasgeneration->total1       = 0.0;
   gasgeneration->totalmass1   = 0.0;
   nonhcgeneration->total1     = 0.0;
   nonhcgeneration->totalmass1 = 0.0;

/* Loop over all the species in the model */
   for (i=0; i<theConfiguration->nspecies; i++) {

/* -- Check the type of the species: oil, gas, or non hydrocarbon */
      if (i != GX_KEROGEN) {
         if (theConfiguration->species[i].mobile
            && (GxHasOilComposition (&theConfiguration->species[i])
               || ((i == GX_ASPHALTENE)
                  && GxIsAsphaltene (&theConfiguration->species[i])))) {

/* -------- Oil: copy name, calculate rate and mass and add to totals */
            strcpy (oilgeneration->speciesname[noils],
                    theConfiguration->species[i].name);
            oilgeneration->volume1[noils]   = 0.0;
            oilgeneration->mass1[noils]     = 0.0;
            for (j=0; j<theGeologicalRunData->nlithologies; j++) {
               oilgeneration->volume1[noils] += generation[j][i] /
                  theGeologicalRunData->species[j][i].density;
               oilgeneration->mass1[noils]   += generation[j][i];
            }
            oilgeneration->volume2[noils] =
               GXC_SM3TOMB * oilgeneration->volume1[noils];
            oilgeneration->mass2[noils] =
               GXC_KGTOIMP * oilgeneration->mass1[noils];
            oilgeneration->total1 += oilgeneration->volume1[noils];
            oilgeneration->totalmass1 += oilgeneration->mass1[noils];
            noils++;
         } else if (GxIsHCGas (&theConfiguration->species[i])) {

/* -------- Gas: copy name, calculate rate and mass and add to totals */
            strcpy (gasgeneration->speciesname[ngasses],
                    theConfiguration->species[i].name);
            gasgeneration->volume1[ngasses] = 0.0;
            gasgeneration->mass1[ngasses]   = 0.0;
            for (j=0; j<theGeologicalRunData->nlithologies; j++) {
               gasgeneration->volume1[ngasses] += generation[j][i] /
                  theGeologicalRunData->species[j][i].density;
               gasgeneration->mass1[ngasses]   += generation[j][i];
            }
            gasgeneration->volume2[ngasses] =
               GXC_SM3TOBCF * gasgeneration->volume1[ngasses];
            gasgeneration->mass2[ngasses] =
               GXC_KGTOIMP * gasgeneration->mass1[ngasses];
            gasgeneration->total1     += gasgeneration->volume1[ngasses];
            gasgeneration->totalmass1 += gasgeneration->mass1[ngasses];
            ngasses++;
         } else if (!GxIsHydrocarbon (&theConfiguration->species[i])) {

/* -------- Non-hc: copy name, calculate rate and mass and add to totals */
            strcpy (nonhcgeneration->speciesname[nnonhcs],
                    theConfiguration->species[i].name);
            nonhcgeneration->volume1[nnonhcs]   = 0.0;
            nonhcgeneration->mass1[nnonhcs] = 0.0;
            for (j=0; j<theGeologicalRunData->nlithologies; j++) {
               nonhcgeneration->volume1[nnonhcs] += generation[j][i] /
                  theGeologicalRunData->species[j][i].density;
               nonhcgeneration->mass1[nnonhcs]   += generation[j][i];
            }
            nonhcgeneration->volume2[nnonhcs] =
               GXC_SM3TOBCF * nonhcgeneration->volume1[nnonhcs];
            nonhcgeneration->mass2[nnonhcs] =
               GXC_KGTOIMP * nonhcgeneration->mass1[nnonhcs];
            nonhcgeneration->total1     += nonhcgeneration->volume1[nnonhcs];
            nonhcgeneration->totalmass1 += nonhcgeneration->mass1[nnonhcs];
            nnonhcs++;
         }
      }
   }

/* Convert totals to field units */
   oilgeneration->total2       = GXC_SM3TOMB  * oilgeneration->total1;
   oilgeneration->totalmass2   = GXC_KGTOIMP  * oilgeneration->totalmass1;
   gasgeneration->total2       = GXC_SM3TOBCF * gasgeneration->total1;
   gasgeneration->totalmass2   = GXC_KGTOIMP * gasgeneration->totalmass1;
   nonhcgeneration->total2     = GXC_SM3TOBCF * nonhcgeneration->total1;
   nonhcgeneration->totalmass2 = GXC_KGTOIMP * nonhcgeneration->totalmass1;

/* Calculate weight fractions (note that the name weightpct is misleading) */
   for (i=0; i<noils; i++) {
      if (oilgeneration->totalmass1 > FLT_MIN) {
         oilgeneration->weightpct[i] = oilgeneration->mass1[i] /
                                       oilgeneration->totalmass1;
      } else {
         oilgeneration->weightpct[i]  = 0.0;
      }
   }
   for (i=0; i<ngasses; i++) {
      if (gasgeneration->totalmass1 > FLT_MIN) {
         gasgeneration->weightpct[i] = gasgeneration->mass1[i] /
                                       gasgeneration->totalmass1;
      } else {
         gasgeneration->weightpct[i]  = 0.0;
      }
   }
   for (i=0; i<nnonhcs; i++) {
      if (nonhcgeneration->totalmass1 > FLT_MIN) {
         nonhcgeneration->weightpct[i] = nonhcgeneration->mass1[i] /
                                         nonhcgeneration->totalmass1;
      } else {
         nonhcgeneration->weightpct[i]  = 0.0;
      }
   }
}


/*
--------------------------------------------------------------------------------
 GxConvertIsotopes - convert isotope data to different struct
--------------------------------------------------------------------------------

 GxConvertIsotopes converts the isotope data from the model to a different
 structure

 Arguments
 ---------
 expulsion      - expulsion volumes for each of the species and lithologies
 isotope        - isotope data for each of the species and lithologies
 oilisotope     - expulsion data for oil species
 gasisotope     - expulsion data for gas species
 nonhcisotope   - expulsion data for non-hydrocarbon species

--------------------------------------------------------------------------------
*/

static void GxConvertIsotopes (double **expulsion, double **isotope, 
                               GxIsotope *oilisotope, GxIsotope *gasisotope,
                               GxIsotope *nonhcisotope)
{
   int     noils=0, ngasses=0, nnonhcs=0, i, j;
   double  totalweightisotope, totalexp;

   assert (expulsion);
   assert (isotope);
   assert (oilisotope);
   assert (gasisotope);
   assert (nonhcisotope);

/* Loop over all the species in the model */
   for (i=0; i<theConfiguration->nspecies; i++) {

/* -- Check the type of the species: oil, gas, or non hudrocarbon */
      if (i != GX_KEROGEN) {
         if (theConfiguration->species[i].mobile
            && (GxHasOilComposition (&theConfiguration->species[i])
               || ((i == GX_ASPHALTENE)
                  && GxIsAsphaltene (&theConfiguration->species[i])))) {

/* -------- Oil: copy name, calculate volume and mass and add to totals */
            strcpy (oilisotope->speciesname[noils],
                    theConfiguration->species[i].name);
            if (GxHasFractionationData (i)) {
               totalexp = 0.0;
               totalweightisotope = 0.0;
 
               for (j=0; j<theGeologicalRunData->nlithologies; j++) {
                  totalexp += expulsion[j][i];
                  totalweightisotope = expulsion[j][i] * isotope[j][i];
               }
               if (totalexp > 0.0) {
                  oilisotope->isovalue[noils] = totalweightisotope / totalexp;
                  oilisotope->valid[noils] = GxTrue;
               } else {
                  oilisotope->valid[noils] = GxFalse;
               }
            } else {
               oilisotope->valid[noils] = GxFalse;
            }
            noils++;
         } else if (GxIsHCGas (&theConfiguration->species[i])) {

/* -------- Gas: copy name, calculate volume and mass and add to totals */
            strcpy (gasisotope->speciesname[ngasses],
                    theConfiguration->species[i].name);
            if (GxHasFractionationData (i)) {
               totalexp = 0.0;
               totalweightisotope = 0.0;
 
               for (j=0; j<theGeologicalRunData->nlithologies; j++) {
                  totalexp += expulsion[j][i];
                  totalweightisotope = expulsion[j][i] * isotope[j][i];
               }
               if (totalexp > 0.0) {
                  gasisotope->isovalue[ngasses] = totalweightisotope / 
                                                  totalexp;
                  gasisotope->valid[ngasses] = GxTrue;
               } else {
                  gasisotope->valid[ngasses] = GxFalse;
               }
            } else {
               gasisotope->valid[ngasses] = GxFalse;
            }
            ngasses++;
         } else if (!GxIsHydrocarbon (&theConfiguration->species[i])) {

/* -------- Non-hc: copy name, calculate volume and mass and add to totals */
            strcpy (nonhcisotope->speciesname[nnonhcs],
                    theConfiguration->species[i].name);
            if (GxHasFractionationData (i)) {
               totalexp = 0.0;
               totalweightisotope = 0.0;
 
               for (j=0; j<theGeologicalRunData->nlithologies; j++) {
                  totalexp += expulsion[j][i];
                  totalweightisotope = expulsion[j][i] * isotope[j][i];
               }
               if (totalexp > 0.0) {
                  nonhcisotope->isovalue[nnonhcs] = totalweightisotope / 
                                                    totalexp;
                  nonhcisotope->valid[nnonhcs] = GxTrue;
               } else {
                  nonhcisotope->valid[nnonhcs] = GxFalse;
               }
            } else {
               nonhcisotope->valid[nnonhcs] = GxFalse;
            }
            nnonhcs++;
         }
      }
   }
}


/*
--------------------------------------------------------------------------------
 GxCreateModelHistory - allocated the memory allocated for a model history
--------------------------------------------------------------------------------

 GxCreateModelHistory allocates the memory that is needed for retrieval of a
 model history. The memory should be freed by calling GxFreeModelHistory.

 Arguments
 ---------
 ntimesteps      - the number of time steps
 modelhistoryptr - a pointer for the model history

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCreateModelHistory (int ntimesteps, double **modelhistoryptr)
{
   GxErrorCode  rc=GXE_NOERROR;

   if ((ntimesteps <= 0) || !modelhistoryptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      rc = GxAllocArray (modelhistoryptr, sizeof (double), 1, ntimesteps);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCreateTPHistory - create a t & p history from data in arrays
--------------------------------------------------------------------------------

 GxCreateTPHistory converts temperature and pressure data in separate arrays
 to a temperature and pressure data structure. It is checked that the
 temperature and pressure history has at least two times.

 Arguments
 ---------
 ntimes       - number of times in the temperature and pressure data
 time         - the times for the t&p history
 depth        - the depths for the t&p history or NULL if unavailable
 temperature  - the temperatures for the t&p history
 pressure     - the pressures for the t&p history
 overbdens    - the density of the overburden
 tempgradient - the temperature gradient
 surfacetemp  - the surface temperature
 tphistory    - temperature and pressure data structure

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxCreateTPHistory (int ntimes, double *time, double *depth,
                                      double *temperature, double *pressure,
                                      double overbdens, double tempgradient,
                                      double surfacetemp,
                                      GxTPHistory *tphistory)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i;
   double       tempdep;
   GxBool       depthpresent=GxFalse;

   assert (ntimes >= 2);
   assert (time);
   assert (temperature);
   assert (pressure);
   assert (overbdens > 0.0);
   assert (tempgradient > 0.0);
   assert (surfacetemp >= 0.0);
   assert (tphistory);

/* Initialize the temperature and pressure structure */
   tphistory->ntimes      = ntimes;
   tphistory->time        = NULL;
   tphistory->temperature = NULL;
   tphistory->pressure    = NULL;

/* Allocate arrays for time, temperature and pressure data */
   rc = GxAllocArray (&tphistory->time, sizeof (double),
                      1, tphistory->ntimes);
   if (!rc) rc = GxAllocArray (&tphistory->temperature, sizeof (double),
                               1, tphistory->ntimes);
   if (!rc) rc = GxAllocArray (&tphistory->pressure, sizeof (double),
                               1, tphistory->ntimes);
   if (!rc) {

/* -- Check whether depth data is available and not all zero */
      if (depth)
         for (i=0; (!depthpresent)&&(i<ntimes); i++)
            if (depth[i] != 0.0) depthpresent = GxTrue;

/* -- For all times for which temperature and pressure data is available */
      for (i=0; i<ntimes; i++) {

/* ----- Copy time and temperature values */
         tphistory->time[i] = time[i];
         tphistory->temperature[i] = temperature[i];

/* ----- If pressure is unavailable but depth is: calculate lithostatic
 * ----- pressure from depth, otherwise first calculate a depth from the
 * ----- temperature
 */
         if (pressure[i] == 0.0) {
            if (depthpresent) {
               tphistory->pressure[i] = GxLithostaticPressure (depth[i],
                                                               overbdens);
            } else {
               tempdep = GxDepth (temperature[i], tempgradient, surfacetemp);
               tphistory->pressure[i] = GxLithostaticPressure (tempdep,
                                                               overbdens);
            }
         } else {
            tphistory->pressure[i] = pressure[i];
         }
      }
   }
   if (rc) GxFreeTPHistory (tphistory);
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFillGeologicalRunData - fill in the geological run data from the project
--------------------------------------------------------------------------------

 GxFillGeologicalRunData retrieves the data needed for a geological model run
 from the project and stores the data in the geological run data struct.

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxFillGeologicalRunData (void)
{
   GxErrorCode     rc=GXE_NOERROR, rc2;
   RefConditions   *refconditions=NULL;
   int             i, j, k;
   Rock            *rock=NULL;
   SubArea         *subarea=NULL;
   HorizonHistory  *horhis=NULL;
   Lithology       *lithology=NULL;
   char            *subareaname, *horizonname, *lithologyname;
   double          aromchain;

   assert (theConfiguration);
   assert (theConfiguration->species);
   assert (theGeologicalRunData);
   assert (theGeologicalRunData->species);
   assert (theGeologicalRunData->stoichiometry);
   assert (theGeologicalRunData->tphistory);
   assert (theGeologicalRunData->charlength);
   assert (theGeologicalRunData->upperbiot);
   assert (theGeologicalRunData->lowerbiot);
   assert (theGeologicalRunData->netthickness);
   assert (theGeologicalRunData->concentration);
   assert (theGeologicalRunData->initkerogenconc);
   assert (theGeologicalRunData->subareas);
   assert (theGeologicalRunData->horizons);
   assert (theGeologicalRunData->lithologies);
   assert (theGeologicalRunData->isofactor);

/* Create temporary data structures for retrieval of data from project */
   rc = GxCreateRock (&rock);
   if (!rc) rc = GxCreateSubArea (&subarea);
   if (!rc) rc = GxCreateLithology (&lithology);
   if (!rc) rc = GxCreateHorizonHistory (&horhis);
   if (!rc) rc = GxCreateRefConditions (&refconditions);

/* Get the reference conditions from the project */
   if (!rc) rc = GxGetRefConditions (refconditions);

/* Retrieve the data for each of the lithologies from the project */
   i = 0;
   lithologyname = GetFirstFromSortedList (theGeologicalRunData->lithologies);
   while (!rc && lithologyname) {
      rc = GxGetLithology (lithologyname, lithology);
      if (!rc) {
         theGeologicalRunData->charlength[i] = lithology->charlength;
         theGeologicalRunData->upperbiot[i]  = lithology->upperbiot;
         theGeologicalRunData->lowerbiot[i]  = lithology->lowerbiot;

/* ----- Copy species data and calculate the variable parameters */
         for (j=0; j<theConfiguration->nspecies; j++)
            theGeologicalRunData->species[i][j] =
               theConfiguration->species[j];
         rc = GxCalcSpecies (theConfiguration->nelements,
               theConfiguration->nspecies, theGeologicalRunData->species[i],
               lithology->hci, lithology->oci, lithology->nci,
               lithology->kerogenlowact, lithology->kerogenhighact, 
               lithology->asphaltenelowact, lithology->asphaltenehighact);

/* ----- Calculate the stoichiometry of the reaction scheme */
         if (!rc) rc = GxCalcStoichiometry (theConfiguration->nelements,
                          theConfiguration->nspecies,
                          theGeologicalRunData->species[i],
                          &theConfiguration->reactionscheme,
                          theGeologicalRunData->stoichiometry[i]);
         if (!rc) {

/* -------- Determine the aromaticity of chains */
            aromchain = GxAromaticityOfChains (theConfiguration->nspecies,
                           theGeologicalRunData->species[i],
                      (const double **)theGeologicalRunData->stoichiometry[i]);

/* -------- Calculate the reference diffusivity for the mobile species */
            for (j=0; j<theConfiguration->nspecies; j++) {
               if (theGeologicalRunData->species[i][j].mobile) {
                  theGeologicalRunData->species[i][j].referencediffusivity =
                     GxReferenceDiffusivity (theGeologicalRunData->species[i],
                        j, aromchain, refconditions->geolreftemperature,
                        refconditions->geolrefpressure,
                        refconditions->geolrefwbo,
                        theConfiguration->expansivity,
                        theConfiguration->compressibility, GX_GEOLOGICALTIME);

               }
            }

/* -------- Determine the initial concentrations for the model run */
            rc = GxCalcInitialConc (theConfiguration->nspecies,
                    theGeologicalRunData->species[i],
                    lithology->toci, theConfiguration->inorganicdensity,
                    theGeologicalRunData->concentration[i]);
            if (!rc) theGeologicalRunData->initkerogenconc[i] =
                        theGeologicalRunData->concentration[i][GX_KEROGEN];
         }

         if (!rc) {
/* -------- Get the isotope fractionation data from configuration and */
/* -------- lithology data structure */
            theGeologicalRunData->isofactor[i].alphalow  = 
                              lithology->fractionationfactor;
            theGeologicalRunData->isofactor[i].highpercentage =
                              theConfiguration->isofract.factor.highpercentage;
            theGeologicalRunData->isofactor[i].templow  =
                              theConfiguration->isofract.factor.templow;
            theGeologicalRunData->isofactor[i].temphigh =
                              theConfiguration->isofract.factor.temphigh;
 
            GxCalcIsoFactorConst (&theGeologicalRunData->isofactor[i]); 
         }
      }
      i++;
      lithologyname =
         GetNextFromSortedList (theGeologicalRunData->lithologies);
   }

/* Initialize the temperature and pressure history data */
   for (i=0; i<theGeologicalRunData->nlocations; i++)
      for (j=0; j<theGeologicalRunData->nhorizons; j++)
         theGeologicalRunData->tphistory[i][j].ntimes = 0;

/* Loop over all subareas and horizons in the model */
   if (!rc) {
      i = 0;
      subareaname = GetFirstFromSortedList (theGeologicalRunData->subareas);
      while (!rc && subareaname) {
         rc = GxGetSubArea (subareaname, subarea);
         j = 0;
         horizonname =
            GetFirstFromSortedList (theGeologicalRunData->horizons);
         while (!rc && horizonname) {
            rc = GxGetHorizonHistory (horizonname, subareaname, horhis);
            if (!rc && (horhis->ntimes < 2))
               rc = GxError (GXE_NOTPHISTDATA, GXT_NOTPHISTDATA);

/* -------- Create a temperature and pressure history from horizon history */
            if (!rc) rc = GxCreateTPHistory (horhis->ntimes, horhis->time,
                             horhis->depth, horhis->temperature,
                             horhis->pressure,
                             theConfiguration->overburdendensity,
                             theConfiguration->temperaturegradient,
                             theConfiguration->surfacetemperature,
                             &theGeologicalRunData->tphistory[i][j]);
            k = 0;
            lithologyname =
               GetFirstFromSortedList (theGeologicalRunData->lithologies);
            while (!rc && lithologyname) {

/* ----------- Get the net thickness for the submodel from the project */
               rc = GxGetRock (lithologyname, subareaname, horizonname,
                               rock);
               if (!rc)
                  theGeologicalRunData->netthickness[i][j][k] =
                     rock->thickness;
               
               k++;
               lithologyname =
                  GetNextFromSortedList (theGeologicalRunData->lithologies);
            }
            j++;
            horizonname =
               GetNextFromSortedList (theGeologicalRunData->horizons);
         }
         i++;
         subareaname = GetNextFromSortedList (theGeologicalRunData->subareas);
      }
   }

/* Free the temporary data structures */
   if (horhis) {
      rc2 = GxFreeHorizonHistory (horhis);
      if (!rc) rc = rc2;
   }
   if (lithology) {
      rc2 = GxFreeLithology (lithology);
      if (!rc) rc = rc2;
   }
   if (subarea) {
      rc2 = GxFreeSubArea (subarea);
      if (!rc) rc = rc2;
   }
   if (rock) {
      rc2 = GxFreeRock (rock);
      if (!rc) rc = rc2;
   }
   if (refconditions) {
      rc2 = GxFreeRefConditions (refconditions);
      if (!rc) rc = rc2;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFillLaboratoryRunData - fill in the laboratory run data from the project
--------------------------------------------------------------------------------

 GxFillLaboratoryRunData retrieves the data needed for a laboratory model run
 from the project and stores the data in the laboratory run data struct.

 Arguments
 ---------
 subarea        - subarea where sample should be taken
 horizon        - horizon where sample should be taken
 lithology      - lithology where sample should be taken
 sampleposition - the sample position relative to characteristic length

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxFillLaboratoryRunData (char *subarea, char *horizon,
                                            char *lithology,
                                            double sampleposition)
{
   GxErrorCode     rc=GXE_NOERROR, rc2;
   int             subareanr, horizonnr, lithologynr, i, j;
   LabExperiment  *labexperiment=NULL;
   RefConditions  *refconditions=NULL;
   Lithology      *lithol=NULL;
   double          aromchain;

   assert (subarea);
   assert (horizon);
   assert (lithology);
   assert ((sampleposition >= 0.0) && (sampleposition <= 1.0));
   assert (theConfiguration);
   assert (theLaboratoryRunData);
   assert (theLaboratoryRunData->species);
   assert (theLaboratoryRunData->stoichiometry);
   assert (theLaboratoryRunData->tphistory);
   assert (theLaboratoryRunData->netthickness);
   assert (theLaboratoryRunData->concentration);

/* Check that the submodel specified actually exists */
   rc = GxFindSubModel (subarea, horizon, lithology, &subareanr, &horizonnr,
                        &lithologynr);

/* Retrieve the reference conditions from the project */
   if (!rc) rc = GxCreateRefConditions (&refconditions);
   if (!rc) rc = GxGetRefConditions (refconditions);
   if (!rc) {

/* -- Calculate the aromaticity of chains */
      aromchain = GxAromaticityOfChains (theConfiguration->nspecies,
                     theGeologicalRunData->species[lithologynr],
           (const double **) theGeologicalRunData->stoichiometry[lithologynr]);

/* -- Use the same species data except for reference diffusivity */
      for (i=0; i<theConfiguration->nspecies; i++) {
         theLaboratoryRunData->species[0][i] =
            theGeologicalRunData->species[lithologynr][i];
         if (theLaboratoryRunData->species[0][i].mobile) {
            theLaboratoryRunData->species[0][i].referencediffusivity =
               GxReferenceDiffusivity (theLaboratoryRunData->species[0],
                                       i, aromchain,
                                       refconditions->labreftemperature,
                                       refconditions->labrefpressure,
                                       refconditions->labrefwbo,
                                       theConfiguration->expansivity,
                                       theConfiguration->compressibility,
                                       GX_LABORATORYTIME);
         }
      }

/* -- Use the same stoichiometry as for the submodel */
      for (i=0; i<theConfiguration->reactionscheme.nreactions; i++) {
         for (j=0; j<theConfiguration->nspecies; j++) {
            theLaboratoryRunData->stoichiometry[0][i][j] =
               theGeologicalRunData->stoichiometry[lithologynr][i][j];
         }
      }

/* -- Copy lab experiment data */
      theLaboratoryRunData->tphistory[0][0].ntimes = 0;
      rc = GxCreateLabExperiment (&labexperiment);
      if (!rc) {
         rc = GxGetLabExperiment (labexperiment);
         if (!rc) {
            theLaboratoryRunData->experimenttype = labexperiment->type;
            theLaboratoryRunData->charlength     = labexperiment->charlength;
            theLaboratoryRunData->upperbiot      = labexperiment->upperbiot;
            theLaboratoryRunData->lowerbiot      = labexperiment->lowerbiot;
            theLaboratoryRunData->netthickness[0][0][0] =
               labexperiment->netthickness;
            rc = GxCreateTPHistory (labexperiment->ntimes,
                    labexperiment->time, NULL, labexperiment->temperature,
                    labexperiment->pressure,
                    theConfiguration->overburdendensity,
                    theConfiguration->temperaturegradient,
                    theConfiguration->surfacetemperature,
                    &theLaboratoryRunData->tphistory[0][0]);
         }
         rc2 = GxFreeLabExperiment (labexperiment);
         if (!rc) rc = rc2;
      }

/* -- Determine concentration and other parameters at sample position */
      if (!rc) rc = GxSampleSubModel (theGeologicalRunData->model, subareanr,
                       horizonnr, lithologynr, sampleposition,
                       theLaboratoryRunData->concentration[0],
                       &theLaboratoryRunData->initkerogenconc,
                       &theLaboratoryRunData->density,
                       &theLaboratoryRunData->toc);

/* -- Get the isotope fractionation data from configuration and */
/* -- lithology data structure */
      if (!rc) {
         if (!rc) rc = GxCreateLithology (&lithol);
    
         if (!rc) {
            rc = GxGetLithology (lithology, lithol);
            if (!rc) {
               theLaboratoryRunData->isofactor.alphalow  = 
                              lithol->fractionationfactor;
               theLaboratoryRunData->isofactor.highpercentage =
                              theConfiguration->isofract.factor.highpercentage;
               theLaboratoryRunData->isofactor.templow  =
                              theConfiguration->isofract.factor.templow;
               theLaboratoryRunData->isofactor.temphigh =
                              theConfiguration->isofract.factor.temphigh;
 
               GxCalcIsoFactorConst (&theLaboratoryRunData->isofactor); 
            }
            rc2 = GxFreeLithology (lithol);
            if (!rc) rc = rc2;
         }
      }
   }

/* Free the temporary data structures */
   if (refconditions) {
      rc2 = GxFreeRefConditions (refconditions);
      if (!rc) rc = rc2;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFillOptimizeRunData - fill in the run data for the char length optimization
--------------------------------------------------------------------------------

 GxFillOptimizeRunData retrieves the data needed for the optimization loop
 of the characteristic length from the project and stores the data in the
 optimize run data struct.

 Arguments
 ---------
 subareaname   - name of subarea of submodel to run optimization on
 horizonname   - name of horizon of submodel to run optimization on
 lithologyname - name of lithology of submodel to run optimization on
 optrundata    - pointer to the optimize run data struct

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxFillOptimizeRunData (char *subareaname,
                                          char *horizonname,
                                          char *lithologyname,
                                          GxOptimizeRunData *optrundata)
{
   GxErrorCode     rc=GXE_NOERROR, rc2;
   Rock            *rock=NULL;
   Lithology       *lithology=NULL;
   RefConditions   *refconditions=NULL;
   HorizonHistory  *horhis=NULL;
   LabExperiment   *labexperiment=NULL;
   int             i;
   double          aromchain;

   assert (subareaname);
   assert (horizonname);
   assert (lithologyname);
   assert (optrundata);
   assert (optrundata->geospecies);
   assert (optrundata->labspecies);
   assert (optrundata->stoichiometry);
   assert (optrundata->geotphistory);
   assert (optrundata->labtphistory);
   assert (optrundata->geonetthickness);
   assert (optrundata->labnetthickness);
   assert (optrundata->geoconcentration);
   assert (theConfiguration);
   assert (theConfiguration->species);

/* Get the reference conditions from the project */
   rc = GxCreateRefConditions (&refconditions);
   if (!rc) rc = GxGetRefConditions (refconditions);

/* Get the laboratory experiment parameters from the project */
   if (!rc) rc = GxCreateLabExperiment (&labexperiment);
   if (!rc) rc = GxGetStandardLabExperiment (GXLE_ROCKEVAL2, labexperiment);

/* Create other temporary data structures and retrieve data from project */
   if (!rc) rc = GxCreateHorizonHistory (&horhis);
   if (!rc) rc = GxCreateRock (&rock);
   if (!rc) rc = GxGetRock (lithologyname, subareaname, horizonname, rock);
   if (!rc) rc = GxCreateLithology (&lithology);
   if (!rc) rc = GxGetLithology (lithologyname, lithology);
   if (!rc) {

/* -- Determine the species data for the geological simulation */
      for (i=0; i<theConfiguration->nspecies; i++)
         optrundata->geospecies[0][i] = theConfiguration->species[i];
      rc = GxCalcSpecies (theConfiguration->nelements,
             theConfiguration->nspecies, optrundata->geospecies[0],
             lithology->hci, lithology->oci, lithology->nci,
             lithology->kerogenlowact, lithology->kerogenhighact, 
             lithology->asphaltenelowact, lithology->asphaltenehighact);

/* -- Calculate the stoichiometry for the geological simulation */
      if (!rc) rc = GxCalcStoichiometry (theConfiguration->nelements,
                       theConfiguration->nspecies, optrundata->geospecies[0],
                       &theConfiguration->reactionscheme,
                       optrundata->stoichiometry[0]);
      if (!rc) {

/* ----- Determine the aromaticity of chains */
         aromchain = GxAromaticityOfChains (theConfiguration->nspecies,
                        optrundata->geospecies[0],
                       (const double **) optrundata->stoichiometry[0]);

/* ----- Use the same species data for lab simulation except for ref.diff. */
         for (i=0; i<theConfiguration->nspecies; i++) {
            optrundata->labspecies[0][i] = optrundata->geospecies[0][i];
            if (optrundata->geospecies[0][i].mobile) {
               optrundata->geospecies[0][i].referencediffusivity =
                  GxReferenceDiffusivity (optrundata->geospecies[0],
                     i, aromchain, refconditions->geolreftemperature,
                     refconditions->geolrefpressure,
                     refconditions->geolrefwbo,
                     theConfiguration->expansivity,
                     theConfiguration->compressibility, GX_GEOLOGICALTIME);
               optrundata->labspecies[0][i].referencediffusivity =
                  GxReferenceDiffusivity (optrundata->labspecies[0],
                     i, aromchain, refconditions->labreftemperature,
                     refconditions->labrefpressure, refconditions->labrefwbo,
                     theConfiguration->expansivity,
                     theConfiguration->compressibility, GX_LABORATORYTIME);
            }
         }

/* ----- Calculate the initial concentrations for each of the species */
         rc = GxCalcInitialConc (theConfiguration->nspecies,
                 optrundata->geospecies[0], lithology->toci,
                 theConfiguration->inorganicdensity,
                 optrundata->geoconcentration[0]);
         if (!rc) optrundata->initkerogenconc =
                     optrundata->geoconcentration[0][GX_KEROGEN];
      }
   }

/* Create the temperature and pressure histories for both simulations */
   optrundata->geotphistory[0][0].ntimes = 0;
   optrundata->labtphistory[0][0].ntimes = 0;
   if (!rc) rc = GxGetHorizonHistory (horizonname, subareaname, horhis);
   if (!rc && (horhis->ntimes < 2))
      rc = GxError (GXE_NOTPHISTDATA, GXT_NOTPHISTDATA);
   if (!rc) rc = GxCreateTPHistory (horhis->ntimes, horhis->time,
                    horhis->depth, horhis->temperature, horhis->pressure,
                    theConfiguration->overburdendensity,
                    theConfiguration->temperaturegradient,
                    theConfiguration->surfacetemperature,
                    &optrundata->geotphistory[0][0]);
   if (!rc) rc = GxCreateTPHistory (labexperiment->ntimes,
                    labexperiment->time, NULL, labexperiment->temperature,
                    labexperiment->pressure,
                    theConfiguration->overburdendensity,
                    theConfiguration->temperaturegradient,
                    theConfiguration->surfacetemperature,
                    &optrundata->labtphistory[0][0]);

   if (!rc) {

/* -- Store the remaining data into the optimization data structure */
      optrundata->geocharlength            = lithology->charlength;
      optrundata->geoupperbiot             = lithology->upperbiot;
      optrundata->geolowerbiot             = lithology->lowerbiot;
      optrundata->geonetthickness[0][0][0] = rock->thickness;
      optrundata->s1tocm                   = rock->s1m / rock->tocm;
      optrundata->labcharlength            = labexperiment->charlength;
      optrundata->labupperbiot             = labexperiment->upperbiot;
      optrundata->lablowerbiot             = labexperiment->lowerbiot;
      optrundata->labnetthickness[0][0][0] = labexperiment->netthickness;
   }

/* Free the memory allocated for temporary data structures */
   if (lithology) {
      rc2 = GxFreeLithology (lithology);
      if (!rc) rc = rc2;
   }
   if (rock) {
      rc2 = GxFreeRock (rock);
      if (!rc) rc = rc2;
   }
   if (horhis) {
      rc2 = GxFreeHorizonHistory (horhis);
      if (!rc) rc = rc2;
   }
   if (labexperiment) {
      rc2 = GxFreeLabExperiment (labexperiment);
      if (!rc) rc = rc2;
   }
   if (refconditions) {
      rc2 = GxFreeRefConditions (refconditions);
      if (!rc) rc = rc2;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFindSpecies - locate a particular species in the geological model
--------------------------------------------------------------------------------

 GxFindSpecies searches for the species identified by <species> in the
 geological model. If it is found the index of the species is returned.
 Otherwise an error is generated.

 Arguments
 ---------
 speciesname  - name of species that is to be found
 speciesnrptr - pointer for species number

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxFindSpecies (char *speciesname, int *speciesnrptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          speciesnr;

   assert (speciesname);
   assert (speciesnrptr);
   assert (theConfiguration);
   assert (theGeologicalRunData);
   assert (theGeologicalRunData->species);

/* Look for a particular species name in the list of species */
   speciesnr = 0;
   while ((speciesnr < theConfiguration->nspecies)
      && (strcmp (theConfiguration->species[speciesnr].name,
                  speciesname) != 0))
      speciesnr++;

/* If not found return an error otherwise return the sequence number */
   if (speciesnr == theConfiguration->nspecies) {
      rc = GxError (GXE_NOTASPECIES, GXT_NOTASPECIES, speciesname);
   } else {
      *speciesnrptr = speciesnr;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFindSubModel - locate a particular submodel in the geological model
--------------------------------------------------------------------------------

 GxFindSubModel searches for the submodel identified by <subarea>, <horizon>
 and <lithology> in the geological model. If it is found the indices of the
 submodel are returned. Otherwise an error is generated.

 Arguments
 ---------
 subarea        - subarea of submodel that is to be found
 horizon        - horizon of submodel that is to be found
 lithology      - lithology of submodel that is to be found
 subareanrptr   - pointer for subarea number
 horizonnrptr   - pointer for horizon number
 lithologynrptr - pointer for lithology number

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxFindSubModel (char *subarea, char *horizon,
                                   char *lithology, int *subareanrptr,
                                   int *horizonnrptr, int *lithologynrptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          subareanr, horizonnr, lithologynr;
   char         *subareaname, *horizonname, *lithologyname;

   assert (subarea);
   assert (horizon);
   assert (lithology);
   assert (subareanrptr);
   assert (horizonnrptr);
   assert (lithologynrptr);
   assert (theGeologicalRunData);
   assert (theGeologicalRunData->subareas);
   assert (theGeologicalRunData->horizons);
   assert (theGeologicalRunData->lithologies);

/* Look for a particular subarea name in the list of modelled subareas */
   subareanr = 0;
   subareaname = GetFirstFromSortedList (theGeologicalRunData->subareas);
   while (subareaname && (strcmp (subareaname, subarea) != 0)) {
      subareanr++;
      subareaname = GetNextFromSortedList (theGeologicalRunData->subareas);
   }

/* Look for a particular horizon name in the list of modelled horizons */
   horizonnr = 0;
   horizonname = GetFirstFromSortedList (theGeologicalRunData->horizons);
   while (horizonname && (strcmp (horizonname, horizon) != 0)) {
      horizonnr++;
      horizonname = GetNextFromSortedList (theGeologicalRunData->horizons);
   }

/* Look for a particular lithology name in the list of modelled lithology */
   lithologynr = 0;
   lithologyname = GetFirstFromSortedList (theGeologicalRunData->lithologies);
   while (lithologyname && (strcmp (lithologyname, lithology) != 0)) {
      lithologynr++;
      lithologyname =
         GetNextFromSortedList (theGeologicalRunData->lithologies);
   }

/* If any of the three was not found return an error */
   if (!subareaname || !horizonname || !lithologyname) {
      rc = GxError (GXE_NOTASUBMODEL, GXT_NOTASUBMODEL, subarea, horizon,
                    lithology);
   } else {
      *subareanrptr   = subareanr;
      *horizonnrptr   = horizonnr;
      *lithologynrptr = lithologynr;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeExpulsion - free the memory allocated for expulsion data
--------------------------------------------------------------------------------

 GxFreeExpulsion frees the memory that was allocated by
 GxGetDownwardExpulsion, GxGetMainExpulsion or GxGetUpwardExpulsion.

 Arguments
 ---------
 oilexpulsion   - the oil expulsion data
 gasexpulsion   - the gas expulsion data
 nonhcexpulsion - the non-hydrocarbon expulsion data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxFreeExpulsion (GxExpulsion *oilexpulsion,
                             GxExpulsion *gasexpulsion,
                             GxExpulsion *nonhcexpulsion)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!oilexpulsion || !gasexpulsion || !nonhcexpulsion) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Free the structures allocated for oils, gasses and non hydrocarbons */
      GxFreeExpulsionStruct (oilexpulsion);
      GxFreeExpulsionStruct (gasexpulsion);
      GxFreeExpulsionStruct (nonhcexpulsion);
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxFreeGasMolarogram - free the memory allocated for gas molarogram data
--------------------------------------------------------------------------------

 GxFreeGasMolarogram frees the memory that was allocated for gas molarogram 
 data

 Arguments
 ---------
 gasmolarogram  - the gas molarogram data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxFreeGasMolarogram (GxGasMolarogram *gasmolarogram)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!gasmolarogram) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Free the structures allocated for gas molarogram data */
      GxFreeGasMolarogramStruct (gasmolarogram);
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxFreeGeneration - free the memory allocated for generation data
--------------------------------------------------------------------------------

 GxFreeGeneration frees the memory that was allocated by GxGetGeneration 

 Arguments
 ---------
 oilgeneration   - the oil generation data
 gasgeneration   - the gas generation data
 nonhcgeneration - the non-hydrocarbon generation data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxFreeGeneration (GxGenRate *oilgeneration,
                              GxGenRate *gasgeneration,
                              GxGenRate *nonhcgeneration)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!oilgeneration || !gasgeneration || !nonhcgeneration) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Free the structures allocated for oils, gasses and non hydrocarbons */
      GxFreeGenerationStruct (oilgeneration);
      GxFreeGenerationStruct (gasgeneration);
      GxFreeGenerationStruct (nonhcgeneration);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeExpulsionStruct - free the structure for expulsion data
--------------------------------------------------------------------------------

 GxFreeExpulsionStruct deallocates the memory for the expulsion data
 structure, that was allocated by GxAllocExpulsionStruct.

 Arguments
 ---------
 expulsionstruct - struct for expulsion data

--------------------------------------------------------------------------------
*/

static void GxFreeExpulsionStruct (GxExpulsion *expulsionstruct)
{
   if (expulsionstruct) {

/* -- Free the arrays for species names, volumes and weight fractions */
      if (expulsionstruct->speciesname)
         GxFreeArray (expulsionstruct->speciesname, 2,
                      expulsionstruct->nspecies, GXL_SPECIESNAME+1);
      if (expulsionstruct->volume1)
         GxFreeArray (expulsionstruct->volume1, 1, expulsionstruct->nspecies);
      if (expulsionstruct->volume2)
         GxFreeArray (expulsionstruct->volume2, 1, expulsionstruct->nspecies);
      if (expulsionstruct->mass1)
         GxFreeArray (expulsionstruct->mass1, 1, expulsionstruct->nspecies);
      if (expulsionstruct->mass2)
         GxFreeArray (expulsionstruct->mass2, 1, expulsionstruct->nspecies);
      if (expulsionstruct->weightpct)
         GxFreeArray (expulsionstruct->weightpct, 1,
                      expulsionstruct->nspecies);
      free (expulsionstruct);
   }
}


/*
--------------------------------------------------------------------------------
 GxFreeGasMolarogramStruct - free the structure for gas molarogram data
--------------------------------------------------------------------------------

 GxFreeGasMolarogramStruct deallocates the memory for the gas molarogram data
 structure, that was allocated by GxAllocGasMolarogramStruct.

 Arguments
 ---------
 gasmolarogramstruct - struct for gas molarogram data

--------------------------------------------------------------------------------
*/

static void GxFreeGasMolarogramStruct (GxGasMolarogram *gasmolarogramstruct)
{
   if (gasmolarogramstruct) {

/* -- Free the arrays for species names, volumes and weight fractions */
      if (gasmolarogramstruct->speciesname)
         GxFreeArray (gasmolarogramstruct->speciesname, 2,
                      gasmolarogramstruct->nspecies, GXL_SPECIESNAME+1);
      if (gasmolarogramstruct->xdata) {
         GxFreeArray (gasmolarogramstruct->xdata, 1, 
                      gasmolarogramstruct->nspecies);
      }
      if (gasmolarogramstruct->ydata) {
         GxFreeArray (gasmolarogramstruct->ydata, 1, 
                      gasmolarogramstruct->nspecies);
      }
      if (gasmolarogramstruct->valid) {
         GxFreeArray (gasmolarogramstruct->valid, 1, 
                      gasmolarogramstruct->nspecies);
      }
      free (gasmolarogramstruct);
   }
}


/*
--------------------------------------------------------------------------------
 GxFreeGenerationStruct - free the structure for generation data
--------------------------------------------------------------------------------

 GxFreeGenerationStruct deallocates the memory for the generation data
 structure, that was allocated by GxAllocGenerationStruct.

 Arguments
 ---------
 generationstruct - struct for generation data

--------------------------------------------------------------------------------
*/

static void GxFreeGenerationStruct (GxGenRate *generationstruct)
{
   if (generationstruct) {

/* -- Free the arrays for species names, volumes and weight fractions */
      if (generationstruct->speciesname)
         GxFreeArray (generationstruct->speciesname, 2,
                      generationstruct->nspecies, GXL_SPECIESNAME+1);
      if (generationstruct->volume1)
         GxFreeArray (generationstruct->volume1, 1, generationstruct->nspecies);
      if (generationstruct->volume2)
         GxFreeArray (generationstruct->volume2, 1, generationstruct->nspecies);
      if (generationstruct->mass1)
         GxFreeArray (generationstruct->mass1, 1, generationstruct->nspecies);
      if (generationstruct->mass2)
         GxFreeArray (generationstruct->mass2, 1, generationstruct->nspecies);
      if (generationstruct->weightpct)
         GxFreeArray (generationstruct->weightpct, 1,
                      generationstruct->nspecies);
      free (generationstruct);
   }
}


/*
--------------------------------------------------------------------------------
 GxFreeGeologicalRunData - free the memory allocated for geological run data
--------------------------------------------------------------------------------

 GxFreeGeologicalRunData frees the memory that was allocated for the
 data for the run of the geological model.

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxFreeGeologicalRunData (void)
{
   GxErrorCode  rc=GXE_NOERROR, rc2;
   int          i, j;

   if (theGeologicalRunData) {

/* -- Free the arrays allocated for isotope factor, concentrations, 
 * -- net thicknesses, Biot numbers, characteristic lengths, temperature and 
 * -- pressure data, stoichiometry and species data
 */
      if (theGeologicalRunData->isofactor)
         GxFreeArray (theGeologicalRunData->isofactor, 1,
                      theGeologicalRunData->nlithologies);
      if (theGeologicalRunData->initkerogenconc)
         GxFreeArray (theGeologicalRunData->initkerogenconc, 1,
                      theGeologicalRunData->nlithologies);
      if (theGeologicalRunData->concentration)
         GxFreeArray (theGeologicalRunData->concentration, 2,
                      theGeologicalRunData->nlithologies,
                      theConfiguration->nspecies);
      if (theGeologicalRunData->netthickness)
         GxFreeArray (theGeologicalRunData->netthickness, 3,
                      theGeologicalRunData->nlocations,
                      theGeologicalRunData->nhorizons,
                      theGeologicalRunData->nlithologies);
      if (theGeologicalRunData->lowerbiot)
         GxFreeArray (theGeologicalRunData->lowerbiot, 1,
                      theGeologicalRunData->nlithologies);
      if (theGeologicalRunData->upperbiot)
         GxFreeArray (theGeologicalRunData->upperbiot, 1,
                      theGeologicalRunData->nlithologies);
      if (theGeologicalRunData->charlength)
         GxFreeArray (theGeologicalRunData->charlength, 1,
                      theGeologicalRunData->nlithologies);
      if (theGeologicalRunData->tphistory) {
         for (i=0; i<theGeologicalRunData->nlocations; i++)
            for (j=0; j<theGeologicalRunData->nhorizons; j++)
               GxFreeTPHistory (&theGeologicalRunData->tphistory[i][j]);
         GxFreeArray (theGeologicalRunData->tphistory, 2,
                      theGeologicalRunData->nlocations,
                      theGeologicalRunData->nhorizons);
      }
      if (theGeologicalRunData->stoichiometry)
         GxFreeArray (theGeologicalRunData->stoichiometry, 3,
                      theGeologicalRunData->nlithologies,
                      theConfiguration->reactionscheme.nreactions,
                      theConfiguration->nspecies);
      if (theGeologicalRunData->species)
         GxFreeArray (theGeologicalRunData->species, 2,
                      theGeologicalRunData->nlithologies,
                      theConfiguration->nspecies);

/* -- Deallocate the model data structure */
      if (theGeologicalRunData->model) {
         rc2 = GxDestroyModel (theGeologicalRunData->model);
         if (!rc) rc = rc2;
      }

/* -- Free the lists of names of the lithologies, horizons and subareas */
      if (theGeologicalRunData->lithologies)
         DeleteSortedList (theGeologicalRunData->lithologies);
      if (theGeologicalRunData->horizons)
         DeleteSortedList (theGeologicalRunData->horizons);
      if (theGeologicalRunData->subareas)
         DeleteSortedList (theGeologicalRunData->subareas);

/* -- Free the geological run data structure itself */
      free (theGeologicalRunData);
      theGeologicalRunData = NULL;
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxFreeIsotope - free the memory allocated for isotope data
--------------------------------------------------------------------------------

 GxFreeIsotope frees the memory that was allocated by
 GxGetDownwardIsotopeSelected, GxGetMainIsotopeSelected or 
 GxGetUpwardIsotopeSelected.

 Arguments
 ---------
 oilisotope   - the oil isotope data
 gasisotope   - the gas isotope data
 nonhcisotope - the non-hydrocarbon isotope data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxFreeIsotope (GxIsotope *oilisotope, GxIsotope *gasisotope, 
                           GxIsotope *nonhcisotope)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!oilisotope || !gasisotope || !nonhcisotope) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Free the structures allocated for oils, gasses and non hydrocarbons */
      GxFreeIsotopeStruct (oilisotope);
      GxFreeIsotopeStruct (gasisotope);
      GxFreeIsotopeStruct (nonhcisotope);
   }
   return (rc);
}
 

/*
--------------------------------------------------------------------------------
 GxFreeIsotopeStruct - free the structure for isotope data
--------------------------------------------------------------------------------

 GxFreeIsotopeStruct deallocates the memory for the isotope data
 structure, that was allocated by GxAllocIsotopeStruct.

 Arguments
 ---------
 isotopestruct - struct for isotope data

--------------------------------------------------------------------------------
*/

static void GxFreeIsotopeStruct (GxIsotope *isotopestruct)
{
   if (isotopestruct) {

/* -- Free the arrays for species names, volumes and weight fractions */
      if (isotopestruct->speciesname)
         GxFreeArray (isotopestruct->speciesname, 2,
                      isotopestruct->nspecies, GXL_SPECIESNAME+1);
      if (isotopestruct->isovalue)
         GxFreeArray (isotopestruct->isovalue, 1, isotopestruct->nspecies);
      if (isotopestruct->valid)
         GxFreeArray (isotopestruct->valid, 1, isotopestruct->nspecies);
      free (isotopestruct);
   }
}


/*
--------------------------------------------------------------------------------
 GxFreeLaboratoryRunData - free the memory allocated for laboratory run data
--------------------------------------------------------------------------------

 GxFreeLaboratoryRunData frees the memory that was allocated for the
 data for the run of the laboratory model.

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxFreeLaboratoryRunData (void)
{
   GxErrorCode  rc=GXE_NOERROR, rc2;

   if (theLaboratoryRunData) {

/* -- Free the arrays allocated for isotope factor, concentrations, 
 * -- net thicknesses, temperature and pressure data, stoichiometry and 
 * -- species data
 */
      if (theLaboratoryRunData->concentration)
         GxFreeArray (theLaboratoryRunData->concentration, 2, 1,
                      theConfiguration->nspecies);
      if (theLaboratoryRunData->netthickness)
         GxFreeArray (theLaboratoryRunData->netthickness, 3, 1, 1, 1);
      if (theLaboratoryRunData->tphistory) {
         GxFreeTPHistory (&theLaboratoryRunData->tphistory[0][0]);
         GxFreeArray (theLaboratoryRunData->tphistory, 2, 1, 1);
      }
      if (theLaboratoryRunData->stoichiometry)
         GxFreeArray (theLaboratoryRunData->stoichiometry, 3, 1,
                      theConfiguration->reactionscheme.nreactions,
                      theConfiguration->nspecies);
      if (theLaboratoryRunData->species)
         GxFreeArray (theLaboratoryRunData->species, 2, 1,
                      theConfiguration->nspecies);

/* -- Deallocate the model data structure */
      if (theLaboratoryRunData->model) {
         rc2 = GxDestroyModel (theLaboratoryRunData->model);
         if (!rc) rc = rc2;
      }

/* -- Free the laboratory run data structure itself */
      free (theLaboratoryRunData);
      theLaboratoryRunData = NULL;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeModelHistory - free the memory allocated for a model history
--------------------------------------------------------------------------------

 GxFreeModelHistory frees the memory that was allocated by
 GxCreateModelHistory for the property history of a submodel.

 Arguments
 ---------
 modelhistory - the model history

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxFreeModelHistory (double *modelhistory)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!modelhistory) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      GxFreeArray (modelhistory, 1);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeOptimizeRunData - free the memory allocated for the optimize run data
--------------------------------------------------------------------------------

 GxFreeOptimizeRunData frees the memory that was allocated for the
 data for the optimization loop of the characteristic length.

 Arguments
 ---------
 optrundata - pointer to the optimize run data struct

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxFreeOptimizeRunData (GxOptimizeRunData *optrundata)
{
   GxErrorCode  rc=GXE_NOERROR, rc2;

   assert (optrundata);


/* -- Free the arrays allocated for concentrations, net thicknesses,
 * -- temperature and pressure data, stoichiometry and species data
 */
   if (optrundata->labconcentration) {
      GxFreeArray (optrundata->labconcentration, 2, 1,
                   theConfiguration->nspecies);
      optrundata->labconcentration = NULL;
   }
   if (optrundata->geoconcentration) {
      GxFreeArray (optrundata->geoconcentration, 2, 1,
                   theConfiguration->nspecies);
      optrundata->geoconcentration = NULL;
   }
   if (optrundata->labnetthickness) {
      GxFreeArray (optrundata->labnetthickness, 3, 1, 1, 1);
      optrundata->labnetthickness = NULL;
   }
   if (optrundata->geonetthickness) {
      GxFreeArray (optrundata->geonetthickness, 3, 1, 1, 1);
      optrundata->geonetthickness = NULL;
   }
   if (optrundata->labtphistory) {
      GxFreeTPHistory (&optrundata->labtphistory[0][0]);
      GxFreeArray (optrundata->labtphistory, 2, 1, 1);
      optrundata->labtphistory = NULL;
   }
   if (optrundata->geotphistory) {
      GxFreeTPHistory (&optrundata->geotphistory[0][0]);
      GxFreeArray (optrundata->geotphistory, 2, 1, 1);
      optrundata->geotphistory = NULL;
   }
   if (optrundata->stoichiometry) {
      GxFreeArray (optrundata->stoichiometry, 3, 1,
                   theConfiguration->reactionscheme.nreactions,
                   theConfiguration->nspecies);
      optrundata->stoichiometry = NULL;
   }
   if (optrundata->labspecies) {
      GxFreeArray (optrundata->labspecies, 2, 1, theConfiguration->nspecies);
      optrundata->labspecies = NULL;
   }
   if (optrundata->geospecies) {
      GxFreeArray (optrundata->geospecies, 2, 1, theConfiguration->nspecies);
      optrundata->geospecies = NULL;
   }

/* Deallocate the model data structures */
   if (optrundata->labmodel) {
      rc2 = GxDestroyModel (optrundata->labmodel);
      if (!rc) rc = rc2;
      optrundata->labmodel = NULL;
   }
   if (optrundata->geomodel) {
      rc2 = GxDestroyModel (optrundata->geomodel);
      if (!rc) rc = rc2;
      optrundata->geomodel = NULL;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeTPHistory  - free the memory allocated for the t & p history
--------------------------------------------------------------------------------

 GxFreeTPHistory frees the memory that was allocated for the temperature and
 pressure history data.

 Arguments
 ---------
 tphistory - temperature and pressure history

--------------------------------------------------------------------------------
*/

static void GxFreeTPHistory (GxTPHistory *tphistory)
{
   if (tphistory && (tphistory->ntimes > 0)) {

/* -- Free the arrays for the times, temperatures and pressures */
      if (tphistory->time)
         GxFreeArray (tphistory->time, 1, tphistory->ntimes);
      if (tphistory->temperature)
         GxFreeArray (tphistory->temperature, 1, tphistory->ntimes);
      if (tphistory->pressure)
         GxFreeArray (tphistory->pressure, 1, tphistory->ntimes);
   }
}



/*
--------------------------------------------------------------------------------
 GxGeologicalModelPresent - returns true if a geological model is present
--------------------------------------------------------------------------------

 GxGeologicalModelPresent returns a value of GxTrue if a geological model
 is present and GxFalse if no geological model is present.

 Return value
 ------------
 GxTrue if geological model present, GxFalse otherwise

--------------------------------------------------------------------------------
*/

GxBool GxGeologicalModelPresent (void)
{
   return (theGeologicalRunData ? GxTrue : GxFalse);
}


/*
--------------------------------------------------------------------------------
 GxGetCnSpeciesName - Retrieves the species name of Cn
--------------------------------------------------------------------------------

 GxGetCnSpeciesName retrieves the species name of Cn

 Arguments
 ---------
 carbonnr - Carbon number

 Return value
 ------------
 name of species identified by carbonnr

--------------------------------------------------------------------------------
*/

char *GxGetCnSpeciesName (int carbonnr)
{
   char *speciesname;

   if (carbonnr < 1 || carbonnr > GXC_MAX_CARBONNR) {
      speciesname = NULL;
   } else {
      speciesname = theConfiguration->species[GX_C1 - carbonnr + 1].name; 
   }

   return speciesname;
}


/*
--------------------------------------------------------------------------------
 GxGetDefIsoFractFactor - returns the default value for the isotope 
                          fractionation factor alpha 
--------------------------------------------------------------------------------

 GxGetDefIsoFractFactor returns the default isotope fractionation factor 
 alpha which has been read in from the configuration file.

 Return value
 ------------
 The default isotope fractionation factor alpha

--------------------------------------------------------------------------------
*/

double GxGetDefIsoFractFactor (void)
{
   return (theConfiguration ? theConfiguration->isofract.factor.alphalow :
                              0.0);
}



/*
--------------------------------------------------------------------------------
 GxGetDataForCharge - get the data for the charge calculations
--------------------------------------------------------------------------------

 GxGetDataForCharge returns the data for the charge calculations.

 Arguments
 ---------
 areas          - array for the areas
 grossthickness - array for the gross thickness
 porosity       - array for the porosity
 nettogross     - array for the net to gross
 temperatureptr - pointer for the trap temperature
 pressureptr    - pointer for the trap pressure
 depthptr       - pointer for the depth of the top of the trap
 salinityptr    - pointer for water salinity

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxGetDataForCharge (double *areas, double *grossthickness,
                                       double *porosity, double *nettogross,
                                       double *temperatureptr,
                                       double *pressureptr, double *depthptr,
                                       double *salinityptr)
{
   GxErrorCode       rc=GXE_NOERROR, rc2;
   int               i, index;
   char              *trapname, *subareaname;
   SortedList        trapnamelist=NULL;
   SubArea           *subarea=NULL;
   Trap              *trap=NULL;
   ReservoirSubArea  *ressub=NULL;

   assert (areas);
   assert (grossthickness);
   assert (porosity);
   assert (nettogross);
   assert (temperatureptr);
   assert (pressureptr);
   assert (depthptr);
   assert (salinityptr);
   assert (theGeologicalRunData);
   assert (theGeologicalRunData->subareas);

/* Get the names of the traps from the project */
   rc = GxGetTrapNameList (&trapnamelist);

/* Create temporary data structures for retrieval of data from project */
   if (!rc) rc = GxCreateTrap (&trap);
   if (!rc) rc = GxCreateSubArea (&subarea);
   if (!rc) rc = GxCreateReservoirSubArea (&ressub);
   if (!rc) {

/* -- Get the data of the first trap (there is currently only one) */
      trapname = GetFirstFromSortedList (trapnamelist);
      if (!trapname) {
         rc = GxError (GXE_NOTRAP, GXT_NOTRAP);
      } else {
         rc = GxGetTrap (trapname, trap);
         if (!rc) {

/* -------- Get subarea and reservoir subarea parameters needed */
            i = 0;
            subareaname =
               GetFirstFromSortedList (theGeologicalRunData->subareas);
            while (!rc && subareaname) {
               rc = GxGetSubArea (subareaname, subarea);
               if (!rc) rc = GxGetReservoirSubArea (trap->reservoir,
                                                    subareaname, ressub);
               areas[i]          = subarea->area;
               grossthickness[i] = ressub->grossThickness;
               porosity[i]       = ressub->netAvgPorosity;
               nettogross[i]     = ressub->netToGross;
               i++;
               subareaname =
                  GetNextFromSortedList (theGeologicalRunData->subareas);
            }
            if (!rc) {

/* ----------- Get t&p data for trap of type field directly or retrieve from
 * ----------- the t&p history for a trap of type prospect (if available)
 */
               if (trap->type == GX_FIELD) {
                  *temperatureptr = trap->hwctemperature;
                  *pressureptr    = trap->hwcpressure;
               } else if (trap->ntimes > 0) {
                  index = MAX (0, MIN (trap->ntimes-1,
                             GxSearchDouble (trap->ntimes, trap->time, 0.0)));
                  *temperatureptr = trap->temperature[index];
                  *pressureptr    = trap->pressure[index];
               } else {
                  *temperatureptr = trap->hwctemperature;
                  *pressureptr    = trap->hwcpressure;
               }
            }
            if (!rc) {

/* ----------- Determine the top depth of the trap */
               index = GxSearchDouble (trap->nslices, trap->area, 0.0);
               *depthptr = trap->depth[index];

/* ----------- Get the salinity from the reservoir subarea */
               rc = GxGetReservoirSubArea (trap->reservoir, trap->crest,
                                           ressub);
               if (!rc) *salinityptr = ressub->waterSalinity;
            }
         }
      }
   }

/* Free the temporary data structures */
   if (trapnamelist) DeleteSortedList (trapnamelist);
   if (trap) {
      rc2 = GxFreeTrap (trap);
      if (!rc) rc = rc2;
   }
   if (subarea) {
      rc2 = GxFreeSubArea (subarea);
      if (!rc) rc = rc2;
   }
   if (ressub) {
      rc2 = GxFreeReservoirSubArea (ressub);
      if (!rc) rc = rc2;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetDownwardExpulsion - get the downward expulsion from the geological model
--------------------------------------------------------------------------------

 GxGetDownwardExpulsion allocates and returns the data for the downward
 expulsion in the geological model since the time specified. The memory for
 the expulsion data should be freed by calling GxFreeExpulsion.

 Arguments
 ---------
 starttime         - start time for the expulsion calculations
 oilexpulsionptr   - pointer for the oil expulsion data
 gasexpulsionptr   - pointer for the gas expulsion data
 nonhcexpulsionptr - pointer for the non-hydrocarbon expulsion data
 gor1ptr           - pointer for gas oil ratio in standard units
 gor2ptr           - pointer for gas oil ratio in alternative units
 cgrptr            - pointer for condensate gas ratio

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetDownwardExpulsion (double starttime,
                                    GxExpulsion **oilexpulsionptr,
                                    GxExpulsion **gasexpulsionptr,
                                    GxExpulsion **nonhcexpulsionptr,
                                    double *gor1ptr, double *gor2ptr,
                                    double *cgrptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxExpulsion  *oilexpulsion=NULL, *gasexpulsion=NULL, *nonhcexpulsion=NULL;
   double       *areas=NULL, **expulsions=NULL;

   if (!oilexpulsionptr || !gasexpulsionptr || !nonhcexpulsionptr || !gor1ptr
      || !gor2ptr || !cgrptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Get the areas of the subareas from the project */
         rc = GxAllocArray (&areas, sizeof (double), 1,
                            theGeologicalRunData->nlocations);
         if (!rc) rc = GxGetSubAreaData (areas);

/* ----- Allocate memory and receive the expulsion data in it */
         if (!rc) rc = GxAllocArray (&expulsions, sizeof (double), 2,
                                     theGeologicalRunData->nlithologies,
                                     theConfiguration->nspecies);
         if (!rc) rc = GxGetTotDownExpulsion (theGeologicalRunData->model,
                                              starttime, areas, expulsions);

/* ----- Allocate the expulsion structures and convert the data to them */
         if (!rc) rc = GxAllocExpulsion (&oilexpulsion, &gasexpulsion,
                                         &nonhcexpulsion);
         if (!rc) GxConvertExpulsion (expulsions, oilexpulsion,
                                      gasexpulsion, nonhcexpulsion,
                                      gor1ptr, gor2ptr, cgrptr);

/* ----- Free the temporary memory */
         if (areas)
            GxFreeArray (areas, 1, theGeologicalRunData->nlocations);
         if (expulsions)
            GxFreeArray (expulsions, 2, theGeologicalRunData->nlithologies,
                         theConfiguration->nspecies);
      }
   }
   if (!rc) {
      *oilexpulsionptr   = oilexpulsion;
      *gasexpulsionptr   = gasexpulsion;
      *nonhcexpulsionptr = nonhcexpulsion;
   }
   return (rc);
}
   

/*
--------------------------------------------------------------------------------
 GxGetDownwardExpulsionSelected - get the downward expulsion from the selected 
                                  submodels in the geological model
--------------------------------------------------------------------------------

 GxGetDownwardExpulsionSelected allocates and returns the data for the downward
 expulsion of the selected submodels in the geological model since the time 
 specified. The memory for the expulsion data should be freed by calling 
 GxFreeExpulsion.

 Arguments
 ---------
 subareas          - list of subareas to be used
 horizons          - list of horizons to be used
 lithologies       - list of lithologies to be used
 starttime         - start time for the expulsion calculations
 oilexpulsionptr   - pointer for the oil expulsion data
 gasexpulsionptr   - pointer for the gas expulsion data
 nonhcexpulsionptr - pointer for the non-hydrocarbon expulsion data
 gor1ptr           - pointer for gas oil ratio in standard units
 gor2ptr           - pointer for gas oil ratio in alternative units
 cgrptr            - pointer for condensate gas ratio

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetDownwardExpulsionSelected (SortedList subareas,
                                            SortedList horizons,
                                            SortedList lithologies,
                                            double starttime,
                                            GxExpulsion **oilexpulsionptr,
                                            GxExpulsion **gasexpulsionptr,
                                            GxExpulsion **nonhcexpulsionptr,
                                            double *gor1ptr, double *gor2ptr,
                                            double *cgrptr)
{
   GxErrorCode   rc=GXE_NOERROR;
   GxExpulsion  *oilexpulsion=NULL, 
                *gasexpulsion=NULL, 
                *nonhcexpulsion=NULL;
   double       *areas=NULL, 
               **expulsions=NULL;
   char         *subarea,
                *horizon,
                *lithology;
   GxSubModelId  submodelId,
                *tempId;
   List          submodelList = NULL;

   if (!subareas || !horizons || !lithologies || !oilexpulsionptr || 
       !gasexpulsionptr || !nonhcexpulsionptr || !gor1ptr || !gor2ptr || 
       !cgrptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      *oilexpulsionptr   = NULL;
      *gasexpulsionptr   = NULL;
      *nonhcexpulsionptr = NULL;

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Get the areas of the subareas from the project */
         rc = GxAllocArray (&areas, sizeof (double), 1,
                            theGeologicalRunData->nlocations);
         if (!rc) rc = GxGetSubAreaData (areas);

/* ----- Construct a sorted list with the submodel identifications */
         if (!rc) {
             submodelList = CreateList (sizeof (GxSubModelId), NULL);
             if (!submodelList) {
                 rc = GxError ( GXE_LIST, GXT_LIST );
             } else {
                 subarea = GetFirstFromSortedList (subareas);
                 while (subarea && !rc) {
                     horizon = GetFirstFromSortedList (horizons);
                     while (horizon && !rc) {
                         lithology = GetFirstFromSortedList (lithologies);
                         while (lithology && !rc) {
                             rc = GxFindSubModel (subarea, horizon, lithology,
                                                  &submodelId.subareanr,
                                                  &submodelId.horizonnr,
                                                  &submodelId.lithologynr);
                             if (!rc) {
                                 tempId = AppendToList (submodelList,
                                                        &submodelId);
                                 if (!tempId) {
                                     rc = GxError ( GXE_LIST, GXT_LIST );
                                 }
                             }
                             lithology = GetNextFromList (lithologies);
                         }
                         horizon = GetNextFromList (horizons);
                     }
                     subarea = GetNextFromList (subareas);
                 }
             }
         }

/* ----- Allocate memory and receive the expulsion data in it */
         if (!rc) rc = GxAllocArray (&expulsions, sizeof (double), 2,
                                     theGeologicalRunData->nlithologies,
                                     theConfiguration->nspecies);
         if (!rc) rc = GxGetTotDownExpulsionSelected (
                                     theGeologicalRunData->model, submodelList,
                                     starttime, areas, expulsions);

/* ----- Allocate the expulsion structures and convert the data to them */
         if (!rc) rc = GxAllocExpulsion (&oilexpulsion, &gasexpulsion,
                                         &nonhcexpulsion);
         if (!rc) GxConvertExpulsion (expulsions, oilexpulsion,
                                      gasexpulsion, nonhcexpulsion,
                                      gor1ptr, gor2ptr, cgrptr);

/* ----- Free the temporary memory */
         if (areas) {
             GxFreeArray (areas, 1, theGeologicalRunData->nlocations);
         }
         if (expulsions) {
             GxFreeArray (expulsions, 2, theGeologicalRunData->nlithologies,
                          theConfiguration->nspecies);
         }
         if (submodelList) {
             DeleteList (submodelList);
         }
      }
   }
   if (!rc) {
      *oilexpulsionptr   = oilexpulsion;
      *gasexpulsionptr   = gasexpulsion;
      *nonhcexpulsionptr = nonhcexpulsion;
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetDownwardIsotopeSelected - get the downward isotope from the selected
                                submodels in the geological model
--------------------------------------------------------------------------------

 GxGetDownwardIsotopeSelected allocates and returns the data for the downward
 isotopes in the selected submodels in the geological model since the time 
 specified. The memory for the isotope data should be freed by calling 
 GxFreeIsotope.

 Arguments
 ---------
 subareas          - list of subareas to be used
 horizons          - list of horizons to be used
 lithologies       - list of lithologies to be used
 starttime         - start time for the expulsion calculations
 oilisotopeptr     - pointer for the oil expulsion data
 gasisotopeptr     - pointer for the gas expulsion data
 nonhcisotopeptr   - pointer for the non-hydrocarbon expulsion data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetDownwardIsotopeSelected (SortedList subareas,
                                          SortedList horizons,
                                          SortedList lithologies,
                                          double starttime,
                                          GxIsotope **oilisotopeptr,
                                          GxIsotope **gasisotopeptr,
                                          GxIsotope **nonhcisotopeptr)
{
   GxErrorCode   rc=GXE_NOERROR;
   GxIsotope    *oilisotope=NULL, 
                *gasisotope=NULL, 
                *nonhcisotope=NULL;
   double       *areas=NULL, 
               **expulsions=NULL,
               **isotopes=NULL;
   char         *subarea,
                *horizon,
                *lithology;
   GxSubModelId  submodelId,
                *tempId;
   List          submodelList = NULL;

   if (!subareas || !horizons || !lithologies || !oilisotopeptr || 
       !gasisotopeptr || !nonhcisotopeptr ) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      *oilisotopeptr   = NULL;
      *gasisotopeptr   = NULL;
      *nonhcisotopeptr = NULL;

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Get the areas of the subareas from the project */
         rc = GxAllocArray (&areas, sizeof (double), 1,
                            theGeologicalRunData->nlocations);
         if (!rc) rc = GxGetSubAreaData (areas);

/* ----- Construct a sorted list with the submodel identifications */
         if (!rc) {
             submodelList = CreateList (sizeof (GxSubModelId), NULL);
             if (!submodelList) {
                 rc = GxError ( GXE_LIST, GXT_LIST );
             } else {
                 subarea = GetFirstFromSortedList (subareas);
                 while (subarea && !rc) {
                     horizon = GetFirstFromSortedList (horizons);
                     while (horizon && !rc) {
                         lithology = GetFirstFromSortedList (lithologies);
                         while (lithology && !rc) {
                             rc = GxFindSubModel (subarea, horizon, lithology,
                                                  &submodelId.subareanr,
                                                  &submodelId.horizonnr,
                                                  &submodelId.lithologynr);
                             if (!rc) {
                                 tempId = AppendToList (submodelList,
                                                        &submodelId);
                                 if (!tempId) {
                                     rc = GxError ( GXE_LIST, GXT_LIST );
                                 }
                             }
                             lithology = GetNextFromList (lithologies);
                         }
                         horizon = GetNextFromList (horizons);
                     }
                     subarea = GetNextFromList (subareas);
                 }
             }
         }

/* ----- Allocate memory and receive the expulsion data in it */
         if (!rc) rc = GxAllocArray (&expulsions, sizeof (double), 2,
                                     theGeologicalRunData->nlithologies,
                                     theConfiguration->nspecies);

/* ----- Allocate memory and receive the isotope data in it */
         if (!rc) rc = GxAllocArray (&isotopes, sizeof (double), 2,
                                     theGeologicalRunData->nlithologies,
                                     theConfiguration->nspecies);
         if (!rc) rc = GxGetTotDownIsotopeSelected (
                                              theGeologicalRunData->model,
                                              submodelList, starttime, 
                                              areas, isotopes, expulsions);

/* ----- Allocate the expulsion structures and convert the data to them */
         if (!rc) rc = GxAllocIsotope (&oilisotope, &gasisotope,
                                         &nonhcisotope);
         if (!rc) GxConvertIsotopes (expulsions, isotopes, oilisotope,
                                     gasisotope, nonhcisotope);

/* ----- Free the temporary memory */
         if (areas) {
             GxFreeArray (areas, 1, theGeologicalRunData->nlocations);
         }
         if (expulsions) {
             GxFreeArray (expulsions, 2, theGeologicalRunData->nlithologies,
                          theConfiguration->nspecies);
         }
         if (isotopes) {
             GxFreeArray (isotopes, 2, theGeologicalRunData->nlithologies,
                          theConfiguration->nspecies);
         }
         if (submodelList) {
             DeleteList (submodelList);
         }
      }
   }
   if (!rc) {
      *oilisotopeptr   = oilisotope;
      *gasisotopeptr   = gasisotope;
      *nonhcisotopeptr = nonhcisotope;
   }
   return (rc);
}
   

/*
--------------------------------------------------------------------------------
 GxGetGasMolaraogramSelected - get the gas molarogram from the selected 
                               geological submodel(s).
--------------------------------------------------------------------------------

 GxGetGeneration allocates and returns the data for the gas molarogram  
 in the geological model since the time specified for the selected submodels. 

 Arguments
 ---------
 subareas          - list of subareas to be used
 horizons          - list of horizons to be used
 lithologies       - list of lithologies to be used
 starttime         - start time for the generation calculations
 updown            - specifies expulsion flux data (up, down or both) to be 
                     used
 gasmolarogramptr  - Gas molarogram data structure

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/
GxErrorCode GxGetGasMolarogramSelected (SortedList subareas,
                                        SortedList horizons,
                                        SortedList lithologies,
                                        double starttime,
                                        GxUpDown updown,
                                        GxGasMolarogram **gasmolarogramptr) 
{
   GxErrorCode      rc=GXE_NOERROR;
   GxExpulsion     *oilexpulsion=NULL, 
                   *gasexpulsion=NULL, 
                   *nonhcexpulsion=NULL;
   GxGasMolarogram *gasmolarogram=NULL;
   double           gor1,
                    gor2,
                    cgr;

   if (!subareas || !horizons || !lithologies || !gasmolarogramptr ){ 
       rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
       if (!GxGeologicalModelPresent ()) {
           rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
       } else {

/* ------- Get the expulsion data for the selected submodels */
           switch (updown) {
               case GX_UP:
                   rc = GxGetUpwardExpulsionSelected ( subareas, horizons, 
                                                       lithologies, starttime, 
                                                       &oilexpulsion, 
                                                       &gasexpulsion, 
                                                       &nonhcexpulsion, 
                                                       &gor1, &gor2, &cgr );
                   break;
               case GX_DOWN:
                   rc = GxGetDownwardExpulsionSelected ( subareas, horizons, 
                                                         lithologies, 
                                                         starttime, 
                                                         &oilexpulsion, 
                                                         &gasexpulsion, 
                                                         &nonhcexpulsion, 
                                                         &gor1, &gor2, &cgr );
                   break;
               case GX_UPANDDOWN:
                   rc = GxGetMainExpulsionSelected ( subareas, horizons, 
                                                     lithologies, starttime, 
                                                     &oilexpulsion, 
                                                     &gasexpulsion, 
                                                     &nonhcexpulsion, 
                                                     &gor1, &gor2, &cgr );
                   break;
           }
           
/* ------- Allocate the gas molarogram struct and convert the data to them */
           if (!rc) {
               rc = GxAllocGasMolarogram (&gasmolarogram);
           }
           if (!rc) {
               GxConvertGasMolarogram (gasexpulsion, gasmolarogram);
           }
           
/* ------- Free temporary allocated expulsion data */
           if (oilexpulsion && gasexpulsion && nonhcexpulsion) {
               rc = GxFreeExpulsion ( oilexpulsion, gasexpulsion, 
                                      nonhcexpulsion );
           }
       }
   }
   if (!rc) {
      *gasmolarogramptr = gasmolarogram;
   }
   return rc;
}


/*
--------------------------------------------------------------------------------
 GxGetGeneration - get the generation from the geological model
--------------------------------------------------------------------------------

 GxGetGeneration allocates and returns the data for the generation 
 in the geological model since the time specified. The memory for the 
 generation data should be freed by calling GxFreeGeneration.

 Arguments
 ---------
 starttime         - start time for the generation calculations
 oilgenerationptr   - pointer for the oil generation data
 gasgenerationptr   - pointer for the gas generation data
 nonhcgenerationptr - pointer for the non-hydrocarbon generation data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGeneration (double starttime,
                             GxGenRate **oilgenerationptr,
                             GxGenRate **gasgenerationptr,
                             GxGenRate **nonhcgenerationptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxGenRate  *oilgeneration=NULL, *gasgeneration=NULL, *nonhcgeneration=NULL;
   double       *areas=NULL, **generations=NULL;

   if (!oilgenerationptr || !gasgenerationptr || !nonhcgenerationptr){ 
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Get the areas of the subareas from the project */
         rc = GxAllocArray (&areas, sizeof (double), 1,
                            theGeologicalRunData->nlocations);
         if (!rc) rc = GxGetSubAreaData (areas);

/* ----- Allocate memory and receive the generation data in it */
         if (!rc) rc = GxAllocArray (&generations, sizeof (double), 2,
                                     theGeologicalRunData->nlithologies,
                                     theConfiguration->nspecies);
         if (!rc) rc = GxGetTotGeneration (theGeologicalRunData->model,
                          starttime, areas, generations);

/* ----- Allocate the generation structures and convert the data to them */
         if (!rc) rc = GxAllocGeneration (&oilgeneration, &gasgeneration,
                                         &nonhcgeneration);
         if (!rc) GxConvertGeneration (generations, oilgeneration,
                                      gasgeneration, nonhcgeneration);

/* ----- Free the temporary memory */
         if (areas)
            GxFreeArray (areas, 1, theGeologicalRunData->nlocations);
         if (generations)
            GxFreeArray (generations, 2, theGeologicalRunData->nlithologies,
                         theConfiguration->nspecies);
      }
   }
   if (!rc) {
      *oilgenerationptr   = oilgeneration;
      *gasgenerationptr   = gasgeneration;
      *nonhcgenerationptr = nonhcgeneration;
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetGenerationSelected - get the generation from the selected geological 
                           submodels
--------------------------------------------------------------------------------

 GxGetGenerationSelected allocates and returns the data for the generation 
 in the selected geological submodels since the time specified. The memory for 
 the generation data should be freed by calling GxFreeGeneration.

 Arguments
 ---------
 subareas           - list of subareas to be used
 horizons           - list of horizons to be used
 lithologies        - list of lithologies to be used
 starttime          - start time for the generation calculations
 oilgenerationptr   - pointer for the oil generation data
 gasgenerationptr   - pointer for the gas generation data
 nonhcgenerationptr - pointer for the non-hydrocarbon generation data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGenerationSelected (SortedList subareas,
                                     SortedList horizons,
                                     SortedList lithologies, double starttime,
                                     GxGenRate **oilgenerationptr,
                                     GxGenRate **gasgenerationptr,
                                     GxGenRate **nonhcgenerationptr)
{
   GxErrorCode    rc=GXE_NOERROR;
   GxGenRate     *oilgeneration=NULL, 
                 *gasgeneration=NULL, 
                 *nonhcgeneration=NULL;
   double        *areas=NULL, 
                **generations=NULL;
   char          *subarea,
                 *horizon,
                 *lithology;
   GxSubModelId   submodelId,
                 *tempId;
   List           submodelList = NULL;


   if (!subareas || !horizons || !lithologies || !oilgenerationptr 
       || !gasgenerationptr || !nonhcgenerationptr){ 
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      *oilgenerationptr   = NULL;
      *gasgenerationptr   = NULL;
      *nonhcgenerationptr = NULL;

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Get the areas of the subareas from the project */
         rc = GxAllocArray (&areas, sizeof (double), 1,
                            theGeologicalRunData->nlocations);
         if (!rc) rc = GxGetSubAreaData (areas);

/* ----- Construct a sorted list with the submodel identifications */
         if (!rc) {
             submodelList = CreateList (sizeof (GxSubModelId), NULL);
             if (!submodelList) {
                 rc = GxError ( GXE_LIST, GXT_LIST );
             } else {
                 subarea = GetFirstFromSortedList (subareas);
                 while (subarea && !rc) {
                     horizon = GetFirstFromSortedList (horizons);
                     while (horizon && !rc) {
                         lithology = GetFirstFromSortedList (lithologies);
                         while (lithology && !rc) {
                             rc = GxFindSubModel (subarea, horizon, lithology,
                                                  &submodelId.subareanr,
                                                  &submodelId.horizonnr,
                                                  &submodelId.lithologynr);
                             if (!rc) {
                                 tempId = AppendToList (submodelList,
                                                        &submodelId);
                                 if (!tempId) {
                                     rc = GxError ( GXE_LIST, GXT_LIST );
                                 }
                             }
                             lithology = GetNextFromList (lithologies);
                         }
                         horizon = GetNextFromList (horizons);
                     }
                     subarea = GetNextFromList (subareas);
                 }
             }
         }

/* ----- Allocate memory and receive the generation data in it */
         if (!rc) rc = GxAllocArray (&generations, sizeof (double), 2,
                                     theGeologicalRunData->nlithologies,
                                     theConfiguration->nspecies);
         if (!rc) rc = GxGetTotGenerationSelected (theGeologicalRunData->model,
                                                   submodelList, starttime, 
                                                   areas, generations);

/* ----- Allocate the generation structures and convert the data to them */
         if (!rc) rc = GxAllocGeneration (&oilgeneration, &gasgeneration,
                                         &nonhcgeneration);
         if (!rc) GxConvertGeneration (generations, oilgeneration,
                                      gasgeneration, nonhcgeneration);

/* ----- Free the temporary memory */
         if (areas) {
             GxFreeArray (areas, 1, theGeologicalRunData->nlocations);
         }
         if (generations) {
             GxFreeArray (generations, 2, theGeologicalRunData->nlithologies,
                          theConfiguration->nspecies);
         }
         if (submodelList) {
             DeleteList (submodelList);
         }
      }
   }
   if (!rc) {
      *oilgenerationptr   = oilgeneration;
      *gasgenerationptr   = gasgeneration;
      *nonhcgenerationptr = nonhcgeneration;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetGeologicalAPI - get API history from geol model
--------------------------------------------------------------------------------

 Get the API of the geological mode.
 The concentration history array passed should
 be allocated by calling GxCreateModelHistory.

 Arguments
 ---------
 begintime     - the begin time for the history
 endtime       - the end time for the history
 api           - array for the api data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/


GxErrorCode GxGetGeologicalAPI (double begintime, double endtime, double *api)
{
   GxErrorCode    rc = GXE_NOERROR;
   double        *areas;

   if (!api) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Get the areas of the subareas from the project */
         rc = GxAllocArray (&areas, sizeof(double), 1,
                                theGeologicalRunData->nlocations);
         if (!rc) rc = GxGetSubAreaData (areas);

         if (!rc) rc = GxGetAPIHist (theGeologicalRunData->model, begintime,
                                 endtime, areas, api );
         if (areas)
            GxFreeArray (areas, 1, theGeologicalRunData->nlocations);
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetGeologicalAPISelected - get API history from geol model for selected
                              submodels
--------------------------------------------------------------------------------

 Get the API of the geological mode.
 The concentration history array passed should
 be allocated by calling GxCreateModelHistory.

 Arguments
 ---------
 subareas      - list of subareas to be used
 horizons      - list of horizons to be used
 lithologies   - list of lithologies to be used
 begintime     - the begin time for the history
 endtime       - the end time for the history
 api           - array for the api data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/


GxErrorCode GxGetGeologicalAPISelected (SortedList subareas, 
                                        SortedList horizons,
                                        SortedList lithologies, 
                                        double begintime, double endtime, 
                                        double *api)
{
   GxErrorCode    rc = GXE_NOERROR;
   double        *areas;
   char          *subarea,
                 *horizon,
                 *lithology;
   GxSubModelId   submodelId,
                 *tempId; 
   List           submodelList = NULL;

   if (!subareas || !horizons || !lithologies || !api) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Get the areas of the subareas from the project */
         rc = GxAllocArray (&areas, sizeof(double), 1,
                                theGeologicalRunData->nlocations);
         if (!rc) rc = GxGetSubAreaData (areas);

/* ----- Construct a sorted list with the submodel identifications */
         if (!rc) {
             submodelList = CreateList (sizeof (GxSubModelId), NULL);
             if (!submodelList) {
                 rc = GxError ( GXE_LIST, GXT_LIST );
             } else {
                 subarea = GetFirstFromSortedList (subareas);
                 while (subarea && !rc) {
                     horizon = GetFirstFromSortedList (horizons);
                     while (horizon && !rc) {
                         lithology = GetFirstFromSortedList (lithologies);
                         while (lithology && !rc) {
                             rc = GxFindSubModel (subarea, horizon, lithology,
                                                  &submodelId.subareanr,
                                                  &submodelId.horizonnr,
                                                  &submodelId.lithologynr);
                             if (!rc) {
                                 tempId = AppendToList (submodelList, 
                                                        &submodelId); 
                                 if (!tempId) {
                                     rc = GxError ( GXE_LIST, GXT_LIST );
                                 }
                             }
                             lithology = GetNextFromList (lithologies);
                         }
                         horizon = GetNextFromList (horizons);
                     }
                     subarea = GetNextFromList (subareas);
                 }
             }
         } 

         if (!rc) rc = GxGetAPIHistSelected (theGeologicalRunData->model, 
                                             submodelList, begintime,
                                             endtime, areas, api );
         if (areas) {
            GxFreeArray (areas, 1, theGeologicalRunData->nlocations);
         }
         if (submodelList) {
             DeleteList (submodelList);
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetGeologicalConcentrations - get concentration history from geol submodel
--------------------------------------------------------------------------------

 GxGetGeologicalConcentrations returns the concentration data for species
 <species> in the submodel of the geological model identified by <subarea>,
 <horizon> and <lithology>. The concentration history array passed should
 be allocated by calling GxCreateModelHistory.

 Arguments
 ---------
 subarea       - subarea of submodel to get concentration data from
 horizon       - horizon of submodel to get concentration data from
 lithology     - lithology of submodel to get concentration data from
 species       - name of species to get concentration data for
 begintime     - the begin time for the history
 endtime       - the end time for the history
 concentration - array for the concentration data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGeologicalConcentrations (char *subarea, char *horizon,
                                           char *lithology, char *species,
                                           double begintime, double endtime,
                                           double *concentration)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          subareanr, horizonnr, lithologynr, speciesnr;

   if (!subarea || (strlen (subarea) == 0)
      || (strlen (subarea) > GXL_SUBAREANAME) || !horizon
      || (strlen (horizon) == 0) || (strlen (horizon) > GXL_HORIZONNAME)
      || !lithology || (strlen (lithology) == 0)
      || (strlen (lithology) > GXL_LITHOLOGYNAME)
      || !species || (strlen (species) == 0)
      || (strlen (species) > GXL_SPECIESNAME) || !concentration) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Check that the submodel and the species both exist */
         rc = GxFindSubModel (subarea, horizon, lithology, &subareanr,
                              &horizonnr, &lithologynr);
         if (!rc) rc = GxFindSpecies (species, &speciesnr);

/* ----- Retrieve the concentration history from the model */
         if (!rc) rc = GxGetConcentrationHist (theGeologicalRunData->model,
                          subareanr, horizonnr, lithologynr, speciesnr,
                          begintime, endtime, concentration);
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetGeologicalConversions - get conversion history from a geological submodel
--------------------------------------------------------------------------------
GxGetGeolgoicalConversions returns the conversions in the submodel of the
geological model identified by <subarea>, <horizon> and <lithology>. The
conversion history array passed should be allocated by calling
GxCreateModelHistory.

Arguments
---------
subarea          - subarea of submodel to get conversion data from
horizon          - horizon of submodel to get conversion data from
lithology        - lithology of submodel to get conversion data from
begintime        - the begin time for the history
endtime          - the end time for the history
kerogenconv      - array for the kerogen conversion data
asphalteneconv   - array for the asphaltene conversion data

Return value
------------
GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGeologicalConversions (char *subarea, char *horizon,
                                        char *lithology, double begintime,
                                        double endtime, double *kerogenconv,
                                        double *asphalteneconv )
{
   GxErrorCode  rc=GXE_NOERROR;
   int          subareanr, horizonnr, lithologynr;

   if (!subarea || (strlen (subarea) == 0)
      || (strlen (subarea) > GXL_SUBAREANAME) || !horizon
      || (strlen (horizon) == 0) || (strlen (horizon) > GXL_HORIZONNAME)
      || !lithology || (strlen (lithology) == 0)
      || (strlen (lithology) > GXL_LITHOLOGYNAME)
      || !kerogenconv || !asphalteneconv ) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      
/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {
         
/* ----- Check that the submodel exist */
         rc = GxFindSubModel (subarea, horizon, lithology, &subareanr,
                              &horizonnr, &lithologynr);

/* ----- Retrieve the conversion history from the model */
         if (!rc) rc = GxGetConversionHist (theGeologicalRunData->model,
                          subareanr, horizonnr, lithologynr, begintime,
                          endtime, kerogenconv, asphalteneconv );
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetGeologicalExpIsoHist - get isotope history for a expelled species in 
                             a geological submodel
--------------------------------------------------------------------------------

 GxGetGeologicalExpIsoHist returns the isotope history data for expelled 
 species <species> in the submodel of the geological model identified by 
 <subarea>, <horizon> and <lithology>. The isotope history array 
 passed should be allocated by calling GxCreateModelHistory.

 Arguments
 ---------
 subarea   - subarea of submodel to get cumm. rate data from
 horizon   - horizon of submodel to get cumm. rate data from
 lithology - lithology of submodel to get gen.  rate data from
 species   - name of species to get expulsion flux data for
 updown    - specifies expulsion data (up, down or both) to be returned
 begintime - the begin time for the history
 endtime   - the end time for the history
 isohist   - array for the isotope history data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGeologicalExpIsoHist (char *subarea, char *horizon,
                                       char *lithology, char *species,
                                       GxUpDown updown,
                                       double begintime, double endtime, 
                                       double *isohist)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          subareanr, horizonnr, lithologynr, speciesnr;

   if (!subarea || (strlen (subarea) == 0)
      || (strlen (subarea) > GXL_SUBAREANAME) || !horizon
      || (strlen (horizon) == 0) || (strlen (horizon) > GXL_HORIZONNAME)
      || !lithology || (strlen (lithology) == 0)
      || (strlen (lithology) > GXL_LITHOLOGYNAME)
      || !species || (strlen (species) == 0)
      || (strlen (species) > GXL_SPECIESNAME) || !isohist) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Check that the submodel and the species both exist */
         rc = GxFindSubModel (subarea, horizon, lithology, &subareanr,
                              &horizonnr, &lithologynr);
         if (!rc) rc = GxFindSpecies (species, &speciesnr);

/* ----- Retrieve istope history of a expelled species from model */
         if (!rc) rc = GxGetExpIsotopeHist (theGeologicalRunData->model,
                              subareanr, horizonnr, lithologynr, speciesnr,
                              updown, begintime, endtime, isohist);

      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetGeologicalExpulsions - get expulsion history from a geological submodel
--------------------------------------------------------------------------------

 GxGetGeologicalExpulsions returns the expulsion data for species <species> in
 the submodel of the geological model identified by <subarea>, <horizon> and
 <lithology>. The expulsion data requested is specified by <updown> and
 <cumulative>. The expulsion history array passed should be allocated by
 calling GxCreateModelHistory.

 Arguments
 ---------
 subarea    - subarea of submodel to get expulsion data from
 horizon    - horizon of submodel to get expulsion data from
 lithology  - lithology of submodel to get expulsion data from
 species    - name of species to get expulsion data for
 updown     - specifies expulsion data (up, down or both) to be returned
 cumulative - specifies whether history should be cumulative
 begintime  - the begin time for the history
 endtime    - the end time for the history
 expulsion  - array for the expulsion data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGeologicalExpulsions (char *subarea, char *horizon,
                                       char *lithology, char *species,
                                       GxUpDown updown,
                                       GxCumulative cumulative,
                                       double begintime, double endtime,
                                       double *expulsion)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          subareanr, horizonnr, lithologynr, speciesnr;

   if (!subarea || (strlen (subarea) == 0)
      || (strlen (subarea) > GXL_SUBAREANAME) || !horizon
      || (strlen (horizon) == 0) || (strlen (horizon) > GXL_HORIZONNAME)
      || !lithology || (strlen (lithology) == 0)
      || (strlen (lithology) > GXL_LITHOLOGYNAME)
      || !species || (strlen (species) == 0)
      || (strlen (species) > GXL_SPECIESNAME) || !expulsion) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Check that the submodel and the species both exist */
         rc = GxFindSubModel (subarea, horizon, lithology, &subareanr,
                              &horizonnr, &lithologynr);
         if (!rc) rc = GxFindSpecies (species, &speciesnr);

/* ----- Retrieve the expulsion history from the model */
         if (!rc) rc = GxGetExpulsionHist (theGeologicalRunData->model,
                          subareanr, horizonnr, lithologynr, speciesnr,
                          updown, cumulative, begintime, endtime, expulsion);
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetGeologicalExpVolume - get expulsion volume history from a geological 
                            submodel
--------------------------------------------------------------------------------

 GxGetGeologicalExpVolume returns the expulsion volume data for species 
 <species> in the submodel of the geological model identified by <subarea>, 
 <horizon> and <lithology>. The expulsion data requested is specified by 
 <updown> and <cumulative>. The expulsion history array passed should be 
 allocated by calling GxCreateModelHistory.

 Arguments
 ---------
 subarea    - subarea of submodel to get expulsion data from
 horizon    - horizon of submodel to get expulsion data from
 lithology  - lithology of submodel to get expulsion data from
 species    - name of species to get expulsion data for
 updown     - specifies expulsion data (up, down or both) to be returned
 cumulative - specifies whether history should be cumulative
 begintime  - the begin time for the history
 endtime    - the end time for the history
 expulsion  - array for the expulsion data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGeologicalExpVolume (char *subarea, char *horizon,
                                      char *lithology, char *species,
                                      GxUpDown updown,
                                      GxCumulative cumulative,
                                      double begintime, double endtime,
                                      double *expulsion)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          subareanr, horizonnr, lithologynr, speciesnr;
   int          ntimes, i;

   if (!subarea || (strlen (subarea) == 0)
      || (strlen (subarea) > GXL_SUBAREANAME) || !horizon
      || (strlen (horizon) == 0) || (strlen (horizon) > GXL_HORIZONNAME)
      || !lithology || (strlen (lithology) == 0)
      || (strlen (lithology) > GXL_LITHOLOGYNAME)
      || !species || (strlen (species) == 0)
      || (strlen (species) > GXL_SPECIESNAME) || !expulsion) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Check that the submodel and the species both exist */
         rc = GxFindSubModel (subarea, horizon, lithology, &subareanr,
                              &horizonnr, &lithologynr);
         if (!rc) rc = GxFindSpecies (species, &speciesnr);

/* ----- Retrieve the expulsion history from the model */
         if (!rc) rc = GxGetExpulsionHist (theGeologicalRunData->model,
                          subareanr, horizonnr, lithologynr, speciesnr,
                          updown, cumulative, begintime, endtime, expulsion);

/* ----- Get number of data points filled */
         if (!rc) rc = GxGetGeologicalTimesteps (begintime, endtime, &ntimes);

/* ----- Make from expulsion masses volumes */
         if (!rc) {
            for (i=0;i<ntimes;i++) {
               expulsion[i] /= theConfiguration->species[speciesnr].density;
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetGeologicalFluxes - get expulsion flux history from a geological submodel
--------------------------------------------------------------------------------

 GxGetGeologicalFluxes returns the expulsion flux data for species <species>
 in the submodel of the geological model identified by <subarea>, <horizon>
 and <lithology>. The expulsion data requested is specified by <updown>.
 The expulsion flux history array passed should be allocated by calling
 GxCreateModelHistory.

 Arguments
 ---------
 subarea   - subarea of submodel to get expulsion flux data from
 horizon   - horizon of submodel to get expulsion flux data from
 lithology - lithology of submodel to get expulsion flux data from
 species   - name of species to get expulsion flux data for
 updown    - specifies expulsion flux data (up, down or both) to be returned
 begintime - the begin time for the history
 endtime   - the end time for the history
 flux      - array for the expulsion flux data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGeologicalFluxes (char *subarea, char *horizon,
                                   char *lithology, char *species,
                                   GxUpDown updown, double begintime,
                                   double endtime, double *flux)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          subareanr, horizonnr, lithologynr, speciesnr;

   if (!subarea || (strlen (subarea) == 0)
      || (strlen (subarea) > GXL_SUBAREANAME) || !horizon
      || (strlen (horizon) == 0) || (strlen (horizon) > GXL_HORIZONNAME)
      || !lithology || (strlen (lithology) == 0)
      || (strlen (lithology) > GXL_LITHOLOGYNAME)
      || !species || (strlen (species) == 0)
      || (strlen (species) > GXL_SPECIESNAME) || !flux) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Check that the submodel and the species both exist */
         rc = GxFindSubModel (subarea, horizon, lithology, &subareanr,
                              &horizonnr, &lithologynr);
         if (!rc) rc = GxFindSpecies (species, &speciesnr);

/* ----- Retrieve the flux history from the model */
         if (!rc) rc = GxGetFluxHist (theGeologicalRunData->model, subareanr,
                          horizonnr, lithologynr, speciesnr, updown,
                          begintime, endtime, flux);
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetGeologicalGenCumRate - get generation cumm. rate history from a 
                             geological submodel
--------------------------------------------------------------------------------

 GxGetGeologicalGenCumRate returns the cummulative generation rate data for 
 species <species> in the submodel of the geological model identified by 
 <subarea>, <horizon> and <lithology>. The expulsion flux history array 
 passed should be allocated by calling GxCreateModelHistory.

 Arguments
 ---------
 subarea   - subarea of submodel to get gen. cumm. rate data from
 horizon   - horizon of submodel to get gen. cumm. rate data from
 lithology - lithology of submodel to get gen. cumm. rate data from
 species   - name of species to get generation rate data for
 begintime - the begin time for the history
 endtime   - the end time for the history
 rate      - array for the generation rate data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGeologicalGenCumRate (char *subarea, char *horizon,
                                       char *lithology, char *species,
                                       double begintime, double endtime, 
                                       double *rate)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          subareanr, horizonnr, lithologynr, speciesnr, i;

   if (!subarea || (strlen (subarea) == 0)
      || (strlen (subarea) > GXL_SUBAREANAME) || !horizon
      || (strlen (horizon) == 0) || (strlen (horizon) > GXL_HORIZONNAME)
      || !lithology || (strlen (lithology) == 0)
      || (strlen (lithology) > GXL_LITHOLOGYNAME)
      || !species || (strlen (species) == 0)
      || (strlen (species) > GXL_SPECIESNAME) || !rate) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Check that the submodel and the species both exist */
         rc = GxFindSubModel (subarea, horizon, lithology, &subareanr,
                              &horizonnr, &lithologynr);
         if (!rc) rc = GxFindSpecies (species, &speciesnr);
         
         if (!rc) rc = GxGetGenerationHist (theGeologicalRunData->model,
                          subareanr, horizonnr, lithologynr, speciesnr,
                          GX_CUMULATIVE, begintime, endtime, rate);
      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxGetGeologicalGenRate - get generation rate history from a geological submodel
--------------------------------------------------------------------------------

 GxGetGeologicalGenCumRate returns the generation rate data for 
 species <species> in the submodel of the geological model identified by 
 <subarea>, <horizon> and <lithology>. The expulsion flux history array 
 passed should be allocated by calling GxCreateModelHistory.

 Arguments
 ---------
 subarea   - subarea of submodel to get cumm. rate data from
 horizon   - horizon of submodel to get cumm. rate data from
 lithology - lithology of submodel to get gen.  rate data from
 species   - name of species to get expulsion flux data for
 begintime - the begin time for the history
 endtime   - the end time for the history
 rate      - array for the generation rate data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGeologicalGenRate (char *subarea, char *horizon,
                                       char *lithology, char *species,
                                       double begintime, double endtime, 
                                       double *rate)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          subareanr, horizonnr, lithologynr, speciesnr;

   if (!subarea || (strlen (subarea) == 0)
      || (strlen (subarea) > GXL_SUBAREANAME) || !horizon
      || (strlen (horizon) == 0) || (strlen (horizon) > GXL_HORIZONNAME)
      || !lithology || (strlen (lithology) == 0)
      || (strlen (lithology) > GXL_LITHOLOGYNAME)
      || !species || (strlen (species) == 0)
      || (strlen (species) > GXL_SPECIESNAME) || !rate) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Check that the submodel and the species both exist */
         rc = GxFindSubModel (subarea, horizon, lithology, &subareanr,
                              &horizonnr, &lithologynr);
         if (!rc) rc = GxFindSpecies (species, &speciesnr);

/* ----- Retrieve generation data from model */
         if (!rc) rc = GxGetGenerationHist (theGeologicalRunData->model,
                              subareanr, horizonnr, lithologynr, speciesnr,
                              GX_NOTCUMULATIVE, begintime, endtime, rate);

      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxGetGeologicalGOR - get  the Gas-Oil Ratios for open and closed
--------------------------------------------------------------------------------

 GxGetGeologicalGOR returns the Gas-Oil Ratios for the open and closed
 situation in a certain time interval. The GOR arrays passed should be
 allocated by calling GxCreateModelHistory.


 Arguments
 ---------
 begintime - the begin time for the history
 endtime   - the end time for the history
 opengor   - array for the GOR data (perfectly open)
 closegor  - array for the GOR data (closed)

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGeologicalGOR (double begintime, double endtime,
                                double *opengor,  double *closegor)
{
   GxErrorCode    rc = GXE_NOERROR;
   double        *areas = NULL, *thickness = NULL, *porosity = NULL,
                 *nettogross = NULL;
   double         temperature, pressure, depth, salinity;

   if (!opengor || !closegor) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Get the areas of the subareas from the project */
         rc = GxAllocArray (&areas, sizeof(double), 1,
                                theGeologicalRunData->nlocations);
         if (!rc) rc = GxAllocArray (&thickness, sizeof(double), 1,
                                theGeologicalRunData->nlocations);
         if (!rc) rc = GxAllocArray (&porosity, sizeof(double), 1,
                                theGeologicalRunData->nlocations);
         if (!rc) rc = GxAllocArray (&nettogross, sizeof(double), 1,
                                theGeologicalRunData->nlocations);

/* ----- Get the trap data, not all returned data is usefull */
         if (!rc) rc = GxGetDataForCharge (areas, thickness, porosity,
                                           nettogross, &temperature, &pressure,
                                          &depth, &salinity );

         if (!rc) rc = GxGetGORHist (theGeologicalRunData->model, begintime,
                                 endtime, areas, thickness, porosity,
                                 nettogross, temperature, pressure, depth,
                                 salinity, opengor, closegor );

         if (areas)
            GxFreeArray (areas, 1, theGeologicalRunData->nlocations);
         if (thickness)
            GxFreeArray (thickness,  1, theGeologicalRunData->nlocations);
         if (porosity)
            GxFreeArray (porosity,   1, theGeologicalRunData->nlocations);
         if (nettogross)
            GxFreeArray (nettogross, 1, theGeologicalRunData->nlocations);
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetGeologicalGORSelected - get the Gas-Oil Ratios of the selected submodels
                              for open and closed
--------------------------------------------------------------------------------

 GxGetGeologicalGORSelected returns the Gas-Oil Ratios for the open and closed
 situation in a certain time interval. The GOR arrays passed should be
 allocated by calling GxCreateModelHistory.


 Arguments
 ---------
 subareas      - list of subareas to be used
 horizons      - list of horizons to be used
 lithologies   - list of lithologies to be used
 begintime     - the begin time for the history
 endtime       - the end time for the history
 opengor       - array for the GOR data (perfectly open)
 closegor      - array for the GOR data (closed)

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGeologicalGORSelected (SortedList subareas,
                                        SortedList horizons,
                                        SortedList lithologies,
                                        double begintime, double endtime,
                                        double *opengor,  double *closegor)
{
   GxErrorCode    rc = GXE_NOERROR;
   double        *areas = NULL, *thickness = NULL, *porosity = NULL,
                 *nettogross = NULL;
   double         temperature, pressure, depth, salinity;
   char          *subarea,
                 *horizon,
                 *lithology;
   GxSubModelId   submodelId,
                 *tempId;
   int            locnr,
                 *tempNr;
   List           submodelList = NULL;
   List           locationList = NULL;
   GxBool         subareaAvailable = GxFalse;

   if (!subareas || !horizons || !lithologies || !opengor || !closegor) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Get the areas of the subareas from the project */
         rc = GxAllocArray (&areas, sizeof(double), 1,
                                theGeologicalRunData->nlocations);
         if (!rc) rc = GxAllocArray (&thickness, sizeof(double), 1,
                                theGeologicalRunData->nlocations);
         if (!rc) rc = GxAllocArray (&porosity, sizeof(double), 1,
                                theGeologicalRunData->nlocations);
         if (!rc) rc = GxAllocArray (&nettogross, sizeof(double), 1,
                                theGeologicalRunData->nlocations);

/* ----- Get the trap data, not all returned data is usefull */
         if (!rc) rc = GxGetDataForCharge (areas, thickness, porosity,
                                           nettogross, &temperature, &pressure,
                                          &depth, &salinity );

/* ----- Construct a sorted list with the submodel identifications */
         if (!rc) {
             submodelList = CreateList (sizeof (GxSubModelId), NULL);
             locationList = CreateList (sizeof (int), NULL);
             if (!submodelList) {
                 rc = GxError ( GXE_LIST, GXT_LIST );
             } else {
                 subarea = GetFirstFromSortedList (subareas);
                 while (subarea && !rc) {
                     horizon = GetFirstFromSortedList (horizons);
                     while (horizon && !rc) {
                         lithology = GetFirstFromSortedList (lithologies);
                         while (lithology && !rc) {
                             rc = GxFindSubModel (subarea, horizon, lithology,
                                                  &submodelId.subareanr,
                                                  &submodelId.horizonnr,
                                                  &submodelId.lithologynr);
                             if (!rc) {
                                 locnr = submodelId.subareanr;
                                 subareaAvailable = GxTrue;
                                 tempId = AppendToList (submodelList,
                                                        &submodelId);
                                 if (!tempId) {
                                     rc = GxError ( GXE_LIST, GXT_LIST );
                                 }
                             }
                             lithology = GetNextFromList (lithologies);
                         }
                         horizon = GetNextFromList (horizons);
                     }
                     if (subareaAvailable) {
                         subareaAvailable = GxFalse;
                         tempNr = AppendToList (locationList, &locnr);
                         if (!tempNr) {
                             rc = GxError (GXE_LIST, GXT_LIST);
                         }
                     }
                     subarea = GetNextFromList (subareas);
                 }
             }
         }

         if (!rc) rc = GxGetGORHistSelected (
                                     theGeologicalRunData->model, locationList,
                                     submodelList, begintime, endtime, areas, 
                                     thickness, porosity, nettogross, 
                                     temperature, pressure, depth, salinity, 
                                     opengor, closegor );

         if (areas)
            GxFreeArray (areas, 1, theGeologicalRunData->nlocations);
         if (thickness)
            GxFreeArray (thickness,  1, theGeologicalRunData->nlocations);
         if (porosity)
            GxFreeArray (porosity,   1, theGeologicalRunData->nlocations);
         if (nettogross)
            GxFreeArray (nettogross, 1, theGeologicalRunData->nlocations);
         if (submodelList) {
             DeleteList (submodelList);
         }
         if (locationList) {
             DeleteList (locationList);
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetGeologicalMasses - get Masses history from a geological submodel
--------------------------------------------------------------------------------

 GxGetGeologicalMasses returns the masses for the submodel of the
 geological model identified by <subarea>, <horizon> and <lithology>. The
 masses history array passed should be allocated by calling
 GxCreateModelHistory.

 Arguments
 ---------
 subareaname   - subarea of submodel to get pressures from
 horizonname   - horizon of submodel to get pressures from
 lithologyname - lithology of submodel to get pressures from
 species       - name of requested species
 begintime     - the begin time for the history
 endtime       - the end time for the history
 masses        - array for the masses data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGeologicalMasses ( char *subareaname, char *horizonname,
                                    char *lithologyname, char *species,
                                    double begintime, double endtime,
                                    double *masses )
{
   GxErrorCode  rc=GXE_NOERROR;
   SubArea     *subarea;
   int          subareanr, horizonnr, lithologynr, speciesnr;

   if (!subareaname || (strlen(subareaname) == 0)
      || (strlen(subareaname) > GXL_SUBAREANAME) || !horizonname
      || (strlen(horizonname) == 0) || (strlen(horizonname) > GXL_HORIZONNAME)
      || !lithologyname || (strlen(lithologyname) == 0)
      || (strlen(lithologyname) > GXL_LITHOLOGYNAME)
      || !species || (strlen(species) == 0)
      || (strlen(species) > GXL_SPECIESNAME) || !masses) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {
         rc = GxFindSubModel (subareaname, horizonname, lithologyname,
                             &subareanr, &horizonnr, &lithologynr);
         if (!rc) rc = GxFindSpecies (species, &speciesnr);
         if (!rc) rc = GxCreateSubArea (&subarea);
         if (!rc) rc = GxGetSubArea (subareaname, subarea);

         if (!rc) rc = GxGetMassesHist (theGeologicalRunData->model,
                          subareanr, horizonnr, lithologynr, speciesnr,
                          begintime, endtime,subarea->area,
                          masses);
      }
   }
   return (rc);
}







/*
--------------------------------------------------------------------------------
 GxGetGeologicalPressures - get pressure history from a geological submodel
--------------------------------------------------------------------------------

 GxGetGeologicalPressures returns the pressures for the submodel of the
 geological model identified by <subarea>, <horizon> and <lithology>. The
 pressure history array passed should be allocated by calling
 GxCreateModelHistory.

 Arguments
 ---------
 subarea   - subarea of submodel to get pressures from
 horizon   - horizon of submodel to get pressures from
 lithology - lithology of submodel to get pressures from
 begintime - the begin time for the history
 endtime   - the end time for the history
 pressures - array for the pressure data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGeologicalPressures (char *subarea, char *horizon,
                                      char *lithology, double begintime,
                                      double endtime, double *pressures)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          subareanr, horizonnr, lithologynr;

   if (!subarea || (strlen (subarea) == 0)
      || (strlen (subarea) > GXL_SUBAREANAME) || !horizon
      || (strlen (horizon) == 0) || (strlen (horizon) > GXL_HORIZONNAME)
      || !lithology || (strlen (lithology) == 0)
      || (strlen (lithology) > GXL_LITHOLOGYNAME) || !pressures) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Check that the submodel exists */
         rc = GxFindSubModel (subarea, horizon, lithology, &subareanr,
                              &horizonnr, &lithologynr);

/* ----- Retrieve the pressure history from the model */
         if (!rc) rc = GxGetPressureHist (theGeologicalRunData->model,
                          subareanr, horizonnr, lithologynr, begintime,
                          endtime, pressures);
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetGeologicalRetIsoHist - get isotope history of a retained species in a 
                             geological submodel
--------------------------------------------------------------------------------

 GxGetGeologicalRetIsoHist returns the isotope history data for a retained
 species <species> in the submodel of the geological model identified by 
 <subarea>, <horizon> and <lithology>. The isotope history array 
 passed should be allocated by calling GxCreateModelHistory.

 Arguments
 ---------
 subarea   - subarea of submodel to get cumm. rate data from
 horizon   - horizon of submodel to get cumm. rate data from
 lithology - lithology of submodel to get gen.  rate data from
 species   - name of species to get expulsion flux data for
 begintime - the begin time for the history
 endtime   - the end time for the history
 isohist   - array for the isotope history data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGeologicalRetIsoHist (char *subarea, char *horizon,
                                       char *lithology, char *species,
                                       double begintime, double endtime, 
                                       double *isohist)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          subareanr, horizonnr, lithologynr, speciesnr;

   if (!subarea || (strlen (subarea) == 0)
      || (strlen (subarea) > GXL_SUBAREANAME) || !horizon
      || (strlen (horizon) == 0) || (strlen (horizon) > GXL_HORIZONNAME)
      || !lithology || (strlen (lithology) == 0)
      || (strlen (lithology) > GXL_LITHOLOGYNAME)
      || !species || (strlen (species) == 0)
      || (strlen (species) > GXL_SPECIESNAME) || !isohist) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Check that the submodel and the species both exist */
         rc = GxFindSubModel (subarea, horizon, lithology, &subareanr,
                              &horizonnr, &lithologynr);
         if (!rc) rc = GxFindSpecies (species, &speciesnr);

/* ----- Retrieve retained isotope history data from model */
         if (!rc) rc = GxGetRetIsotopeHist (theGeologicalRunData->model,
                              subareanr, horizonnr, lithologynr, speciesnr,
                              begintime, endtime, isohist);

      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetGeologicalRunData - get the geological run data from the project
--------------------------------------------------------------------------------

 GxGetGeologicalRunData retrieves the data needed for a geological model run
 from the project, allocates memory for the data and stores the data in
 the geological run data struct.

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxGetGeologicalRunData (void)
{
   GxErrorCode  rc=GXE_NOERROR;

   rc = GxAllocGeologicalRunData ();
   if (!rc) rc = GxFillGeologicalRunData ();
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetGeologicalTemperatures - get temperature history from a geol submodel
--------------------------------------------------------------------------------

 GxGetGeologicalTemperatures returns the temperatures for the submodel of the
 geological model identified by <subarea>, <horizon> and <lithology>. The
 temperature history array passed should be allocated by calling
 GxCreateModelHistory.

 Arguments
 ---------
 subarea      - subarea of submodel to get temperatures from
 horizon      - horizon of submodel to get temperatures from
 lithology    - lithology of submodel to get temperatures from
 begintime    - the begin time for the history
 endtime      - the end time for the history
 temperatures - array for the temperature data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGeologicalTemperatures (char *subarea, char *horizon,
                                         char *lithology, double begintime,
                                         double endtime,
                                         double *temperatures)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          subareanr, horizonnr, lithologynr;

   if (!subarea || (strlen (subarea) == 0)
      || (strlen (subarea) > GXL_SUBAREANAME) || !horizon
      || (strlen (horizon) == 0) || (strlen (horizon) > GXL_HORIZONNAME)
      || !lithology || (strlen (lithology) == 0)
      || (strlen (lithology) > GXL_LITHOLOGYNAME) || !temperatures) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Check that the submodel exists */
         rc = GxFindSubModel (subarea, horizon, lithology, &subareanr,
                              &horizonnr, &lithologynr);

/* ----- Retrieve the temperature history from the model */
         if (!rc) rc = GxGetTemperatureHist (theGeologicalRunData->model,
                          subareanr, horizonnr, lithologynr, begintime,
                          endtime, temperatures);
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetGeologicalTimeRange - get the time range from the geological model
--------------------------------------------------------------------------------

 GxGetGeologicalTimeRange returns the begin and end times for which the
 geological model properties have been calculated.

 Arguments
 ---------
 begintimeptr - pointer for the begin time for the history
 endtimeptr   - pointer for the end time for the history

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGeologicalTimeRange (double *begintimeptr,
                                      double *endtimeptr)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!begintimeptr || !endtimeptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Retrieve the time range from the model */
         rc = GxGetTimeRange (theGeologicalRunData->model, begintimeptr,
                              endtimeptr);
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetGeologicalTimes - get the time history from the geological model
--------------------------------------------------------------------------------

 GxGetGeologicalTimes returns the times at which the geological model
 properties have been calculated. The time history array passed should be
 allocated by calling GxCreateModelHistory.

 Arguments
 ---------
 begintime - the begin time for the history
 endtime   - the end time for the history
 times     - array for the time data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGeologicalTimes (double begintime, double endtime,
                                  double *times)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!times) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Retrieve the time history from the model */
         rc = GxGetTimeHist (theGeologicalRunData->model, begintime, endtime,
                             times);
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetGeologicalTimesteps - get the number of timesteps from the geol model
--------------------------------------------------------------------------------

 GxGetGeologicalTimesteps returns the number of times at which
 the geological model properties have been calculated. This number should
 be used to allocate property history arrays with GxCreateModelHistory.

 Arguments
 ---------
 begintime - the begin time for the history
 endtime   - the end time for the history
 ntimesprt - pointer for number of times

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGeologicalTimesteps (double begintime, double endtime,
                                      int *ntimesptr)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!ntimesptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Retrieve the number of times from the model */
         rc = GxGetNumberOfTimes (theGeologicalRunData->model, begintime,
                                  endtime, ntimesptr);
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetGeologicalVanKrevelen - get Van Krevelen data from a geological submodel
--------------------------------------------------------------------------------

 GxGetGeologicalVanKrevelen returns the Van Krevelen values for the submodel 
 of the geological model identified by <subarea>, <horizon> and <lithology>. 
 The Van Krevelen history array passed should be allocated by calling 
 GxCreateModelHistory.

 Arguments
 ---------
 subarea   - subarea of submodel to get pressures from
 horizon   - horizon of submodel to get pressures from
 lithology - lithology of submodel to get pressures from
 begintime - the begin time for the history
 endtime   - the end time for the history
 hc        - array for the Hydrogen over carbon data
 oc        - array for the Oxygen over carbon data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/
GxErrorCode GxGetGeologicalVanKrevelen ( char *subarea, char *horizon,
                                         char *lithology, double begintime,
                                         double endtime, double *hc,
                                         double *oc )
{
   GxErrorCode rc=GXE_NOERROR;
   int subareanr, horizonnr, lithologynr;

   if (!subarea || (strlen (subarea) == 0)
      || (strlen (subarea) > GXL_SUBAREANAME) || !horizon
      || (strlen (horizon) == 0) || (strlen (horizon) > GXL_HORIZONNAME)
      || !lithology || (strlen (lithology) == 0)
      || (strlen (lithology) > GXL_LITHOLOGYNAME) || !hc || !oc ) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError ( GXE_NOGEOMODEL, GXT_NOGEOMODEL );
      } else {

/* ----- Check that the submodel exists */
         rc = GxFindSubModel ( subarea, horizon, lithology,
                              &subareanr, &horizonnr, &lithologynr );

/* ----- Retrieve the Van Krevelen data from the model */
         if (!rc) rc = GxGetVanKrevelen ( theGeologicalRunData->model,
                                          subareanr, horizonnr, lithologynr,
                                          begintime, endtime, hc, oc );
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetGeologicalVRE - get vre history from a geological submodel
--------------------------------------------------------------------------------

 GxGetGeologicalVRE returns the VRE values for the submodel of the
 geological model identified by <subarea>, <horizon> and <lithology>. The
 VRE history array passed should be allocated by calling GxCreateModelHistory.

 Arguments
 ---------
 subarea   - subarea of submodel to get pressures from
 horizon   - horizon of submodel to get pressures from
 lithology - lithology of submodel to get pressures from
 begintime - the begin time for the history
 endtime   - the end time for the history
 vre       - array for the VRE data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetGeologicalVRE (char *subarea, char *horizon, char *lithology,
                                double begintime, double endtime, double *vre)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          subareanr, horizonnr, lithologynr;

   if (!subarea || (strlen (subarea) == 0)
      || (strlen (subarea) > GXL_SUBAREANAME) || !horizon
      || (strlen (horizon) == 0) || (strlen (horizon) > GXL_HORIZONNAME)
      || !lithology || (strlen (lithology) == 0)
      || (strlen (lithology) > GXL_LITHOLOGYNAME) || !vre) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Check that the submodel exists */
         rc = GxFindSubModel (subarea, horizon, lithology, &subareanr,
                              &horizonnr, &lithologynr);

/* ----- Retrieve the VRE history from the model */
         if (!rc) rc = GxGetVREHist (theGeologicalRunData->model, subareanr,
                          horizonnr, lithologynr, GxGeologicalVRE,
                          GX_INITIALGEOLTAU, begintime, endtime, vre);
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetLaboratoryConcentrations - get concentration history from lab model
--------------------------------------------------------------------------------

 GxGetLaboratoryConcentrations returns the concentration data for the
 laboratory model. The concentration history array passed should be allocated
 by calling GxCreateModelHistory.

 Arguments
 ---------
 species       - name of species to get concentration data for
 begintime     - the begin time for the history
 endtime       - the end time for the history
 concentration - array for the concentration data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLaboratoryConcentrations (char *species, double begintime,
                                           double endtime,
                                           double *concentration)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          speciesnr;

   if (!species || (strlen (species) == 0)
      || (strlen (species) > GXL_SPECIESNAME) || !concentration) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the laboratory model is present */
      if (!GxLaboratoryModelPresent ()) {
         rc = GxError (GXE_NOLABMODEL, GXT_NOLABMODEL);
      } else {

/* ----- Check that the species exists */
         rc = GxFindSpecies (species, &speciesnr);

/* ----- Retrieve the concentration history from the model */
         if (!rc) rc = GxGetConcentrationHist (theLaboratoryRunData->model,
                          0, 0, 0, speciesnr, begintime, endtime,
                          concentration);
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetLaboratoryConversions - get conversion history from lab model
--------------------------------------------------------------------------------

 GxGetLaboratoryConversions returns the conversion data for the
 laboratory model. The conversion history array passed should be allocated
 by calling GxCreateModelHistory.

 Arguments
 ---------
 begintime      - the begin time for the history
 endtime        - the end time for the history
 kerogenconv    - array for the kerogen conversion data
 asphalteneconv - array for the asphaltene converion data
 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLaboratoryConversions ( double begintime, double endtime,
                                         double *kerogenconv, 
                                         double *asphalteneconv )
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!kerogenconv || !asphalteneconv ) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the laboratory model is present */
      if (!GxLaboratoryModelPresent ()) {
         rc = GxError (GXE_NOLABMODEL, GXT_NOLABMODEL);
      } else {

/* ----- Retrieve the conversion history from the model */
         rc = GxGetConversionHist (theLaboratoryRunData->model,
                     0, 0, 0, begintime, endtime, kerogenconv, asphalteneconv );
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetLaboratoryExpulsions - get expulsion history from laboratory model
--------------------------------------------------------------------------------

 GxGetLaboratoryExpulsions returns the expulsion data for the laboratory
 model. The expulsion data requested is specified by <updown> and
 <cumulative>. The expulsion history array passed should be allocated by
 calling GxCreateModelHistory.

 Arguments
 ---------
 species    - name of species to get expulsion data for
 updown     - specifies expulsion data (up, down or both) to be returned
 cumulative - specifies whether history should be cumulative
 begintime  - the begin time for the history
 endtime    - the end time for the history
 expulsion  - array for the expulsion data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLaboratoryExpulsions (char *species, GxUpDown updown,
                                       GxCumulative cumulative,
                                       double begintime, double endtime,
                                       double *expulsion)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          speciesnr;

   if (!species || (strlen (species) == 0)
      || (strlen (species) > GXL_SPECIESNAME) || !expulsion) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the laboratory model is present */
      if (!GxLaboratoryModelPresent ()) {
         rc = GxError (GXE_NOLABMODEL, GXT_NOLABMODEL);
      } else {

/* ----- Check that the species exists */
         rc = GxFindSpecies (species, &speciesnr);

/* ----- Retrieve the expulsion history from the model */
         if (!rc) rc = GxGetExpulsionHist (theLaboratoryRunData->model, 0, 0,
                          0, speciesnr, updown, cumulative, begintime,
                          endtime, expulsion);
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetLaboratoryExpIsoHist - get isotope history for a expelled species in 
                             a laboratory submodel
--------------------------------------------------------------------------------

 GxGetLaboratoryExpIsoHist returns the isotope history data for expelled 
 species <species> in the laboratory model. The isotope history array 
 passed should be allocated by calling GxCreateModelHistory.

 Arguments
 ---------
 species   - name of species to get expulsion flux data for
 updown    - specifies expulsion data (up, down or both) to be returned
 begintime - the begin time for the history
 endtime   - the end time for the history
 isohist   - array for the isotope history data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLaboratoryExpIsoHist (char *species, GxUpDown updown,
                                       double begintime, double endtime, 
                                       double *isohist)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          subareanr, horizonnr, lithologynr, speciesnr;

   if (!species || (strlen (species) == 0)
      || (strlen (species) > GXL_SPECIESNAME) || !isohist) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxLaboratoryModelPresent ()) {
         rc = GxError (GXE_NOLABMODEL, GXT_NOLABMODEL);
      } else {

/* ----- Check that the species exist */
         rc = GxFindSpecies (species, &speciesnr);

/* ----- Retrieve istope history of a expelled species from model */
         if (!rc) rc = GxGetExpIsotopeHist (theLaboratoryRunData->model,
                              0, 0, 0, speciesnr,
                              updown, begintime, endtime, isohist);

      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetLaboratoryExpVolume - get expulsion volume history from a laboratory 
                            submodel
--------------------------------------------------------------------------------

 GxGetLaboratoryExpVolume returns the expulsion volume data for species 
 <species> in the submodel of the laboratory model. The expulsion data 
 requested is specified by <updown> and <cumulative>. The expulsion history 
 array passed should be allocated by calling GxCreateModelHistory.

 Arguments
 ---------
 species    - name of species to get expulsion data for
 updown     - specifies expulsion data (up, down or both) to be returned
 cumulative - specifies whether history should be cumulative
 begintime  - the begin time for the history
 endtime    - the end time for the history
 expulsion  - array for the expulsion data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLaboratoryExpVolume (char *species, GxUpDown updown,
                                      GxCumulative cumulative,
                                      double begintime, double endtime,
                                      double *expulsion)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          speciesnr;
   int          ntimes, i;

   if (!species || (strlen (species) == 0)
      || (strlen (species) > GXL_SPECIESNAME) || !expulsion) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the laboratory model is present */
      if (!GxLaboratoryModelPresent ()) {
         rc = GxError (GXE_NOLABMODEL, GXT_NOLABMODEL);
      } else {

/* ----- Check that the species exist */
         rc = GxFindSpecies (species, &speciesnr);

/* ----- Retrieve the expulsion history from the model */
         if (!rc) rc = GxGetExpulsionHist (theLaboratoryRunData->model,
                          0, 0, 0, speciesnr, updown, cumulative, begintime, 
                          endtime, expulsion);

/* ----- Get number of data points filled */
         if (!rc) rc = GxGetLaboratoryTimesteps (begintime, endtime, &ntimes);

/* ----- Make from expulsion masses volumes */
         if (!rc) {
            for (i=0;i<ntimes;i++) {
               expulsion[i] /= theConfiguration->species[speciesnr].density;
            }
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetLaboratoryGenCumRate - get generation cumm. rate history from a 
                             geological submodel
--------------------------------------------------------------------------------

 GxGetLaboratoryGenCumRate returns the cummulative generation rate data for 
 species <species> in the submodel of the geological model identified by 
 <subarea>, <horizon> and <lithology>. The expulsion flux history array 
 passed should be allocated by calling GxCreateModelHistory.

 Arguments
 ---------
 subarea   - subarea of submodel to get gen. cumm. rate data from
 horizon   - horizon of submodel to get gen. cumm. rate data from
 lithology - lithology of submodel to get gen. cumm. rate data from
 species   - name of species to get generation rate data for
 begintime - the begin time for the history
 endtime   - the end time for the history
 rate      - array for the generation rate data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLaboratoryGenCumRate (char *species, double begintime, 
                                       double endtime, double *rate)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          speciesnr;

   if ( !species || (strlen (species) == 0)
      || (strlen (species) > GXL_SPECIESNAME) || !rate) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxLaboratoryModelPresent ()) {
         rc = GxError (GXE_NOLABMODEL, GXT_NOLABMODEL);
      } else {

/* ----- Check that the species exists */
         rc = GxFindSpecies (species, &speciesnr);

         if (!rc) rc = GxGetGenerationHist (theLaboratoryRunData->model,
                          0, 0, 0, speciesnr,
                          GX_CUMULATIVE, begintime, endtime, rate);
      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxGetLaboratoryGenRate - get generation rate history from a geological submodel
--------------------------------------------------------------------------------

 GxGetLaboratoryGenCumRate returns the generation rate data for 
 species <species> in the submodel of the geological model identified by 
 <subarea>, <horizon> and <lithology>. The expulsion flux history array 
 passed should be allocated by calling GxCreateModelHistory.

 Arguments
 ---------
 species   - name of species to get expulsion flux data for
 begintime - the begin time for the history
 endtime   - the end time for the history
 rate      - array for the generation rate data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLaboratoryGenRate ( char *species, double begintime, 
                                     double endtime, double *rate)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          speciesnr;

   if ( !species || (strlen (species) == 0)
        || (strlen (species) > GXL_SPECIESNAME) || !rate) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxLaboratoryModelPresent ()) {
         rc = GxError (GXE_NOLABMODEL, GXT_NOLABMODEL);
      } else {

/* ----- Check that the species exists */
         rc = GxFindSpecies (species, &speciesnr);

         if (!rc) rc = GxGetGenerationHist (theLaboratoryRunData->model,
                          0, 0, 0, speciesnr, GX_NOTCUMULATIVE, 
                          begintime, endtime, rate);
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetLaboratoryFluxes - get expulsion flux history from laboratory model
--------------------------------------------------------------------------------

 GxGetLaboratoryFluxes returns the expulsion flux data for the laboratory
 model. The expulsion flux data requested is specified by <updown>. The
 expulsion flux history array passed should be allocated by calling
 GxCreateModelHistory.

 Arguments
 ---------
 species   - name of species to get expulsion flux data for
 updown    - specifies expulsion flux data (up, down or both) to be returned
 begintime - the begin time for the history
 endtime   - the end time for the history
 flux      - array for the expulsion flux data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLaboratoryFluxes (char *species, GxUpDown updown,
                                   double begintime, double endtime,
                                   double *flux)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          speciesnr;

   if (!species || (strlen (species) == 0)
      || (strlen (species) > GXL_SPECIESNAME) || !flux) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the laboratory model is present */
      if (!GxLaboratoryModelPresent ()) {
         rc = GxError (GXE_NOLABMODEL, GXT_NOLABMODEL);
      } else {

/* ----- Check that the species exists */
         rc = GxFindSpecies (species, &speciesnr);

/* ----- Retrieve the flux history from the model */
         if (!rc) rc = GxGetFluxHist (theLaboratoryRunData->model, 0, 0, 0,
                          speciesnr, updown, begintime, endtime, flux);
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetLaboratoryPressures - get pressure history from laboratory model
--------------------------------------------------------------------------------

 GxGetLaboratoryPressures returns the pressures for the laboratory model. The
 pressure history array passed should be allocated by calling
 GxCreateModelHistory.

 Arguments
 ---------
 begintime - the begin time for the history
 endtime   - the end time for the history
 pressures - array for the pressure data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLaboratoryPressures (double begintime, double endtime,
                                      double *pressures)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!pressures) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the laboratory model is present */
      if (!GxLaboratoryModelPresent ()) {
         rc = GxError (GXE_NOLABMODEL, GXT_NOLABMODEL);
      } else {

/* ----- Retrieve the pressure history from the model */
         rc = GxGetPressureHist (theLaboratoryRunData->model, 0, 0, 0,
                 begintime, endtime, pressures);
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetLaboratoryRatios - get hc, oc and nc data from a laboratory submodel
--------------------------------------------------------------------------------

 GxGetLaboratoryRatios returns the hc, oc and nc values for the laboratory
 model. The HC, OC and NC arrays passed should be allocated by calling
 GxCreateModelHistory.

 Arguments
 ---------
 begintime - the begin time for the history
 endtime   - the end time for the history
 hc        - array for the Hydrogen over carbon data
 oc        - array for the Oxygen over carbon data
 nc        - array for the Nitogen over carbon data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLaboratoryRatios ( double begintime, double endtime,
                                    double *hc, double *oc, double *nc )
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!hc || !oc || !nc) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the laboratory model is present */
      if (!GxLaboratoryModelPresent ()) {
         rc = GxError (GXE_NOLABMODEL, GXT_NOLABMODEL);
      } else {

/* ----- Retrieve HC, OC and NC data from the model */
         rc = GxGetRatios (theLaboratoryRunData->model, 0, 0, 0,
                           begintime, endtime, hc, oc, nc);
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetLaboratoryRunData - get the laboratory run data from the project
--------------------------------------------------------------------------------

 GxGetLaboratoryRunData retrieves the data needed for a laboratory model run
 from the project and the geological model, allocates memory for the data and
 stores the data in the laboratory run data struct.

 Arguments
 ---------
 subarea        - subarea where sample should be taken
 horizon        - horizon where sample should be taken
 lithology      - lithology where sample should be taken
 sampleposition - the sample position relative to characteristic length

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxGetLaboratoryRunData (char *subarea, char *horizon,
                                           char *lithology,
                                           double sampleposition)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (subarea);
   assert (horizon);
   assert (lithology);
   assert ((sampleposition >= 0.0) && (sampleposition <= 1.0));

   rc = GxAllocLaboratoryRunData ();
   if (!rc) rc = GxFillLaboratoryRunData (subarea, horizon, lithology,
                                          sampleposition);
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetLaboratoryRetIsoHist - get isotope history of a retained species in a 
                             laboratory submodel
--------------------------------------------------------------------------------

 GxGetLaboratoryRetIsoHist returns the isotope history data for a retained
 species <species> in the submodel of the laboratory model. The isotope 
 history array passed should be allocated by calling GxCreateModelHistory.

 Arguments
 ---------
 species   - name of species to get expulsion flux data for
 begintime - the begin time for the history
 endtime   - the end time for the history
 isohist   - array for the isotope history data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLaboratoryRetIsoHist (char *species, double begintime, 
                                       double endtime, double *isohist)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          speciesnr;

   if (!species || (strlen (species) == 0)
      || (strlen (species) > GXL_SPECIESNAME) || !isohist) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the laboratory model is present */
      if (!GxLaboratoryModelPresent ()) {
         rc = GxError (GXE_NOLABMODEL, GXT_NOLABMODEL);
      } else {

/* ----- Check that the species both exist */
         rc = GxFindSpecies (species, &speciesnr);

/* ----- Retrieve retained isotope history data from model */
         if (!rc) rc = GxGetRetIsotopeHist (theLaboratoryRunData->model,
                              0, 0, 0, speciesnr, begintime, endtime, isohist);

      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetLaboratoryTemperatures - get temperature history from laboratory model
--------------------------------------------------------------------------------

 GxGetLaboratoryTemperatures returns the temperatures for the laboratory
 model. The temperature history array passed should be allocated by calling
 GxCreateModelHistory.

 Arguments
 ---------
 begintime    - the begin time for the history
 endtime      - the end time for the history
 temperatures - arrays for the temperature data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLaboratoryTemperatures (double begintime, double endtime,
                                         double *temperatures)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!temperatures) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the laboratory model is present */
      if (!GxLaboratoryModelPresent ()) {
         rc = GxError (GXE_NOLABMODEL, GXT_NOLABMODEL);
      } else {

/* ----- Retrieve the temperature history from the model */
         rc = GxGetTemperatureHist (theLaboratoryRunData->model, 0, 0, 0,
                 begintime, endtime, temperatures);
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetLaboratoryTimeRange - get the time range from the laboratory model
--------------------------------------------------------------------------------

 GxGetLaboratoryTimeRange returns the begin and end times for which the
 laboratory model properties have been calculated.

 Arguments
 ---------
 begintimeptr - pointer for the begin time for the history
 endtimeptr   - pointer for the end time for the history

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLaboratoryTimeRange (double *begintimeptr,
                                      double *endtimeptr)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!begintimeptr || !endtimeptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the laboratory model is present */
      if (!GxLaboratoryModelPresent ()) {
         rc = GxError (GXE_NOLABMODEL, GXT_NOLABMODEL);
      } else {

/* ----- Get the time range from the model */
         rc = GxGetTimeRange (theLaboratoryRunData->model, begintimeptr,
                 endtimeptr);
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetLaboratoryTimes - get the time history from the laboratory model
--------------------------------------------------------------------------------

 GxGetLaboratoryTimes returns the times at which the laboratory model
 properties have been calculated. The time history array passed should be
 allocated by calling GxCreateModelHistory.

 Arguments
 ---------
 begintime - the begin time for the history
 endtime   - the end time for the history
 times     - array for the time data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLaboratoryTimes (double begintime, double endtime,
                                  double *times)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!times) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the laboratory model is present */
      if (!GxLaboratoryModelPresent ()) {
         rc = GxError (GXE_NOLABMODEL, GXT_NOLABMODEL);
      } else {

/* ----- Retrieve the time history from the model */
         rc = GxGetTimeHist (theLaboratoryRunData->model, begintime, endtime,
                 times);
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetLaboratoryTimesteps - get the number of timesteps from the lab model
--------------------------------------------------------------------------------

 GxGetLaboratoryTimesteps returns the number of times at which
 the laboratory model properties have been calculated. This number should
 be used to allocate property history arrays with GxCreateModelHistory.

 Arguments
 ---------
 begintime - the begin time for the history
 endtime   - the end time for the history
 ntimesprt - pointer for number of times

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLaboratoryTimesteps (double begintime, double endtime,
                                      int *ntimesptr)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!ntimesptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the laboratory model is present */
      if (!GxLaboratoryModelPresent ()) {
         rc = GxError (GXE_NOLABMODEL, GXT_NOLABMODEL);
      } else {

/* ----- Get the number of times from the model */
         rc = GxGetNumberOfTimes (theLaboratoryRunData->model, begintime,
                 endtime, ntimesptr);
      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxGetGeologicalVanKrevelen - get Van Krevelen data from a geological submodel
--------------------------------------------------------------------------------

 GxGetLaboratoryVanKrevelen returns the Van Krevelen values for the laboratory
 model. The Van Krevelen history array passed should be allocated by calling
 GxCreateModelHistory.

 Arguments
 ---------
 begintime - the begin time for the history
 endtime   - the end time for the history
 hc        - array for the Hydrogen over carbon data
 oc        - array for the Oxygen over carbon data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetLaboratoryVanKrevelen ( double begintime, double endtime,
                                         double *hc,  double *oc )
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!hc || !oc ) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the laboratory model is present */
      if (!GxLaboratoryModelPresent ()) {
         rc = GxError (GXE_NOLABMODEL, GXT_NOLABMODEL);
      } else {

/* ----- Retrieve Van Krevelen data from the model */
         rc = GxGetVanKrevelen (theLaboratoryRunData->model, 0, 0, 0,
                                begintime, endtime, hc, oc);
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetMainExpulsion - get the main expulsion from the geological model
--------------------------------------------------------------------------------

 GxGetMainExpulsion allocates and returns the data for the expulsion into the
 main secondary migration system in the geological model since the time
 specified. The memory for the expulsion data should be freed by calling
 GxFreeExpulsion.

 Arguments
 ---------
 starttime         - start time for the expulsion calculations
 oilexpulsionptr   - pointer for the oil expulsion data
 gasexpulsionptr   - pointer for the gas expulsion data
 nonhcexpulsionptr - pointer for the non-hydrocarbon expulsion data
 gor1ptr           - pointer for gas oil ratio in standard units
 gor2ptr           - pointer for gas oil ratio in alternative units
 cgrptr            - pointer for condensate gas ratio

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetMainExpulsion (double starttime,
                                GxExpulsion **oilexpulsionptr,
                                GxExpulsion **gasexpulsionptr,
                                GxExpulsion **nonhcexpulsionptr,
                                double *gor1ptr, double *gor2ptr,
                                double *cgrptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxExpulsion  *oilexpulsion=NULL, *gasexpulsion=NULL, *nonhcexpulsion=NULL;
   double       *areas=NULL, **expulsions=NULL;

   if (!oilexpulsionptr || !gasexpulsionptr || !nonhcexpulsionptr || !gor1ptr
      || !gor2ptr || !cgrptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Get the areas of the subareas from the project */
         rc = GxAllocArray (&areas, sizeof (double), 1,
                            theGeologicalRunData->nlocations);
         if (!rc) rc = GxGetSubAreaData (areas);

/* ----- Allocate memory and receive the expulsion data in it */
         if (!rc) rc = GxAllocArray (&expulsions, sizeof (double), 2,
                                     theGeologicalRunData->nlithologies,
                                     theConfiguration->nspecies);
         if (!rc) rc = GxGetTotMainExpulsion (theGeologicalRunData->model,
                          starttime, areas, expulsions);

/* ----- Allocate the expulsion structures and convert the data to them */
         if (!rc) rc = GxAllocExpulsion (&oilexpulsion, &gasexpulsion,
                                         &nonhcexpulsion);
         if (!rc) GxConvertExpulsion (expulsions, oilexpulsion,
                                      gasexpulsion, nonhcexpulsion,
                                      gor1ptr, gor2ptr, cgrptr);

/* ----- Free the temporary memory */
         if (areas)
            GxFreeArray (areas, 1, theGeologicalRunData->nlocations);
         if (expulsions)
            GxFreeArray (expulsions, 2, theGeologicalRunData->nlithologies,
                         theConfiguration->nspecies);
      }
   }
   if (!rc) {
      *oilexpulsionptr   = oilexpulsion;
      *gasexpulsionptr   = gasexpulsion;
      *nonhcexpulsionptr = nonhcexpulsion;
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetMainExpulsionSelected - get the main expulsion from the selected
                              submodels of the geological model
--------------------------------------------------------------------------------

 GxGetMainExpulsionSelected allocates and returns the data for the expulsion 
 into the main secondary migration system of the selected submodels in the 
 geological model since the time specified. The memory for the expulsion data 
 should be freed by calling GxFreeExpulsion.

 Arguments
 ---------
 subareas          - list of subareas to be used
 horizons          - list of horizons to be used
 lithologies       - list of lithologies to be used
 starttime         - start time for the expulsion calculations
 oilexpulsionptr   - pointer for the oil expulsion data
 gasexpulsionptr   - pointer for the gas expulsion data
 nonhcexpulsionptr - pointer for the non-hydrocarbon expulsion data
 gor1ptr           - pointer for gas oil ratio in standard units
 gor2ptr           - pointer for gas oil ratio in alternative units
 cgrptr            - pointer for condensate gas ratio

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetMainExpulsionSelected (SortedList subareas,
                                        SortedList horizons,
                                        SortedList lithologies,
                                        double starttime,
                                        GxExpulsion **oilexpulsionptr,
                                        GxExpulsion **gasexpulsionptr,
                                        GxExpulsion **nonhcexpulsionptr,
                                        double *gor1ptr, double *gor2ptr,
                                        double *cgrptr)
{
   GxErrorCode   rc=GXE_NOERROR;
   GxExpulsion  *oilexpulsion=NULL, 
                *gasexpulsion=NULL, 
                *nonhcexpulsion=NULL;
   double       *areas=NULL, 
               **expulsions=NULL;
   char         *subarea,
                *horizon,
                *lithology;
   GxSubModelId  submodelId,
                *tempId;
   List          submodelList = NULL;

   if (!subareas || !horizons || !lithologies || !oilexpulsionptr || 
       !gasexpulsionptr || !nonhcexpulsionptr || !gor1ptr || !gor2ptr || 
       !cgrptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      *oilexpulsionptr   = NULL;
      *gasexpulsionptr   = NULL;
      *nonhcexpulsionptr = NULL;

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Get the areas of the subareas from the project */
         rc = GxAllocArray (&areas, sizeof (double), 1,
                            theGeologicalRunData->nlocations);
         if (!rc) rc = GxGetSubAreaData (areas);

/* ----- Construct a sorted list with the submodel identifications */
         if (!rc) {
             submodelList = CreateList (sizeof (GxSubModelId), NULL);
             if (!submodelList) {
                 rc = GxError ( GXE_LIST, GXT_LIST );
             } else {
                 subarea = GetFirstFromSortedList (subareas);
                 while (subarea && !rc) {
                     horizon = GetFirstFromSortedList (horizons);
                     while (horizon && !rc) {
                         lithology = GetFirstFromSortedList (lithologies);
                         while (lithology && !rc) {
                             rc = GxFindSubModel (subarea, horizon, lithology,
                                                  &submodelId.subareanr,
                                                  &submodelId.horizonnr,
                                                  &submodelId.lithologynr);
                             if (!rc) {
                                 tempId = AppendToList (submodelList,
                                                        &submodelId);
                                 if (!tempId) {
                                     rc = GxError ( GXE_LIST, GXT_LIST );
                                 }
                             }
                             lithology = GetNextFromList (lithologies);
                         }
                         horizon = GetNextFromList (horizons);
                     }
                     subarea = GetNextFromList (subareas);
                 }
             }
         }

/* ----- Allocate memory and receive the expulsion data in it */
         if (!rc) rc = GxAllocArray (&expulsions, sizeof (double), 2,
                                     theGeologicalRunData->nlithologies,
                                     theConfiguration->nspecies);
         if (!rc) rc = GxGetTotMainExpulsionSelected (
                                     theGeologicalRunData->model, submodelList,
                                     starttime, areas, expulsions);

/* ----- Allocate the expulsion structures and convert the data to them */
         if (!rc) rc = GxAllocExpulsion (&oilexpulsion, &gasexpulsion,
                                         &nonhcexpulsion);
         if (!rc) GxConvertExpulsion (expulsions, oilexpulsion,
                                      gasexpulsion, nonhcexpulsion,
                                      gor1ptr, gor2ptr, cgrptr);

/* ----- Free the temporary memory */
         if (areas) {
             GxFreeArray (areas, 1, theGeologicalRunData->nlocations);
         }
         if (expulsions) {
             GxFreeArray (expulsions, 2, theGeologicalRunData->nlithologies,
                          theConfiguration->nspecies);
         }
         if (submodelList) {
             DeleteList (submodelList);
         }
      }
   }
   if (!rc) {
      *oilexpulsionptr   = oilexpulsion;
      *gasexpulsionptr   = gasexpulsion;
      *nonhcexpulsionptr = nonhcexpulsion;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetMainIsotopeSelected - get the downward and upward isotope from the 
                            selected submodels in the geological model
--------------------------------------------------------------------------------

 GxGetMainIsotopeSelected allocates and returns the data for the downward and
 downward isotopes in the selected submodels in the geological model since the 
 time specified. The memory for the isotope data should be freed by calling 
 GxFreeIsotope.

 Arguments
 ---------
 subareas          - list of subareas to be used
 horizons          - list of horizons to be used
 lithologies       - list of lithologies to be used
 starttime         - start time for the expulsion calculations
 oilisotopeptr     - pointer for the oil expulsion data
 gasisotopeptr     - pointer for the gas expulsion data
 nonhcisotopeptr   - pointer for the non-hydrocarbon expulsion data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetMainIsotopeSelected (SortedList subareas,
                                      SortedList horizons,
                                      SortedList lithologies,
                                      double starttime,
                                      GxIsotope **oilisotopeptr,
                                      GxIsotope **gasisotopeptr,
                                      GxIsotope **nonhcisotopeptr)
{
   GxErrorCode   rc=GXE_NOERROR;
   GxIsotope    *oilisotope=NULL, 
                *gasisotope=NULL, 
                *nonhcisotope=NULL;
   double       *areas=NULL, 
               **expulsions=NULL,
               **isotopes=NULL;
   char         *subarea,
                *horizon,
                *lithology;
   GxSubModelId  submodelId,
                *tempId;
   List          submodelList = NULL;

   if (!subareas || !horizons || !lithologies || !oilisotopeptr || 
       !gasisotopeptr || !nonhcisotopeptr ) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      *oilisotopeptr   = NULL;
      *gasisotopeptr   = NULL;
      *nonhcisotopeptr = NULL;

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Get the areas of the subareas from the project */
         rc = GxAllocArray (&areas, sizeof (double), 1,
                            theGeologicalRunData->nlocations);
         if (!rc) rc = GxGetSubAreaData (areas);

/* ----- Construct a sorted list with the submodel identifications */
         if (!rc) {
             submodelList = CreateList (sizeof (GxSubModelId), NULL);
             if (!submodelList) {
                 rc = GxError ( GXE_LIST, GXT_LIST );
             } else {
                 subarea = GetFirstFromSortedList (subareas);
                 while (subarea && !rc) {
                     horizon = GetFirstFromSortedList (horizons);
                     while (horizon && !rc) {
                         lithology = GetFirstFromSortedList (lithologies);
                         while (lithology && !rc) {
                             rc = GxFindSubModel (subarea, horizon, lithology,
                                                  &submodelId.subareanr,
                                                  &submodelId.horizonnr,
                                                  &submodelId.lithologynr);
                             if (!rc) {
                                 tempId = AppendToList (submodelList,
                                                        &submodelId);
                                 if (!tempId) {
                                     rc = GxError ( GXE_LIST, GXT_LIST );
                                 }
                             }
                             lithology = GetNextFromList (lithologies);
                         }
                         horizon = GetNextFromList (horizons);
                     }
                     subarea = GetNextFromList (subareas);
                 }
             }
         }

/* ----- Allocate memory and receive the expulsion data in it */
         if (!rc) rc = GxAllocArray (&expulsions, sizeof (double), 2,
                                     theGeologicalRunData->nlithologies,
                                     theConfiguration->nspecies);

/* ----- Allocate memory and receive the isotope data in it */
         if (!rc) rc = GxAllocArray (&isotopes, sizeof (double), 2,
                                     theGeologicalRunData->nlithologies,
                                     theConfiguration->nspecies);
         if (!rc) rc = GxGetTotMainIsotopeSelected (
                                              theGeologicalRunData->model,
                                              submodelList, starttime, 
                                              areas, isotopes, expulsions);

/* ----- Allocate the isotope structures and convert the data to them */
         if (!rc) rc = GxAllocIsotope (&oilisotope, &gasisotope,
                                         &nonhcisotope);
         if (!rc) GxConvertIsotopes (expulsions, isotopes, oilisotope,
                                     gasisotope, nonhcisotope);

/* ----- Free the temporary memory */
         if (areas) {
             GxFreeArray (areas, 1, theGeologicalRunData->nlocations);
         }
         if (expulsions) {
             GxFreeArray (expulsions, 2, theGeologicalRunData->nlithologies,
                          theConfiguration->nspecies);
         }
         if (isotopes) {
             GxFreeArray (isotopes, 2, theGeologicalRunData->nlithologies,
                          theConfiguration->nspecies);
         }
         if (submodelList) {
             DeleteList (submodelList);
         }
      }
   }
   if (!rc) {
      *oilisotopeptr   = oilisotope;
      *gasisotopeptr   = gasisotope;
      *nonhcisotopeptr = nonhcisotope;
   }
   return (rc);
}
   

/*
--------------------------------------------------------------------------------
 GxGetMobileSpeciesNameList - return the names of the mobile species
--------------------------------------------------------------------------------

 GxGetMobileSpeciesNameList returns a list with the names of the mobile
 species in Genex.

 Arguments
 ---------
 specieslistptr - pointer to the list of name

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetMobileSpeciesNameList (List *specieslistptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   List         specieslist;
   int          i;

   if (!specieslistptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      if (!theConfiguration) {
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      } else {

/* ----- Create an empty list for the names */
         specieslist = CreateList (GXL_SPECIESNAME+1,
                          (int (*) (const void *, const void *)) strcmp);
         if (!specieslist) {
            rc = GxError (GXE_LIST, GXT_LIST);
         } else {

/* -------- Append the names of the species to the list */
            for (i=0; i<theConfiguration->nspecies; i++) {
               if (theConfiguration->species[i].mobile) {
                  if (!AppendToList (specieslist,
                                     theConfiguration->species[i].name)) {
                     DeleteList (specieslist);
                     rc = GxError (GXE_LIST, GXT_LIST);
                  }
               }
            }
            if (!rc) *specieslistptr = specieslist;
         }
      }
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxGetSpeciesNameList - return the names of the species
--------------------------------------------------------------------------------

 GxGetSpeciesNameList returns a list with the names of the 
 species in Genex.

 Arguments
 ---------
 specieslistptr - pointer to the list of name

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetSpeciesNameList (List *specieslistptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   List         specieslist;
   int          i;

   if (!specieslistptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      if (!theConfiguration) {
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      } else {

/* ----- Create an empty list for the names */
         specieslist = CreateList (GXL_SPECIESNAME+1,
                          (int (*) (const void *, const void *)) strcmp);
         if (!specieslist) {
            rc = GxError (GXE_LIST, GXT_LIST);
         } else {

/* -------- Append the names of the species to the list */
            for (i=0; i<theConfiguration->nspecies; i++) {
               if (!AppendToList (specieslist,
                                     theConfiguration->species[i].name)) {
                  DeleteList (specieslist);
                  rc = GxError (GXE_LIST, GXT_LIST);
               }
            }
            if (!rc) *specieslistptr = specieslist;
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetOptimizeRunData - get the run data for the char length optimization
--------------------------------------------------------------------------------

 GxGetOptimizeRunData retrieves the data needed for the optimization of the
 characteristic length from the project, allocates memory for the data and
 stores the data in the optimize run data struct.

 Arguments
 ---------
 subareaname   - name of subarea of submodel to run optimization on
 horizonname   - name of horizon of submodel to run optimization on
 lithologyname - name of lithology of submodel to run optimization on
 optrundata    - pointer to the optimize run data struct

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxGetOptimizeRunData (char *subareaname, char *horizonname,
                                         char *lithologyname,
                                         GxOptimizeRunData *optrundata)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (subareaname);
   assert (horizonname);
   assert (lithologyname);
   assert (optrundata);

   rc = GxAllocOptimizeRunData (optrundata);
   if (!rc) rc = GxFillOptimizeRunData (subareaname, horizonname,
                                        lithologyname, optrundata);
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetRockEval2Analysis - perform Rock-Eval II analysis of laboratory model
--------------------------------------------------------------------------------

 GxGetRockEval2Analysis performs an analysis of the laboratory model that is
 a simulation of the Rock-Eval II experiment performed in the laboratory.

 Arguments
 ---------
 s1ptr    - pointer to the variable for the S1
 s2ptr    - pointer to the variable for the S2
 s1tocptr - pointer to the variable for the S1 over TOC
 hiptr    - pointer to the variable for the HI
 tmaxptr  - pointer to the variable for the Tmax

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetRockEval2Analysis (double *s1ptr, double *s2ptr,
                                    double *s1tocptr, double *hiptr,
                                    double *tmaxptr)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!s1ptr || !s2ptr || !s1tocptr || !hiptr || !tmaxptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the laboratory model is present */
      if (!theConfiguration || !GxLaboratoryModelPresent ()) {
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      } else {

/* ----- Check the experiment type is the standard Rock-Eval II experiment */
         if (theLaboratoryRunData->experimenttype != GXLE_ROCKEVAL2) {
            rc = GxError (GXE_NOTROCKEVAL2, GXT_NOTROCKEVAL2);
         } else {

/* -------- Calculate the results of the simulated Rock-Eval II experiment */
            rc = GxCalcRockEval2Results (theLaboratoryRunData->model, 0, 0, 0,
                    theLaboratoryRunData->density,
                    theLaboratoryRunData->toc, s1ptr, s2ptr, s1tocptr, hiptr,
                    tmaxptr);
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxGetSubAreaData - get the data for the subareas from the project
--------------------------------------------------------------------------------

 GxGetSubAreaData returns the data for the subareas in the project.

 Arguments
 ---------
 areas - array for the areas

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxGetSubAreaData (double *areas)
{
   GxErrorCode  rc=GXE_NOERROR, rc2;
   int          i;
   char         *subareaname;
   SubArea      *subarea=NULL;

   assert (areas);
   assert (theGeologicalRunData);
   assert (theGeologicalRunData->subareas);

/* Create a temporary subarea structure */
   rc = GxCreateSubArea (&subarea);

/* Retrieve the areas for all the subareas in the subarea list */
   i = 0;
   subareaname = GetFirstFromSortedList (theGeologicalRunData->subareas);
   while (!rc && subareaname) {
      rc = GxGetSubArea (subareaname, subarea);
      areas[i] = subarea->area;
      i++;
      subareaname = GetNextFromSortedList (theGeologicalRunData->subareas);
   }

/* Free the temporary subarea structure */
   if (subarea) {
      rc2 = GxFreeSubArea (subarea);
      if (!rc) rc = rc2;
   }
   return (rc);
}

/*
--------------------------------------------------------------------------------
 GxGetSubBurialTimeRange - get specific time range
--------------------------------------------------------------------------------

 GxGetSubBurialTimeRange returns the earliest start and the last end
 time of a specific submodel

 Arguments
 ---------
 subarea           - name of drainage subarea
 horizon           - name of horizon
 lithology         - name of lithology
 starttime         - start time for the burial history
 endtime           - end time of burial history

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetSubBurialTimeRange (char *subarea, char *horizon,
				     char *lithology, double *begintime,
				     double *endtime)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxTPHistory  **history;
   int          subareanr, horizonnr, lithologynr, lasttime;

   if (!subarea || (strlen(subarea) == 0)
      || (strlen(subarea) > GXL_SUBAREANAME) || !horizon
      || (strlen(horizon) == 0) || (strlen(horizon) > GXL_HORIZONNAME)
      || !lithology || (strlen(lithology) == 0) 
      || (strlen(lithology) > GXL_LITHOLOGYNAME) || !begintime || !endtime ) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

	 rc = GxFindSubModel (subarea, horizon, lithology, &subareanr, 
				&horizonnr, &lithologynr );
	 if (!rc) {
	    history = theGeologicalRunData->tphistory;
	    lasttime   = history[subareanr][horizonnr].ntimes - 1; 
            *begintime = history[subareanr][horizonnr].time[0];
	    *endtime   = history[subareanr][horizonnr].time[lasttime];
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetUpwardExpulsion - get the upward expulsion from the geological model
--------------------------------------------------------------------------------

 GxGetUpwardExpulsion allocates and returns the data for the upward
 expulsion in the geological model since the time specified. The memory for
 the expulsion data should be freed by calling GxFreeExpulsion.

 Arguments
 ---------
 starttime         - start time for the expulsion calculations
 oilexpulsionptr   - pointer for the oil expulsion data
 gasexpulsionptr   - pointer for the gas expulsion data
 nonhcexpulsionptr - pointer for the non-hydrocarbon expulsion data
 gor1ptr           - pointer for gas oil ratio in standard units
 gor2ptr           - pointer for gas oil ratio in alternative units
 cgrptr            - pointer for condensate gas ratio

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetUpwardExpulsion (double starttime,
                                  GxExpulsion **oilexpulsionptr,
                                  GxExpulsion **gasexpulsionptr,
                                  GxExpulsion **nonhcexpulsionptr,
                                  double *gor1ptr, double *gor2ptr,
                                  double *cgrptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   GxExpulsion  *oilexpulsion=NULL, *gasexpulsion=NULL, *nonhcexpulsion=NULL;
   double       *areas=NULL, **expulsions=NULL;

   if (!oilexpulsionptr || !gasexpulsionptr || !nonhcexpulsionptr || !gor1ptr
      || !gor2ptr || !cgrptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Get the areas of the subareas from the project */
         rc = GxAllocArray (&areas, sizeof (double), 1,
                            theGeologicalRunData->nlocations);
         if (!rc) rc = GxGetSubAreaData (areas);

/* ----- Allocate memory and receive the expulsion data in it */
         if (!rc) rc = GxAllocArray (&expulsions, sizeof (double), 2,
                                     theGeologicalRunData->nlithologies,
                                     theConfiguration->nspecies);
         if (!rc) rc = GxGetTotUpExpulsion (theGeologicalRunData->model,
                          starttime, areas, expulsions);

/* ----- Allocate the expulsion structures and convert the data to them */
         if (!rc) rc = GxAllocExpulsion (&oilexpulsion, &gasexpulsion,
                                         &nonhcexpulsion);
         if (!rc) GxConvertExpulsion (expulsions, oilexpulsion,
                                      gasexpulsion, nonhcexpulsion,
                                      gor1ptr, gor2ptr, cgrptr);

/* ----- Free the temporary memory */
         if (areas)
            GxFreeArray (areas, 1, theGeologicalRunData->nlocations);
         if (expulsions)
            GxFreeArray (expulsions, 2, theGeologicalRunData->nlithologies,
                         theConfiguration->nspecies);
      }
   }
   if (!rc) {
      *oilexpulsionptr   = oilexpulsion;
      *gasexpulsionptr   = gasexpulsion;
      *nonhcexpulsionptr = nonhcexpulsion;
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetUpwardExpulsionSelected - get the upward expulsion from the selected 
                                submodels in the geological model
--------------------------------------------------------------------------------

 GxGetUpwardExpulsionSelected allocates and returns the data for the upward
 expulsion of the selected submodels in the geological model since the time 
 specified. The memory for the expulsion data should be freed by calling 
 GxFreeExpulsion.

 Arguments
 ---------
 subareas          - list of subareas to be used
 horizons          - list of horizons to be used
 lithologies       - list of lithologies to be used
 starttime         - start time for the expulsion calculations
 oilexpulsionptr   - pointer for the oil expulsion data
 gasexpulsionptr   - pointer for the gas expulsion data
 nonhcexpulsionptr - pointer for the non-hydrocarbon expulsion data
 gor1ptr           - pointer for gas oil ratio in standard units
 gor2ptr           - pointer for gas oil ratio in alternative units
 cgrptr            - pointer for condensate gas ratio

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetUpwardExpulsionSelected (SortedList subareas,
                                          SortedList horizons,
                                          SortedList lithologies,
                                          double starttime,
                                          GxExpulsion **oilexpulsionptr,
                                          GxExpulsion **gasexpulsionptr,
                                          GxExpulsion **nonhcexpulsionptr,
                                          double *gor1ptr, double *gor2ptr,
                                          double *cgrptr)
{
   GxErrorCode   rc=GXE_NOERROR;
   GxExpulsion  *oilexpulsion=NULL, 
                *gasexpulsion=NULL, 
                *nonhcexpulsion=NULL;
   double       *areas=NULL, 
               **expulsions=NULL;
   char         *subarea,
                *horizon,
                *lithology;
   GxSubModelId  submodelId,
                *tempId;
   List          submodelList = NULL;

   if (!subareas || !horizons || !lithologies || !oilexpulsionptr || 
       !gasexpulsionptr || !nonhcexpulsionptr || !gor1ptr || !gor2ptr || 
       !cgrptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      *oilexpulsionptr   = NULL;
      *gasexpulsionptr   = NULL;
      *nonhcexpulsionptr = NULL;

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Get the areas of the subareas from the project */
         rc = GxAllocArray (&areas, sizeof (double), 1,
                            theGeologicalRunData->nlocations);
         if (!rc) rc = GxGetSubAreaData (areas);

/* ----- Construct a sorted list with the submodel identifications */
         if (!rc) {
             submodelList = CreateList (sizeof (GxSubModelId), NULL);
             if (!submodelList) {
                 rc = GxError ( GXE_LIST, GXT_LIST );
             } else {
                 subarea = GetFirstFromSortedList (subareas);
                 while (subarea && !rc) {
                     horizon = GetFirstFromSortedList (horizons);
                     while (horizon && !rc) {
                         lithology = GetFirstFromSortedList (lithologies);
                         while (lithology && !rc) {
                             rc = GxFindSubModel (subarea, horizon, lithology,
                                                  &submodelId.subareanr,
                                                  &submodelId.horizonnr,
                                                  &submodelId.lithologynr);
                             if (!rc) {
                                 tempId = AppendToList (submodelList,
                                                        &submodelId);
                                 if (!tempId) {
                                     rc = GxError ( GXE_LIST, GXT_LIST );
                                 }
                             }
                             lithology = GetNextFromList (lithologies);
                         }
                         horizon = GetNextFromList (horizons);
                     }
                     subarea = GetNextFromList (subareas);
                 }
             }
         }

/* ----- Allocate memory and receive the expulsion data in it */
         if (!rc) rc = GxAllocArray (&expulsions, sizeof (double), 2,
                                     theGeologicalRunData->nlithologies,
                                     theConfiguration->nspecies);
         if (!rc) rc = GxGetTotUpExpulsionSelected (
                                     theGeologicalRunData->model, submodelList,
                                     starttime, areas, expulsions);

/* ----- Allocate the expulsion structures and convert the data to them */
         if (!rc) rc = GxAllocExpulsion (&oilexpulsion, &gasexpulsion,
                                         &nonhcexpulsion);
         if (!rc) GxConvertExpulsion (expulsions, oilexpulsion,
                                      gasexpulsion, nonhcexpulsion,
                                      gor1ptr, gor2ptr, cgrptr);

/* ----- Free the temporary memory */
         if (areas) {
             GxFreeArray (areas, 1, theGeologicalRunData->nlocations);
         }
         if (expulsions) {
             GxFreeArray (expulsions, 2, theGeologicalRunData->nlithologies,
                          theConfiguration->nspecies);
         }
         if (submodelList) {
             DeleteList (submodelList);
         }
      }
   }
   if (!rc) {
      *oilexpulsionptr   = oilexpulsion;
      *gasexpulsionptr   = gasexpulsion;
      *nonhcexpulsionptr = nonhcexpulsion;
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxGetUpwardIsotopeSelected - get the upward isotope from the selected 
                              submodels in the geological model
--------------------------------------------------------------------------------

 GxGetUpwardIsotopeSelected allocates and returns the data for the upward 
 isotopes in the selected submodels in the geological model since the 
 time specified. The memory for the isotope data should be freed by calling 
 GxFreeIsotope.

 Arguments
 ---------
 subareas          - list of subareas to be used
 horizons          - list of horizons to be used
 lithologies       - list of lithologies to be used
 starttime         - start time for the expulsion calculations
 oilisotopeptr     - pointer for the oil expulsion data
 gasisotopeptr     - pointer for the gas expulsion data
 nonhcisotopeptr   - pointer for the non-hydrocarbon expulsion data

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxGetUpwardIsotopeSelected (SortedList subareas,
                                        SortedList horizons,
                                        SortedList lithologies,
                                        double starttime,
                                        GxIsotope **oilisotopeptr,
                                        GxIsotope **gasisotopeptr,
                                        GxIsotope **nonhcisotopeptr)
{
   GxErrorCode   rc=GXE_NOERROR;
   GxIsotope    *oilisotope=NULL, 
                *gasisotope=NULL, 
                *nonhcisotope=NULL;
   double       *areas=NULL, 
               **expulsions=NULL,
               **isotopes=NULL;
   char         *subarea,
                *horizon,
                *lithology;
   GxSubModelId  submodelId,
                *tempId;
   List          submodelList = NULL;

   if (!subareas || !horizons || !lithologies || !oilisotopeptr || 
       !gasisotopeptr || !nonhcisotopeptr ) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      *oilisotopeptr   = NULL;
      *gasisotopeptr   = NULL;
      *nonhcisotopeptr = NULL;

/* -- Check that the geological model is present */
      if (!GxGeologicalModelPresent ()) {
         rc = GxError (GXE_NOGEOMODEL, GXT_NOGEOMODEL);
      } else {

/* ----- Get the areas of the subareas from the project */
         rc = GxAllocArray (&areas, sizeof (double), 1,
                            theGeologicalRunData->nlocations);
         if (!rc) rc = GxGetSubAreaData (areas);

/* ----- Construct a sorted list with the submodel identifications */
         if (!rc) {
             submodelList = CreateList (sizeof (GxSubModelId), NULL);
             if (!submodelList) {
                 rc = GxError ( GXE_LIST, GXT_LIST );
             } else {
                 subarea = GetFirstFromSortedList (subareas);
                 while (subarea && !rc) {
                     horizon = GetFirstFromSortedList (horizons);
                     while (horizon && !rc) {
                         lithology = GetFirstFromSortedList (lithologies);
                         while (lithology && !rc) {
                             rc = GxFindSubModel (subarea, horizon, lithology,
                                                  &submodelId.subareanr,
                                                  &submodelId.horizonnr,
                                                  &submodelId.lithologynr);
                             if (!rc) {
                                 tempId = AppendToList (submodelList,
                                                        &submodelId);
                                 if (!tempId) {
                                     rc = GxError ( GXE_LIST, GXT_LIST );
                                 }
                             }
                             lithology = GetNextFromList (lithologies);
                         }
                         horizon = GetNextFromList (horizons);
                     }
                     subarea = GetNextFromList (subareas);
                 }
             }
         }

/* ----- Allocate memory and receive the expulsion data in it */
         if (!rc) rc = GxAllocArray (&expulsions, sizeof (double), 2,
                                     theGeologicalRunData->nlithologies,
                                     theConfiguration->nspecies);

/* ----- Allocate memory and receive the isotope data in it */
         if (!rc) rc = GxAllocArray (&isotopes, sizeof (double), 2,
                                     theGeologicalRunData->nlithologies,
                                     theConfiguration->nspecies);
         if (!rc) rc = GxGetTotUpIsotopeSelected (
                                              theGeologicalRunData->model,
                                              submodelList, starttime, 
                                              areas, isotopes, expulsions);

/* ----- Allocate the isotope structures and convert the data to them */
         if (!rc) rc = GxAllocIsotope (&oilisotope, &gasisotope,
                                         &nonhcisotope);
         if (!rc) GxConvertIsotopes (expulsions, isotopes, oilisotope,
                                     gasisotope, nonhcisotope);

/* ----- Free the temporary memory */
         if (areas) {
             GxFreeArray (areas, 1, theGeologicalRunData->nlocations);
         }
         if (expulsions) {
             GxFreeArray (expulsions, 2, theGeologicalRunData->nlithologies,
                          theConfiguration->nspecies);
         }
         if (isotopes) {
             GxFreeArray (isotopes, 2, theGeologicalRunData->nlithologies,
                          theConfiguration->nspecies);
         }
         if (submodelList) {
             DeleteList (submodelList);
         }
      }
   }
   if (!rc) {
      *oilisotopeptr   = oilisotope;
      *gasisotopeptr   = gasisotope;
      *nonhcisotopeptr = nonhcisotope;
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxHasFractionationdata - Determines if there is isotopic fractionation
                          data availaible for specified species
--------------------------------------------------------------------------------

 Determines if there is isotopic fractionation data availaible for specified 
 species

 Arguments
 ---------
 speciesnr - number of species

 Return value
 ------------
 GxTrue    Data available
 GxFalse   No data available

--------------------------------------------------------------------------------
*/

static GxBool GxHasFractionationData (int speciesnr)
{
   GxBool retvalue = GxFalse;
   int    i;

   for (i=0; i < theConfiguration->isofract.nreactions; i++) {
      if ( theConfiguration->isofract.fractmethod[i][speciesnr] != 
                                           GX_ISO_FRAC_NOT_SPECIFIED && 
           theConfiguration->isofract.fractmethod[i][speciesnr] != 
                                           GX_ISO_FRAC_NONE) {
         retvalue = GxTrue;
      }
   }
   
   return retvalue; 
}


/*
--------------------------------------------------------------------------------
 GxIsSpeciesMobile - Determines if specy is mobile
--------------------------------------------------------------------------------

 Check if specy is mobile

 Arguments
 ---------
 name - Name of the specified specy

 Return value
 ------------
 GxTrue    It is mobile
 GxFalse   It is immobile

--------------------------------------------------------------------------------
*/

GxBool GxIsSpeciesMobile (char *name)
{
   int  i;

   assert (name);
   assert (theConfiguration);

   for (i=0; i<theConfiguration->nspecies; i++) {
       if (strcmp (theConfiguration->species[i].name, name) == 0) {
           if (theConfiguration->species[i].mobile) {
               return GxTrue;
           } else {
               return GxFalse;
           } 
       }
   }
   return GxFalse;
}


/*
--------------------------------------------------------------------------------
 GxIsSpeciesHCGas - Determines if specy is a hydrocarbon gas
--------------------------------------------------------------------------------

 Check if specy is a hydrocarbon gas

 Arguments
 ---------
 name - Name of the specified specy

 Return value
 ------------
 GxTrue    It is a hydrocarbon gas
 GxFalse   It is something else

--------------------------------------------------------------------------------
*/

GxBool GxIsSpeciesHCGas (char *name)
{
   int  i;

   assert (name);
   assert (theConfiguration);

   for (i=0; i<theConfiguration->nspecies; i++) {
      if (theConfiguration->species[i].mobile) {
          if (strcmp (theConfiguration->species[i].name, name) == 0) {
              if (GxIsHCGas (&theConfiguration->species[i])) {
                  return GxTrue;
              } else {
                  return GxFalse;
              }
           }
       }
   }
   return GxFalse;
}


/*
--------------------------------------------------------------------------------
 GxIsSpeciesHCWetGas - Determines if specy is a wet hydrocarbon gas
--------------------------------------------------------------------------------

 Check if specy is a wet hydrocarbon gas

 Arguments
 ---------
 name - Name of the specified specy

 Return value
 ------------
 GxTrue    It is a hydrocarbon gas
 GxFalse   It is something else

--------------------------------------------------------------------------------
*/

GxBool GxIsSpeciesHCWetGas (char *name)
{
   int  i;

   assert (name);
   assert (theConfiguration);

   for (i=0; i<theConfiguration->nspecies; i++) {
      if (theConfiguration->species[i].mobile) {
          if (strcmp (theConfiguration->species[i].name, name) == 0) {
              if (GxIsHCWetGas (&theConfiguration->species[i])) {
                  return GxTrue;
              } else {
                  return GxFalse;
              }
           }
       }
   }
   return GxFalse;
}


/*
--------------------------------------------------------------------------------
 GxIsSpeciesOil - Determines if specy is an oil
--------------------------------------------------------------------------------

 Check if specy is a oil

 Arguments
 ---------
 name - Name of the specified specy

 Return value
 ------------
 GxTrue    It is an oil
 GxFalse   It is something else

--------------------------------------------------------------------------------
*/

GxBool GxIsSpeciesOil (char *name)
{
   int  i;

   assert (name);
   assert (theConfiguration);

   for (i=0; i<theConfiguration->nspecies; i++) {
      if (theConfiguration->species[i].mobile) {
          if (strcmp (theConfiguration->species[i].name, name) == 0) {
              if (GxIsHydrocarbon (&theConfiguration->species[i]) &&
                  !GxIsHCGas (&theConfiguration->species[i])) { 
                  return GxTrue;
              } else {
                  return GxFalse;
              }
           }
       }
   }
   return GxFalse;
}


/*
--------------------------------------------------------------------------------
 GxIsSpeciesOilGas - Determines if specy is an oil or a gas.
--------------------------------------------------------------------------------

 Check if specy is asphaltene, HCGas or a hydrocarbon

 Arguments
 ---------
 name - Name of the specified specy

 Return value
 ------------
 GxTrue    It is an oil or a gas
 GxFalse   It is something else

--------------------------------------------------------------------------------
*/


GxBool GxIsSpeciesOilGas (char *name)
{
   GxBool isOilorGas;

   assert (name);
   assert (theConfiguration);

   isOilorGas = GxIsSpeciesOil (name);
   if (!isOilorGas) {
       isOilorGas = GxIsSpeciesHCGas (name);
   }
   return isOilorGas;
}


/*
--------------------------------------------------------------------------------
 GxLaboratoryModelPresent - returns true if a laboratory model is present
--------------------------------------------------------------------------------

 GxLaboratoryModelPresent returns a value of GxTrue if a laboratory model
 is present and GxFalse if no laboratory model is present.

 Return value
 ------------
 GxTrue if laboratory model present, GxFalse otherwise

--------------------------------------------------------------------------------
*/

GxBool GxLaboratoryModelPresent (void)
{
   return (theLaboratoryRunData ? GxTrue : GxFalse);
}



/*
--------------------------------------------------------------------------------
 GxNitrogenPartOfModel - returns true if the nitrogen element is used within
                         the model
--------------------------------------------------------------------------------

 GxNitrogenPartOfModel returns a value of GxTrue if the nitrogen element is
 used within the mode and GxFalse if nitrogen is not used.

 Return value
 ------------
 GxTrue if nitrogen is used, GxFalse otherwise

--------------------------------------------------------------------------------
*/

GxBool GxNitrogenPartOfModel (void)
{
   return ((theConfiguration->nelements == 4) ? GxTrue : GxFalse);
}



/*
--------------------------------------------------------------------------------
 GxOpen - open Genex for modelling
--------------------------------------------------------------------------------

 GxOpen opens Genex for modeling by reading in the configuration file and
 initializing the project.

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxOpen (void)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (theConfiguration || theGeologicalRunData || theLaboratoryRunData) {
      rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
   } else {

/* -- Read the configuration file */
      rc = GxReadConfiguration (NULL, &theConfiguration);

/* -- Initialize the project */
      if (!rc) rc = GxInitializeProject ();
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxOptimizationPass - runs a characteristic length optimization pass
--------------------------------------------------------------------------------

 GxOptimizationPass runs a pass for the optimization of the characteristic
 length. It takes the optimization run data as input and runs a geological
 and a Rock-Eval II laboratory simulation for this length. GxOptimizationPass
 returns the value of S1/TOC from the model.

 Arguments
 ---------
 optrundata - the optimization run data with the characteristic length
 s1tocptr   - pointer to the variable for S1/TOC

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxOptimizationPass (GxOptimizeRunData *optrundata,
                                double *s1tocptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   double       dummy, s1, s2, hi, tmax;

   if (!optrundata || !s1tocptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      if (!theConfiguration) {
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      } else {

/* ----- Run a geological simulation */
         rc = GxRunModel (optrundata->geomodel,
                 NULL, NULL, theConfiguration->overburdendensity,
                 theConfiguration->inorganicdensity,
                 theConfiguration->surfacetemperature,
                 theConfiguration->temperaturegradient,
                 theConfiguration->expansivity,
                 theConfiguration->compressibility,
                 GX_GEOLOGICALTIME, optrundata->geospecies,
                 optrundata->stoichiometry, optrundata->geotphistory,
                 &optrundata->geocharlength, &optrundata->geoupperbiot,
                 &optrundata->geolowerbiot, optrundata->geonetthickness,
                 optrundata->referenceposition, optrundata->geoconcentration,
                 &optrundata->initkerogenconc);

/* ----- Take a sample from a submodel of the geological model */
         if (!rc) rc = GxSampleSubModel (optrundata->geomodel, 0, 0, 0,
                 optrundata->sampleposition, optrundata->labconcentration[0],
                 &dummy, &optrundata->density, &optrundata->toc);

/* ----- Run a laboratory simulation on the sample */
         if (!rc) rc = GxRunModel (optrundata->labmodel,
                 NULL, NULL, theConfiguration->overburdendensity,
                 theConfiguration->inorganicdensity,
                 theConfiguration->surfacetemperature,
                 theConfiguration->temperaturegradient,
                 theConfiguration->expansivity,
                 theConfiguration->compressibility,
                 GX_LABORATORYTIME, optrundata->labspecies,
                 optrundata->stoichiometry, optrundata->labtphistory,
                 &optrundata->labcharlength, &optrundata->labupperbiot,
                 &optrundata->lablowerbiot, optrundata->labnetthickness,
                 optrundata->referenceposition, optrundata->labconcentration,
                 &optrundata->initkerogenconc);

/* ----- Analyse the laboratory run in terms of the Rock-Eval II experiment */
         if (!rc) rc = GxCalcRockEval2Results (optrundata->labmodel,
                 0, 0, 0, optrundata->density, optrundata->toc,
                 &s1, &s2, s1tocptr, &hi, &tmax);
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxOptimizeCharLength - optimize the characteristic length of a model
--------------------------------------------------------------------------------

 GxOptimizeCharLength optimizes the characteristic length of a particular
 submodel identified by <subarea>, <horizon> and <lithology>. A geological
 run and a Rock-Eval II laboratory simulation are run until the accuracy
 requested in the run options or the maximum number of iterations is reached.
 After each laboratory run the characteristic length of the geological model
 is optimized. The final characteristic length is returned in the argument
 <charlengthptr>. It is not stored in the project data. Also, the error in
 the result is returned in <errorptr>, and the number of iterations in
 <niterationsptr>.

 Arguments
 ---------
 subarea        - subarea where sample should be taken
 horizon        - horizon where sample should be taken
 lithology      - lithology where sample should be taken
 sampleposition - the sample position relative to characteristic length
 charlengthptr  - pointer to variable for optimized characteristic length
 errorptr       - pointer to variable for remaining error
 niterationsptr - pointer to variable for number of iterations

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxOptimizeCharLength (char *subarea, char *horizon,
                                  char *lithology, double sampleposition,
                                  double *charlengthptr, double *errorptr,
                                  int *niterationsptr)
{
   GxErrorCode        rc=GXE_NOERROR, rc2;
   GxOptimizeRunData  optrundata;
   double             b, lowerchlen, upperchlen, lowerb, upperb;
   int                i;

   if (!subarea || (strlen (subarea) == 0)
      || (strlen (subarea) > GXL_SUBAREANAME) || !horizon
      || (strlen (horizon) == 0) || (strlen (horizon) > GXL_HORIZONNAME)
      || !lithology || (strlen (lithology) == 0)
      || (strlen (lithology) > GXL_LITHOLOGYNAME) || (sampleposition < 0.0)
      || (sampleposition > 1.0) || !charlengthptr || !errorptr
      || !niterationsptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      if (!theConfiguration) {
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      } else {

/* ----- Retrieve the data for the optimization run from the project */
         rc = GxGetOptimizeRunData (subarea, horizon, lithology, &optrundata);
         if (!rc) {
            optrundata.sampleposition = sampleposition;
            i = 1;

/* -------- Perform one pass with the user's best guess */
            rc = GxOptimizationPass (&optrundata, &b);
            if (!rc) {

/* ----------- Is the first guess already good enough ? */
               if (ABS (log (b) - log (optrundata.s1tocm))
                   > optrundata.maxerror) {
                  if (b > optrundata.s1tocm) {

/* ----------------- Guess is too high: loop until lower boundary found */
                     while (!rc && (b > optrundata.s1tocm)
                        && (i < optrundata.maxiterations)) {
                        upperchlen = optrundata.geocharlength;
                        upperb     = b;

/* -------------------- Determine new guess */
                        optrundata.geocharlength /= optrundata.factor;
                        i++;

			if ( optrundata.geocharlength < GXC_LOWERCHARLENGTH ) {
			   rc = GxError ( GXE_CANNOTOPTIMIZE, GXT_CANNOTOPTIMIZE );
			} else {
/* -------------------- Run another pass with the new guess */
                           rc = GxOptimizationPass (&optrundata, &b);
                           if (!rc && (upperb <= b))
                              rc = GxError (GXE_NOCONVERGANCE,
                                            GXT_NOCONVERGANCE);
			}
                     }
                     lowerchlen = optrundata.geocharlength;
                     lowerb     = b;
                  } else {

/* ----------------- Guess is too low: loop until upper boundary found */
                     while (!rc && (b <= optrundata.s1tocm)
                        && (i < optrundata.maxiterations)) {
                        lowerchlen = optrundata.geocharlength;
                        lowerb     = b;
                        i++;

/* -------------------- Determine new guess */
                        optrundata.geocharlength *= optrundata.factor;
			if (  optrundata.geocharlength > GXC_UPPERCHARLENGTH ){
			    rc = GxError (GXE_CANNOTOPTIMIZE, GXT_CANNOTOPTIMIZE);
			} else {
/* -------------------- Run another pass with the new guess */
                           rc = GxOptimizationPass (&optrundata, &b);
                           if (!rc && (lowerb >= b))
                              rc = GxError (GXE_NOCONVERGANCE, 
					    GXT_NOCONVERGANCE);
			}
                     }
                     upperchlen = optrundata.geocharlength;
                     upperb     = b;
                  }
               }

/* ----------- Is the guess now good enough ? */
               if (ABS (log (b) - log (optrundata.s1tocm))
                   > optrundata.maxerror) {

/* -------------- Determine new char length from the lower and upper limit */
                  optrundata.geocharlength =
                     (lowerchlen * (upperb - optrundata.s1tocm) -
                      upperchlen * (lowerb - optrundata.s1tocm)) /
                     (upperb - lowerb);

/* -------------- Keep on closing in until it is accurate enough */
                  while (!rc && (i < optrundata.maxiterations)
                     && (ABS (log (b) - log (optrundata.s1tocm))
                         > optrundata.maxerror)) {
                     i++;

/* ----------------- Run another pass with the new guess */
                     rc = GxOptimizationPass (&optrundata, &b);
                     if (!rc) {
                        if (b > optrundata.s1tocm) {

/* ----------------------- Guess is too high: adjust upper limit */
                           upperb     = b;
                           upperchlen = optrundata.geocharlength;
                        } else {

/* ----------------------- Guess is too low: adjust lower limit */
                           lowerb     = b;
                           lowerchlen = optrundata.geocharlength;
                        }

/* -------------------- Determine char length from lower and upper limit */
                        optrundata.geocharlength =
                           (lowerchlen * (upperb - optrundata.s1tocm) -
                            upperchlen * (lowerb - optrundata.s1tocm)) /
                           (upperb - lowerb);
                     }
                  }
               }
            }

/* -------- See whether we are finally accurate enough */
            if (!rc && (i == optrundata.maxiterations)
              && (ABS (log (b) - log (optrundata.s1tocm))
                > optrundata.maxerror))
              rc = GxError (GXE_MAXITERREACHED, GXT_MAXITERREACHED);
/* -------- See wether end result is reasonable */
            if (!rc &&  optrundata.geocharlength > GXC_FINALCHARLENGTH )
              rc = GxError (GXE_CANNOTOPTIMIZE, GXT_CANNOTOPTIMIZE);

/* -------- Free the temporary structure */
            rc2 = GxFreeOptimizeRunData (&optrundata);
            if (!rc) rc = rc2;
	    rc2 = GxFreeGeologicalRunData ();
	    if (!rc) rc = rc2;
            if (!rc) {

/* ----------- Return the optimized value, its error and the iterations */
               *charlengthptr  = optrundata.geocharlength;
               *errorptr       = ABS (log (b) - log (optrundata.s1tocm));
               *niterationsptr = i;
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxRunGeologicalModel - run a complete geological model
--------------------------------------------------------------------------------

 GxRunGeologicalModel performs the model calculations for all the locations,
 temperature and pressure histories, and lithologies in the project.

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxRunGeologicalModel (void)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i,j,k,l,m;

   if (!theConfiguration) {
      rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
   } else {

/* -- Free any run data from previous runs */
      rc = GxFreeGeologicalRunData ();

/* -- Get the current run data from the project */
      if (!rc) rc = GxGetGeologicalRunData ();

/* -- Run the model using the run data */
      if (!rc) rc = GxRunModel (theGeologicalRunData->model,
                       &theConfiguration->isofract,
                       theGeologicalRunData->isofactor,
                       theConfiguration->overburdendensity,
                       theConfiguration->inorganicdensity,
                       theConfiguration->surfacetemperature,
                       theConfiguration->temperaturegradient,
                       theConfiguration->expansivity,
                       theConfiguration->compressibility,
                       GX_GEOLOGICALTIME, theGeologicalRunData->species,
                       theGeologicalRunData->stoichiometry,
                       theGeologicalRunData->tphistory,
                       theGeologicalRunData->charlength,
                       theGeologicalRunData->upperbiot,
                       theGeologicalRunData->lowerbiot,
                       theGeologicalRunData->netthickness,
                       theGeologicalRunData->referenceposition,
                       theGeologicalRunData->concentration,
                       theGeologicalRunData->initkerogenconc);
      if (rc) (void) GxFreeGeologicalRunData ();
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxRunLaboratoryModel - run a complete laboratory model
--------------------------------------------------------------------------------

 GxRunLaboratoryModel performs the model calculations for a laboratory
 simulation. A geological model should have been run first. The laboratory
 model is run using the results of the geological model. The arguments
 of GxRunLaboratoryModel identify the position where a sample should be
 taken from the geological model.

 Arguments
 ---------
 subarea        - subarea where sample should be taken
 horizon        - horizon where sample should be taken
 lithology      - lithology where sample should be taken
 sampleposition - the sample position relative to characteristic length

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxRunLaboratoryModel (char *subarea, char *horizon,
                                  char *lithology, double sampleposition)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (!subarea || (strlen (subarea) == 0)
      || (strlen (subarea) > GXL_SUBAREANAME) || !horizon
      || (strlen (horizon) == 0) || (strlen (horizon) > GXL_HORIZONNAME)
      || !lithology || (strlen (lithology) == 0)
      || (strlen (lithology) > GXL_LITHOLOGYNAME) || (sampleposition < 0.0)
      || (sampleposition > 1.0)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check that the geological model is present */
      if (!theConfiguration || !GxGeologicalModelPresent ()) {
         rc = GxError (GXE_STATE, GXT_STATE, __FILE__, __LINE__);
      } else {

/* ----- Free any run data from previous runs */
         rc = GxFreeLaboratoryRunData ();

/* ----- Get the current run data from the project */
         if (!rc) rc = GxGetLaboratoryRunData (subarea, horizon, lithology,
                                               sampleposition);

/* ----- Run the model using the run data */
         if (!rc) rc = GxRunModel (theLaboratoryRunData->model,
                          &theConfiguration->isofract,
                          &theLaboratoryRunData->isofactor,
                          theConfiguration->overburdendensity,
                          theConfiguration->inorganicdensity,
                          theConfiguration->surfacetemperature,
                          theConfiguration->temperaturegradient,
                          theConfiguration->expansivity,
                          theConfiguration->compressibility,
                          GX_LABORATORYTIME, theLaboratoryRunData->species,
                          theLaboratoryRunData->stoichiometry,
                          theLaboratoryRunData->tphistory,
                          &theLaboratoryRunData->charlength,
                          &theLaboratoryRunData->upperbiot,
                          &theLaboratoryRunData->lowerbiot,
                          theLaboratoryRunData->netthickness,
                          theLaboratoryRunData->referenceposition,
                          theLaboratoryRunData->concentration,
                          &theLaboratoryRunData->initkerogenconc);
         if (rc) (void) GxFreeLaboratoryRunData ();
      }
   }
   return (rc);
}
