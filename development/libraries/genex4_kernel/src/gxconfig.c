/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxconfig.c,v 25.0 2005/07/05 08:03:44 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxconfig.c,v $
 * Revision 25.0  2005/07/05 08:03:44  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:42:53  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:19  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:07  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:40  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:13  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:15  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:24  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:33  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:03  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:25:42  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:03  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:27  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:09:54  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:00:54  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:55:48  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:00  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:52:56  ibs
 * Add headers.
 * */
/* ================================================================== */
/*
--------------------------------------------------------------------------------
 GxConfig.c   Genex configuration file reading functions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        24-JUN-1992

 Description: GxConfig.c contains the Genex functions for reading the
              configuration file. The following functions are available:

                GxFreeConfiguration - free memory allocated for configuration
                GxReadConfiguration - read the configuration file

              To support these functions this file contains a number of
              local functions that can not be used outside this file.

 Usage notes: - In order to use the functions in this source file the header
                file gxconfig.h should be included.
              - All functions return an error code. If an error is detected
                the error handler function is called and an error code unequal
                to GXE_NOERROR is returned to the caller.


 History
 -------
 24-JUN-1992  P. Alphenaar  initial version
 04-MAR-1993  M van Houtert Check for / in pathname added
 02-MAR-1995  Th. Berkers   Integration of Isotopes Fractionation

--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  <assert.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  "gxconfig.h"
#include  "gxconsts.h"
#include  "gxerror.h"
#include  "gxphchem.h"
#include  "gxutils.h"
#include  "list.h"

/* Macro definitions */
/* ----------------- */
#define  GX_CONFIGURATIONFILE  "genex.cfg"
#define  GXL_CONFIGKEYWORD     32
#define  GXL_CONFIGVALUE       22
/* -- keyword and special character definitions -- */
#define  GX_AUTOMATICRATIO     "*"
#define  GX_COMPOSITION        "composition"
#define  GX_COMPRESSIBILITY    "compressibility"
#define  GX_CRACKINGENERGY1    "crackingenergy1"
#define  GX_CRACKINGENERGY2    "crackingenergy2"
#define  GX_CRACKINGENTROPY    "crackingentropy"
#define  GX_CRACKINGVOLUME     "crackingvolume"
#define  GX_DENSITY            "density"
#define  GX_DIFFUSIONENERGY    "diffusionenergy"
#define  GX_DIFFUSIONENTROPY   "diffusionentropy"
#define  GX_DIFFUSIONVOLUME    "diffusionvolume"
#define  GX_ELEMENTS           "elements"
#define  GX_EXPANSIVITY        "expansivity"
#define  GX_INORGANICDENS      "inorganicdensity"
#define  GX_JUMPLENGTH         "jumplength"
#define  GX_RATIO              "ratio"
#define  GX_ORDER              "order"
#define  GX_OVERBURDENDENS     "overburdendensity"
#define  GX_REACTION           "reaction"
#define  GX_SPECIES            "species"
#define  GX_SURFACETEMP        "surfacetemperature"
#define  GX_TEMPERATUREGRAD    "temperaturegradient"
#define  GX_ISOTOPE            "isotope"
#define  GX_ISOFRACTFACTLOW    "fractfactalphalow"
#define  GX_ISOFRACTHIGHPERC   "fracthighpercentage"
#define  GX_ISOFRACTTEMPLOW    "fracttemplow"
#define  GX_ISOFRACTTEMPHIGH   "fracttemphigh"

#define  GX_NO_METHOD               "NONE"
#define  GX_METHOD_RAYLEIGH_KEROGEN "RAYLEIGH_FRACT_KEROGEN"
#define  GX_METHOD_RAYLEIGH_C1      "RAYLEIGH_FRACT_C1"
#define  GX_METHOD_RAYLEIGH_C2C5    "RAYLEIGH_FRACT_C2C5"
#define  GX_METHOD_HISTORY          "FRACT_HISTORY"

/* -- default values -- */
#define  GX_DEFAULTOVERBDENS   2300.0
#define  GX_DEFAULTSURFTEMP    (GXC_ZEROCELCIUS+20.0)
#define  GX_DEFAULTTEMPGRAD    0.038
/* -- error text definitions -- */
#define  GXT_INVCONFIGFILE     "The configuration file %s is incorrect.\n"
#define  GXT_NOCONFIGFILE      "Can't find the configuration file %s.\n"
/* Function prototypes */
/* ------------------- */
static GxErrorCode  GxAllocIsotopeFract (int, int, GxIsoFract *);
static GxErrorCode  GxAllocReactionScheme (int, GxReactionScheme *);
static GxErrorCode  GxCheckIsotopeFract (const char *, GxConfiguration *);
static GxErrorCode  GxCheckReactionScheme (const char *, GxConfiguration *);
static void         GxFreeIsotopeFract (GxIsoFract *);
static void         GxFreeReactionScheme (GxReactionScheme *);
static GxErrorCode  GxLookupIsotopeFractMethod (const char *, const char *, 
                                                GxIsoFractMethod *);
static GxErrorCode  GxLookupSpeciesName (const char *, const char *, int,
                                         const GxSpecies *, int *);
static GxErrorCode  GxReadComposition (FILE *, const char *,
                                       GxConfiguration *, List);
static GxErrorCode  GxReadCompressibility (FILE *, const char *,
                                           GxConfiguration *);
static GxErrorCode  GxReadConfigContents (FILE *, const char *,
                                          GxConfiguration **);
static GxErrorCode  GxReadCrackingEnergy1 (FILE *, const char *,
                                           GxConfiguration *);
static GxErrorCode  GxReadCrackingEnergy2 (FILE *, const char *,
                                           GxConfiguration *);
static GxErrorCode  GxReadCrackingEntropy (FILE *, const char *,
                                           GxConfiguration *);
static GxErrorCode  GxReadCrackingVolume (FILE *, const char *,
                                          GxConfiguration *);
static GxErrorCode  GxReadDensity (FILE *, const char *, GxConfiguration *);
static GxErrorCode  GxReadDiffusionEnergy (FILE *, const char *,
                                           GxConfiguration *);
static GxErrorCode  GxReadDiffusionEntropy (FILE *, const char *,
                                            GxConfiguration *);
static GxErrorCode  GxReadDiffusionVolume (FILE *, const char *,
                                           GxConfiguration *);
static GxErrorCode  GxReadDouble (FILE *, const char *, double *);
static GxErrorCode  GxReadElements (FILE *, const char *, GxConfiguration *);
static GxErrorCode  GxReadExpansivity (FILE *, const char *,
                                       GxConfiguration *);
static GxErrorCode  GxReadInorganicDens (FILE *, const char *,
                                         GxConfiguration *);
static GxErrorCode  GxReadIsoFractFactorAlpha (FILE *, const char *,
                                               GxConfiguration *, GxBool);
static GxErrorCode  GxReadIsoFractionationTemp (FILE *, const char *,
                                                GxConfiguration *, GxBool);
static GxErrorCode  GxReadIsoFractMethods (FILE *, const char *,
                                           GxConfiguration *, List);
static GxErrorCode  GxReadJumpLength (FILE *, const char *,
                                      GxConfiguration *);
static GxErrorCode  GxReadList (FILE *, const char *, List);
static GxErrorCode  GxReadOrder (FILE *, const char *, GxConfiguration *);
static GxErrorCode  GxReadOverburdenDens (FILE *, const char *,
                                          GxConfiguration *);
static GxErrorCode  GxReadRatio (FILE *, const char *, GxConfiguration *,
                                 List);
static GxErrorCode  GxReadReaction (FILE *, const char *, GxConfiguration *,
                                    List);
static GxErrorCode  GxReadSpecies (FILE *, const char *, GxConfiguration *,
                                   List);
static GxErrorCode  GxReadSpeciesName (FILE *, const char *, char *);
static GxErrorCode  GxReadSurfaceTemp (FILE *, const char *,
                                       GxConfiguration *);
static GxErrorCode  GxReadTemperatureGrad (FILE *, const char *,
                                           GxConfiguration *);



/*
--------------------------------------------------------------------------------
 GxAlloccwIsotopeFract- allocate memory for the isotope fractionation data
--------------------------------------------------------------------------------

 GxAllocIsotopeFract allocates memory for the isotope fractionation data of
 the species in the reactions 

 Arguments
 ---------
 nreactions     - the number of reactions
 nspecies       - the number of species
 reactionscheme - pointer to the reaction scheme

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocIsotopeFract (int nreactions, int nspecies,
                                        GxIsoFract *isofract)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i,j;

   assert (nreactions > 0);
   assert (nspecies > 0);
   assert (isofract);

   isofract->nreactions  = nreactions;
   isofract->nspecies    = nspecies;
   isofract->fractmethod = NULL;

/* Allocate array for the fractionation method for each species in each    */
/* reaction                                                                */
   rc = GxAllocArray (&isofract->fractmethod, sizeof (GxIsoFractMethod), 
                      2, nreactions, nspecies);

   if (!rc) {
/* -- Initialize the isotope fractionation methods                         */
      for (i=0; i<nreactions; i++) {
         for (j=0; j<nspecies; j++) {
             isofract->fractmethod[i][j] = GX_ISO_FRAC_NOT_SPECIFIED;
         }
      }
   } else {
      GxFreeIsotopeFract (isofract);
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxAllocReactionScheme - allocate memory for the reaction scheme
--------------------------------------------------------------------------------

 GxAllocReactionScheme allocates memory for all reactions in the reaction
 scheme.

 Arguments
 ---------
 nspecies       - the number of species
 reactionscheme - pointer to the reaction scheme

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxAllocReactionScheme (int nspecies,
                                          GxReactionScheme *reactionscheme)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (nspecies > 0);
   assert (reactionscheme);

   reactionscheme->nspecies   = nspecies;
   reactionscheme->nreactions = 0;
   reactionscheme->nproducts  = NULL;
   reactionscheme->product    = NULL;
   reactionscheme->nratios    = NULL;
   reactionscheme->ratio      = NULL;

/* Allocate array for the number of products of each reaction */
   rc = GxAllocArray (&reactionscheme->nproducts, sizeof (int), 1, nspecies);

/* Allocate array for the numbers of the species produced in each reaction */
   if (!rc) rc = GxAllocArray (&reactionscheme->product, sizeof (int),
                               2, nspecies, nspecies);

/* Allocate array for the number of ratios specified for each reaction */
   if (!rc) rc = GxAllocArray (&reactionscheme->nratios, sizeof (int),
                               1, nspecies);

/* Allocate array for the ratios speciefies for each reaction */
   if (!rc) rc = GxAllocArray (&reactionscheme->ratio, sizeof (GxRatio),
                               2, nspecies, nspecies);
   if (rc) GxFreeReactionScheme (reactionscheme);
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxCheckIsotopeFract - check if all necessary isotope fractionation method have
                       been read from configuration file
--------------------------------------------------------------------------------

 GxCheckIsotopeFract checks if all necessary isotope fractionation method have
 been read from the configuration file.

 Arguments
 ---------
 filename      - the name of the configuration file
 configuration - the configuration for which reaction scheme should be checked

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxCheckIsotopeFract (const char *filename,
                                        GxConfiguration *configuration)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i,j,productnr;

   assert (filename);
   assert (configuration);

/* Loop over all the reactions */
   for (i=0; (!rc)&&(i<configuration->reactionscheme.nreactions); i++) {
      for (j=0; (!rc)&&(j<configuration->reactionscheme.nproducts[i]); j++) {
         productnr = configuration->reactionscheme.product[i][j];
         if (configuration->isofract.fractmethod[i][productnr] ==
             GX_ISO_FRAC_NOT_SPECIFIED) {
            rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
         }
      }
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxCheckReactionScheme - check a reaction scheme read from configuration file
--------------------------------------------------------------------------------

 GxCheckReactionScheme checks a reaction scheme that was read from the
 configuration file.

 Arguments
 ---------
 filename      - the name of the configuration file
 configuration - the configuration for which reaction scheme should be checked

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxCheckReactionScheme (const char *filename,
                                          GxConfiguration *configuration)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i, j, k;
   GxBool       generated;

   assert (filename);
   assert (configuration);

/* Loop over all the reactions */
   for (i=0; (!rc)&&(i<configuration->reactionscheme.nreactions); i++) {

/* -- Check that species (except if it is kerogen) is generated by another */
      generated = GxFalse;
      for (j=0; j<i; j++)
         for (k=0; k<configuration->reactionscheme.nproducts[j]; k++)
            if (configuration->reactionscheme.product[j][k] == i)
               generated = GxTrue;
      if ((!generated && (i != GX_KEROGEN))
         || !configuration->species[i].reactive
         || (configuration->reactionscheme.nproducts[i] < 2)) {
            rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
      } else {

/* ----- Check that ratios are either specified or species is asphaltene */
         for (j=0; (!rc)&&(j<configuration->reactionscheme.nratios[i]); j++) {
            if (!configuration->reactionscheme.ratio[i][j].specified
               && ((i != GX_ASPHALTENE)
                   || !GxIsAsphaltene (&configuration->species[i])))
               rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeConfiguration - free memory allocated for configuration
--------------------------------------------------------------------------------

 GxFreeConfiguration frees the memory that was allocated for the configuration
 data when the configuration file was read.

 Arguments
 ---------
 configuration - the configuration to be freed

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxFreeConfiguration (GxConfiguration *configuration)
{
   GxErrorCode  rc=GXE_NOERROR;

   if (configuration) {
      GxFreeReactionScheme (&configuration->reactionscheme);
      GxFreeIsotopeFract (&configuration->isofract);
      if ((configuration->nspecies > 0) && configuration->species)
         GxFreeArray (configuration->species, 1, configuration->nspecies);
      free (configuration);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxFreeIsotopeFract - free memory allocated for the isotope fractionation data
--------------------------------------------------------------------------------

 GxFreeIsotopeFract frees the memory that was allocated for the isotope 
 fractionation method data when the configuration file was read.

 Arguments
 ---------
 isofract - pointer to the isotope fractionation data

--------------------------------------------------------------------------------
*/

static void GxFreeIsotopeFract (GxIsoFract *isofract)
{
   assert (isofract);

   if (isofract->nreactions > 0 && isofract->nspecies > 0) {
      if (isofract->fractmethod) {
         GxFreeArray (isofract->fractmethod, 2, isofract->nreactions, 
                      isofract->nspecies);
      }
      isofract->nreactions  = 0;
      isofract->nspecies    = 0;
      isofract->fractmethod = NULL;
   }
}


/*
--------------------------------------------------------------------------------
 GxFreeReactionScheme - free memory allocated for the reaction scheme
--------------------------------------------------------------------------------

 GxFreeReactionScheme frees the memory that was allocated for the reactions
 in the reaction scheme when the configuration file was read.

 Arguments
 ---------
 reactionscheme - pointer to the reaction scheme

--------------------------------------------------------------------------------
*/

static void GxFreeReactionScheme (GxReactionScheme *reactionscheme)
{
   assert (reactionscheme);

   if (reactionscheme->nspecies > 0) {
      if (reactionscheme->nproducts)
         GxFreeArray (reactionscheme->nproducts, 1, reactionscheme->nspecies);
      if (reactionscheme->product)
         GxFreeArray (reactionscheme->product, 2, reactionscheme->nspecies,
                      reactionscheme->nspecies);
      if (reactionscheme->nratios)
         GxFreeArray (reactionscheme->nratios, 1, reactionscheme->nspecies);
      if (reactionscheme->ratio)
         GxFreeArray (reactionscheme->ratio, 2, reactionscheme->nspecies,
                      reactionscheme->nspecies);
      reactionscheme->nspecies   = 0;
      reactionscheme->nreactions = 0;
      reactionscheme->nproducts  = NULL;
      reactionscheme->product    = NULL;
      reactionscheme->nratios    = NULL;
      reactionscheme->ratio      = NULL;
   }
}



/*
--------------------------------------------------------------------------------
 GxLookupIsotopeFractMethod - lookup a isotope fracttionation method name
--------------------------------------------------------------------------------

 GxLookupIsotopeFractMethod looks up a method name. If the nethod name the
 corresponding enumerated type is returned.

 Arguments
 ---------
 filename     - the name of the file
 methodname   - name of the method to be looked up
 methodptr    - pointer to the method enumerated type sequence number

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxLookupIsotopeFractMethod (const char *filename,
                                               const char *methodname,
                                               GxIsoFractMethod *methodptr)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (filename);
   assert (methodname);
   assert (methodptr);

   if ( strcmp (methodname, GX_NO_METHOD) == 0 ) {
      *methodptr = GX_ISO_FRAC_NONE;
   } else if (strcmp (methodname, GX_METHOD_RAYLEIGH_KEROGEN) == 0 ) {
      *methodptr = GX_ISO_RAYLEIGH_FRAC_KEROGEN;
   } else if (strcmp (methodname, GX_METHOD_RAYLEIGH_C1) == 0 ) {
      *methodptr = GX_ISO_RAYLEIGH_FRAC_C1;
   } else if (strcmp (methodname, GX_METHOD_RAYLEIGH_C2C5) == 0 ) {
      *methodptr = GX_ISO_RAYLEIGH_FRAC_C2C5;
   } else if (strcmp (methodname, GX_METHOD_HISTORY) == 0 ) {
      *methodptr = GX_ISO_FRAC_HISTORY;
   } else {
      rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxLookupSpeciesName - lookup a species name in species data
--------------------------------------------------------------------------------

 GxLookupSpeciesName looks up a species name in the species data. If the
 species is found the sequence number of the species is returned.

 Arguments
 ---------
 filename     - the name of the file
 speciesname  - name of the species to be looked up
 nspecies     - the number of species in species data
 species      - the data for each of the species
 speciesnrptr - pointer to the species sequence number

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxLookupSpeciesName (const char *filename,
                                        const char *speciesname,
                                        int nspecies,
                                        const GxSpecies *species,
                                        int *speciesnrptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          i;

   assert (filename);
   assert (speciesname);
   assert (nspecies > 0);
   assert (species);
   assert (speciesnrptr);

   i = -1;
   while ((i++ < nspecies) &&
          (strcmp (speciesname, species->name) != 0)) species++;
   if (i >= nspecies) {
      rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
   } else {
      *speciesnrptr = i;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadComposition - read the species composition from configuration file
--------------------------------------------------------------------------------

 GxReadComposition reads the composition of a species from the
 configuration file and updates the species data.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration in which the composition should be stored
 stringlist    - string list for temporary use

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadComposition (FILE *filehandle, const char *filename,
                                      GxConfiguration *configuration,
                                      List stringlist)
{
   GxErrorCode  rc=GXE_NOERROR;
   char         speciesname[GXL_SPECIESNAME+1];
   int          speciesnr, i, nstrings;
   char         *natoms;

   assert (filehandle);
   assert (filename);
   assert (configuration);
   assert (configuration->species);
   assert ((configuration->nelements >= 0) && (configuration->nelements <= GXN_ELEMENTS));
   assert (stringlist);

/* Get the species name from the file and look it up in the list */
   rc = GxReadSpeciesName (filehandle, filename, speciesname);
   if (!rc) rc = GxLookupSpeciesName (filename, speciesname,
                                      configuration->nspecies,
                                      configuration->species,
                                      &speciesnr);

/* Get the composition from the file */
   if (!rc) rc = GxReadList (filehandle, filename, stringlist);
   if (!rc) {

/* -- Check the number of values */
      nstrings = (int) CountList (stringlist);
      if (nstrings > configuration->nelements) {
         rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
      } else {

/* ----- Convert the values from string to number representation */
         natoms = GetFirstFromList (stringlist);
         for (i=0; (!rc)&&(i<nstrings); i++) {
            configuration->species[speciesnr].composition[i] = atof (natoms);
            if (configuration->species[speciesnr].composition[i] < 0.0)
               rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
            natoms = GetNextFromList (stringlist);
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadCompressibility - read the compressibility from the configuration file
--------------------------------------------------------------------------------

 GxReadCompressibility reads the compressibility from the configuration file.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where the compressibility should be stored

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadCompressibility (FILE *filehandle,
                                          const char *filename,
                                          GxConfiguration *configuration)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (filehandle);
   assert (filename);
   assert (configuration);

/* Read a double precision number from file skipping over equal sign first */
   rc = GxReadDouble (filehandle, filename, &configuration->compressibility);

/* Validate the number read */
   if (!rc && (configuration->compressibility <= 0.0))
      rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadConfigContents - read the contents of the configuration file
--------------------------------------------------------------------------------

 GxReadConfigContents reads the data that is contained in the configuration
 file.

 Arguments
 ---------
 filehandle       - handle of file from which data should be read
 filename         - the name of the file
 configurationptr - pointer for the configuration to be read

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadConfigContents (FILE *filehandle,
                                         const char *filename,
                                         GxConfiguration **configurationptr)
{
   GxErrorCode      rc=GXE_NOERROR;
   char             keyword[GXL_CONFIGKEYWORD+1];
   List             stringlist;
   GxConfiguration  *configuration=NULL;

   assert (filehandle);
   assert (filename);
   assert (configurationptr);

/* Allocate memory for the configuration structure and initialize it */
   configuration = ( GxConfiguration * ) malloc (sizeof (GxConfiguration));
   if (!configuration) {
      rc = GxError (GXE_MEMORY, GXT_MEMORY);
   } else {
      configuration->nelements               = 0;
      configuration->nspecies                = 0;
      configuration->species                 = NULL;
      configuration->reactionscheme.nspecies = 0;
      configuration->overburdendensity       = GX_DEFAULTOVERBDENS;
      configuration->surfacetemperature      = GX_DEFAULTSURFTEMP;
      configuration->temperaturegradient     = GX_DEFAULTTEMPGRAD;
      configuration->isofract.nreactions     = 0;
      configuration->isofract.nspecies       = 0;
      configuration->isofract.fractmethod    = NULL;

/* -- Create a temporary list for strings, used in different places */
      stringlist = CreateList (GXL_CONFIGVALUE+1, NULL);
      if (!stringlist) {
         DeleteList (stringlist);
         rc = GxError (GXE_LIST, GXT_LIST);
      } else {

/* ----- Skip a possible initial line feed character */
         if (fscanf (filehandle, "\n")) {
            rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
         } else {

/* -------- Loop for all statements in the configuration file */
            while (!rc && fscanf (filehandle,
                   "%32[^.= \n]", keyword) == 1) {
               if (keyword[0] == '!') {

/* -------------- Skip line beginning with ! (comments) */
                  while (fgetc (filehandle) != '\n');
                  (void) fscanf (filehandle, "\n");
               } else {

/* -------------- Switch to the statements corresponding to keyword found */
                  if (strcmp (keyword, GX_COMPOSITION) == 0) {
                     rc = GxReadComposition (filehandle, filename,
                                             configuration, stringlist);
                  } else if (strcmp (keyword, GX_COMPRESSIBILITY) == 0) {
                     rc = GxReadCompressibility (filehandle, filename,
                                                 configuration);
                  } else if (strcmp (keyword, GX_CRACKINGENERGY1) == 0) {
                     rc = GxReadCrackingEnergy1 (filehandle, filename,
                                                 configuration);
                  } else if (strcmp (keyword, GX_CRACKINGENERGY2) == 0) {
                     rc = GxReadCrackingEnergy2 (filehandle, filename,
                                                 configuration);
                  } else if (strcmp (keyword, GX_CRACKINGENTROPY) == 0) {
                     rc = GxReadCrackingEntropy (filehandle, filename,
                                                 configuration);
                  } else if (strcmp (keyword, GX_CRACKINGVOLUME) == 0) {
                     rc = GxReadCrackingVolume (filehandle, filename,
                                                configuration);
                  } else if (strcmp (keyword, GX_DENSITY) == 0) {
                     rc = GxReadDensity (filehandle, filename, configuration);
                  } else if (strcmp (keyword, GX_DIFFUSIONENERGY) == 0) {
                     rc = GxReadDiffusionEnergy (filehandle, filename,
                                                 configuration);
                  } else if (strcmp (keyword, GX_DIFFUSIONENTROPY) == 0) {
                     rc = GxReadDiffusionEntropy (filehandle, filename,
                                                  configuration);
                  } else if (strcmp (keyword, GX_DIFFUSIONVOLUME) == 0) {
                     rc = GxReadDiffusionVolume (filehandle, filename,
                                                 configuration);
                  } else if (strcmp (keyword, GX_ELEMENTS) == 0) {
                     rc = GxReadElements (filehandle, filename,
                                          configuration);
                  } else if (strcmp (keyword, GX_EXPANSIVITY) == 0) {
                     rc = GxReadExpansivity (filehandle, filename,
                                             configuration);
                  } else if (strcmp (keyword, GX_INORGANICDENS) == 0) {
                     rc = GxReadInorganicDens (filehandle, filename,
                                               configuration);
                  } else if (strcmp (keyword, GX_JUMPLENGTH) == 0) {
                     rc = GxReadJumpLength (filehandle, filename,
                                            configuration);
                  } else if (strcmp (keyword, GX_ORDER) == 0) {
                     rc = GxReadOrder (filehandle, filename, configuration);
                  } else if (strcmp (keyword, GX_OVERBURDENDENS) == 0) {
                     rc = GxReadOverburdenDens (filehandle, filename,
                                                configuration);
                  } else if (strcmp (keyword, GX_REACTION) == 0) {
                     rc = GxReadReaction (filehandle, filename, configuration,
                                          stringlist);
                  } else if (strcmp (keyword, GX_RATIO) == 0) {
                     rc = GxReadRatio (filehandle, filename, configuration,
                                       stringlist);
                  } else if (strcmp (keyword, GX_SPECIES) == 0) {
                     rc = GxReadSpecies (filehandle, filename, configuration,
                                         stringlist);
                     if (!rc)
                        rc = GxAllocReactionScheme (configuration->nspecies,
                                &configuration->reactionscheme);
                  } else if (strcmp (keyword, GX_SURFACETEMP) == 0) {
                     rc = GxReadSurfaceTemp (filehandle, filename,
                                             configuration);
                  } else if (strcmp (keyword, GX_TEMPERATUREGRAD) == 0) {
                     rc = GxReadTemperatureGrad (filehandle, filename,
                                                 configuration);
                  } else if (strcmp (keyword, GX_ISOTOPE) == 0) {
/* ----------------- When first isotope data to be read in allocate memory */
                     if (configuration->isofract.fractmethod == NULL) {
                        rc = GxAllocIsotopeFract (
                                 configuration->reactionscheme.nreactions,
                                 configuration->nspecies,
                                 &configuration->isofract);
                     }
                     if (!rc) {
                        rc = GxReadIsoFractMethods (filehandle, filename,
                                                    configuration, stringlist);
                     }
                  } else if (strcmp (keyword, GX_ISOFRACTFACTLOW) == 0) {
                     rc = GxReadIsoFractFactorAlpha (filehandle, filename,
                                                     configuration, GxTrue);
                  } else if (strcmp (keyword, GX_ISOFRACTHIGHPERC) == 0) {
                     rc = GxReadIsoFractFactorAlpha (filehandle, filename,
                                                     configuration, GxFalse);
                  } else if (strcmp (keyword, GX_ISOFRACTTEMPLOW) == 0) {
                     rc = GxReadIsoFractionationTemp (filehandle, filename,
                                                      configuration, GxTrue);
                  } else if (strcmp (keyword, GX_ISOFRACTTEMPHIGH) == 0) {
                     rc = GxReadIsoFractionationTemp (filehandle, filename,
                                                      configuration, GxFalse);
                  } else {

/* ----------------- The keyword found is unknown */
                     rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE,
                                   filename);
                  }
               }
            }

/* -------- Perform a check of the reaction scheme read */
            if (!rc) rc = GxCheckReactionScheme (filename, configuration);

/* -------- Perform a check of the istope fractionation methods read */
            if (!rc) rc = GxCheckIsotopeFract (filename, configuration);
         }

/* ----- Delete the temporary list */
         DeleteList (stringlist);
      }
   }
   if (!rc) {
      *configurationptr = configuration;
   } else {
      (void) GxFreeConfiguration (configuration);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadConfiguration - read the configuration file
--------------------------------------------------------------------------------

 GxReadConfiguration reads species and reaction data from the configuration
 file.

 Arguments
 ---------
 filename         - name of file from which configuration should be read
 configurationptr - pointer to the configuration

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

GxErrorCode GxReadConfiguration (const char *filename,
                                 GxConfiguration **configurationptr)
{
   GxErrorCode  rc=GXE_NOERROR;
   char         *directory, configfilename[GXL_FILENAME+1];
   FILE         *filehandle;
   int          len;

   if (!configurationptr) {
      rc = GxError (GXE_ARGUMENT, GXT_ARGUMENT, __FILE__, __LINE__);
   } else {
      if (filename) {

/* ----- Configuration file name was specified, so use that */
         (void) strncpy (configfilename, filename, GXL_FILENAME);
         configfilename[GXL_FILENAME] = '\0';
      } else {

/* ----- Configuration file name was no specified, so use default */
/* ----- Determine the path by translating the environment variable */
         directory = getenv (GX_DIRECTORYVARIABLE);
         if (!directory) {
            rc = GxError (GXE_VARUNDEFINED, GXT_VARUNDEFINED,
                          GX_DIRECTORYVARIABLE);
         } else {

/* -------- Build the complete file name */
            (void) strcpy (configfilename, directory);
            len = strlen (configfilename);
	    if (configfilename[len - 1] != '/') {
	        (void) strcat (configfilename, "/");
            }
            (void) strcat (configfilename, GX_CONFIGURATIONFILE);

/* -------- Open the file for reading as a text file */
            filehandle = fopen (configfilename, "rt");
            if (!filehandle) {
               rc = GxError (GXE_NOCONFIGFILE, GXT_NOCONFIGFILE, configfilename);
            } else {

/* ----------- Read and check the contents of the configuration file */
               rc = GxReadConfigContents (filehandle, configfilename,
                                          configurationptr);

/* ----------- Close the file (even in the case of errors) */
               if (fclose (filehandle) && !rc)
                  rc = GxError (GXE_FILECLOSE, GXT_FILECLOSE,
                                configfilename);
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadCrackingEnergy1 - read the species cracking energy number 1
--------------------------------------------------------------------------------

 GxReadCrackingEnergy1 reads the first activation energy for cracking of a
 species from the configuration file and updates the configuration data
 structure.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where the cracking energy should be stored

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadCrackingEnergy1 (FILE *filehandle,
                                          const char *filename,
                                          GxConfiguration *configuration)
{
   GxErrorCode  rc=GXE_NOERROR;
   char         speciesname[GXL_SPECIESNAME+1];
   int          speciesnr;

   assert (filehandle);
   assert (filename);
   assert (configuration);

/* Get the species name from the file and look it up in the list */
   rc = GxReadSpeciesName (filehandle, filename, speciesname);
   if (!rc) rc = GxLookupSpeciesName (filename, speciesname,
                                      configuration->nspecies,
                                      configuration->species, &speciesnr);

/* Read a double precision number from file skipping over equal sign first */
   if (!rc) rc = GxReadDouble (filehandle, filename,
                    &configuration->species[speciesnr].crackingenergy1);

/* Validate the number read */
   if (!rc && (configuration->species[speciesnr].crackingenergy1 <= 0.0))
      rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
   if (!rc) {

/* -- Set the high cracking energy if not specified */
      if (configuration->species[speciesnr].crackingenergy2 == 0.0)
         configuration->species[speciesnr].crackingenergy2 =
            configuration->species[speciesnr].crackingenergy1;

/* -- Mark species as reactive */
      configuration->species[speciesnr].reactive = GxTrue;
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadCrackingEnergy2 - read the species cracking energy number 2
--------------------------------------------------------------------------------

 GxReadCrackingEnergy2 reads the second activation energy for cracking of a
 species from the configuration file and updates the configuration data
 structure.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where the cracking energy should be stored

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadCrackingEnergy2 (FILE *filehandle,
                                          const char *filename,
                                          GxConfiguration *configuration)
{
   GxErrorCode  rc=GXE_NOERROR;
   char         speciesname[GXL_SPECIESNAME+1];
   int          speciesnr;

   assert (filehandle);
   assert (filename);
   assert (configuration);

/* Get the species name from the file and look it up in the list */
   rc = GxReadSpeciesName (filehandle, filename, speciesname);
   if (!rc) rc = GxLookupSpeciesName (filename, speciesname,
                                      configuration->nspecies,
                                      configuration->species, &speciesnr);

/* Read a double precision number from file skipping over equal sign first */
   if (!rc) rc = GxReadDouble (filehandle, filename,
                    &configuration->species[speciesnr].crackingenergy2);

/* Validate the number read */
   if (!rc && (configuration->species[speciesnr].crackingenergy2 <= 0.0))
      rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);

/* Mark species as reactive */
   if (!rc) configuration->species[speciesnr].reactive = GxTrue;
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadCrackingEntropy - read the species cracking entropy
--------------------------------------------------------------------------------

 GxReadCrackingEntropy reads the activation entropy for cracking of a
 species from the configuration file and updates the configuration data
 structure.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where the cracking entropy should be stored

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadCrackingEntropy (FILE *filehandle,
                                          const char *filename,
                                          GxConfiguration *configuration)
{
   GxErrorCode  rc=GXE_NOERROR;
   char         speciesname[GXL_SPECIESNAME+1];
   int          speciesnr;

   assert (filehandle);
   assert (filename);
   assert (configuration);

/* Get the species name from the file and look it up in the list */
   rc = GxReadSpeciesName (filehandle, filename, speciesname);
   if (!rc) rc = GxLookupSpeciesName (filename, speciesname,
                                      configuration->nspecies,
                                      configuration->species, &speciesnr);

/* Read a double precision number from file skipping over equal sign first */
   if (!rc) rc = GxReadDouble (filehandle, filename,
                    &configuration->species[speciesnr].crackingentropy);

/* Mark species as reactive */
   if (!rc) configuration->species[speciesnr].reactive = GxTrue;
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadCrackingVolume - read the species cracking volume
--------------------------------------------------------------------------------

 GxReadCrackingVolume reads the activation volume for cracking of a
 species from the configuration file and updates the configuration data
 structure.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where the cracking volume should be stored

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadCrackingVolume (FILE *filehandle,
                                         const char *filename,
                                         GxConfiguration *configuration)
{
   GxErrorCode  rc=GXE_NOERROR;
   char         speciesname[GXL_SPECIESNAME+1];
   int          speciesnr;

   assert (filehandle);
   assert (filename);
   assert (configuration);

/* Get the species name from the file and look it up in the list */
   rc = GxReadSpeciesName (filehandle, filename, speciesname);
   if (!rc) rc = GxLookupSpeciesName (filename, speciesname,
                                      configuration->nspecies,
                                      configuration->species, &speciesnr);

/* Read a double precision number from file skipping over equal sign first */
   if (!rc) rc = GxReadDouble (filehandle, filename,
                    &configuration->species[speciesnr].crackingvolume);

/* Validate the number read */
   if (!rc && (configuration->species[speciesnr].crackingvolume <= 0.0))
      rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);

/* Mark species as reactive */
   if (!rc) configuration->species[speciesnr].reactive = GxTrue;
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadDensity - read the species density
--------------------------------------------------------------------------------

 GxReadDensity reads the density of a species from the configuration file and
 updates the configuration data structure.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where the density should be stored

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadDensity (FILE *filehandle, const char *filename,
                                  GxConfiguration *configuration)
{
   GxErrorCode  rc=GXE_NOERROR;
   char         speciesname[GXL_SPECIESNAME+1];
   int          speciesnr;

   assert (filehandle);
   assert (filename);
   assert (configuration);

/* Get the species name from the file and look it up in the list */
   rc = GxReadSpeciesName (filehandle, filename, speciesname);
   if (!rc) rc = GxLookupSpeciesName (filename, speciesname,
                                      configuration->nspecies,
                                      configuration->species, &speciesnr);

/* Read a double precision number from file skipping over equal sign first */
   if (!rc) rc = GxReadDouble (filehandle, filename,
                    &configuration->species[speciesnr].density);

/* Validate the number read */
   if (!rc && (configuration->species[speciesnr].density <= 0.0))
      rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadDiffusionEnergy - read the species diffusion energy
--------------------------------------------------------------------------------

 GxReadDiffusionEnergy reads the activation energy for diffusion of a
 species from the configuration file and updates the configuration data
 structure.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where the diffusion energy should be stored

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadDiffusionEnergy (FILE *filehandle,
                                          const char *filename,
                                          GxConfiguration *configuration)
{
   GxErrorCode  rc=GXE_NOERROR;
   char         speciesname[GXL_SPECIESNAME+1];
   int          speciesnr;

   assert (filehandle);
   assert (filename);
   assert (configuration);

/* Get the species name from the file and look it up in the list */
   rc = GxReadSpeciesName (filehandle, filename, speciesname);
   if (!rc) rc = GxLookupSpeciesName (filename, speciesname,
                                      configuration->nspecies,
                                      configuration->species, &speciesnr);

/* Read a double precision number from file skipping over equal sign first */
   if (!rc) rc = GxReadDouble (filehandle, filename,
                    &configuration->species[speciesnr].diffusionenergy);

/* Validate the number read */
   if (!rc && (configuration->species[speciesnr].diffusionenergy <= 0.0))
      rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);

/* Mark species as mobile */
   if (!rc) configuration->species[speciesnr].mobile = GxTrue;
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadDiffusionEntropy - read the species diffusion entropy
--------------------------------------------------------------------------------

 GxReadDiffusionEntropy reads the activation entropy for diffusion of a
 species from the configuration file and updates the configuration data
 structure.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where the diffusion entropy should be stored

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadDiffusionEntropy (FILE *filehandle,
                                           const char *filename,
                                           GxConfiguration *configuration)
{
   GxErrorCode  rc=GXE_NOERROR;
   char         speciesname[GXL_SPECIESNAME+1];
   int          speciesnr;

   assert (filehandle);
   assert (filename);
   assert (configuration);

/* Get the species name from the file and look it up in the list */
   rc = GxReadSpeciesName (filehandle, filename, speciesname);
   if (!rc) rc = GxLookupSpeciesName (filename, speciesname,
                                      configuration->nspecies,
                                      configuration->species, &speciesnr);

/* Read a double precision number from file skipping over equal sign first */
   if (!rc) rc = GxReadDouble (filehandle, filename,
                    &configuration->species[speciesnr].diffusionentropy);

/* Mark species as mobile */
   if (!rc) configuration->species[speciesnr].mobile = GxTrue;
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadDiffusionVolume - read the species diffusion volume
--------------------------------------------------------------------------------

 GxReadDiffusionEnergy reads the activation volume for diffusion of a
 species from the configuration file and updates the configuration data
 structure.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where the diffusion volume should be stored

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadDiffusionVolume (FILE *filehandle,
                                          const char *filename,
                                          GxConfiguration *configuration)
{
   GxErrorCode  rc=GXE_NOERROR;
   char         speciesname[GXL_SPECIESNAME+1];
   int          speciesnr;

   assert (filehandle);
   assert (filename);
   assert (configuration);

/* Get the species name from the file and look it up in the list */
   rc = GxReadSpeciesName (filehandle, filename, speciesname);
   if (!rc) rc = GxLookupSpeciesName (filename, speciesname,
                                      configuration->nspecies,
                                      configuration->species, &speciesnr);

/* Read a double precision number from file skipping over equal sign first */
   if (!rc) rc = GxReadDouble (filehandle, filename,
                    &configuration->species[speciesnr].diffusionvolume);

/* Validate the number read */
   if (!rc && (configuration->species[speciesnr].diffusionvolume <= 0.0))
      rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);

/* Mark species as mobile */
   if (!rc) configuration->species[speciesnr].mobile = GxTrue;
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadDouble - read a double precision number from configuration file
--------------------------------------------------------------------------------

 GxReadDouble reads the assignment character "=" and a double precision
 floating point number from the configuration file.

 Arguments
 ---------
 filehandle - handle of file from which data should be read
 filename   - the name of the file
 doubleptr  - pointer to where the double should be stored

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadDouble (FILE *filehandle, const char *filename,
                                 double *doubleptr)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (filehandle);
   assert (filename);
   assert (doubleptr);

/* Skip over space and equal sign */
   (void) fscanf (filehandle, " ");
   if (fgetc (filehandle) != '=') {
      rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
   } else {

/* -- Read a double precision number */
      if (fscanf (filehandle, "%lf", doubleptr) != 1) {
         rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
      } else {
         (void) fscanf (filehandle, " ");
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadElements - read the number of elements from configuration file
--------------------------------------------------------------------------------

 GxReadElements reads the number of the elements that will be modeled from
 the configuration file.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where the number of elements should be stored

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadElements (FILE *filehandle, const char *filename,
                                   GxConfiguration *configuration)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (filehandle);
   assert (filename);
   assert (configuration);

/* Skip over space and equal sign */
   (void) fscanf (filehandle, " ");
   if (fgetc (filehandle) != '=') {
      rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
   } else {

/* -- Read an integer from the configuration file */
      if (fscanf (filehandle, "%d", &configuration->nelements) != 1) {
         rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
      } else {

/* ----- Validate the number of elements */
         if ((configuration->nelements <= 0) ||
             (configuration->nelements > GXN_ELEMENTS)) {
            rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
         } else {
            (void) fscanf (filehandle, " ");
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadExpansivity - read the expansivity from the configuration file
--------------------------------------------------------------------------------

 GxReadExpansivity reads the expansivity from the configuration file.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where the expansivity should be stored

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadExpansivity (FILE *filehandle, const char *filename,
                                      GxConfiguration *configuration)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (filehandle);
   assert (filename);
   assert (configuration);

/* Read a double precision number from file skipping over equal sign first */
   rc = GxReadDouble (filehandle, filename, &configuration->expansivity);

/* Validate the number read */
   if (!rc && (configuration->expansivity <= 0.0))
      rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadInorganicDens - read inorganic matter density from configuration file
--------------------------------------------------------------------------------

 GxReadInorganicDens reads the density of the inorganic matter from the
 configuration file.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where the inorganic density should be stored

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadInorganicDens (FILE *filehandle,
                                        const char *filename,
                                        GxConfiguration *configuration)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (filehandle);
   assert (filename);
   assert (configuration);

/* Read a double precision number from file skipping over equal sign first */
   rc = GxReadDouble (filehandle, filename,
                      &configuration->inorganicdensity);

/* Validate the number read */
   if (!rc && (configuration->inorganicdensity <= 0.0))
      rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxReadIsoFractFactorAlpha - read isotope fractionation factor alpha from 
                             configuration file
--------------------------------------------------------------------------------

 GxReadIsoFractFactorAlpha reads the isotope fractionation factor alpha from 
 the configuration file.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where the isotope fractionation factor alpha 
                 should be stored
 low           - boolean indicating if low value or high value has to be read

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadIsoFractFactorAlpha (FILE *filehandle,
                                              const char *filename,
                                              GxConfiguration *configuration,
                                              GxBool low)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (filehandle);
   assert (filename);
   assert (configuration);

   if (low == GxTrue) { 
/*--- Read a double precision number from file skipping over equal sign first */
      rc = GxReadDouble (filehandle, filename,
                         &configuration->isofract.factor.alphalow);

/*--- Validate the number read */
      if (!rc && (configuration->isofract.factor.alphalow < 0.0))
         rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
   } else {
/*--- Read a double precision number from file skipping over equal sign first */
      rc = GxReadDouble (filehandle, filename,
                         &configuration->isofract.factor.highpercentage);

/*--- Validate the number read */
      if (!rc && ((configuration->isofract.factor.highpercentage < 0.0) ||
                  (configuration->isofract.factor.highpercentage > 100.0)))
         rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxReadIsoFractionationTemp - read isotope fractionation temperature from 
                              configuration file
--------------------------------------------------------------------------------

 GxReadIsoFractionationTemp reads the isotope fractionation temperature alpha 
 from the configuration file.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where the isotope fractionation factor alpha 
                 should be stored
 low           - boolean indicating if low value or high value has to be read

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadIsoFractionationTemp (FILE *filehandle,
                                               const char *filename,
                                               GxConfiguration *configuration,
                                               GxBool low)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (filehandle);
   assert (filename);
   assert (configuration);

   if (low == GxTrue) { 
/*--- Read a double precision number from file skipping over equal sign first */
      rc = GxReadDouble (filehandle, filename,
                         &configuration->isofract.factor.templow);

/*--- Validate the number read */
      if (!rc && (configuration->isofract.factor.templow < 0.0))
         rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
   } else {
/*--- Read a double precision number from file skipping over equal sign first */
      rc = GxReadDouble (filehandle, filename,
                         &configuration->isofract.factor.temphigh);

/*--- Validate the number read */
      if (!rc && (configuration->isofract.factor.temphigh < 0.0))
         rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
   }
   return (rc);
}


/*
--------------------------------------------------------------------------------
 GxReadIsoFractMethods - read a isotope fractionation method from 
                         configuration file
--------------------------------------------------------------------------------

 GxReadIsoFractMethods reads a isotope fractionation method from the 
 configuration file and adds to the array of isotope fractionation methods

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where isotope fractionation method should be 
                 stored
 stringlist    - string list for temporary use

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadIsoFractMethods (FILE *filehandle, 
                                          const char *filename,
                                          GxConfiguration *configuration,
                                          List stringlist)
{
   GxErrorCode      rc=GXE_NOERROR;
   char             reactantname[GXL_SPECIESNAME+1], *productname, *methodname;
   int              reactantnr, productnr;
   int              i;
   GxIsoFractMethod isofractmethod;
   GxBool           productinreaction;
   

   assert (filehandle);
   assert (filename);
   assert (configuration);
   assert (stringlist);

/* Get the species name from the file and look it up in the list */
   rc = GxReadSpeciesName (filehandle, filename, reactantname);
   if (!rc) rc = GxLookupSpeciesName (filename, reactantname,
                                      configuration->nspecies,
                                      configuration->species, &reactantnr);

/* Read a list of strings from the configuration file */
   if (!rc) rc = GxReadList (filehandle, filename, stringlist);
   if (!rc) {

/* -- Check that there are two strings */
      if (CountList (stringlist) != 2) {
         rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
      } else {

/* ----- The first string is the product names, the second the isotope     */
/*       fractionation method to be used for the product name in this      */
/*       reaction                                                          */
         productname = GetFirstFromList (stringlist);
         methodname  = GetNextFromList (stringlist);

/* ----- Look up the product name */
         rc = GxLookupSpeciesName (filename, productname,
                 configuration->nspecies, configuration->species,
                 &productnr);

         if (!rc) {
/* -------- Look up the method name */
            rc = GxLookupIsotopeFractMethod (filename, methodname, 
                                             &isofractmethod);
         }

         if (!rc) {
/* -------- Look up the products in the product list of the reaction */
            productinreaction = GxFalse;
            for (i=0; i<configuration->reactionscheme.nproducts[reactantnr];
                 i++) {
               if (configuration->reactionscheme.product[reactantnr][i]
                  == productnr) {
                  productinreaction = GxTrue;
               }
            }
            if ( productinreaction == GxFalse ) {
               rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
            } else {

/* ----------- Add the method to the configuration structure */
               configuration->isofract.fractmethod[reactantnr][productnr] =
                   isofractmethod;
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadJumpLength - read the species diffusion jump length
--------------------------------------------------------------------------------

 GxReadJumpLength reads the jump length for diffusion of a species from the
 configuration file and updates the configuration data structure.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where the jump length should be stored

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadJumpLength (FILE *filehandle, const char *filename,
                                     GxConfiguration *configuration)
{
   GxErrorCode  rc=GXE_NOERROR;
   char         speciesname[GXL_SPECIESNAME+1];
   int          speciesnr;

   assert (filehandle);
   assert (filename);
   assert (configuration);

/* Get the species name from the file and look it up in the list */
   rc = GxReadSpeciesName (filehandle, filename, speciesname);
   if (!rc) rc = GxLookupSpeciesName (filename, speciesname,
                                      configuration->nspecies,
                                      configuration->species, &speciesnr);

/* Read a double precision number from file skipping over equal sign first */
   if (!rc) rc = GxReadDouble (filehandle, filename,
                    &configuration->species[speciesnr].jumplength);

/* Validate the number read */
   if (!rc && (configuration->species[speciesnr].jumplength <= 0.0))
      rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);

/* Mark species as mobile */
   if (!rc) configuration->species[speciesnr].mobile = GxTrue;
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadList - read a list of strings from configuration file
--------------------------------------------------------------------------------

 GxReadList reads a list of strings from the configuration file and stores
 them in a list.

 Arguments
 ---------
 filehandle - handle of file from which data should be read
 filename   - the name of the file
 stringlist - list of strings

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadList (FILE *filehandle, const char *filename,
                               List stringlist)
{
   GxErrorCode  rc=GXE_NOERROR;
   char         string[GXL_CONFIGVALUE+1];
   int          c, length;
   GxBool       endoflist=GxFalse;

   assert (filehandle);
   assert (filename);
   assert (stringlist);

/* Empty the temporary list */
   ClearList (stringlist);

/* Skip over space and equal sign */
   (void) fscanf (filehandle, " ");
   if (fgetc (filehandle) != '=') {
      rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
   } else {

/* -- Skip over space and opening parenthesis */
      (void) fscanf (filehandle, " ");
      if (fgetc (filehandle) != '(') {
         rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
      } else {
         (void) fscanf (filehandle, " ");

/* ----- Read list of strings separated by comma's and ending with ) */
         while (!rc && (!endoflist)) {
/*            if (fscanf (filehandle, "%" STRNG(GXL_CONFIGVALUE) "[^,)\n]",*/
            if (fscanf (filehandle, "%22[^,)\n]",
                        string) != 1) {
               rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
            } else {

/* ----------- Get rid of trailing spaces */
               length = strlen (string);
               while ((length > 0) && string[length-1] == ' ')
                  string[--length] = '\0';
               if (!AppendToList (stringlist, string)) {
                  rc = GxError (GXE_LIST, GXT_LIST);
               } else {
                  (void) fscanf (filehandle, " ");
                  c = fgetc (filehandle);
                  (void) fscanf (filehandle, " ");
                  if (c == ')') {
                     endoflist = GxTrue;
                  } else if (c != ',') {
                     rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE,
                                   filename);
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
 GxReadOrder - read the order for the species cracking reaction
--------------------------------------------------------------------------------

 GxReadOrder reads the order of the cracking reaction of a species from the
 configuration file and updates the configuration data structure.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where the order should be stored

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadOrder (FILE *filehandle, const char *filename,
                                GxConfiguration *configuration)
{
   GxErrorCode  rc=GXE_NOERROR;
   char         speciesname[GXL_SPECIESNAME+1];
   int          speciesnr;

   assert (filehandle);
   assert (filename);
   assert (configuration);

/* Get the species name from the file and look it up in the list */
   rc = GxReadSpeciesName (filehandle, filename, speciesname);
   if (!rc) rc = GxLookupSpeciesName (filename, speciesname,
                                      configuration->nspecies,
                                      configuration->species, &speciesnr);

/* Read a double precision number from file skipping over equal sign first */
   if (!rc) rc = GxReadDouble (filehandle, filename,
                    &configuration->species[speciesnr].order);

/* Validate the number read */
   if (!rc && (configuration->species[speciesnr].order <= 0.0))
      rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);

/* Mark species as reactive */
   if (!rc) configuration->species[speciesnr].reactive = GxTrue;
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadOverburdenDens - read density of overburden from configuration file
--------------------------------------------------------------------------------

 GxReadOverburdenDens reads the density of the overburden from the
 configuration file.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where the overburden density should be stored

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadOverburdenDens (FILE *filehandle,
                                         const char *filename,
                                         GxConfiguration *configuration)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (filehandle);
   assert (filename);
   assert (configuration);

/* Read a double precision number from file skipping over equal sign first */
   rc = GxReadDouble (filehandle, filename,
                      &configuration->overburdendensity);

/* Validate the number read */
   if (!rc && (configuration->overburdendensity <= 0.0))
      rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadRatio - read a ratio from configuration file
--------------------------------------------------------------------------------

 GxReadRatio reads a reaction ratio from the configuration file and adds
 it to the list of ratios for the reaction.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where ratio should be stored
 stringlist    - string list for temporary use

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadRatio (FILE *filehandle, const char *filename,
                                GxConfiguration *configuration,
                                List stringlist)
{
   GxErrorCode  rc=GXE_NOERROR;
   char         reactantname[GXL_SPECIESNAME+1], *product1name, *product2name,
                *ratiotext;
   int          reactantnr, product1nr, product2nr, nratios, i;
   GxRatio      ratio;

   assert (filehandle);
   assert (filename);
   assert (configuration);
   assert (stringlist);

/* Get the species name from the file and look it up in the list */
   rc = GxReadSpeciesName (filehandle, filename, reactantname);
   if (!rc) rc = GxLookupSpeciesName (filename, reactantname,
                                      configuration->nspecies,
                                      configuration->species, &reactantnr);

/* Read a list of strings from the configuration file */
   if (!rc) rc = GxReadList (filehandle, filename, stringlist);
   if (!rc) {

/* -- Check that there are three strings */
      if (CountList (stringlist) != 3) {
         rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
      } else {

/* ----- The first two strings are product names, the third is the ratio */
         product1name = GetFirstFromList (stringlist);
         product2name = GetNextFromList (stringlist);
         ratiotext    = GetNextFromList (stringlist);

/* ----- Look up the two product names */
         rc = GxLookupSpeciesName (filename, product1name,
                 configuration->nspecies, configuration->species,
                 &product1nr);
         if (!rc) rc = GxLookupSpeciesName (filename, product2name,
                          configuration->nspecies, configuration->species,
                          &product2nr);
         if (!rc) {

/* -------- Check whether the ratio is "automatic" */
            if (strcmp (GX_AUTOMATICRATIO, ratiotext) == 0) {
               ratio.specified = GxFalse;
            } else {
               ratio.ratio     = atof (ratiotext);
               ratio.specified = GxTrue;
            }

/* -------- Look up the products in the product list of the reaction */
            ratio.product1 = -1;
            ratio.product2 = -1;
            for (i=0; i<configuration->reactionscheme.nproducts[reactantnr];
                 i++) {
               if (configuration->reactionscheme.product[reactantnr][i]
                  == product1nr) ratio.product1 = i;
               if (configuration->reactionscheme.product[reactantnr][i]
                  == product2nr) ratio.product2 = i;
            }
            if ((ratio.product1 == -1) || (ratio.product2 == -1)) {
               rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
            } else {

/* ----------- Add the ratio to the ratios of the reaction */
               configuration->reactionscheme.nratios[reactantnr]++;
               nratios = configuration->reactionscheme.nratios[reactantnr];
               configuration->reactionscheme.ratio[reactantnr][nratios-1] =
                  ratio;
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadReaction - read a reaction from configuration file
--------------------------------------------------------------------------------

 GxReadReaction reads a reaction from the configuration file and adds it to
 the reaction scheme.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where species should be stored
 stringlist    - string list for temporary use

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadReaction (FILE *filehandle, const char *filename,
                                   GxConfiguration *configuration,
                                   List stringlist)

{
   GxErrorCode  rc=GXE_NOERROR;
   char         reactantname[GXL_SPECIESNAME+1], *productname;
   int          reactantnr, nproducts, i;

   assert (filehandle);
   assert (filename);
   assert (configuration);
   assert (stringlist);

   configuration->reactionscheme.nreactions++;

/* Get the species name from the file and look it up in the list */
   rc = GxReadSpeciesName (filehandle, filename, reactantname);
   if (!rc) rc = GxLookupSpeciesName (filename, reactantname,
                                      configuration->nspecies,
                                      configuration->species, &reactantnr);
   if (!rc) {

/* -- Mark the cracking species as reactive */
      configuration->species[reactantnr].reactive = GxTrue;

/* -- Read the list of products of the reaction */
      rc = GxReadList (filehandle, filename, stringlist);
      if (!rc) {
         nproducts = (int) CountList (stringlist);
         if (nproducts <= 0) {
            rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
         } else {

/* -------- Store the number of products and initialize number of ratios */
            configuration->reactionscheme.nproducts[reactantnr] = nproducts;
            configuration->reactionscheme.nratios[reactantnr]   = 0;

/* -------- Look up the products in the species list and store them */
            productname = GetFirstFromList (stringlist);
            for (i=0; (!rc)&&(i<nproducts); i++) {
               rc = GxLookupSpeciesName (filename, productname,
                       configuration->nspecies, configuration->species,
                       &configuration->reactionscheme.product[reactantnr][i]);
               productname = GetNextFromList (stringlist);
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadSpecies - read the species names from configuration file
--------------------------------------------------------------------------------

 GxReadSpecies reads the list of species names from the configuration file
 and creates an array for the data for these species.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where species should be stored
 stringlist    - string list for temporary use

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadSpecies (FILE *filehandle, const char *filename,
                                  GxConfiguration *configuration,
                                  List stringlist)
{
   GxErrorCode    rc=GXE_NOERROR;
   int            i, j;
   char           *speciesname;

   assert (filehandle);
   assert (filename);
   assert (configuration);
   assert (stringlist);

   if (configuration->nspecies != 0) {
      rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
   } else {

/* -- Read the list with the species names */
      rc = GxReadList (filehandle, filename, stringlist);
      if (!rc) {

/* ----- Count the species and allocate an array for the species data */
         configuration->nspecies = (int) CountList (stringlist);
         rc = GxAllocArray (&configuration->species, sizeof (GxSpecies),
                            1, configuration->nspecies);
         if (!rc) {

/* -------- Copy the species name into the array and initialize other data */
            speciesname = GetFirstFromList (stringlist);
            for (i=0; i<configuration->nspecies; i++) {
               (void) strcpy (configuration->species[i].name, speciesname);
               for (j=0; j<GXN_ELEMENTS; j++)
                  configuration->species[i].composition[j]    = 0.0;
               configuration->species[i].reactive             = GxFalse;
               configuration->species[i].mobile               = GxFalse;
               configuration->species[i].crackingenergy1      = 0.0;
               configuration->species[i].crackingenergy2      = 0.0;
               configuration->species[i].crackingentropy      = 0.0;
               configuration->species[i].crackingvolume       = 0.0;
               configuration->species[i].diffusionentropy     = 0.0;
               configuration->species[i].diffusionvolume      = 0.0;
               configuration->species[i].jumplength           = 0.0;
               configuration->species[i].order                = 1.0;
               configuration->species[i].formulaweight        = 0.0;
               configuration->species[i].density              = 0.0;
               configuration->species[i].aromaticity          = 0.0;
               configuration->species[i].referencediffusivity = 0.0;
               speciesname = GetNextFromList (stringlist);
            }
         }
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadSpeciesName - read a species name
--------------------------------------------------------------------------------

 GxReadSpeciesName reads a species name from the configuration file.

 Arguments
 ---------
 filehandle - handle of file from which data should be read
 filename   - the name of the file
 name       - the name of the species

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadSpeciesName (FILE *filehandle, const char *filename,
                                      char *name)
{
   GxErrorCode  rc=GXE_NOERROR;
   int          length;

   assert (filehandle);
   assert (filename);
   assert (name);

/* Skip a period */
   if (fgetc (filehandle) != '.') {
      rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
   } else {

/* -- Read a string ending with an equal sign or line feed */
/*      if (fscanf (filehandle, "%" STRNG(GXL_SPECIESNAME) "[^=\n]",*/
      if (fscanf (filehandle, "%22[^=\n]",
                  name) != 1) {
         rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
      } else {

/* ----- Get rid of trailing spaces */
         name[GXL_SPECIESNAME+1] = '\0';
         length = strlen (name);
         while ((length > 0) && name[length-1] == ' ')
            name[--length] = '\0';
      }
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadSurfaceTemp - read the surface temperature from the configuration file
--------------------------------------------------------------------------------

 GxReadSurfaceTemp reads the temperature at the surface from the configuration
 file.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where the surface temperature should be stored

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadSurfaceTemp (FILE *filehandle,
                                      const char *filename,
                                      GxConfiguration *configuration)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (filehandle);
   assert (filename);
   assert (configuration);

/* Read a double precision number from file skipping over equal sign first */
   rc = GxReadDouble (filehandle, filename,
                      &configuration->surfacetemperature);
   if (!rc) {

/* -- Validate the number read */
      configuration->surfacetemperature += GXC_ZEROCELCIUS;
      if (configuration->surfacetemperature < 0.0)
         rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
   }
   return (rc);
}



/*
--------------------------------------------------------------------------------
 GxReadTemperatureGrad - read temperature gradient from the configuration file
--------------------------------------------------------------------------------

 GxReadTemperatureGrad reads the temperature gradient from the configuration
 file.

 Arguments
 ---------
 filehandle    - handle of file from which data should be read
 filename      - the name of the file
 configuration - configuration where the temperature gradient should be stored

 Return value
 ------------
 GXE_NOERROR or code of the error encountered

--------------------------------------------------------------------------------
*/

static GxErrorCode GxReadTemperatureGrad (FILE *filehandle,
                                          const char *filename,
                                          GxConfiguration *configuration)
{
   GxErrorCode  rc=GXE_NOERROR;

   assert (filehandle);
   assert (filename);
   assert (configuration);

/* Read a double precision number from file skipping over equal sign first */
   rc = GxReadDouble (filehandle, filename,
                      &configuration->temperaturegradient);

/* Validate the number read */
   if (!rc && (configuration->temperaturegradient <= 0.0))
      rc = GxError (GXE_INVCONFIGFILE, GXT_INVCONFIGFILE, filename);
   return (rc);
}
