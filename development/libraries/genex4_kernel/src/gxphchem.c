/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxphchem.c,v 25.0 2005/07/05 08:03:48 ibs Stable $ */
/* ================================================================== */
/* 
$Log: gxphchem.c,v $
Revision 25.0  2005/07/05 08:03:48  ibs
IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.

Revision 24.1  2004/10/04 14:53:30  ibs
First time in.

Revision 24.2  2004/04/21 14:36:18  ibs
mistype

* Revision 24.1  2004/04/21 14:30:46  ibs
* Added brackets to solve a 64 bit compile error on the sun.
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
 * Revision 21.0  2001/05/02 08:59:25  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:41  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:51  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:17  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:12  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:19  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:53  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:36  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.1  1997/07/03  14:34:01  ibs
 * removed nested comment.
 *
 * Revision 13.0  1997/02/10  16:03:48  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.2  1996/10/31  12:47:37  cs261
 * correction for MIN MAX
 *
 * Revision 12.1  1996/10/31  10:36:57  cs261
 * temp change for wiekert: very small time steps.
 *
 * Revision 12.0  1996/07/15  14:56:12  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:30  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:52:36  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 GxPhChem.c   Genex physics and chemistry functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        25-JUN-1992

 Description: GxPhChem.c contains the Genex functions for physics and
              chemistry calculations. The following functions are available:

                GxAPIGravity            - calculate API gravity from density
                GxAromaticity           - calculate aromaticity for a species
                GxAromaticityOfChains   - calculate aromaticity of chains
                GxAsphalteneComp        - determine asphaltene composition
                GxAsphalteneTransf      - calculate trans.ratio of aspaltene
                GxAtomicRatios          - calculate the atomic ratios
                GxBubblePointGOR        - calculate GOR for bubble point
		GxCalcFreeGas		- calculate gas volume - gas in H2O
                GxCalcInitialConc       - calculate the initial concentration
                GxCalcRatios            - calculate H/C, O/C and N/C
                GxCalcSpecies           - calculate the variable species data
                GxCalcStoichiometry     - calculate the stoichiometry
                GxCalcVanKrevelen       - calculate H/C and O/C
                GxConcLumped            - calculted lumped concentrations
                GxConvMassTransfer      - calculate convective mass transfer
                GxCrackingRate          - calculate rate of cracking
                GxDepth                 - estimate the depth from temperature
                GxDetermineFieldType    - determines the type of the field
                GxFailPressure          - calculate the failure pressure
                GxFormulaWeight         - calculate formula weight for species
                GxFourierNumber         - calculate diffusion Fourier number
                GxGasInWater            - calculate volume of gas in water
                GxGeneration            - generation for species and time step
                GxGenerationDiffusion   - generation and diffusion
                GxGeologicalVRE         - calculate next geological VRE value
                GxHasOilComposition     - return whether species has oil comp
                GxImmobileDensity       - calculate immobile species density
                GxIsAsphaltene          - return whether species is asphaltene
                GxIsHCGas               - return whether species is hc gas
                GxIsHydrocarbon         - return whether species is hydrocarb.
                GxIsN2Gas               - return whether species is N2 gas
                GxKerogenTransf         - calculate the trans.ratio of kerogen
                GxLithostaticPressure   - calculate the lithostatic pressure
                GxMassStoichFactor      - calc mass stoich factor from molar
                GxOverPressure          - calculate the over pressure
                GxReferenceDiffusivity  - calculate the reference diffusivity
                GxSaturationCGR         - calculate the saturation CGR
                GxTotalOrganicCarbon    - calculate the total organic carbon
                GxVolumeFractionOM      - calculate fraction of organic matter
                GxWaterDensity          - calculate the density of water

              To support these functions this file contains a number of
              local functions that can not be used outside this file.

 Usage notes: - In order to use the functions in this source file the header
                file gxphchem.h should be included.
              - Some functions return an error code. If an error is detected
                the error handler function is called and an error code unequal
                to GXE_NOERROR is returned to the caller.


 History
 -------
 25-JUN-1992  P. Alphenaar       initial version
 29-JUL-1992  M.J.A. van Houtert GxCalcVanKrevelen added 
  5-AUG-1992  M.J.A. van Houtert GxCalcFreeGas added
 17-AUG-1992  M.J.A. van Houtert Check for API < 0.0 in GxAPIGravity
 28-MAY-1993  M.J.A. van Houtert Generation data added to GxGeneration and
                                 GxGenerationDiffusion
 18-AUG-1993  Th. Berkers        Net Generation implemented in submodel using 
                                 notes from J.G. Stainforth, 6/93.
                                 Changes have been implemented in: 
                                   GxGeneration
                                   GxGenerationDiffusion
 06-DEC-1993  Th. Berkers        When calculating the corrected H/C for hydro-
                                 carbons only the Oxygene composition is taken 
                                 instead of O/C ration. This has been changed in
                                 function GxCorrectedHC
 22-DEC-1993  Th. Berkers        Implementing Nitrogen model:
                                 GxAsphalteneComp exteneded with nitrogen part
                                 GxCalcSpecies parameter-list extended with
                                 'kerogenn' (nci of lithology in use).
                                 GxIsN2Gas added.
                                 All N2 related functionality placed between 
                                 if (GxNitrogenPartOfModel)) 
                                 Printing of calculated stoichiometry factors
                                 placed between compiler condition CONF_TEST
 29-DEC-1993  Th. Berkers        The stoichiometry factor for NH4 when cracking
                                 Kerogene may be smaller then zero
 04-JAN-1994  Th. Berkers        Implementing Nitrogen model: To parameter-list
                                 of GxOverPressure n2mass added as double
                                 GxN2InWater added as interface function,
                                 GxCalcHCGasInWaterSaturation function added.
 10-JAN-1994  Th. Berkers        Implementing Nitrogen model:
                                     GxCalcRatios added
 14-DEC-1994  Th. Berkers        GxIsHCWetGas added
 02-MAR-1995  Th. Berkers        Integration of Isotopes Fractionation
--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <stdio.h>
#include  <assert.h>
#include  <float.h>
#include  <math.h>
#include  <string.h>
#include  "gxconsts.h"
#include  "gxerror.h"
#include  "gxnumer.h"
#include  "gxphchem.h"
#include  "gxutils.h"
#include  "gxkernel.h"

/* Macro definitions */
/* ----------------- */
#define  GX_DEFAULTKEROGENC      1.0
/* -- defintions for species criteria -- */
#define   GX_ASPHALTENEPREFIX    "asphaltene"
#define   GX_C_MINHC             1.0
#define   GX_H_MINGAS            2.35
#define   GX_H_MINHC             0.01
#define   GX_H_MINOIL            0.5
#define   GX_N_MAXHC             0.5
#define   GX_O_MAXHC             0.5
#define   GX_S_MAXHC             1.0

#define   GX_H_MINHCDRYGAS       4.0

#define   GX_N_MINN2             2.0
#define   GX_C_MAXN2             0.01
#define   GX_H_MAXN2             0.01
#define   GX_O_MAXN2             0.01
#define   GX_S_MAXN2             0.01
/* -- constants for Lopatin algorithm -- */
#define   GXC_LOPATINCALA        0.0821
#define   GXC_LOPATINCALF        5.1
#define   GXC_LOPATINECALB       0.53526
#define   GXC_LOPATINTEMPA       (105.0+GXC_ZEROCELCIUS)
#define   GXC_LOPATINTEMPB       10.0
/* -- error text definitions -- */
#define   GXT_INVREACTIONSCHEME  "There is an error in the reaction " \
                                 "scheme!\n" \
                                 "One or more of the stoichiometric factors " \
                                 "is smaller than zero.\n"
/* -- constants for Isotope fractionation algorithm -- */
#define   GXC_MAX_CONVERSION     0.995


/* Function prototypes */
/* ------------------- */
static double  GxActUDiffusion (double, double);
static double  GxActUReaction (double, double, double);
static double  GxAromaticityDependence (double, double);
static double  GxArrheniusRateFactor (double, double, double, double, double,
                                      double);
static double  GxCalcIsoFractFactor (double, GxIsoFactor *);
static double  GxCalcIsotopeFractCn (int, double, double, double, double, 
                                     GxIsoFactor *);
static double  GxCalcIsotopeFractC1 (double, double, double, double, 
                                     GxIsoFactor *);
static double  GxCalcIsotopeFractionation (int, double, double, double, double,
                                           double, GxIsoFactor *, 
                                           GxIsoFractMethod);
static double  GxConcFactor (const double *, const double *);
static double  GxCorrectedHC (const double *);
static double  GxDiffusionEntropy (double, double);
static double  GxDiffusionRate (const GxSpecies *, double, double, double,
                                double, double, double);
static double  GxDiffusionVolume (double, double);

/* Global variables */
/* ---------------- */
/* -- file scope variables -- */
static double atomicWeight[GXN_ELEMENTS]={
   12.011, 1.0079, 15.9994, 14.0067, 32.06
};



/*
--------------------------------------------------------------------------------
 GxActUDiffusion - calculate the activation energy for diffusion
--------------------------------------------------------------------------------

 GxActUDiffusion calculates the activation internal energy for diffusion
 as a function of the aromaticity of chains.

 Arguments
 ---------
 activationenergyref - reference activation internal energy
 aromchain           - the aromaticity of chains

 Return value
 ------------
 the internal activation energy for diffusion

--------------------------------------------------------------------------------
*/

static double GxActUDiffusion (double activationenergyref, double aromchain)
{
   assert (activationenergyref >= 0.0);
   assert ((aromchain >= 0.0) && (aromchain <= 1.0));

   return (activationenergyref / (1.0 - aromchain));
}



/*
--------------------------------------------------------------------------------
 GxActUReaction - calculate the activation energy for cracking
--------------------------------------------------------------------------------

 GxActUReaction calculates the activation internal energy for the cracking
 reaction at a given transformation ratio.

 Arguments
 ---------
 activationenergy1   - the first activation internal energy
 activationenergy2   - the second activation internal energy
 transformationratio - the transformation ratio

 Return value
 ------------
 the internal activation energy for cracking

--------------------------------------------------------------------------------
*/

static double GxActUReaction (double activationenergy1,
                              double activationenergy2,
                              double transformationratio)
{
   assert (activationenergy1 >= 0.0);
   assert (activationenergy2 >= 0.0);
   assert ((transformationratio >= 0.0) && (transformationratio <= 1.0));

   return (activationenergy1 + transformationratio *
           (activationenergy2 - activationenergy1));
}



/*
--------------------------------------------------------------------------------
 GxAPIGravity - calculates the API gravity from the density
--------------------------------------------------------------------------------

 GxAPIGravity calculates the API gravity as a function of the density passed
 in the argument list. Standard formula.

 Arguments
 ---------
 density - the density from which the API gravity should be calculated

 Return value
 ------------
 the calculated API gravity

--------------------------------------------------------------------------------
*/

double GxAPIGravity (double density)
{
   double   api;

   assert (density > 0.0);

   api = 141.5 / density * 1000.0 - 131.5;
/* 
   Test for negative API, negative API will result in a crash on several
   machines !!!
   In case of a negative API, 0.001 will be used (S.O. Scholten)
*/
   if (api < 0.0) api = 0.001;
   return (api);
}



/*
--------------------------------------------------------------------------------
 GxAromaticity - calculate the aromaticity for a given species
--------------------------------------------------------------------------------

 GxAromaticity calculates the aromaticity of the species passed in the
 argument list.

 Arguments
 ---------
 species - the species for which the aromaticity is to be calculated

 Return value
 ------------
 the aromaticity of the species

--------------------------------------------------------------------------------
*/

double GxAromaticity (const GxSpecies *species)
{
   double  aromaticity, correctedhc, x, a, b, c;

   assert (species);

   if (!GxIsHydrocarbon (species)) {
      aromaticity = 0.0;
   } else {

/*
 *--- quadratic for fa given corrected hc
 *--- my correl of Van Krevelen's model params for coal,
 *--- Table XVI, 6, p. 323
 */
      correctedhc = GxCorrectedHC (species->composition);
      x = 6.9;
      a = 5.2;
      b = - (x + a * (2.0 - correctedhc));
      c = x * (2.0 - correctedhc) - 1.0;
      aromaticity = (-b - sqrt (b * b - 4.0 * a * c)) / (2.0 * a);
      aromaticity = MAX (0.0, MIN (1.0, aromaticity));
   }
   return (aromaticity);
}



/*
--------------------------------------------------------------------------------
 GxAromaticityDependence - calculate the dependence of diffusion on aromaticity
--------------------------------------------------------------------------------

 GxAromaticityDependence calculates the dependence of diffusivity on
 aromaticity of solid orgainic matter waso.
 cf. Bitter

 Arguments
 ---------
 waso -
 wbo  -

 Return value
 ------------
 the aromaticity dependence

--------------------------------------------------------------------------------
*/

static double GxAromaticityDependence (double waso, double wbo)
{
   assert ((waso >= 0.0) && (waso <= 1.0));
   assert ((wbo >= 0.0) && (wbo <= 1.0));

   return (wbo * (1.0 - waso) * (1.0 - waso));
}



/*
--------------------------------------------------------------------------------
 GxAromaticityOfChains - calculate the aromaticity of chains for a species
--------------------------------------------------------------------------------

 GxAromaticityOfChains calculates the aromaticity of aliphatic chains in
 asphaltene, if present, otherwise in kerogen.

 Arguments
 ---------
 nspecies      - number of species to be modeled
 species       - data for each of the species
 stoichiometry - the stoichiometric factors for all reactions

 Return value
 ------------
 the aromaticity of chains in the species

--------------------------------------------------------------------------------
*/

double GxAromaticityOfChains (int nspecies, const GxSpecies *species,
                              const double **stoichiometry)
{
   int     speciesnr, i;
   double  aromaticpart=0.0, totalmobilehc=0.0;

   assert (nspecies > 0);
   assert (species);
   assert (stoichiometry);

   if (GxIsAsphaltene (&species[GX_ASPHALTENE])) {
      speciesnr = GX_ASPHALTENE;
   } else {
      speciesnr = GX_KEROGEN;
   }

   for (i=speciesnr+1; i<nspecies; i++) {
      if (species[i].mobile && GxIsHydrocarbon (&species[i])) {
         totalmobilehc += stoichiometry[speciesnr][i];
         if (species[i].aromaticity > 0.1)
            aromaticpart +=
               species[i].aromaticity * stoichiometry[speciesnr][i];
      }
   }
   return (aromaticpart / totalmobilehc);
}



/*
--------------------------------------------------------------------------------
 GxArrheniusRateFactor - calculate the Arrhenius rate factor
--------------------------------------------------------------------------------

 GxArrheniusRateFactor calculates the Arrhenius rate factor for an activated
 process of a particular species.

 Arguments
 ---------
 internalenergy - the internal energy
 volume         - the activation volume for the process
 entropy        - the activation entropy for the process
 temperature    - the temperature in Kelvin
 pressure       - the pressure
 timeunitfactor - the number of seconds in a time unit

 Return value
 ------------
 the Arrhenius rate factor

--------------------------------------------------------------------------------
*/

static double GxArrheniusRateFactor (double internalenergy, double volume,
                                     double entropy, double temperature,
                                     double pressure, double timeunitfactor)
{
   double  freqf, RT;

   assert (volume >= 0.0);
   assert (temperature >= 0.0);
   assert (pressure >= 0.0);
   assert (timeunitfactor > 0.0);

   RT    = GXC_UNIVERSALGAS * temperature;
   freqf = RT / (GXC_AVOGADRO * GXC_PLANCK) * timeunitfactor;
   return (freqf * exp (entropy / GXC_UNIVERSALGAS -
                        (internalenergy + pressure * volume) / RT));
}



/*
--------------------------------------------------------------------------------
 GxAsphalteneComp - determine the asphaltene composition from kerogen
--------------------------------------------------------------------------------

 GxAsphalteneComp determines the composition of asphaltene from that of
 kerogen using a simple relation described in e.g. Durand-Souron 82.

 Arguments
 ---------
 nelements      - number of elements to be modeled
 kerogencomp    - the composition of kerogen
 asphaltenecomp - the composition of asphaltene

--------------------------------------------------------------------------------
*/

void GxAsphalteneComp (int nelements, const double *kerogencomp,
                       double *asphaltenecomp)
{
   assert ((nelements > 0) && (nelements <= GXN_ELEMENTS));
   assert (kerogencomp);
   assert (asphaltenecomp);

   asphaltenecomp[0] = 1.0;
   if (nelements > 1) asphaltenecomp[1] = 0.95 * kerogencomp[1];
   if (nelements > 2) asphaltenecomp[2] = 0.7 * kerogencomp[2] - 0.03;
   if (nelements > 3) asphaltenecomp[3] = 0.009 + 0.3 * kerogencomp[3];
}



/*
--------------------------------------------------------------------------------
 GxAsphalteneTransf - calculate the transformation ratio of aspahltene
--------------------------------------------------------------------------------

 GxAsphalteneTransf calculates the transformation ratio of asphaltene.

 Arguments
 ---------
 initialkerogenconc    - the initial concentration of kerogen
 currentkerogenconc    - the current concentration of kerogen
 currentasphalteneconc - the current concentration of asphaltene
 stoichfactor          - the stoich factor between kerogen and asphaltene

 Return value
 ------------
 the transformation ratio of the asphaltene

--------------------------------------------------------------------------------
*/

double GxAsphalteneTransf (double initialkerogenconc,
                           double currentkerogenconc,
                           double currentasphalteneconc,
                           double stoichfactor)
{
   double  transformation;

   assert (initialkerogenconc >= 0.0);
   assert ((currentkerogenconc >= 0.0) && (currentkerogenconc <= initialkerogenconc));
   assert (currentasphalteneconc >= 0.0);
   assert (stoichfactor >= 0.0);

   transformation = 1.0 - (currentkerogenconc + currentasphalteneconc /
                    stoichfactor) / initialkerogenconc;
   return (MAX (0.0, transformation));
}



/*
--------------------------------------------------------------------------------
 GxAtomicRatios - calculate atomic ratios for immobile species
--------------------------------------------------------------------------------

 GxAtomicRatios calculates the atomic ratios (H/C, O/C) of the immobile
 species.

 Arguments
 ---------
 nspecies      - number of species to be modeled
 species       - the species data
 concentration - concentration for each of the species
 atomiccptr    - pointer to variable for atomic carbon
 hovercptr     - pointer to variable for h/c
 oovercptr     - pointer to variable for o/c

--------------------------------------------------------------------------------
*/

void GxAtomicRatios (int nspecies, GxSpecies *species, double *concentration,
                     double *atomiccptr, double *hovercptr, double *oovercptr)
{
   double  atomicc, atomich, atomico, cmol;
   int     i;

   assert (nspecies > 0);
   assert (species);
   assert (concentration);
   assert (atomiccptr);
   assert (hovercptr);
   assert (oovercptr);

   atomicc = 0.0;
   atomich = 0.0;
   atomico = 0.0;

   for (i=0; i<nspecies; i++) {
      cmol     = concentration[i] / species[i].formulaweight;
      atomicc += cmol * species[i].composition[GX_CARBON];
      atomich += cmol * species[i].composition[GX_HYDROGEN];
      atomico += cmol * species[i].composition[GX_OXYGEN];
   }
   *atomiccptr = atomicc;
   *hovercptr  = (atomicc > FLT_MIN) ? atomich / atomicc : 0.0;
   *oovercptr  = (atomicc > FLT_MIN) ? atomico / atomicc : 0.0;
}



/*
--------------------------------------------------------------------------------
 GxBubblePointGOR - calculate GOR at bubble point
--------------------------------------------------------------------------------

 GxBubblePointGOR uses Glaso's correlation based on North Sea oils for
 producing GOR at bubble point (sat) pressure and temperature.

 Arguments
 ---------
 oildensity  - the density of oil
 gasdensity  - the density of gas
 temperature - the bubble point temperature
 pressure    - the bubble point pressure

 Return value
 ------------
 the GOR at bubble point

--------------------------------------------------------------------------------
*/

double GxBubblePointGOR (double oildensity, double gasdensity,
                         double temperature, double pressure)
{
   double  c, pbs, api, gorsf;

   assert (oildensity > 0.0);
   assert (gasdensity > 0.0);
   assert (temperature >= 0.0);
   assert (pressure > 0.0);

   c     = -0.394581 - log10 (pressure / 1000000.0);
   pbs   =  GxLocalPow (10.0, GxSolveQuadratic (-0.30218, 1.7447, c));
   api   = GxAPIGravity (oildensity);
   gorsf = (gasdensity / 1.223) * GxLocalPow (pbs, 1.2255) * 
	   GxLocalPow (api, 1.212) /
           GxLocalPow ((temperature - GXC_ZEROCELCIUS) * 1.8 + 32.0, 0.2107);
   return (gorsf / GXC_GORCONVERSION);
}


/*
--------------------------------------------------------------------------------
 GxCalcFreeGas - Calculate the gas volume minus gas in water
--------------------------------------------------------------------------------

 GxCalcFreeGas calculates the amount of gas which is solved in water.
 The volume is subtracted from the total volume.

 Arguments
 ---------
 oilvolume     - total volume of oil
 oildensity    - density of the oil
 gasvolume     - total volume of gas
 gasdensity    - density of the gas
 watervolume   - volume of water in reservoir (trap crest)
 temperature   - Trap temperature
 salinity      - Guess what
 pressure      - Trap pressure

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/


double GxCalcFreeGas(double oilvolume, double oildensity, double gasvolume,
                     double gasdensity, double watervolume,
                     double temperature, double salinity,
                     double pressure)
{
    double saturation, gasinwater;

    saturation = GxBubblePointGOR (oildensity, gasdensity, temperature,
                                                              pressure);
    if (saturation > 0.0) {
        gasinwater = GxGasInWater (saturation, pressure, salinity, gasvolume,
                                                     oilvolume, watervolume);
    } else {
 	gasinwater = 0.0;
    }

    return (gasvolume - gasinwater);
}


/*
--------------------------------------------------------------------------------
 GxCalcHCGasInWaterSaturation - calculate HC Gas in water saturation
--------------------------------------------------------------------------------
 
 Calculates the HC Gas in water saturation given the salinity, the pressure 
 and the volume of water in sms

 Arguments
 ---------
 salinity    - the salinity of water in sms
 pressure    - the pressure within in sms
 watervolume - the volume of water in sms

 Return value
 ------------
 the saturation of HC Gas in water

--------------------------------------------------------------------------------
*/

static double GxCalcHCGasWaterSaturation ( double salinity, double pressure,
                                           double watervolume )
{
   double gwsat;

   assert (pressure >= 0.0);
   assert ((salinity >= 0.0) && (salinity <= 1.0));
   assert (watervolume >= 0.0);
   
   if (salinity < 0.28) {
      gwsat = (1.05 + 0.08 * pressure / 1000000.0) *
              (1.0 - salinity / 0.28) * watervolume;
   } else {
      gwsat = 0.0;
   }
   return gwsat;
}


/*
--------------------------------------------------------------------------------
 GxCalcInitialConc - calculate the initial concentrations
--------------------------------------------------------------------------------

 GxCalcInitialConc calculates the initial concentration for each of the
 species from the source rock properties.

 Arguments
 ---------
 nspecies      - number of species to be modeled
 species       - data for each of the species
 initialtoc    - initial toc of source rock (wt C/rock)
 inorganicdens - the density of inorganic matter
 concentration - concentrations for each of the species

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCalcInitialConc (int nspecies, GxSpecies *species,
                               double initialtoc, double inorganicdens,
                               double *concentration)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i;
   double       wori, vio, vori;

   if ((nspecies <= 0) || !species || (initialtoc < 0.0)
      || (initialtoc > 1.0) || (inorganicdens <= 0.0) || !concentration) {
/*
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
*/
   } else {
      for (i=0; i<nspecies; i++) concentration[i] = 0.0;
      wori = species[GX_KEROGEN].formulaweight * initialtoc /
             atomicWeight[GX_CARBON];
      vio  = species[GX_KEROGEN].density / inorganicdens * (1.0 / wori - 1.0);
      vori = 1.0 / (vio + 1.0);
      concentration[GX_KEROGEN] = species[GX_KEROGEN].density * vori;
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxCalcIsoFactorConst - Calculate the constants for calculating the isotope
                        fractionation factor aplha 
--------------------------------------------------------------------------------
 
 GxCalcIsoFactorConst calculates the constants for calculating the isotope
 fractionation factor aplha

 Arguments
 ---------
 isofactor      - isotope fractionation factor data

 Return value
 ------------
 none

--------------------------------------------------------------------------------
*/
void GxCalcIsoFactorConst(
   GxIsoFactor *isofactor
)
{
   double highvalue;

   assert (isofactor);
   assert (isofactor->templow != isofactor->temphigh);
    
   highvalue = 1.0 + (isofactor->alphalow - 1.0) * (isofactor->highpercentage /
                                                   100.0);  
 
   isofactor->constB = (isofactor->alphalow - highvalue) /
                       (isofactor->templow - isofactor->temphigh);

   isofactor->constA = highvalue - isofactor->constB * isofactor->temphigh; 
}


/*
--------------------------------------------------------------------------------
 GxCalcIsoFractFactor - Calculate the isotope fractionation factor alpha 
--------------------------------------------------------------------------------
 
 GxCalcIsoFractFactor calculates teh isotope fractionation factor alpha using
 the following equation: alpha = a+b*temperature.
 Calculated fractionation factor must be between alpha low and 1.0.

 Arguments
 ---------
 temperature    - the current temperature 
 isofactor      - isotope fractionation factor data

 Return value
 ------------
 calculated fractionation factor

--------------------------------------------------------------------------------
*/
static double GxCalcIsoFractFactor(
   double       temperature,
   GxIsoFactor *isofactor
)
{
   double fractfactor;

   assert (temperature > GXC_ZEROCELCIUS);
   assert (isofactor);

   fractfactor = isofactor->constA + isofactor->constB * 
                 (temperature - GXC_ZEROCELCIUS);
   
   if ((isofactor->alphalow >= 1.0 && fractfactor > isofactor->alphalow) ||
       (isofactor->alphalow < 1.0 && fractfactor < isofactor->alphalow)) {
      fractfactor = isofactor->alphalow;
   } 

   if ((isofactor->alphalow >= 1.0 && fractfactor < 1.0) ||
       (isofactor->alphalow < 1.0 && fractfactor > 1.0)) {
      fractfactor = 1.0;
   } 

   return fractfactor;
}


/*
--------------------------------------------------------------------------------
 GxCalcIsotopeFractCn - Calculate the isotope fractionation coefficient 
--------------------------------------------------------------------------------
 
 GxCalcIsotopeFractC1 calculates the isotope fractionation using the RAYLEIGH 
 formula for Cn 

 Arguments
 ---------
 speciesnr      - number of species for which the isotope fractionation must be
                  calculated 
 convasphaltene - asphaltene conversiona
 initialSRfract - Initial fractionation coefficient for the source rock
 histfractasph  - Fractionation coefficient of asphaltene one time step earlier
 temperature    - current temperature 
 isofactor      - isotope fractionation factor data

 Return value
 ------------
 calculated fractionation coefficient

--------------------------------------------------------------------------------
*/
static double GxCalcIsotopeFractCn(
   int          speciesnr,
   double       convasphaltene, 
   double       initialSRfract, 
   double       histfractasph,
   double       temperature,
   GxIsoFactor *isofactor
)
{
   double fractionation = 0.0;
   double fractSR, fractC1;
   int    nrC;
   
   nrC = abs (speciesnr - GX_C1) + 1;
 
   fractSR = GxCalcIsotopeFractSR (convasphaltene, initialSRfract, 
                                   histfractasph, temperature, isofactor);
   fractC1 = GxCalcIsotopeFractC1 (convasphaltene, initialSRfract, 
                                   histfractasph, temperature, isofactor);

   fractionation = (-1.0 / nrC) * (fractSR - fractC1) + fractSR; 

   return fractionation;
}


/*
--------------------------------------------------------------------------------
 GxCalcIsotopeFractC1 - Calculate the isotope fractionation coefficient 
--------------------------------------------------------------------------------
 
 GxCalcIsotopeFractC1 calculates the isotope fractionation using the RAYLEIGH 
 formula for C1 

 Arguments
 ---------
 convasphaltene - asphaltene conversiona
 initialSRfract - Initial fractionation coefficient for the source rock
 histfractasph  - Fractionation coefficient of asphaltene one time step earlier
 temperature    - current temperature 
 isofactor      - isotope fractionation factor data

 Return value
 ------------
 calculated fractionation coefficient

--------------------------------------------------------------------------------
*/
static double GxCalcIsotopeFractC1(
   double       convasphaltene, 
   double       initialSRfract, 
   double       histfractasph,
   double       temperature,
   GxIsoFactor *isofactor
)
{
   double fractionation = 0.0;
   double calcPow;
   double fractfactor;

   fractfactor = GxCalcIsoFractFactor (temperature, isofactor);

   if (convasphaltene < GXC_MAX_CONVERSION) {
      calcPow = pow (1.0 - convasphaltene, ( 1.0 / fractfactor - 1.0 ) ); 
      fractionation = ((1.0 / fractfactor) *
                       (initialSRfract / 1000.0 + 1.0) * calcPow - 1.0) * 
                       1000.0;
   } else {
      fractionation = (1.0 / fractfactor) * histfractasph;
   }

   return fractionation;
}


/*
--------------------------------------------------------------------------------
 GxCalcIsotopeFractSR - Calculate the isotope fractionation coefficient 
--------------------------------------------------------------------------------

 GxCalcIsotopeFractSR calculates the isotope fractionation using the RAYLEIGH 
 formula for source rock 

 Arguments
 ---------
 convasphaltene - asphaltene conversiona
 initialSRfract - Initial fractionation coefficient for the source rock
 histfractasph  - Fractionation coefficient of asphaltene one time step earlier
 temperature    - current temperature 
 isofactor      - isotope fractionation factor data

 Return value
 ------------
 calculated fractionation coefficient

--------------------------------------------------------------------------------
*/
double GxCalcIsotopeFractSR(
   double       convasphaltene, 
   double       initialSRfract, 
   double       histfractasph,
   double       temperature,
   GxIsoFactor *isofactor
)
{
   double fractionation = 0.0;
   double calcPow;
   double fractfactor;

   fractfactor = GxCalcIsoFractFactor (temperature, isofactor);

   if (convasphaltene < GXC_MAX_CONVERSION) {
      calcPow = pow (1.0 - convasphaltene, ( 1.0 / fractfactor - 1.0 ) ); 
      fractionation = ((initialSRfract / 1000.0 + 1.0) * calcPow - 1.0) * 
                      1000.0;
   } else {
      fractionation = histfractasph;
   }

   return fractionation;
}


/*
--------------------------------------------------------------------------------
 GxCalcIsotopeFractionation - calculate the isotope fractionation 
--------------------------------------------------------------------------------

 GxCalcIsotopeFractionation for the specified species using the method 
 specified 

 Arguments
 ---------

 speciesnr      - number of species for which the isotope fractionation must be
                  calculated 
 convasphaltene - asphaltene conversiona
 reactantfract  - fractionation coefficient of reactant 
 initialSRfract - Initial fractionation coefficient for the source rock
 histfractasph  - Fractionation coefficient of asphaltene one time step earlier
 temperature    - current temperature 
 isofactor      - isotope fractionation factor data
 method         - method to be used for calculating the fractionation coeff. 

 Return value
 ------------
 isotope fractionation coefficient

--------------------------------------------------------------------------------
*/
static double GxCalcIsotopeFractionation (
   int               speciesnr,
   double            convasphaltene,
   double            reactantfract,
   double            initialSRfract,
   double            histfractasph,
   double            temperature,
   GxIsoFactor      *isofactor,
   GxIsoFractMethod  method 
)
{
   double fractionation = 0.0;

   switch (method) {
      case GX_ISO_RAYLEIGH_FRAC_KEROGEN:
           fractionation = GxCalcIsotopeFractSR (convasphaltene, 
                                                 initialSRfract, histfractasph,
                                                 temperature, isofactor);  
           break;
      case GX_ISO_RAYLEIGH_FRAC_C1:
           fractionation = GxCalcIsotopeFractC1 (convasphaltene, 
                                                 initialSRfract, histfractasph,
                                                 temperature, isofactor);  
           break;
      case GX_ISO_RAYLEIGH_FRAC_C2C5:
           fractionation = GxCalcIsotopeFractCn (speciesnr, convasphaltene, 
                                                 initialSRfract, histfractasph,
                                                 temperature, isofactor);  
           break;
      case GX_ISO_FRAC_HISTORY:
           fractionation = reactantfract;
           break;
   }

   return fractionation;
}


/*
--------------------------------------------------------------------------------
 GxCalcRatios - calculate the H/C, O/C and N/C 
--------------------------------------------------------------------------------

 GxCalcRatios calculates the H/C, O/C and N/C for a particular concentration
 history using the given species

 Arguments
 ---------

 species       - data structure of given species
 size          - size of the history
 concentration - concentration history
 atomich       - array for atomic H values of specified species
 atomicc       - array for atomic C values of specified species
 atomico       - array for atomic O values of specified species
 atomicn       - array for atomic N values of specified species

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/
void GxCalcRatios (GxSpecies *species, int size, double *concentration, 
                   double *atomich, double *atomicc, double *atomico,
                   double *atomicn )
{
   double cmol;
   int    i;

   assert (species);
   assert (size > 0);
   assert (concentration);
   assert (atomich);
   assert (atomicc);
   assert (atomico);
   assert (atomicn);

   for ( i = 0; i < size; i++ ) {
      cmol = concentration[i] / species->formulaweight;

      atomicc[i] += cmol * species->composition[GX_CARBON];
      atomich[i] += cmol * species->composition[GX_HYDROGEN];
      atomico[i] += cmol * species->composition[GX_OXYGEN];
      if (GxNitrogenPartOfModel () ) {
          atomicn[i] += cmol * species->composition[GX_NITROGEN];
      }
   }
}


/*
--------------------------------------------------------------------------------
 GxCalcSpecies - calculate the variable data in the species data
--------------------------------------------------------------------------------

 GxCalcSpecies calculates the data that is variable in the species data. These
 are the numbers that are influenced by the input from the user.

 Arguments
 ---------
 nelements         - number of elements to be modeled
 nspecies          - number of species in model
 species           - data for each of the species
 kerogenh          - number of hydrogen atoms for kerogen
 kerogeno          - number of oxygen atoms for kerogen
 kerogenlowact     - lower limit on activation energy for kerogen cracking
 kerogenhighact    - upper limit on activation energy for kerogen cracking
 asphaltenelowact  - lower limit on activation energy for asphaltene cracking
 asphaltenehighact - upper limit on activation energy for asphaltene cracking

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCalcSpecies (int nelements, int nspecies, GxSpecies *species,
                           double kerogenh, double kerogeno, double kerogenn,
                           double kerogenlowact, double kerogenhighact,
                           double asphaltenelowact, double asphaltenehighact)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i;

   if ((nelements <= 0) || (nelements > GXN_ELEMENTS) || (nspecies <= 0)
      || !species || (kerogenh < 0.0) || (kerogeno < 0.0)
      || (kerogenlowact < 0.0) || (kerogenhighact < 0.0)
      || (asphaltenelowact < 0.0) || (asphaltenehighact < 0.0)) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Store parameters provided in the argument list */
      species[GX_KEROGEN].composition[GX_CARBON]   = GX_DEFAULTKEROGENC;
      species[GX_KEROGEN].composition[GX_HYDROGEN] = kerogenh;
      species[GX_KEROGEN].composition[GX_OXYGEN]   = kerogeno;
      if (GxNitrogenPartOfModel () ) {
          species[GX_KEROGEN].composition[GX_NITROGEN] = kerogenn;
      }
      species[GX_KEROGEN].crackingenergy1          = kerogenlowact;
      species[GX_KEROGEN].crackingenergy2          = kerogenhighact;
      if (GxIsAsphaltene (&species[GX_ASPHALTENE])) {
         species[GX_ASPHALTENE].crackingenergy1    = asphaltenelowact;
         species[GX_ASPHALTENE].crackingenergy2    = asphaltenehighact;

/* ----- Determine composition of asphaltene from that of kerogen */
         GxAsphalteneComp (nelements, species[GX_KEROGEN].composition,
                           species[GX_ASPHALTENE].composition);
      }

/* -- Calculate additional data for each of the species */
      for (i=0; i<nspecies; i++) {
         species[i].formulaweight = GxFormulaWeight (nelements,
                                                     species[i].composition);
         species[i].aromaticity = GxAromaticity (&species[i]);
         if (!species[i].mobile
            || ((i == GX_ASPHALTENE))
               && GxIsAsphaltene (&species[GX_ASPHALTENE])) {
            species[i].density = GxImmobileDensity (&species[i]);
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCalcStoichiometry - calculate the stoichiometry for a reaction scheme
--------------------------------------------------------------------------------

 GxCalcStoichiometry calculates the stoichiometry for a particular reaction
 scheme using the reaction and species data.

 Arguments
 ---------
 nelements     - number of elements to be modeled
 nspecies      - number of species in model
 species       - data for each of the species
 reaction      - data for each of the reactions
 stoichiometry - stoichiometric factors for the reaction scheme

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxCalcStoichiometry (int nelements, int nspecies,
                                 const GxSpecies *species,
                                 const GxReactionScheme *reactionscheme,
                                 double **stoichiometry)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, j, k, ieq, ncomponents;
   double       **equation, *solution;

   if ((nelements <= 0) || (nelements > GXN_ELEMENTS) || (nspecies <= 0)
      || !species || !reactionscheme || !stoichiometry) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {

/* -- Check the number of available reactions against number of unknowns */
      for (i=0; (!rc)&&(i<reactionscheme->nreactions); i++) {
         ncomponents = 0;
         for (j=0; j<nelements; j++)
            if (species[i].composition[j] > 0.0) ncomponents++;
         if ((ncomponents+reactionscheme->nratios[i])
            != reactionscheme->nproducts[i]){
#ifdef CONF_TEST
            printf ("Reaction: %d, components: %d, ratios: %d, products: %d\n",
                     i + 1, ncomponents, reactionscheme->nratios[i],
                     reactionscheme->nproducts[i]);
#endif
            rc = GxError (GXE_INVREACTIONSCHEME, GXT_INVREACTIONSCHEME);
            }
      }

      if (!rc) {

/* ----- Initialize stoichiometry array */
         for (i=0; i<reactionscheme->nreactions; i++)
            for (j=0; j<nspecies; j++)
               stoichiometry[i][j] = 0.0;

/* ----- Allocate necessary temporary arrays */
         rc = GxAllocArray (&equation, sizeof (double), 2, nspecies,
                            nspecies+1);
         if (!rc) rc = GxAllocArray (&solution, sizeof (double), 1, nspecies);
         if (!rc) {

/* -------- Solve the equations separately for each reaction */
            for (i=0; i<reactionscheme->nreactions; i++) {
               ieq = 0;

/* ----------- Set up the linear equations to be solved for the reaction:
 * -----------  A) The equations balancing the number of atoms of each
 * -----------     element
 */
               for (j=0; j<nelements; j++) {
                  if (species[i].composition[j] > 0.0) {
                     for (k=0; k<reactionscheme->nproducts[i]; k++)
                        equation[ieq][k] =
                        species[reactionscheme->product[i][k]].composition[j];
                     equation[ieq][reactionscheme->nproducts[i]] =
                        species[i].composition[j];
                     ieq++;
                  }
               }

/* -----------  B) The equations specified in the reaction scheme as ratios
 * -----------     between the stoichiometric factors. These are specified
 * -----------     as a number or, in the case of asphaltene, possibly as
 * -----------     an automatically calculated ratio.
 */
               for (j=0; j<reactionscheme->nratios[i]; j++) {
                  for (k=0; k<=reactionscheme->nproducts[i]; k++)
                     equation[ieq][k] = 0.0;
                  equation[ieq][reactionscheme->ratio[i][j].product1] = 1.0;
                  if (reactionscheme->ratio[i][j].specified) {

/* ----------------- Specified ratio */
                     equation[ieq][reactionscheme->ratio[i][j].product2] =
                        -reactionscheme->ratio[i][j].ratio;
                  } else {

/* ----------------- Calculated ratio */
                     equation[ieq][reactionscheme->ratio[i][j].product2] =
                        species[GX_KEROGEN].aromaticity /
                        (species[GX_KEROGEN].aromaticity - 1.0);
                  }
                  ieq++;
               }

/* ----------- Solve the set of linear equations for this reaction */
               GxSolveLinearEquations (reactionscheme->nproducts[i], equation,
                                       solution);

/* ----------- Convert the calculated molar stoich factors to mass factors */
               for (j=0; (!rc)&&(j<reactionscheme->nproducts[i]); j++) {
                  stoichiometry[i][reactionscheme->product[i][j]] =
                     GxMassStoichFactor (&species[i],
                                      &species[reactionscheme->product[i][j]],
                                      solution[j]);
#ifdef CONF_TEST
                  printf ("Stoichiometry factor for %s in reaction %d is: %f\n",
			   species[reactionscheme->product[i][j]].name, 
                           i + 1, 
			   stoichiometry[i][reactionscheme->product[i][j]]); 
#endif
	          if ((strcmp (species[reactionscheme->product[i][j]].name, 
                              "H2O") != 0)
		      && 
                      (strcmp (species[reactionscheme->product[i][j]].name, 
                               "COx") != 0)
                      &&
                      ((i != GX_KEROGEN) || 
                       (strcmp (species[reactionscheme->product[i][j]].name,
                                "NH4") != 0)))
		  {
                     if (stoichiometry[i][reactionscheme->product[i][j]]<0.0) {
                        rc = GxError (GXE_INVREACTIONSCHEME,
                                                  GXT_INVREACTIONSCHEME);
		     }
		  }
               }
            }

/* -------- Free the temporary arrays */
            GxFreeArray (solution, 1, nspecies);
            GxFreeArray (equation, 2, nspecies, nspecies+1);
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxCalcVanKrevelen - calculate the H/C and O/C for Van Krevelen
--------------------------------------------------------------------------------

 GxCalcVanKrevelen calculates the H/C and O/C for a particular concentration
 history using the given species

 Arguments
 ---------

 species       - data structure of given species
 size          - size of the history
 concentration - concentration history
 hc            - array for H/C
 oc            - array for O/C

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/
void GxCalcVanKrevelen (GxSpecies *species, int size,
                        double *concentration, double *atomich, double *atomicc,
                        double *atomico )
{
   double cmol;
   int    i;

   assert (species);
   assert (size > 0);
   assert (concentration);
   assert (atomich);
   assert (atomicc);
   assert (atomico);

   for ( i = 0; i < size; i++ ) {
      cmol = concentration[i] / species->formulaweight;

      atomicc[i] += cmol * species->composition[GX_CARBON];
      atomich[i] += cmol * species->composition[GX_HYDROGEN];
      atomico[i] += cmol * species->composition[GX_OXYGEN];

  }
}


/*
--------------------------------------------------------------------------------
 GxConcFactor - calculate the concentration factor in diffusion coefficient
--------------------------------------------------------------------------------

 GxConcFactor calculates the concentration factor in the diffusion coefficient
 for a particular species at the boundary of two slices.

 Arguments
 ---------
 upperlump - lumped species concentrations in the upper slice
 lowerlump - lumped species concentrations in the lower slice

 Return value
 ------------
 the concentration factor

--------------------------------------------------------------------------------
*/

static double GxConcFactor (const double *upperlump, const double *lowerlump)
{
   double  conctotalom, waso, wbo;

   assert (upperlump);
   assert (lowerlump);

   conctotalom = upperlump[GX_TOTALOM] + lowerlump[GX_TOTALOM];
   waso = (upperlump[GX_SOLID] + lowerlump[GX_SOLID]) / conctotalom;
   wbo  = (upperlump[GX_BITUMEN] + lowerlump[GX_BITUMEN]) / conctotalom;
   return (GxAromaticityDependence (waso, wbo));
}



/*
--------------------------------------------------------------------------------
 GxConcLumped - calculate the concentration for lumped species
--------------------------------------------------------------------------------

 GxConcLumped calculates the concentrations for the lumped species.

 Arguments
 ---------
 nspecies       - the number of species to be modeled
 nslices        - number of slices to be modeled
 species        - the species data
 concentration  - the concentration for each of the species
 lumpedconc     - concentrations for the lumped species

--------------------------------------------------------------------------------
*/

void GxConcLumped (int nspecies, int nslices, const GxSpecies *species,
                   const double **concentration, double **conclumped)
{
   int  i, j;

   assert (nspecies > 0);
   assert (nslices > 0);
   assert (concentration);
   assert (conclumped);

   for (i=0; i<nslices; i++) {

/* -- Intialize the concentrations for the lumped species */
      conclumped[i][GX_TOTALOM]  = 0.0;
      conclumped[i][GX_BITUMEN]  = 0.0;
      conclumped[i][GX_SOLID]    = 0.0;

/* -- Loop over all species */
      for (j=0; j<nspecies; j++) {

/* ----- Add concentration to concentration of total organic matter */
         conclumped[i][GX_TOTALOM] += concentration[i][j];

/* ----- Add concentration to bitumen or solid matter concentration */
         if (species[j].mobile) {
            conclumped[i][GX_BITUMEN] += concentration[i][j];
         } else {
            conclumped[i][GX_SOLID] +=
               species[j].aromaticity * concentration[i][j];
         }
      }
   }
}



/*
--------------------------------------------------------------------------------
 GxConvMassTransfer - calculate the convective mass transfer coefficient
--------------------------------------------------------------------------------

 GxConvMassTransfer calculates the convective mass transfer coefficient at
 expulsion surface.

 Arguments
 ---------
 biotnumber - Biot number for the expulsion surface
 refdiff    - reference diffusion coefficient
 charlength - characteristic length

 Return value
 ------------
 the convective mass transfer coefficient

--------------------------------------------------------------------------------
*/

double GxConvMassTransfer (double biotnumber, double refdiff,
                           double charlength)
{
   assert (charlength > 0.0);

   return (2.0 * biotnumber * refdiff / charlength);
}



/*
--------------------------------------------------------------------------------
 GxCorrectedHC - correct the h/c ratio for o-groups
--------------------------------------------------------------------------------

 GxCorrectedHC calculates the h/c ratio roughly corrected for o-groups
 (n & s neglected). Van Krevelen, Table XVI, 4, p.321.

 Arguments
 ---------
 composition - composition for which the corrected h/c should be calculated

 Return value
 ------------
 the corrected h/c ratio

--------------------------------------------------------------------------------
*/

static double GxCorrectedHC (const double *composition)
{
   assert (composition);

   return (composition [GX_HYDROGEN] / composition [GX_CARBON] +
           (composition [GX_OXYGEN] / composition [GX_CARBON] ) / 2.0);
}



/*
--------------------------------------------------------------------------------
 GxCrackingRate - calculate the cracking rate
--------------------------------------------------------------------------------

 GxCrackingRate calculates the rate of cracking of a particular species,
 using its properties and the physical conditions.

 Arguments
 ---------
 species             - species for which the cracking rate should be calculated
 temperature         - the temperature in Kelvin
 pressure            - the pressure in Pascal
 timeunitfactor      - the number of seconds in a time unit
 transformationratio - the transformation ratio to be used for the rate

 Return value
 ------------
 the cracking rate

--------------------------------------------------------------------------------
*/

double GxCrackingRate (const GxSpecies *species, double temperature,
                       double pressure, double timeunitfactor,
                       double transformationratio)
{
   double  internalenergy, rate;

   assert (species);
   assert (temperature >= 0.0);
   assert (pressure >= 0.0);
   assert (timeunitfactor > 0.0);
   assert ((transformationratio >= 0.0) && (transformationratio <= 1.0));

   if (species->reactive) {
      internalenergy = GxActUReaction (species->crackingenergy1,
                                       species->crackingenergy2,
                                       transformationratio);
      rate = GxArrheniusRateFactor (internalenergy, species->crackingvolume,
                                    species->crackingentropy, temperature,
                                    pressure, timeunitfactor);
   } else {
      rate = 0.0;
   }
   return (rate);
}



/*
--------------------------------------------------------------------------------
 GxDepth - estimate the depth from the temperature
--------------------------------------------------------------------------------

 GxDepth very roughly estimates the depth from the temperature.

 Arguments
 ---------
 temperature  - the temperature from which the depth should be estimated
 tempgradient - the temperature gradient
 surfacetemp  - the surface temperature

 Return value
 ------------
 the estimated depth

--------------------------------------------------------------------------------
*/

double GxDepth (double temperature, double tempgradient, double surfacetemp)
{
   assert (temperature >= 0.0);
   assert (tempgradient > 0.0);
   assert (surfacetemp >= 0.0);

   return (MAX (0.0, (temperature - surfacetemp) / tempgradient));
}



/*
--------------------------------------------------------------------------------
 GxDetermineFieldType - determines the type of the field
--------------------------------------------------------------------------------

 GxDetermineFieldType determines the type of the field from calculated
 properties as: the oil and gas charge, the volume of the water, and the
 gas oil ratio.

 Arguments
 ---------
 oilvolume     - the volume of the oil in the field
 gasvolume     - the volume of the gas in the field
 watervolume   - the volume of the water in the field
 gor           - the gas oil ratio
 saturationgor - the saturation gas oil ratio
 saturationcgr - the saturation condensate gas ratio

 Return value
 ------------
 the type of the field

--------------------------------------------------------------------------------
*/

GxFieldType GxDetermineFieldType (double oilvolume, double gasvolume,
                                  double watervolume, double gor,
                                  double saturationgor, double saturationcgr)
{
   GxFieldType  fieldtype;

   assert (oilvolume >= 0.0);
   assert (gasvolume >= 0.0);
   assert (watervolume >= 0.0);
   assert (gor >= 0.0);
   assert (saturationgor >= 0.0);
   assert (saturationcgr >= 0.0);

   if ((oilvolume < watervolume * 0.00001)
      && (gasvolume < watervolume * 0.01)) {
      fieldtype = GXF_DRYHOLE;
   } else if ((oilvolume < watervolume * 0.0001) && (gor < 1000.0)) {
      fieldtype = GXF_OILSHOWS;
   } else if ((gasvolume < watervolume) && (gor > 1000.0)) {
      fieldtype = GXF_GASSHOWS;
   } else if (gor < saturationgor) {
      fieldtype = GXF_OILFIELD;
   } else if (gor < 1.0 / saturationcgr) {
      fieldtype = GXF_OILGASFIELD;
   } else {
      fieldtype = GXF_GASFIELD;
   }
   return (fieldtype);
}



/*
--------------------------------------------------------------------------------
 GxDiffusionEntropy - estimate the diffusion entropy from diffusion energy
--------------------------------------------------------------------------------

 GxDiffusionEntropy estimates the diffusion entropy for a species from the
 diffusion energy for that species.

 Arguments
 ---------
 diffusionenergy - the diffusion energy for the species
 expansivity     - the expansivity

 Return value
 ------------
 estimate for the diffusion entropy for the species

--------------------------------------------------------------------------------
*/

static double GxDiffusionEntropy (double diffusionenergy, double expansivity)
{
   assert (diffusionenergy >= 0.0);
   assert (expansivity >= 0.0);

   return (4.0 * expansivity * diffusionenergy);
}



/*
--------------------------------------------------------------------------------
 GxDiffusionRate - calculate the Arrhenius rate factor for diffusion
--------------------------------------------------------------------------------

 GxDiffusionRate calculates the Arrhenius rate factor for diffusion.

 Arguments
 ---------
 species         - species for which the diffusion rate should be calculated
 aromchain       - the aromaticity of aliphatic chains
 temperature     - the temperature in Kelvin
 pressure        - the pressure in Pascal
 expansivity     - the expansivity
 compressibility - the compressibility
 timeunitfactor  - the number of seconds in a time unit

 Return value
 ------------
 the Arrhenius rate factor for diffusion

--------------------------------------------------------------------------------
*/

static double GxDiffusionRate (const GxSpecies *species, double aromchain,
                               double temperature, double pressure,
                               double expansivity, double compressibility,
                               double timeunitfactor)
{
   double  internalenergy, entropy, volume, rate;

   assert (species);
   assert ((aromchain >= 0.0) && (aromchain <= 1.0));
   assert (temperature >= 0.0);
   assert (pressure >= 0.0);
   assert (expansivity >= 0.0);
   assert (compressibility >= 0.0);
   assert (timeunitfactor > 0.0);

   if (species->mobile) {
      internalenergy = GxActUDiffusion (species->diffusionenergy, aromchain);
      entropy = GxDiffusionEntropy (internalenergy, expansivity);
      volume  = GxDiffusionVolume (internalenergy, compressibility);
      rate    = GxArrheniusRateFactor (internalenergy, volume, entropy,
                                       temperature, pressure, timeunitfactor);
   } else {
      rate = 0.0;
   }
   return (rate);
}



/*
--------------------------------------------------------------------------------
 GxDiffusionVolume - estimate the diffusion volume from diffusion energy
--------------------------------------------------------------------------------

 GxDiffusionVolume estimates the diffusion volume for a species from the
 diffusion energy for that species.

 Arguments
 ---------
 diffusionenergy - the diffusion energy for the species
 compressibility - the compressibility

 Return value
 ------------
 estimate for the diffusion volume for the species

--------------------------------------------------------------------------------
*/

static double GxDiffusionVolume (double diffusionenergy,
                                 double compressibility)
{
   assert (diffusionenergy >= 0.0);
   assert (compressibility >= 0.0);

   return (4.0 * compressibility * diffusionenergy);
}



/*
--------------------------------------------------------------------------------
 GxFailPressure - calculate the failure pressure
--------------------------------------------------------------------------------

 GxFailPressure calculates a rough approximation of the failure pressure of
 the overburden.

 Arguments
 ---------
 failurecriterion    - failure criterion
 lithostaticpressure - the lithostatic pressure

 Return value
 ------------
 the calculated failure pressure

--------------------------------------------------------------------------------
*/

double GxFailPressure (double failurecriterion, double lithostaticpressure)
{
   assert (failurecriterion >= 0.0);
   assert (lithostaticpressure >= 0.0);

   return (failurecriterion * lithostaticpressure);
}



/*
--------------------------------------------------------------------------------
 GxFormulaWeight - calculate the formula weight for a species composition
--------------------------------------------------------------------------------

 GxFormulaWeight calculates the formula weight of a species given its
 composition.

 Arguments
 ---------
 nelements   - number of elements to be modeled
 composition - the composition of the species

 Return value
 ------------
 the formula weight

--------------------------------------------------------------------------------
*/

double GxFormulaWeight (int nelements, const double *composition)
{
   double  weight=0;
   int     i;

   assert ((nelements > 0) && (nelements <= GXN_ELEMENTS));
   assert (composition);

   for (i=0; i<nelements; i++)
      weight += atomicWeight[i] * composition[i];
   return (weight);
}



/*
--------------------------------------------------------------------------------
 GxFourierNumber - calculate the Fourier number for diffusion
--------------------------------------------------------------------------------

 GxFourierNumber calculates the computational Fourier number used in the
 diffusion algorithm.

 Arguments
 ---------
 species         - species for which the Fourier number should be calculated
 aromchain       - the aromaticity of aliphatic chains
 temperature     - the temperature in Kelvin
 pressure        - the pressure in Pascal
 deltatime       - the time step
 slicethickness  - the thickness of a slice
 expansivity     - the expansivity
 compressibility - the compressibility
 timeunitfactor  - the number of seconds in a time unit

 Return value
 ------------
 the Fourier number

--------------------------------------------------------------------------------
*/

double GxFourierNumber (const GxSpecies *species, double aromchain,
                        double temperature, double pressure,
                        double deltatime, double slicethickness,
                        double expansivity, double compressibility,
                        double timeunitfactor)
{
   double  mulfac, diff;

   assert (species);
   assert ((aromchain >= 0.0) && (aromchain <= 1.0));
   assert (temperature >= 0.0);
   assert (pressure >= 0.0);
   assert (deltatime > 0.0);
   assert (slicethickness > 0.0);
   assert (expansivity >= 0.0);
   assert (compressibility >= 0.0);
   assert (timeunitfactor > 0.0);

   mulfac = species->jumplength * species->jumplength * deltatime /
            (slicethickness * slicethickness);
   diff = GxDiffusionRate (species, aromchain, temperature,
                                     pressure, expansivity, compressibility,
                                     timeunitfactor);
   return (mulfac * diff);
}



/*
--------------------------------------------------------------------------------
 GxGasInWater - calculate the volume of gas in water
--------------------------------------------------------------------------------

 GxGasInWater calculates the saturation volume of gas in water in carrier
 beds, as a function of pressure, salinity, etc. The temperature dependence
 is ignored (small): Schlum fgw-1; Dodson and Standing 1945 etc

 Arguments
 ---------
 gor         - the gas oil ratio
 pressure    - the pressure
 salinity    - the water salinity
 gasvolume   - the volume of the gas
 oilvolume   - the volume of the oil
 watervolume - the volume of the water

 Return value
 ------------
 the saturation volume of the gas in water

--------------------------------------------------------------------------------
*/

double GxGasInWater (double gor, double pressure, double salinity,
                     double gasvolume, double oilvolume, double watervolume)
{
   double  gwsat, gosat, gasinwater;

   assert (gor >= 0.0);
   assert (pressure >= 0.0);
   assert ((salinity >= 0.0) && (salinity <= 1.0));
   assert (gasvolume >= 0.0);
   assert (oilvolume >= 0.0);
   assert (watervolume >= 0.0);
   
   gwsat = GxCalcHCGasWaterSaturation ( salinity, pressure, watervolume );

   gosat = gor * oilvolume;
   if (gasvolume <= gosat + gwsat) {
      gasinwater = gasvolume * (gwsat / (gosat + gwsat));
   } else {
      gasinwater = gwsat;
   }
   return (gasinwater);
}


/*
--------------------------------------------------------------------------------
 GxGeneration - generation of species for one species and time step
--------------------------------------------------------------------------------

 GxGeneration performs the calculations for all the cracking reactions for one
 product, resulting in new concentrations for all nodes for that species.

 Arguments
 ---------
 isofract       - isotope fractionation data
 isofactor      - isotope fractionation factor data
 lithology      - structure with lithology data
 nslices        - the number of slices in the model
 nreactions     - the reactions to be modeled
 speciesnr      - the number of the species to be modeled
 timestepnr     - the number of the current timestep
 stoichiometry  - the stoichiometric factors for all reactions
 crackingrate   - the cracking rate for all species
 deltatime      - the time step
 asphalteneconv - the asphalteneconversion for this time step.
 concentration  - the concentration for each of the species
 negativegen    - the negative generation rate (cracked) for each of the species
 positivegen    - the positive generation rate for each of the species
 isotopehist    - the isotopehist
 temperature    - the current temperature
--------------------------------------------------------------------------------
*/

void GxGeneration (GxIsoFract *isofract, GxIsoFactor *isofactor, 
                   Lithology *lithology, int nslices, int nreactions, 
                   int speciesnr, int timestepnr, const double **stoichiometry,
                   const double *crackingrate, double deltatime, 
                   double asphalteneconv, double **concentration, 
                   double **negativegen, double **positivegen, 
                   double **isotopehist, double temperature)
{
   int     i, j, jend;
   double  gen, slicegen, fractionation;

   assert (nslices > 0);
   assert (nreactions > 0);
   assert (speciesnr >= 0);
   assert (timestepnr >= 0);
   assert (stoichiometry);
   assert (crackingrate);
   assert (deltatime > 0.0);
   assert (concentration);
   assert (negativegen);
   assert (positivegen);

/* Look only at species that can generate this one and that are reactive */
   jend = MIN(speciesnr,nreactions);

   if (isotopehist) {
      isotopehist[timestepnr][speciesnr] = 0.0;
   }

/* Loop over all the slices in the model */
   for (i=0; i<nslices; i++) {

/* -- Initialize generation and add contributions of individual species */
      gen = 0.0;
      for (j=0; j<jend; j++)
         gen += crackingrate[j] * stoichiometry[j][speciesnr] *
                concentration[i][j];

/* -- Calculate concentration taking generation and cracking into account */
      concentration[i][speciesnr] =
         (concentration[i][speciesnr] + gen * deltatime) /
         (1.0 + crackingrate[speciesnr] * deltatime);

      gen = 0.0;
      for (j=0; j<jend; j++) {
         slicegen = crackingrate[j] * stoichiometry[j][speciesnr] *
                    concentration[i][j];
         gen += slicegen;

/* ----- When needed calculate the isotope fractionation data for this slice */
         if (isotopehist && 
             isofract->fractmethod[j][speciesnr] != GX_ISO_FRAC_NOT_SPECIFIED  
             && isofract->fractmethod[j][speciesnr] != GX_ISO_FRAC_NONE ) {

             fractionation = GxCalcIsotopeFractionation (speciesnr, 
                                asphalteneconv, isotopehist[timestepnr][j],
                                lithology->initfractionation, 
                                isotopehist[timestepnr - 1][GX_ASPHALTENE],
                                temperature, isofactor, 
                                isofract->fractmethod[j][speciesnr]);
         
             isotopehist[timestepnr][speciesnr] += fractionation * slicegen;
         }
      }

      positivegen[i][speciesnr] = gen;
      negativegen[i][speciesnr] = -crackingrate[speciesnr] * 
                                              concentration[i][speciesnr];
   }
}



/*
--------------------------------------------------------------------------------
 GxGenerationDiffusion - perform generation and diffusion
--------------------------------------------------------------------------------

 GxGenerationDiffusion performs the calculations for generation and diffusion
 between nodes of one species for one time step.

 Arguments
 ---------
 isofract       - isotope fractionation data
 isofactor      - isotope fractionation factor data
 lithology      - structure with lithology data
 nslices        - number of slices to be modeled
 nreactions     - the reactions to be modeled
 speciesnr      - the number of the species to be modeled
 timestepnr     - the number of the current timestep
 stoichiometry  - the stoichiometric factors for all reactions
 crackingrate   - the cracking rate for all species
 deltatime      - the time step
 aspahlteneconv - the asphalteneconversion for this time step.
 concentration  - the concentration for each of the species
 negativegen    - the negative generation rate (cracked) for each of the species
 positivegen    - the positive generation rate for each of the species
 isotopehist    - the isotopehist
 lumpedconc     - concentrations for the lumped species
 fouriernumber  - computational Fourier number
 lowerconv      - lower convective mass transfer coefficient
 upperconv      - upper convective mass transfer coefficient
 slicethickness - characteristic slice thickness
 equation       - two-dimensional array for the diffusion equations
 temperature    - the current temperature

--------------------------------------------------------------------------------
*/

void GxGenerationDiffusion (GxIsoFract *isofract, GxIsoFactor *isofactor, 
                            Lithology *lithology, int nslices, 
                            int nreactions, int speciesnr, int timestepnr, 
                            const double **stoichiometry, 
                            const double *crackingrate, double deltatime,
                            double asphalteneconv, double **concentration, 
                            double **negativegen, double **positivegen, 
                            double **isotopehist, const double **lumpedconc, 
                            double fouriernumber, double lowerconv, 
                            double upperconv, double slicethickness, 
                            double **equations, double temperature)
{
   int     i, j, jend;
   double  lowerconcfactor, upperconcfactor;
   double  slicegen, fractionation;

   assert (nslices > 0);
   assert (nreactions > 0);
   assert (speciesnr >= 0);
   assert (stoichiometry);
   assert (crackingrate);
   assert (deltatime > 0.0);
   assert (concentration);
   assert (negativegen);
   assert (positivegen);
   assert (lumpedconc);
   assert (slicethickness > 0.0);
   assert (equations);

   
/* Set up equation for top slice */
   lowerconcfactor = GxConcFactor (lumpedconc[0], lumpedconc[1]);
   equations[0][0] = -2.0 * fouriernumber * lowerconcfactor;
   equations[3][0] = 1.0 - equations[0][0] + (2.0 * upperconv /
                     slicethickness + crackingrate[speciesnr]) * deltatime;

/* Set up equations for internal slices */
   for (i=1; i<nslices-1; i++) {
      upperconcfactor = lowerconcfactor;
      lowerconcfactor = GxConcFactor (lumpedconc[i], lumpedconc[i+1]);
      equations[0][i] = -fouriernumber * lowerconcfactor;
      equations[1][i] = -fouriernumber * upperconcfactor;
      equations[3][i] = 1.0 - (equations[1][i] + equations[0][i]) +
                        crackingrate[speciesnr] * deltatime;
   }

/* Set up equation for bottom slice */
   upperconcfactor         = lowerconcfactor;
   equations[1][nslices-1] = -2.0 * fouriernumber * upperconcfactor;
   equations[3][nslices-1] = 1.0 - equations[1][nslices-1] +
                             (2.0 * lowerconv / slicethickness +
                             crackingrate[speciesnr]) * deltatime;

/* Set up the generation terms for the equations */
   jend = MIN(speciesnr,nreactions);
   for (i=0; i<nslices; i++) {
      equations[2][i] = concentration[i][speciesnr];
      
      for (j=0; j<jend; j++) {
         equations[2][i] += crackingrate[j] * stoichiometry[j][speciesnr] *
                            concentration[i][j] * deltatime;
      }
   }

/* Solve the set of equations */
   GxSolveTriDiagonal (nslices, equations[0], equations[1], equations[2],
                       equations[3], equations[4]);

   if (isotopehist) {
      isotopehist[timestepnr][speciesnr] = 0.0;
   }

/* Copy the resulting concentrations */
   for (i=0; i<nslices; i++) {
       concentration[i][speciesnr] = equations[4][i];
       positivegen[i][speciesnr] = 0.0;
       negativegen[i][speciesnr] = 0.0;
      
       for (j=0; j<jend; j++) {
           slicegen = crackingrate[j] * 
                          stoichiometry[j][speciesnr] * concentration[i][j];
           positivegen[i][speciesnr] += slicegen;

/* ------- When needed calculate the isotope fractionation data for this slice*/
           if (isotopehist &&
               isofract->fractmethod[j][speciesnr] != GX_ISO_FRAC_NOT_SPECIFIED
               && isofract->fractmethod[j][speciesnr] != GX_ISO_FRAC_NONE ) {

               fractionation = GxCalcIsotopeFractionation (speciesnr,
                                  asphalteneconv, isotopehist[timestepnr][j],
                                  lithology->initfractionation, 
                                  isotopehist[timestepnr - 1][GX_ASPHALTENE],
                                  temperature, isofactor,
                                  isofract->fractmethod[j][speciesnr]);

               isotopehist[timestepnr][speciesnr] += fractionation * slicegen;
           }
       }
       negativegen[i][speciesnr] = -crackingrate[speciesnr] * 
                                                  concentration[i][speciesnr];
   }
}



/*
--------------------------------------------------------------------------------
 GxGeologicalVRE - calculate the next value of the geological VRE
--------------------------------------------------------------------------------

 GxGeologicalVRE calculates the value of the VRE after a time step. The VRE
 is calculated with a modified Lopatin algorithm (Stainforth, 1986). In
 addition to the VRE value that is returned this function modifies the
 tau value that is passed in the argument list.

 Arguments
 ---------
 deltatime   - the size of the time step in millions of years
 temperature - the current temperature in Kelvin
 tauptr      - pointer to the value of Tau

 Return value
 ------------
 the value of the VRE after the time step

--------------------------------------------------------------------------------
*/

double GxGeologicalVRE (double deltatime, double temperature,
                        double *tauptr)
{
   assert (deltatime >= 0.0);
   assert (temperature >= 0.0);
   assert (tauptr);

   *tauptr = *tauptr + deltatime *
             GxLocalPow (GXC_LOPATINCALF,
                  (temperature - GXC_LOPATINTEMPA) / GXC_LOPATINTEMPB);
   return ( GxLocalPow (*tauptr, GXC_LOPATINCALA) * GXC_LOPATINECALB);
}



/*
--------------------------------------------------------------------------------
 GxHasOilComposition - determine whether a species has an oil composition
--------------------------------------------------------------------------------

 GxHasOilComposition determines whether the species passed in the argument
 list has an oil composition or not.

 Arguments
 ---------
 species - the species for which the criterion should be applied

 Return value
 ------------
 GxTrue if the species has an oil composition, GxFalse otherwise

--------------------------------------------------------------------------------
*/

GxBool GxHasOilComposition (const GxSpecies *species)
{
   assert (species);

   return ((species->composition[GX_CARBON] >= GX_C_MINHC)
           && (species->composition[GX_HYDROGEN] >= GX_H_MINOIL)
           && (species->composition[GX_HYDROGEN] < GX_H_MINGAS));
}



/*
--------------------------------------------------------------------------------
 GxImmobileDensity - calculate the density of an immobile species
--------------------------------------------------------------------------------

 GxImmobileDensity calculates the density of an immobile species given its
 aromaticity, composition and formula weight.

 Arguments
 ---------
 species - the species for which the density should be calculated

 Return value
 ------------
 the density of the species

--------------------------------------------------------------------------------
*/

double GxImmobileDensity (const GxSpecies *species)
{
   double  rdash, rcratio, volring, formvol;

   assert (species);

/* ring cond index given fa and corrected hc,
 * Van Krevelen, eqn XVI, 10, p.322
 */
   rdash = 2.0 - species->aromaticity - GxCorrectedHC (species->composition);
   if (rdash < 0.0) rdash = 0.0;

/* approximate r/c, Van Krevelen, p. 322 */
   rcratio = rdash / 2.0;

/* volume of rings, Van Krevelen and Chermin '54, VK eqn, XVI, 4, p. 317 */
   volring = 9.1 - 3.65 * species->composition[GX_HYDROGEN];

/* formula volume, Van Krevelen, eqn. XVI, 6, p. 317 */
   formvol = 9.9 + 3.1 * species->composition[GX_HYDROGEN] +
             3.75 * species->composition[GX_OXYGEN] - volring * rcratio;

/* density */
   return (species->formulaweight / formvol * 1000.0);
}



/*
--------------------------------------------------------------------------------
 GxIsAsphaltene - determine whether a species is asphaltene
--------------------------------------------------------------------------------

 GxIsAsphaltene determines whether the species passed in the argument list
 is asphaltene or not.

 Arguments
 ---------
 species - the species for which the criterion should be applied

 Return value
 ------------
 GxTrue if the species is asphaltene, GxFalse otherwise

--------------------------------------------------------------------------------
*/

GxBool GxIsAsphaltene (const GxSpecies *species)
{
   assert (species);

   return (strncmp (GX_ASPHALTENEPREFIX, species->name,
                    strlen (GX_ASPHALTENEPREFIX)) == 0);
}



/*
--------------------------------------------------------------------------------
 GxIsHCGas - determine whether a species is a hydrocarbon gas
--------------------------------------------------------------------------------

 GxIsHCGas determines whether the species passed in the argument list is a
 hydrocarbon gas or not.

 Arguments
 ---------
 species - the species for which the criterion should be applied

 Return value
 ------------
 GxTrue if the species is a hydrocarbon gas, GxFalse otherwise

--------------------------------------------------------------------------------
*/

GxBool GxIsHCGas (const GxSpecies *species)
{
   assert (species);

   return ((species->composition[GX_CARBON] >= GX_C_MINHC)
           && (species->composition[GX_HYDROGEN] >= GX_H_MINGAS));
}



/*
--------------------------------------------------------------------------------
 GxIsHCWetGas - determine whether a species is a hydrocarbon wet gas
--------------------------------------------------------------------------------

 GxIsHCGas determines whether the species passed in the argument list is a
 hydrocarbon wet gas or not.

 Arguments
 ---------
 species - the species for which the criterion should be applied

 Return value
 ------------
 GxTrue if the species is a hydrocarbon gas, GxFalse otherwise

--------------------------------------------------------------------------------
*/

GxBool GxIsHCWetGas (const GxSpecies *species)
{
   assert (species);

   return ((species->composition[GX_CARBON] >= GX_C_MINHC)
           && (species->composition[GX_HYDROGEN] >= GX_H_MINGAS)
           && (species->composition[GX_HYDROGEN] < GX_H_MINHCDRYGAS));
}



/*
--------------------------------------------------------------------------------
 GxIsHydrocarbon - determine whether a species is a hydrocarbon
--------------------------------------------------------------------------------

 GxIsHydrocarbon determines whether the species passed in the argument list
 is a hydrocarbon or not.

 Arguments
 ---------
 species - the species for which the criterion should be applied

 Return value
 ------------
 GxTrue if the species is a hydrocarbon, GxFalse otherwise

--------------------------------------------------------------------------------
*/

GxBool GxIsHydrocarbon (const GxSpecies *species)
{
   assert (species);

   return ((species->composition[GX_HYDROGEN] >= GX_H_MINHC)
           && (species->composition[GX_OXYGEN] <= GX_O_MAXHC)
           && (species->composition[GX_NITROGEN] <= GX_N_MAXHC)
           && (species->composition[GX_SULFUR] <= GX_S_MAXHC));
}


/*
--------------------------------------------------------------------------------
 GxIsN2Gas - determine whether a species is a nitrogen gas
--------------------------------------------------------------------------------

 GxIsN2Gas determines whether the species passed in the argument list is a
 nitrogen gas or not.

 Arguments
 ---------
 species - the species for which the criterion should be applied

 Return value
 ------------
 GxTrue if the species is a nitrogen gas, GxFalse otherwise

--------------------------------------------------------------------------------
*/

GxBool GxIsN2Gas (const GxSpecies *species)
{
   assert (species);
   
   return ((species->composition[GX_NITROGEN] >= GX_N_MINN2)
           && (species->composition[GX_CARBON] <= GX_C_MAXN2)
           && (species->composition[GX_HYDROGEN] <= GX_H_MAXN2)
           && (species->composition[GX_OXYGEN] <= GX_O_MAXN2)
           && (species->composition[GX_SULFUR] <= GX_S_MAXN2));
}



/*
--------------------------------------------------------------------------------
 GxKerogenTransf - calculate the transformation ratio of kerogen
--------------------------------------------------------------------------------

 GxKerogenTransf calculates the transformation ratio of kerogen.

 Arguments
 ---------
 initialconc    - the initial concentration of kerogen
 kerogenconc    - the current concentration of kerogen
 asphalteneconc - the current concentration of asphaltene

 Return value
 ------------
 the transformation ratio of the kerogen

--------------------------------------------------------------------------------
*/

double GxKerogenTransf (double initialconc, double kerogenconc,
                        double asphalteneconc)
{
   double  transformation;

   assert (initialconc >= 0.0);
   assert (kerogenconc >= 0.0);
   assert (asphalteneconc >= 0.0);
/*   assert ((asphalteneconc + kerogenconc) <= initialconc); */

   transformation = 1.0 - (asphalteneconc + kerogenconc) / initialconc;
   return (MAX (0.0, MIN(transformation,  1.0)));
}



/*
--------------------------------------------------------------------------------
 GxLithostaticPressure - calculate the lithostatic pressure from the depth
--------------------------------------------------------------------------------

 GxLithostaticPressure calculates the lithostatic pressure from the depth.

 Arguments
 ---------
 depth     - the depth for which the pressure should be estimated
 overbdens - the density of the overburden

 Return value
 ------------
 the calculated lithostatic pressure

--------------------------------------------------------------------------------
*/

double GxLithostaticPressure (double depth, double overbdens)
{
   assert (depth >= 0.0);
   assert (overbdens > 0.0);

   return (depth * overbdens * GXC_GRAVITY);
}



/*
--------------------------------------------------------------------------------
 GxMassStoichFactor - calculate the mass stoich factor from the molar
--------------------------------------------------------------------------------

 GxMassStoichFactor calculates the mass stoichiometric factor for a species
 in a reaction given the molar stoichiometric factor.

 Arguments
 ---------
 reactant - reactant of the reaction
 product  - product of the reaction for which the msf should be determined
 molarsf  - molar stoichiometric factor for the product

 Return value
 ------------
 the mass stoichiometric factor

--------------------------------------------------------------------------------
*/

double GxMassStoichFactor (const GxSpecies *reactant,
                           const GxSpecies *product, double molarsf)
{
   assert (reactant);
   assert (product);

   return (molarsf * product->formulaweight / reactant->formulaweight);
}


/*
--------------------------------------------------------------------------------
 GxN2InWater - calculate the volume of N2 in water
--------------------------------------------------------------------------------

 GxN2InWater calculates the saturation volume of N2 in water in carrier
 beds, as a function of pressure, salinity, etc using the saturation of HC gas
 in water. The temperature dependence is ignored (small): 
 Schlum fgw-1; Dodson and Standing 1945 etc

 Arguments
 ---------
 pressure    - the pressure
 salinity    - the water salinity
 n2volume    - the volume of N2 gas expelled 
 watervolume - the volume of the water

 Return value
 ------------
 the saturation volume of N2 in water

--------------------------------------------------------------------------------
*/

double GxN2InWater (double pressure, double salinity, double n2volume, 
                    double watervolume)
{
   double  gwsat, n2sat, n2inwater;

   assert (pressure >= 0.0);
   assert ((salinity >= 0.0) && (salinity <= 1.0));
   assert (n2volume >= 0.0);
   assert (watervolume >= 0.0);
   
   gwsat = GxCalcHCGasWaterSaturation ( salinity, pressure, watervolume );
   n2sat = gwsat * ( 0.5465 + 0.00465 * pressure / 1000000.0);

   if (n2volume < n2sat) {
      n2inwater = n2volume;
   } else {
      n2inwater = n2sat;
   }
   return (n2inwater);
}



/*
--------------------------------------------------------------------------------
 GxOverPressure - calculate the over pressure
--------------------------------------------------------------------------------

 GxOverPressure calculates the approximate overpressure in the secondary
 migration system as a function of the expelled masses of oil, gas and n2, and
 the mass of water in the sms.

 Arguments
 ---------
 oilmass   - mass of expelled oil
 gasmass   - mass of expelled gas
 n2mass    - mass of expelled n2
 watermass - mass of the water in the sms

 Return value
 ------------
 the calculated overpressure

--------------------------------------------------------------------------------
*/

double GxOverPressure (double oilmass, double gasmass, double n2mass,
                       double watermass)
{
   assert (oilmass >= 0.0);
   assert (gasmass >= 0.0);
   assert (n2mass >= 0.0);
   assert (watermass >= 0.0);

   return (1000000.0 * (800.0 * oilmass + 2000.0 * gasmass + 2000.0 * n2mass) 
           / watermass);
}



/*
--------------------------------------------------------------------------------
 GxReferenceDiffusivity - calculate the reference diffusivity for a species
--------------------------------------------------------------------------------

 GxReferenceDiffusivity calculates the diffusivity of a species at reference
 conditions.

 Arguments
 ---------
 species         - data for each of the species
 speciesnr       - species for which reference diffusivity is to be calculated
 aromchain       - the aromaticity of aliphatic chains
 temperature     - the reference temperature in Kelvin
 pressure        - the reference pressure in Pascal
 wbo             - the reference wbo
 expansivity     - the expansivity
 compressibility - the compressibility
 timeunitfactor  - the number of seconds in a time unit

 Return value
 ------------
 the reference diffusivity for the species

--------------------------------------------------------------------------------
*/

double GxReferenceDiffusivity (const GxSpecies *species, int speciesnr,
                               double aromchain, double temperature,
                               double pressure, double wbo,
                               double expansivity, double compressibility,
                               double timeunitfactor)
{
   double  mulfac;

   assert (species);
   assert (speciesnr >= 0);
   assert ((aromchain >= 0.0) && (aromchain <= 1.0));
   assert (temperature >= 0.0);
   assert (pressure >= 0.0);
   assert ((wbo >= 0.0) && (wbo <= 1.0));
   assert (expansivity >= 0.0);
   assert (compressibility >= 0.0);
   assert (timeunitfactor > 0.0);

   mulfac = species[speciesnr].jumplength * species[speciesnr].jumplength *
            GxAromaticityDependence (species[GX_KEROGEN].aromaticity, wbo);

   return (mulfac * GxDiffusionRate (&species[speciesnr], aromchain,
                                     temperature, pressure, expansivity,
                                     compressibility, timeunitfactor));
}



/*
--------------------------------------------------------------------------------
 GxSaturationCGR - calculate the saturation CGR of methane
--------------------------------------------------------------------------------

 GxSaturationCGR calculates the saturation CGR of methane using fig.1
 of Price 1983

 Arguments
 ---------
 temperature - the gas temperature
 pressure    - the gas pressure

 Return value
 ------------
 the saturation CGR

--------------------------------------------------------------------------------
*/

double GxSaturationCGR (double temperature, double pressure)
{
   assert (temperature >= 0.0);
   assert (pressure >= 0.0);

   return (0.000097 * GxLocalPow (10.0, 0.000000013 * pressure +
           0.0035 * (temperature - 100.0 - GXC_ZEROCELCIUS)));
}



/*
--------------------------------------------------------------------------------
 GxTotalOrganicCarbon - calculate the total organic carbon
--------------------------------------------------------------------------------

 GxTotalOrganicCarbon calculates the total organic carbon (the weight
 fraction of C atoms per rock) given the atomic carbon and the density.

 Arguments
 ---------
 atomicc - the atomic carbon
 density - the density of the rock

 Return value
 ------------
 the total organic carbon

--------------------------------------------------------------------------------
*/

double GxTotalOrganicCarbon (double atomicc, double density)
{
   assert (atomicc >= 0.0);
   assert (density > 0.0);

   return (atomicc * atomicWeight[GX_CARBON] / density);
}



/*
--------------------------------------------------------------------------------
 GxVolumeFractionOM - calculate the volume fraction of organic matter
--------------------------------------------------------------------------------

 GxVolumeFractionOM calculates the volume fraction of the total organic
 matter from the concentrations.

 Arguments
 ---------
 nspecies      - number of species
 species       - data for each of the species
 concentration - the concentration for each of the species

 Return value
 ------------
 the volume fraction of organic matter

--------------------------------------------------------------------------------
*/

double GxVolumeFractionOM (int nspecies, const GxSpecies *species,
                           const double *concentration)
{
   double  vorock, denssub;
   int     i;

   assert (nspecies > 0);
   assert (species);
   assert (concentration);

   vorock = 0.0;
   for (i=0; i<nspecies; i++) {
      if (species[i].density > 500.0) {
         denssub = species[i].density;
      } else {
         denssub = 500.0;
      }
      vorock += concentration[i] / denssub;
   }
   return (vorock);
}



/*
--------------------------------------------------------------------------------
 GxWaterDensity - calculate the density of water
--------------------------------------------------------------------------------

 GxWaterDensity calculates the approximate density of water as a function
 of pressure, temperature and salinity (see Schowalter 1979, fig 2),
 assuming alpha = 7.2e-4/K, beta = 4.83e-4/MPa, gamma = 6.48e-1/NaCl
 @ 93C = 220F

 Arguments
 ---------
 temperature - the temperature of water
 pressure    - the pressure of the water
 salinity    - the salinity of water

 Return value
 ------------
 the approximate density of the water

--------------------------------------------------------------------------------
*/

double GxWaterDensity (double temperature, double pressure, double salinity)
{
   assert (temperature >= 0.0);
   assert (pressure >= 0.0);
   assert ((salinity >= 0.0) && (salinity <= 1.0));

   return (966.0 + 648.0 * salinity -
           0.72 * (temperature - 93.0 - GXC_ZEROCELCIUS) +
           0.483e-6 * pressure);
}
