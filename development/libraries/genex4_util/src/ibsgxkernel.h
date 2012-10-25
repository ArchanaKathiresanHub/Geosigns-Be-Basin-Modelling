/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_util/src/ibsgxkernel.h,v 25.0 2005/07/05 08:03:57 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: ibsgxkernel.h,v $
 * Revision 25.0  2005/07/05 08:03:57  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.1  2004/10/04 14:42:48  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:19  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:51  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:56  ibs
 * RS6000 relkease
 *
 * Revision 21.1  2001/06/07 11:45:15  ibs
 * moved the Log keyword to keep RCS happy.
 *
 * Revision 17.0  1998/12/15 14:26:57  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:44  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:50:32  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.2  1997/09/18  15:14:18  kspre0
 * put back the file log history.
 *
 * Revision 14.1  1997/09/18  15:10:37  kspre0
 * Modif used by XMIG to get more species.
 *
 * Revision 14.0  1997/09/16  18:11:44  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:07:09  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:57:30  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:43:09  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.3  1996/01/04  15:33:09  ibs
 * cplusplus corrections added.
 *
 * Revision 9.2  1995/11/21  08:51:41  ibs
 * Extended the items for recognising species : C1-C5  etc.
 *
 * Revision 9.1  1995/11/20  13:09:10  ibs
 * Routines to fill and access the genex4 kernel.
 * Original in libgui.
 * */
/* ================================================================== */
/*

Author:		E. de Koster (BSO/AT Zoetermeer bv)

Date:		19-JUN-1992

Description:	include file for the IBS extensions to Genex kernel functions.

		ibsgxkernel.h contains prototypes, typedefs and macro
		definitions for the extension of the high level to Genex
		functions created for IBS.

History
-------
21-SEP-1993	ErKo	initial version: GxSpeciesGroup and ...GroupExpulsion
			prototype
22-SEP-1993	ErKo	added GX_SPECIES... constants and
			GxIsSpecies... prototypes
27-SEP-1993	ErKo	added GxIsInit, GxSetInit, GxClearInit
 7-OCT-1993	ErKo	added GxCalcHCI and GxCalcOCI
17-DEC-1993     PJH     Changed GxIsSpeciesOil, GxIsSpeciesHCGas,
                        to GxIsSpeciesOilNam, GxIsSpeciesHCGasNam
31-MAR-199	ErKo	added GX_N2_SPECIESNAME
--------------------------------------------------------------------------------
*/

#ifndef   IBSGXKERNEL_H
#define   IBSGXKERNEL_H

#ifdef __cplusplus
extern "C" {
#endif



/* Include files */
/* ------------- */
#include  "gxkernel.h"

/* Macro definitions */
/* ----------------- */
/* -- species group definitions -- */
typedef enum {
   GX_OIL_SPECIES,
   GX_HC_GAS_SPECIES,
   GX_DRY_GAS_SPECIES,
   GX_WET_GAS_SPECIES,
   GX_COX_SPECIES
} GxSpeciesGroup;

#define GX_N2_SPECIESNAME    "N2"
#define GX_OILAH_SPECIESNAME "oilAH"
#define GX_OILAM_SPECIESNAME "oilAM"
#define GX_OILAL_SPECIESNAME "oilAL"
#define GX_OILPH_SPECIESNAME "oilPH"
#define GX_OILPM_SPECIESNAME "oilPM"
#define GX_OILPL_SPECIESNAME "oilPL"
#define GX_C5_SPECIESNAME    "C5"
#define GX_C4_SPECIESNAME    "C4"
#define GX_C3_SPECIESNAME    "C3"
#define GX_C2_SPECIESNAME    "C2"
#define GX_C1_SPECIESNAME    "C1"
#define GX_COX_SPECIESNAME   "COx"

/* Type definitions */
/* ---------------- */

/* Function prototypes */
/* ------------------- */
double GxCalcHCI
   (double S2,
    double S3,
    double TOC)
;
/* -------------------------------------------------------------------------- */
double GxCalcOCI
   (double S3,
    double TOC)
;
/* -------------------------------------------------------------------------- */
double GxCalcS2
   (double HCI,
    double OCI,
    double TOC)
;
/* -------------------------------------------------------------------------- */
double GxCalcS3
   (double OCI,
    double TOC)
;
/* -------------------------------------------------------------------------- */
GxErrorCode GxClearInit
   (void)
;
/* -------------------------------------------------------------------------- */
GxErrorCode GxGetGeologicalGroupExpulsions 
   (char *         SubAreaName,
    char *         HorizonName,
    char *         LithologyName,
    GxSpeciesGroup SpeciesGroup,
    GxUpDown       UpAndOrDown,
    GxCumulative   NotOrCumulative,
    double         BeginTime,
    double         EndTime,
    double *       GroupExpulsions)
;
/* -------------------------------------------------------------------------- */
GxErrorCode GxGetGeologicalGroupFluxes 
   (char *         SubAreaName,
    char *         HorizonName,
    char *         LithologyName,
    GxSpeciesGroup SpeciesGroup,
    GxUpDown       UpAndOrDown,
    double         BeginTime,
    double         EndTime,
    double *       GroupFluxes)
;
/* -------------------------------------------------------------------------- */
GxErrorCode GxGetGeologicalGroupGenCumRates
   (char *         SubAreaName,
    char *         HorizonName,
    char *         LithologyName,
    GxSpeciesGroup SpeciesGroup,
    double         BeginTime,
    double         EndTime,
    double *       GroupCumRates)
;
/* -------------------------------------------------------------------------- */
GxErrorCode GxGetGeologicalGroupGenRates
   (char *         SubAreaName,
    char *         HorizonName,
    char *         LithologyName,
    GxSpeciesGroup SpeciesGroup,
    double         BeginTime,
    double         EndTime,
    double *       GroupRates)
;
/* -------------------------------------------------------------------------- */
GxBool GxIsInit
   (void)
;
/* -------------------------------------------------------------------------- */
GxBool GxIsSpeciesCOx
   (char * SpeciesName)
;
/* -------------------------------------------------------------------------- */
GxBool GxIsSpeciesDryGas
   (char * SpeciesName)
;
/* -------------------------------------------------------------------------- */
GxBool GxIsSpeciesHCGasNam
   (char * SpeciesName)
;
/* -------------------------------------------------------------------------- */
GxBool GxIsSpeciesOilNam
   (char * SpeciesName)
;
/* -------------------------------------------------------------------------- */
GxBool GxIsSpeciesWetGas
   (char * SpeciesName)
;
/* -------------------------------------------------------------------------- */
GxErrorCode GxSetInit
   (void)
;
/* -------------------------------------------------------------------------- */

/* Constant values */
/*-----------------*/
   /* These constants are used to determine the species types. They are required
      because the GENEX kernel is not (yet) able to determine whether a species
      is member of a set of species. If the kernel is extended with the function
      which are able to determine whether a species is memeber of a group of
      species these constants can be removed. See the GxIsSpecies... functions
      listed above.
      The _1 is there to ensure that more than one species can be defined as
      a member of a group.
   */
#define GX_SPECIES_COX_1       "COx"
#define GX_SPECIES_DRY_GAS_1   "CH4"
#define GX_SPECIES_DRY_GAS_2   "C1"
#define GX_SPECIES_WET_GAS_1   "C2"
#define GX_SPECIES_WET_GAS_2   "C3"
#define GX_SPECIES_WET_GAS_3   "C4"
#define GX_SPECIES_WET_GAS_4   "C5"


#ifdef __cplusplus
}
#endif

#endif

