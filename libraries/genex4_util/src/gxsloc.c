/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_util/src/gxsloc.c,v 25.0 2005/07/05 08:03:55 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxsloc.c,v $
 * Revision 25.0  2005/07/05 08:03:55  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.1  2004/10/04 14:42:44  ibs
 * First time in.
 *
 * Revision 24.2  2004/09/24 08:52:32  ibs
 * Input temperatures reset to near zero when they are zero or lower.
 * Some computations are done, but no real generation of course will result.
 * This fix prevents asserts in the genex code, and will allow(fast)cauldron to continue with funny projects.
 *
 * Revision 24.1  2004/08/11 11:13:57  nlome0
 * Code Enhancement: A VES history can now be supplyed to the GENEX project and kernel via an array of float in the gxsloc function
 *
 * Revision 24.0  2004/04/15 13:11:18  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:49  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:50  ibs
 * RS6000 relkease
 *
 * Revision 21.1  2001/06/07 11:44:55  ibs
 * moved the Log keyword to keep RCS happy.
 *
 * Revision 21.0  2001/05/02 08:59:37  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:59  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:18:21  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:33  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:48  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:37  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:50:17  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:11:28  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:06:13  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:57:15  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:42:48  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.1  1995/11/20  13:08:50  ibs
 * Routines to fill and access the genex4 kernel.
 * Original in libgui.
 * */
/* ================================================================== */
/*

Author:		E. de Koster (BSO/AT Zoetermeer bv)

Date:		22-SEP-1993

Description:	Set the location specific parameters in the GENEX
                datastructures

Usage notes:	

History:
--------
23-SEP-1993	ErKo	initial version
29-SEP-1993	ErKo	included setting of history values
 7-OCT-1993	ErKo	if TOC content, Rock Eval II S2 and Rock Eval II S3
			are not equal to IBS_UNKOWN these values are
			translated into AtomHCKerogen and AtomOCKerogen
 8-OCT-1993	ErKo	- translate fraction to percentage
			- translate energy values
 			- translated temperatures to Kelvin
31-MAR-1994	ErKo	added AtomNCKerogen
------------------------------------------------------------------------------
*/

/* Include files */
/* ------------- */
#include  "gxconsts.h"
#include  "gxgenerl.h"
#include  "gxconfig.h"
#include  "gxerror.h"
#include  "gxhist.h"
#include  "gxlithol.h"
#include  "gxsbarea.h"
#include  "gxrock.h"
#include  "gxkernel.h"
#include  "ibsgxkernel.h"
#include  "ibsgx.h"

/* Prototypes of local functions */
/* ----------------------------- */

/* Local variables */
/* --------------- */

/*
------------------------------------------------------------------------------
gxsloc - Set location specific parameters in the GENEX project and kernel
------------------------------------------------------------------------------

------------------------------------------------------------------------------
*/
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
{
   GxErrorCode      rc;
   int              gxSLocRc;
   Rock             sRock;
   Lithology        sLithology;
   HorizonHistory * pHorizonHistory;
   int              i;
   float            Pressure;

   rc = GXE_NOERROR;
   gxSLocRc = IBSGXE_NOERROR;

/* ----- Set Rock values ----- */
   /* Find Rock */
   rc = GxGetRock (IBS_GX_LITHOLOGY, IBS_GX_SUBAREA, IBS_GX_SRHORIZON, & sRock);
   if (rc != GXE_NOERROR)
   {
      gxSLocRc = IBSGXE_GETDATAERROR;
   }

   if (gxSLocRc == IBSGXE_NOERROR)
   {
      /* Set Rock data */
      sROCK_Thickness (sRock) = (double) *Thickness;

      rc = GxReplaceRock (& sRock);
      if (rc != GXE_NOERROR)
      {
         gxSLocRc = IBSGXE_SETDATAERROR;
      }
   }

/* ----- Set Lithology values ----- */
   if (gxSLocRc == IBSGXE_NOERROR)
   {
      /* Find Lithology */
      rc = GxGetLithology (IBS_GX_LITHOLOGY, & sLithology);
      if (rc != GXE_NOERROR)
      {
         gxSLocRc = IBSGXE_GETDATAERROR;
      }
   }

   if (gxSLocRc == IBSGXE_NOERROR)
   {
      /* Set Lithology data */
/* ---- The Genex3 kernel expects a fraction for TOC ----  */
      sLITHOLOGY_TOCContent    (sLithology) = (double) *InitTOCContent / 100.0;
      sLITHOLOGY_InitREIIS1    (sLithology) = (double) *InitREIIS1;
      sLITHOLOGY_InitREIIS2    (sLithology) = (double) *InitREIIS2;
      sLITHOLOGY_InitREIIS3    (sLithology) = (double) *InitREIIS3;

      /* Determine Atomic values based on Rock Eval II S2 and S3 
         and TOC values */
      if (*EstimateAtomicRatios == FORTRAN_TRUE)
      {
/* ---- The H/C formula expect a percentage for TOC ----   */
         *AtomHCKerogen = (float) GxCalcHCI ( (double) *InitREIIS2,
                                              (double) *InitREIIS3,
                                              (double) *InitTOCContent);
         *AtomOCKerogen = (float) GxCalcOCI ( (double) *InitREIIS3,
                                              (double) *InitTOCContent);
      }
      sLITHOLOGY_AtomHCKerogen (sLithology) = (double) *AtomHCKerogen;
      sLITHOLOGY_AtomOCKerogen (sLithology) = (double) *AtomOCKerogen;
      sLITHOLOGY_AtomNCKerogen (sLithology) = (double) *AtomNCKerogen;


      sLITHOLOGY_SCEAsphalt    (sLithology) = (double) *StartCrackEnergyAsphalt
                                                       * 1000.0;
      sLITHOLOGY_ECEAsphalt    (sLithology) = (double) *EndCrackEnergyAsphalt
                                                       * 1000.0;
      sLITHOLOGY_SCEKerogen    (sLithology) = (double) *StartCrackEnergyKerogen
                                                       * 1000.0;
      sLITHOLOGY_ECEKerogen    (sLithology) = (double) *EndCrackEnergyKerogen
                                                       * 1000.0;
      sLITHOLOGY_LowerBiot     (sLithology) = (double) *LowerBiot;
      sLITHOLOGY_UpperBiot     (sLithology) = (double) *UpperBiot;
      sLITHOLOGY_CharLength    (sLithology) = (double) *CharLength;

      rc = GxReplaceLithology (& sLithology);
      if (rc != GXE_NOERROR)
      {
         gxSLocRc = IBSGXE_SETDATAERROR;
      }
   }

/* ----- Set History values ----- */
   if (gxSLocRc == IBSGXE_NOERROR)
   {
      rc = GxCreateHorizonHistory (& pHorizonHistory);
      if (rc != GXE_NOERROR)
      {
         gxSLocRc = IBSGXE_MEMORYALLOCERROR;
      }
      else
      {
         /* Set identification in Horizon structure */
         strcpy (pHORIZONHISTORY_SubAreaName (pHorizonHistory), IBS_GX_SUBAREA);
         strcpy (pHORIZONHISTORY_HorizonName (pHorizonHistory), IBS_GX_SRHORIZON);
      }
   }

   if (gxSLocRc == IBSGXE_NOERROR)
   {
      /* Find Horizon history */
      rc = GxGetHorizonHistory (IBS_GX_SRHORIZON, IBS_GX_SUBAREA,
                                pHorizonHistory);
      if (rc != GXE_NOERROR)
      {
         gxSLocRc = IBSGXE_GETDATAERROR;
      }
   }

   if (gxSLocRc == IBSGXE_NOERROR)
   {
      /* remove old items (in reverse order) */
      for (i = pHORIZONHISTORY_NValues (pHorizonHistory); i > 0; i--)
      {
         rc = GxDeleteHorizonHistoryItem (pHorizonHistory, (i-1));
         if (rc != GXE_NOERROR)
         {
            gxSLocRc = IBSGXE_SETDATAERROR;
         }
      }

      /* insert new items */
      for (i = 0; i < *NHistoryValues; i++)
      {

         if (PressureValues != NULL)
         {
            /* pressure available: insert value */
            Pressure = PressureValues[i];
         }
         else
         {
            /* no pressure available: insert default value.
	       pressure will be evaluated by GENEX at the start of geological run*/
            Pressure = DEFAULT_PRESSURE;
         }
      /* adjust zero or lower celcius temperature. reset to near zero.... No more asserts later on */
         if ( TempValues[i]<0.0001 )
         {
           TempValues[i] = 0.0001;
         }

         rc = GxInsertHorizonHistoryItem (pHorizonHistory,
                                          (double) TimeValues [i],
                                          (double) DepthValues [i],
                                          (double) (TempValues [i] + GXC_ZEROCELCIUS),
                                          (double) Pressure);

         if (rc != GXE_NOERROR)
         {
            gxSLocRc = IBSGXE_SETDATAERROR;
         }
      }

      if (gxSLocRc == IBSGXE_NOERROR)
      {
         rc = GxReplaceHorizonHistory (pHorizonHistory);
         if (rc != GXE_NOERROR)
         {
            gxSLocRc = IBSGXE_SETDATAERROR;
         }
      }

      rc = GxFreeHorizonHistory (pHorizonHistory);
      if (rc != GXE_NOERROR)
      {
         gxSLocRc = IBSGXE_MEMORYFREEERROR;
      }
   }

   return (gxSLocRc);
}
