/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_util/src/ibsgx.h,v 25.0 2005/07/05 08:03:57 ibs Stable $ */
/* ================================================================== */
/*
 * $Log: ibsgx.h,v $
 * Revision 25.0  2005/07/05 08:03:57  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.1  2004/10/04 14:42:47  ibs
 * First time in.
 *
 * Revision 24.1  2004/08/11 11:15:00  nlome0
 * Code Enhancement: A VES history can now be supplyed to the GENEX project and kernel via an array of float in the gxsloc function
 *
 * Revision 24.0  2004/04/15 13:11:19  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:50  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:54  ibs
 * RS6000 relkease
 *
 * Revision 21.1  2001/06/07 11:45:09  ibs
 * moved the Log keyword to keep RCS happy.
 *
 * Revision 21.0  2001/05/02 08:59:40  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:53:03  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:18:27  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:37  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:54  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:42  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:50:27  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.1  1997/09/18  15:14:52  kspre0
 * Modif used by XMIG to get more species.
 *
 * Revision 14.0  1997/09/16  18:11:44  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:06:49  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:57:17  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:42:52  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.3  1996/02/12  11:30:58  ibs
 * The default values for Default Pressure and Area size changed
 * to be in line with genex4 stand alone program.
 * Pressure to 0.0; area to 100,000,000.000
 *
 * Revision 9.2  1996/01/04  15:33:00  ibs
 * cplusplus corrections added.
 *
 * Revision 9.1  1995/11/20  13:09:05  ibs
 * Routines to fill and access the genex4 kernel.
 * Original in libgui.
 * */
/* ================================================================== */
/*
Author:		E. de Koster (BSO/AT Zoetermeer bv)

Date:		22-SEP-1993

Description:	include file for the incorporation of GENEX-3 in IBS.

		ibsgx.h contains prototypes, typedefs and macro
		definitions for the incorporation of GENEX-3 in IBS.

History
-------
23-SEP-1993	ErKo	initial version
24-SEP-1993	ErKo	- added SRHORIZON, SUBAREA and RUNOPTIONS macro's
			- added prototype for gxgsrs, gxggrs
27-SEP-1993	ErKo	- added IBSGXE_PROJECTNOTCLOSED, IBSGXE_COMMITERROR,
			  IBSGXE_RUNMODELERROR, IBSGXE_UNKNOWNSPECIESNUMBER
			- added prototype for gxrun
                        - added 'FORTRAN' constants to identify output segments,
                          species groups, true and false values
                        - added a number of ROCK macro's
28-SEP-1993	ErKo	- added horizonhistory macro's
29-SEP-1993	ErKo	- added IBSGXE_MEMORYALLOCERROR
			- updated and added function prototypes
30-SEP-1993	ErKo	changed GENEX access macro's from
			'pointer to structs' into 'direct struct'
			For example:      #define TYPE_field(pType) pType->field
			is replaced with: #define TYPE_field(sType) sType.field
 1-OCT-1993	ErKo	added prototypes for IbsGetVRE, IbsGetExpAPI and
			IbsGetCnvrs
 6-OCT-1993	ErKo	- updated prototypes due to the removal of the TR2
			  output segment and the renaming of the TR1 output
			  segment to TRG
			- added SRHorizon_Age macro
			- updates prototype of gxspar to accept age of horizon
 7-OCT-1993	ErKo	added IBS_UNKNOWN constant
11-OCT-1993	ErKo	added pSTRUCT_... #define statements to access
			structure members using pointers to the structure
31-MAR-1994	ErKo	- added macro's to access N/C ratio
			- updated function prototypes
			- added N2_SPECIES
20-APR-1994	ErKo	added gxkerr and IbsGxErrHandler
--------------------------------------------------------------------------------
*/

#ifndef   IBSGX_H
#define   IBSGX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gxgenerl.h"

/* Include files */
/* ------------- */

/* Macro definitions */
/* ----------------- */
/* ----- Lithilogy macro's ----- */
#define pLITHOLOGY_LithologyName(pLithology) pLithology->name
#define sLITHOLOGY_LithologyName(sLithology) sLithology.name

#define pLITHOLOGY_TOCContent(pLithology)    pLithology->toci
#define sLITHOLOGY_TOCContent(sLithology)    sLithology.toci

#define pLITHOLOGY_InitREIIS1(pLithology)    pLithology->s1i
#define sLITHOLOGY_InitREIIS1(sLithology)    sLithology.s1i

#define pLITHOLOGY_InitREIIS2(pLithology)    pLithology->s2i
#define sLITHOLOGY_InitREIIS2(sLithology)    sLithology.s2i

#define pLITHOLOGY_InitREIIS3(pLithology)    pLithology->s3i
#define sLITHOLOGY_InitREIIS3(sLithology)    sLithology.s3i

#define pLITHOLOGY_AtomHCKerogen(pLithology) pLithology->hci
#define sLITHOLOGY_AtomHCKerogen(sLithology) sLithology.hci

#define pLITHOLOGY_AtomOCKerogen(pLithology) pLithology->oci
#define sLITHOLOGY_AtomOCKerogen(sLithology) sLithology.oci

#define pLITHOLOGY_AtomNCKerogen(pLithology) pLithology->nci
#define sLITHOLOGY_AtomNCKerogen(sLithology) sLithology.nci

#define pLITHOLOGY_SCEAsphalt(pLithology)    pLithology->asphaltenelowact
#define sLITHOLOGY_SCEAsphalt(sLithology)    sLithology.asphaltenelowact

#define pLITHOLOGY_ECEAsphalt(pLithology)    pLithology->asphaltenehighact
#define sLITHOLOGY_ECEAsphalt(sLithology)    sLithology.asphaltenehighact

#define pLITHOLOGY_SCEKerogen(pLithology)    pLithology->kerogenlowact
#define sLITHOLOGY_SCEKerogen(sLithology)    sLithology.kerogenlowact

#define pLITHOLOGY_ECEKerogen(pLithology)    pLithology->kerogenhighact
#define sLITHOLOGY_ECEKerogen(sLithology)    sLithology.kerogenhighact

#define pLITHOLOGY_LowerBiot(pLithology)     pLithology->lowerbiot
#define sLITHOLOGY_LowerBiot(sLithology)     sLithology.lowerbiot

#define pLITHOLOGY_UpperBiot(pLithology)     pLithology->upperbiot
#define sLITHOLOGY_UpperBiot(sLithology)     sLithology.upperbiot

#define pLITHOLOGY_CharLength(pLithology)    pLithology->charlength
#define sLITHOLOGY_CharLength(sLithology)    sLithology.charlength

/* ----- SR Horizon macro's ----- */
#define pSRHORIZON_HorizonName(pSRHorizon) pSRHorizon->name
#define sSRHORIZON_HorizonName(sSRHorizon) sSRHorizon.name

#define pSRHORIZON_Age(pSRHorizon)         pSRHorizon->age
#define sSRHORIZON_Age(sSRHorizon)         sSRHorizon.age

/* ----- SR Horizon history macro's ----- */
#define pHORIZONHISTORY_SubAreaName(pHorizonHistory) pHorizonHistory->subarea
#define sHORIZONHISTORY_SubAreaName(sHorizonHistory) sHorizonHistory.subarea

#define pHORIZONHISTORY_HorizonName(pHorizonHistory) pHorizonHistory->horizon
#define sHORIZONHISTORY_HorizonName(sHorizonHistory) sHorizonHistory.horizon

#define pHORIZONHISTORY_NValues(pHorizonHistory)     pHorizonHistory->ntimes
#define sHORIZONHISTORY_NValues(sHorizonHistory)     sHorizonHistory.ntimes

/* ----- Subarea macoro's ----- */
#define pSUBAREA_SubAreaName(pSubArea) pSubArea->name
#define sSUBAREA_SubAreaName(sSubArea) sSubArea.name

#define pSUBAREA_AreaSize(pSubArea)    pSubArea->area
#define sSUBAREA_AreaSize(sSubArea)    sSubArea.area

/* ----- Rock macro's ----- */
#define pROCK_LithologyName(pRock) pRock->lithology
#define sROCK_LithologyName(sRock) sRock.lithology

#define pROCK_SubAreaName(pRock)   pRock->subarea
#define sROCK_SubAreaName(sRock)   sRock.subarea

#define pROCK_HorizonName(pRock)   pRock->horizon
#define sROCK_HorizonName(sRock)   sRock.horizon

#define pROCK_Thickness(pRock)     pRock->thickness
#define sROCK_Thickness(sRock)     sRock.thickness

/* ----- Run options macro's ----- */
#define pRUNOPTIONS_NSlices(pRunOptions)    pRunOptions->nnodes
#define sRUNOPTIONS_NSlices(sRunOptions)    sRunOptions.nnodes

#define pRUNOPTIONS_NTimeSteps(pRunOptions) pRunOptions->ntimesteps
#define sRUNOPTIONS_NTimeSteps(sRunOptions) sRunOptions.ntimesteps

/* Type definitions */
/* ---------------- */

/* Function prototypes */
/* ------------------- */
#ifdef RS6000
   int gxexit
#else
   int gxexit_
#endif
      (void)
;
/* ------------------------------------------------------------------------ */
#ifdef RS6000
   int gxggrs
#else
   int gxggrs_
#endif
      (int *   NOutputTimeValues, /* I/O Value :
                                     If NOutputTimeValues equals 0:
                                     delta time is used
                                   */
       float * OutputTimeValues,  /* I/O Array :
                                     Should be large enough to hold:
                                     - <NOutputTimeValues>
                                       if <NOutputTimeValues> != 0
                                     OR
                                     - (StartTime - EndTime) / DeltaTime values
                                       if <NOutputTimeValues> == 0
                                   */
       float * OutputStartTime,   /* I   Value */
       float * OutputEndTime,     /* I   Value */
       float * OutputDeltaTime,   /* I   Value */
       float * OutputXGOValue,    /*   O Value : Expulsion Gas-Oil ratio */
       float * OutputXCGValue,    /*   O Value : Expulsion Condesate-Gas ratio */
       float * OutputTRGValues,   /*   O Array : Kerogen conversion ratio's */
       float * OutputAPIValues,   /*   O Array : Expulsion API's */
       float * OutputVREValues)   /*   O Array : Maturity %'s */
;
/* ------------------------------------------------------------------------ */
#ifdef RS6000
   int gxgloc
#else
   int gxgloc_
#endif
      (float * Thickness,
       float * InitTOCContent,
       float * UpperBiot,
       float * LowerBiot,
       float * CharLength,
       float * InitREIIS1,
       float * InitREIIS2,
       float * InitREIIS3,
       float * AtomHCKerogen,
       float * AtomOCKerogen,
       float * AtomNCKerogen,
       float * StartCrackEnergyAsphalt,
       float * EndCrackEnergyAsphalt,
       float * StartCrackEnergyKerogen,
       float * EndCrackEnergyKerogen)
;
/* ------------------------------------------------------------------------ */
#ifdef RS6000
   int gxgpar
#else
   int gxgpar_
#endif
      (int * NSlices,
       int * NGxTimeSteps)
;
/* ------------------------------------------------------------------------ */
#ifdef RS6000
   int gxgsrs
#else
   int gxgsrs_
#endif
      (int *   NOutputTimeValues, /* I/O Value :
                                     If NOutputTimeValues equals 0:
                                     delta time is used
                                   */
       float * OutputTimeValues,  /* I/O Array :
                                     Should be large enough to hold:
                                     - <NOutputTimeValues>
                                       if <NOutputTimeValues> != 0
                                     OR
                                     - (StartTime - EndTime) / DeltaTime values
                                       if <NOutputTimeValues> == 0
                                   */
       float * OutputStartTime,   /* I   Value */
       float * OutputEndTime,     /* I   Value */
       float * OutputDeltaTime,   /* I   Value */
       int *   OutputSpecies,     /* I   Value : Species to get results for */
       int *   OutputSegments,    /* I   Array : Segments to get results for */
       float * OutputGCValues,    /*   O Array : Generated cumulative */
       float * OutputGRValues,    /*   O Array : Generated rate */
       float * OutputXCValues,    /*   O Array : Expelled cumulative */
       float * OutputXRValues,    /*   O Array : Expulsion rate */
       float * OutputUCValues,    /*   O Array : Expelled upward cumulative */
       float * OutputDCValues,    /*   O Array : Expelled downward cumulative */
       float * OutputURValues,    /*   O Array : Expulsion upward rate */
       float * OutputDRValues)    /*   O Array : Expulsion downward rate */
;
/* ------------------------------------------------------------------------ */
#ifdef RS6000
   int gxinit
#else
   int gxinit_
#endif
      (char * ProjectFileName,
       int *  ProjectFileNameLength)
;
/* ------------------------------------------------------------------------ */
#ifdef RS6000
   void gxkerr
#else
   void gxkerr_
#endif
      (char * sExpression)
;
/* ------------------------------------------------------------------------ */
#ifdef RS6000
   int gxrun
#else
   int gxrun_
#endif
      (void)
;
/* ------------------------------------------------------------------------ */
#ifdef RS6000
   int gxsloc
#else
   int gxsloc_
#endif
      (float * Thickness,               /* I Value */
       float * InitTOCContent,          /* I Value */
       float * UpperBiot,               /* I Value */
       float * LowerBiot,               /* I Value */
       float * CharLength,              /* I Value */
       float * InitREIIS1,              /* I Value */
       float * InitREIIS2,              /* I Value */
       float * InitREIIS3,              /* I Value */
       int *   EstimateAtomicRatios,    /* I Value */
       float * AtomHCKerogen,           /* I Value */
       float * AtomOCKerogen,           /* I Value */
       float * AtomNCKerogen,           /* I Value */
       float * StartCrackEnergyAsphalt, /* I Value */
       float * EndCrackEnergyAsphalt,   /* I Value */
       float * StartCrackEnergyKerogen, /* I Value */
       float * EndCrackEnergyKerogen,   /* I Value */
       int *   NHistoryValues,          /* I Value */
       float * TimeValues,              /* I Array */
       float * DepthValues,             /* I Array */
       float * TempValues,              /* I Array */
       float * PressureValues)          /* I Array */
;
/* ------------------------------------------------------------------------ */
#ifdef RS6000
   int gxspar
#else
   int gxspar_
#endif
      (int *   NSlices,
       int *   NGxTimeSteps,
       float * HorizonAge)              /* I Value */
;
/* ------------------------------------------------------------------------ */
GxErrorCode IbsGetExpAPI
   (double         StartTimeFullRange,
    double         EndTimeFullRange,
    int            NFullRangeValues,
    double *       FullRangeTimeValues,
    double *       FullRangeAPIResults,
    int            NOutputTimeValues,
    float *        OutputTimeValues,
    float *        OutputAPIResults)
;
/* ------------------------------------------------------------------------ */
GxErrorCode IbsGetCnvrs
   (double         StartTimeFullRange,
    double         EndTimeFullRange,
    int            NFullRangeValues,
    double *       FullRangeTimeValues,
    double *       FullRangeKerogenResults,
    double *       FullRangeAsphalteneResults,
    int            NOutputTimeValues,
    float *        OutputTimeValues,
    float *        OutputKerogenResults)
;
/* ------------------------------------------------------------------------ */
GxErrorCode IbsGetExpMC
   (GxBool         IndividualSpecies,
    GxSpeciesGroup SpeciesGroup,
    char *         SpeciesName,
    GxUpDown       UpAndOrDown,
    double         StartTimeFullRange,
    double         EndTimeFullRange,
    int            NFullRangeValues,
    double *       FullRangeTimeValues,
    double *       FullRangeResults,
    int            NOutputTimeValues,
    float *        OutputTimeValues,
    float *        OutputResults)
;
/* ------------------------------------------------------------------------ */
GxErrorCode IbsGetExpR
   (GxBool         IndividualSpecies,
    GxSpeciesGroup SpeciesGroup,
    char *         SpeciesName,
    GxUpDown       UpAndOrDown,
    double         StartTimeFullRange,
    double         EndTimeFullRange,
    int            NFullRangeValues,
    double *       FullRangeTimeValues,
    double *       FullRangeResults,
    int            NOutputTimeValues,
    float *        OutputTimeValues,
    float *        OutputResults)
;
/* ------------------------------------------------------------------------ */
GxErrorCode IbsGetGenMC
   (GxBool         IndividualSpecies,
    GxSpeciesGroup SpeciesGroup,
    char *         SpeciesName,
    double         StartTimeFullRange,
    double         EndTimeFullRange,
    int            NFullRangeValues,
    double *       FullRangeTimeValues,
    double *       FullRangeResults,
    int            NOutputTimeValues,
    float *        OutputTimeValues,
    float *        OutputResults)
;
/* ------------------------------------------------------------------------ */
GxErrorCode IbsGetGenR
   (GxBool         IndividualSpecies,
    GxSpeciesGroup SpeciesGroup,
    char *         SpeciesName,
    double         StartTimeFullRange,
    double         EndTimeFullRange,
    int            NFullRangeValues,
    double *       FullRangeTimeValues,
    double *       FullRangeResults,
    int            NOutputTimeValues,
    float *        OutputTimeValues,
    float *        OutputResults)
;
/* ------------------------------------------------------------------------ */
GxErrorCode IbsGetVRE
   (double         StartTimeFullRange,
    double         EndTimeFullRange,
    int            NFullRangeValues,
    double *       FullRangeTimeValues,
    double *       FullRangeVREResults,
    int            NOutputTimeValues,
    float *        OutputTimeValues,
    float *        OutputVREResults)
;
/* ------------------------------------------------------------------------ */
GxErrorCode IbsGxCfgPrjct
   (void)
;
/* ------------------------------------------------------------------------ */

void IbsGxErrHandler
   (GxErrorCode ErrCode,
    const char *      ErrMessage)
;
/* ------------------------------------------------------------------------ */

/* Constant values */
/*-----------------*/
/* Error codes */
#define IBSGXE_NOERROR               ((int)  0)
#define IBSGXE_GENEXNOTINIT          ((int)  1)
#define IBSGXE_PROJECTNOTOPEN        ((int)  2)
#define IBSGXE_CONFIGERROR           ((int)  3)
#define IBSGXE_GETDATAERROR          ((int)  4)
#define IBSGXE_SETDATAERROR          ((int)  5)
#define IBSGXE_GENEXNOTCLOSED        ((int)  6)
#define IBSGXE_COMMITERROR           ((int)  7)
#define IBSGXE_RUNMODELERROR         ((int)  8)
#define IBSGXE_UNKNOWNSPECIESNUMBER  ((int)  9)
#define IBSGXE_TRANSLATERESULTSERROR ((int) 10)
#define IBSGXE_MEMORYALLOCERROR      ((int) 11)
#define IBSGXE_MEMORYFREEERROR       ((int) 12)

/* Default values */
#define SUBAREA_SIZE      ((double) 100000000.0) /* 1 m2 */
#define HORIZON_THICKNESS ((double) 1.0) /* 1 m */
#define DEFAULT_PRESSURE  ((double) 0.0) /* 1 Pa */

/* Standard names for GENEX data items */
#define IBS_GX_SUBAREA   "Sarea"
#define IBS_GX_SRHORIZON "SRhori"
#define IBS_GX_LITHOLOGY "SRLitho"

/* Constants to identify species (groups) from FORTRAN */
#define OIL_SPECIESGROUP     ((int) 1)
#define HC_GAS_SPECIESGROUP  ((int) 2)
#define DRY_GAS_SPECIESGROUP ((int) 3)
#define WET_GAS_SPECIESGROUP ((int) 4)
#define COX_SPECIESGROUP     ((int) 5)
#define N2_SPECIES           ((int) 6)
#define OILAH_SPECIES        ((int) 7)
#define OILAM_SPECIES        ((int) 8)
#define OILAL_SPECIES        ((int) 9)
#define OILPH_SPECIES        ((int) 10)
#define OILPM_SPECIES        ((int) 11)
#define OILPL_SPECIES        ((int) 12)
#define C5_SPECIES           ((int) 13)
#define C4_SPECIES           ((int) 14)
#define C3_SPECIES           ((int) 15)
#define C2_SPECIES           ((int) 16)
#define C1_SPECIES           ((int) 17)
#define COX_SPECIES          ((int) 18)

/* Constants to identify output segments from FORTRAN (In FORTRAN use +1) */
#define OUTSEG_GEN_CUM_TOTAL      ((int) 0) /* cumulative generated */
#define OUTSEG_GEN_RATE           ((int) 1) /* generated rate */
#define OUTSEG_EXP_UP_CUM_TOTAL   ((int) 2) /* cumulative expelled upward */
#define OUTSEG_EXP_DOWN_CUM_TOTAL ((int) 3) /* cumulative expelled downward */
#define OUTSEG_EXP_UP_RATE        ((int) 4) /* expelled upward rate */
#define OUTSEG_EXP_DOWN_RATE      ((int) 5) /* expelled downward rate */
#define OUTSEG_EXP_CUM_TOTAL      ((int) 6) /* cumulative expelled */
#define OUTSEG_EXP_RATE           ((int) 7) /* expelled rate */

/* Constant identifying FORTRAN True or False */
#define FORTRAN_TRUE ((int) 1)
#define FORTRAN_FALSE ((int) 0)

/* Constant representing IBS UNKNOW value */
#define IBS_UNKNOWN ((double) 1.0E-31)


#ifdef __cplusplus
}
#endif

#endif
