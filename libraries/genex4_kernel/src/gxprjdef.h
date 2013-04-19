/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxprjdef.h,v 25.0 2005/07/05 08:03:49 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxprjdef.h,v $
 * Revision 25.0  2005/07/05 08:03:49  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.1  2004/10/04 14:53:32  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:13  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:45  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:32  ibs
 * RS6000 relkease
 *
 * Revision 21.1  2001/06/07 09:46:12  ibs
 * moved the Log keyword to keep RCS happy.
 *
 * Revision 21.0  2001/05/02 08:59:27  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:43  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:55  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:19  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:16  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:20  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:56  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:44  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:04:07  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:51  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:42:18  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.3  1996/01/04  15:18:02  ibs
 * cplusplus corrections added.
 *
 * Revision 9.2  1995/10/30  09:53:34  ibs
 * Add headers.
 * */
/* ================================================================== */
#ifndef   GXPRJDEF_H
#define   GXPRJDEF_H

#ifdef __cplusplus
extern "C" {
#endif
    

/*
--------------------------------------------------------------------------------
 gxprjdef.h   include file with general Genex project definitions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        19-JUN-1992

 Description: gxprjdef.h contains type and macro definitions for use in the
              project management part of Genex.

 History
 -------
 19-JUN-1992  P. Alphenaar  initial version

--------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  "gxgenerl.h"

/* Macro definitions */
/* ----------------- */
/* -- general project error definitions -- */
enum {
  GXE_LINK           = GXBE_PRJDEF | GXF_APPLICATION | GXF_ERROR | 1,
  GXE_LITHOLOGYNOTPRESENT, GXE_LOCATIONNOTPRESENT, GXE_RVHORIZONNOTPRESENT,
  GXE_SRHORIZONNOTPRESENT, GXE_SUBAREANOTPRESENT,
  GXE_INVALIDPROJECT = GXBE_PRJDEF | GXF_USER        | GXF_ERROR | 11
};
#define  GXT_INVALIDPROJECT       "The file %s is not a valid project file.\n"
#define  GXT_LINK                 "The internal project links " \
                                  "have been corrupted.\n"
#define  GXT_LITHOLOGYNOTPRESENT  "Lithology %s is not present in project.\n"
#define  GXT_LOCATIONNOTPRESENT   "Location %s is not present in project.\n"
#define  GXT_SUBAREANOTPRESENT    "Subarea %s is not present in project.\n"
#define  GXT_RVHORIZONNOTPRESENT  "Reservoir horizon %s is not present " \
                                  "in project.\n"
#define  GXT_SRHORIZONNOTPRESENT  "Source rock horizon %s is not present " \
                                  "in project.\n"
/* -- maximum lengths and sizes -- */
#define  GXN_DATAUNITS              12
#define  GXL_HORIZONNAME            8
#define  GXL_LITHOLOGYNAME          8
#define  GXL_LOCATIONID             20
#define  GXL_SUBAREANAME            8
#define  GXL_TRAPNAME               8

/* Type definitions */
/* ---------------- */
typedef  struct GxDataItem          GxDataItem;
typedef  struct GxFile              GxFile;
typedef  struct GxHorizon           GxHorizon;
typedef  struct GxHorizonHistory    GxHorizonHistory;
typedef  struct GxLabExperiment     GxLabExperiment;
typedef  struct GxLithology         GxLithology;
typedef  struct GxLocation          GxLocation;
typedef  struct GxRefConditions     GxRefConditions;
typedef  struct GxReservoirSubArea  GxReservoirSubArea;
typedef  struct GxRock              GxRock;
typedef  struct GxRunOptions        GxRunOptions;
typedef  struct GxSubArea           GxSubArea;
typedef  struct GxTrap              GxTrap;
typedef  struct GxGasComposition    GxGasComposition;
typedef  enum {
   GX_FILE, GX_HORIZON, GX_HORIZONHISTORY, GX_LABEXPERIMENT, GX_LITHOLOGY,
   GX_LOCATION, GX_REFCONDITIONS, GX_RESERVOIRSUBAREA, GX_ROCK, GX_RUNOPTIONS,
   GX_SUBAREA, GX_TRAP
} GxDataUnit;
typedef  enum {
   GX_RESERVOIR, GX_SOURCEROCK
} HorizonType;
typedef  enum {
   GX_FIELD, GX_PROSPECT
} TrapType;
struct GxDataItem {
   int  seqnr;
};
struct GxFile {
   GxDataItem  dataitem;
   char        name[GXL_FILENAME+1];
   GxLocation  *location;
   int         locationnr;
};
struct GxGasComposition {
   double  molpercentage;
   double  fractionation;
};
struct GxHorizon {
   GxDataItem   dataitem;
   char         name[GXL_HORIZONNAME+1];
   HorizonType  type;
   double       age;
};
struct GxHorizonHistory {
   GxDataItem  dataitem;
   GxHorizon   *horizon;
   int         horizonnr;
   GxSubArea   *subarea;
   int         subareanr;
   int         ntimes;
   double      **physprop;
};
struct GxLabExperiment {
   GxDataItem  dataitem;
   char        name[GXL_LABEXPERIMENTNAME+1];
   double      charlength, lowerbiot, netthickness, upperbiot;
   int         ntimes;
   double      **physprop;
};
struct GxLocation {
   GxDataItem  dataitem;
   char        name[GXL_LOCATIONID+1];
   double      easting, northing;
   GxFile      *file;
   int         filenr;
   GxSubArea   *subarea;
   int         subareanr;
   int         nhorizons, *ntimes;
   double      ***physprop;
};
struct GxLithology {
   GxDataItem  dataitem;
   char        name[GXL_LITHOLOGYNAME+1];
   double      toci, s1i, s2i, s3i, tmaxi, hci, oci, sci, nci, charlength,
               upperbiot, lowerbiot, kerogenlowact, kerogenhighact,
               asphaltenelowact, asphaltenehighact, initfractionation,
               fractionationfactor;
};
struct GxRefConditions {
   GxDataItem  dataitem;
   double      geolreftemperature, geolrefpressure, geolrefwbo,
               labreftemperature, labrefpressure, labrefwbo;
};
struct GxReservoirSubArea {
   GxDataItem  dataitem;
   GxHorizon   *horizon;
   int         horizonnr;
   GxSubArea   *subarea;
   int         subareanr;
   double      topDepth, grossThickness, netToGross, netAvgPorosity,
               waterSalinity;
};
struct GxRock {
   GxDataItem       dataitem;
   GxLithology      *lithology;
   int              lithologynr;
   GxHorizon        *horizon;
   int              horizonnr;
   GxSubArea        *subarea;
   int              subareanr;
   double           tocm, s1m, s2m, s3m, tmaxm, hcm, ocm, scm, ncm, thickness,
                    vrm, transr;
   GxGasComposition gascomp[GXC_MAX_CARBONNR];
   GxBool           edited;
};
struct GxRunOptions {
   GxDataItem  dataitem;
   int         nnodes, ntimesteps, maxchleniterations;
   double      referenceposition, maxchlenerror, chlenfactor;
};
struct GxSubArea {
   GxDataItem  dataitem;
   char        name[GXL_SUBAREANAME+1];
   GxLocation  *location;
   int         locationnr;
   double      easting, northing, area;
};
struct GxTrap {
   GxDataItem         dataitem;
   char               name[GXL_TRAPNAME+1];
   TrapType           type;
   GxHorizon         *horizon;
   int                horizonnr;
   GxSubArea         *subarea;
   int                subareanr;
   double             formationtime, hwctemperature, hwcpressure, stoiip, giip,
                      oilgravity, gasgravity;
   int                nslices;
   double            *depth, *area;
   int                ntimes;
   double           **physprop;
   GxGasComposition   gascomp[GXC_MAX_CARBONNR];
};
#define  GXN_HISTPHYSPROP   4
enum {
   GX_HISTTIME, GX_HISTDEPTH, GX_HISTTEMPERATURE, GX_HISTPRESSURE
};
#define  GXN_LABPHYSPROP    3
enum {
   GX_LABTIME, GX_LABTEMPERATURE, GX_LABPRESSURE
};
#define  GXN_LOCATPHYSPROP  5
enum {
   GX_LOCATTIME, GX_LOCATDEPTH, GX_LOCATTEMPERATURE, GX_LOCATPRESSURE,
   GX_LOCATVRE
};
#define  GXN_TRAPPHYSPROP   4
enum {
   GX_TRAPTIME, GX_TRAPTEMPERATURE, GX_TRAPPRESSURE, GX_TRAPSTRUCTVOL
};


#ifdef __cplusplus
}
#endif

#endif

