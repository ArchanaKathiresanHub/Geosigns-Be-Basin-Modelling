/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxgenerl.h,v 25.0 2005/07/05 08:03:45 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxgenerl.h,v $
 * Revision 25.0  2005/07/05 08:03:45  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:47:50  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:21  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:08  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:41  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:17  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:17  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:28  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:37  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:06  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:25:51  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:07  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:32  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:10:02  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:01:33  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.1  1997/01/06  09:58:16  ibs
 * file name buffer from 128 to 256
 *
 * Revision 12.0  1996/07/15  14:56:36  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:42:00  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.3  1996/01/04  15:17:33  ibs
 * cplusplus corrections added.
 *
 * Revision 9.2  1995/10/30  09:54:35  ibs
 * Add headers.
 * */
/* ================================================================== */
#ifndef   GXGENERL_H
#define   GXGENERL_H

#ifdef __cplusplus
extern "C" {
#endif
    

/*
--------------------------------------------------------------------------------
 gxgenerl.h   include file with general Genex definitions
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        19-JUN-1992

 Description: gxgenerl.h contains general type and macro definitions for use
              in other Genex header files.

 History
 -------
 19-JUN-1992  P. Alphenaar  initial version
  7-JUL-1992  M.J.A. van Houtert   Errorcodes changed !!! (bug fixing)
 04-JAN-1994  Th. Berkers          In structure GxCharge elements n2inwater, 
                                   n2volume1 and n2volume2 added
 23-SEP-1994  Th. Berkers          GxCharge structure extended with mass data 
                                   for water, oil and gasses
 02-MAR-1995  Th. Berkers          Integration of Isotopes Fractionation
--------------------------------------------------------------------------------
*/

/* Macro definitions */
/* ----------------- */
#define  STRNG(m)            STRNG2(m)
#define  STRNG2(m)           #m
/* -- bit, flag and other conversions -- */
#define  GX_NUMBER(e)        ((e)&0x03FF)
#define  GX_SOURCEMASK(c)    ((c)<<12)
#define  GX_SOURCE(e)        (((e)>>12)&0x03)
#define  GX_SEVERITYMASK(c)  ((c)<<10)
#define  GX_SEVERITY(e)      (((e)>>10)&0x03)
/* -- error source codes and flag definitions -- */
enum {GXC_APPLICATION, GXC_SYSTEM, GXC_TOOLBOX, GXC_USER};
#define  GXF_APPLICATION     GX_SOURCEMASK(GXC_APPLICATION)
#define  GXF_SYSTEM          GX_SOURCEMASK(GXC_SYSTEM)
#define  GXF_TOOLBOX         GX_SOURCEMASK(GXC_TOOLBOX)
#define  GXF_USER            GX_SOURCEMASK(GXC_USER)
/* -- error severity codes and flag definitions -- */
enum {GXC_INFORMATIONAL, GXC_ERROR, GXC_SEVEREERROR, GXC_FATAL};
#define  GXF_ERROR           GX_SEVERITYMASK(GXC_ERROR)
#define  GXF_SEVEREERROR     GX_SEVERITYMASK(GXC_SEVEREERROR)
#define  GXF_FATAL           GX_SEVERITYMASK(GXC_FATAL)
/* -- enumeration of base errors for files where errors can be produced --*/
enum {
  GXBE_CONFIG=100,  GXBE_ERROR=125,   GXBE_FILE=150,     GXBE_GENERL=175,
  GXBE_HIST=200,    GXBE_HORIZN=225,  GXBE_INSTANC=250,  GXBE_KERNEL=275,
  GXBE_LABEXP=300,  GXBE_LITHOL=325,  GXBE_LOCAT=350,    GXBE_MODEL=375,
  GXBE_NUMER=400,   GXBE_OPTION=425,  GXBE_PHCHEM=450,   GXBE_PRJCT=475,
  GXBE_PRJDEF=500,  GXBE_RDFILE=525,  GXBE_RESSUB=550,   GXBE_ROCK=575,
  GXBE_SBAREA=600,  GXBE_SYSDEP=625,  GXBE_TRAP=650,     GXBE_UTILS=657
};
/* -- standard error definitions -- */
enum {
  GXE_NOERROR,
  GXE_ARGUMENT       = GXBE_GENERL | GXF_APPLICATION | GXF_ERROR | 1,
  GXE_INCONSIST, GXE_INTERNWRITE, GXE_STATE, GXE_VARUNDEFINED,
  GXE_MEMORY         = GXBE_GENERL | GXF_SYSTEM      | GXF_ERROR | 11,
  GXE_FILECLOSE, GXE_FILEDELETE, GXE_FILEOPENR, GXE_FILEOPENW, GXE_FILEREAD,
  GXE_FILEWRITE,
  GXE_LIST           = GXBE_GENERL | GXF_TOOLBOX     | GXF_ERROR | 21
};
#define  GXT_ARGUMENT        "Function called with incorrect argument(s),\n" \
                             GXT_FILELINE
#define  GXT_FILELINE        "file %s -- line %d\n"
#define  GXT_FILECLOSE       "Unable to close file %s.\n"
#define  GXT_FILEDELETE      "Unable to delete file %s.\n"
#define  GXT_FILEOPENR       "Unable to open file %s for input.\n"
#define  GXT_FILEOPENW       "Unable to open file %s for output.\n"
#define  GXT_FILEREAD        "An error ocurred reading from file %s.\n"
#define  GXT_FILEWRITE       "An error ocurred writing to file %s.\n"
#define  GXT_INCONSIST       "An inconsistency has been detected in\n" \
                             GXT_FILELINE
#define  GXT_INTERNWRITE     "An error occured during an internal write,\n" \
                             GXT_FILELINE
#define  GXT_LIST            "An error ocurred in the linked list toolbox\n"
#define  GXT_MEMORY          "Unable to allocate block of memory.\n"
#define  GXT_STATE           "Function called in incorrect state,\n" \
                             GXT_FILELINE
#define  GXT_VARUNDEFINED    "Environment variable %s is undefined.\n"
/* -- maximum lengths and sizes -- */
#define  GXL_FILENAME           256
#define  GXL_LABEXPERIMENTNAME  25
#define  GXL_SPECIESNAME        16
/* -- special species sequence numbers -- */
#define  GX_KEROGEN             0
#define  GX_ASPHALTENE          1
#define  GX_C1                  15
/* -- miscellaneous -- */
#define  GX_DIRECTORYVARIABLE   "GENEXDIR"
#define  GXC_MAX_CARBONNR       5
/* -- number and enumeration of elements -- */
#define  GXN_ELEMENTS           5
enum {
  GX_CARBON, GX_HYDROGEN, GX_OXYGEN, GX_NITROGEN, GX_SULFUR
};

/* Type definitions */
/* ---------------- */
typedef  unsigned int             GxErrorCode;
typedef  struct GxCharge          GxCharge;
typedef  struct GxRatio           GxRatio;
typedef  struct GxReactionScheme  GxReactionScheme;
typedef  struct GxSpecies         GxSpecies;
typedef  struct GxTPHistory       GxTPHistory;
typedef  struct GxIsoFract        GxIsoFract; 
typedef  struct GxIsoFactor       GxIsoFactor; 
typedef  enum {
   GxFalse, GxTrue
} GxBool;
typedef enum {
   GX_CUMULATIVE, GX_NOTCUMULATIVE
} GxCumulative;
typedef enum {
   GXF_DRYHOLE, GXF_OILSHOWS, GXF_GASSHOWS, GXF_OILFIELD, GXF_OILGASFIELD,
   GXF_GASFIELD
} GxFieldType;
typedef enum {
   GX_UP, GX_DOWN, GX_UPANDDOWN
} GxUpDown;
typedef enum {
   GX_ISO_FRAC_NOT_SPECIFIED, GX_ISO_FRAC_NONE, GX_ISO_RAYLEIGH_FRAC_KEROGEN, 
   GX_ISO_RAYLEIGH_FRAC_C1, GX_ISO_RAYLEIGH_FRAC_C2C5, GX_ISO_FRAC_HISTORY
} GxIsoFractMethod;
typedef enum {
   GX_VERSION_3, GX_VERSION_4
} GxPosGenexVersion;
struct GxCharge {
   double       watervolume, gasinwater, n2inwater, watermass, gasmassinwater,
                n2massinwater, fluidpressure, overpressure, lithostaticpressure,
                failpressure, oilvolume1, oilvolume2, oilmass1, oilmass2,
                apigravity, gasvolume1, gasvolume2, gasmass1, gasmass2,
                n2volume1, n2volume2, n2mass1, n2mass2, gor1, gor2, cgr,
                saturationgor, saturationcgr;
   GxFieldType  fieldtype;
};
struct GxRatio {
   int     product1, product2;
   GxBool  specified;
   double  ratio;
};
struct GxIsoFactor {
   double    alphalow;
   double    highpercentage;
   double    templow;
   double    temphigh;
   double    constA;
   double    constB;
};
struct GxIsoFract {
   int                nreactions, nspecies;
   GxIsoFractMethod **fractmethod;
   GxIsoFactor        factor;
};
struct GxReactionScheme {
   int          nspecies, nreactions, *nproducts, **product, *nratios;
   GxRatio    **ratio;
};
struct GxSpecies {
   char    name[GXL_SPECIESNAME+1];
   double  composition[GXN_ELEMENTS];
   GxBool  reactive, mobile;
   double  crackingenergy1, crackingenergy2, crackingentropy, crackingvolume,
           order, diffusionenergy, diffusionentropy, diffusionvolume,
           jumplength, formulaweight, density, aromaticity,
           referencediffusivity;
};
struct GxTPHistory {
   int     ntimes;
   double  *time, *temperature, *pressure;
};


#ifdef __cplusplus
}
#endif

#endif

