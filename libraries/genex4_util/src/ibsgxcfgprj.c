/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_util/src/ibsgxcfgprj.c,v 25.0 2005/07/05 08:03:57 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: ibsgxcfgprj.c,v $
 * Revision 25.0  2005/07/05 08:03:57  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.1  2004/10/04 14:42:47  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:19  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:50  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:55  ibs
 * RS6000 relkease
 *
 * Revision 21.1  2001/06/07 11:45:11  ibs
 * moved the Log keyword to keep RCS happy.
 *
 * Revision 21.0  2001/05/02 08:59:41  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:53:04  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:18:27  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:38  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:55  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:43  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:50:28  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:11:42  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:06:56  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:57:27  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.2  1996/06/19  12:33:04  ibs
 * Used Free instead of Delete.
 *
 * Revision 10.1  1996/06/19  12:19:33  ibs
 * Removed leakages.
 *
 * Revision 10.0  1996/03/11  12:43:05  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.1  1995/11/20  13:09:07  ibs
 * Routines to fill and access the genex4 kernel.
 * Original in libgui.
 * */
/* ================================================================== */
/*

Author:		E. de Koster (BSO/AT Zoetermeer bv)

Date:		24-SEP-1993

Description:	Configure the genex project as required for the BURTHA (IBS)
		calculation.

Usage notes:	

History:
--------
24-SEP-1993	ErKo	intial version:
			- remove all but one subarea, lithology and
			  horizon structures
			- rename the subarea, lithology and horizon
27-SEP-1993	ErKo	add a Rock structure related to the subarea, lithology
			and horizon
28-SEP-1993	ErKo	add a horizon history related to the subarea and
                        horizon
 7-OCT-1993	ErKo	added removal of traps
 8-OCT-1993	ErKo	added calls to GxRename... routines for Lithology,
			SRHorizon and SubArea
11-OCT-1993	ErKo	added calls to GxCreate... routines to create a
			Rock, Lithology, Horizon and SubArea if they
			were not present
20-DEC-1993     ErKo    do not init rock thickness to 1.0
05-APR-1994	ErKo	before renaming components check whether names
			are correct already (whether a rename is required)
*/
/* ================================================================== */

/* Include files */
/* ------------- */
#include  "gxgenerl.h"
#include  "gxconfig.h"
#include  "gxerror.h"
#include  "gxtrap.h"
#include  "gxrock.h"
#include  "gxlithol.h"
#include  "gxhorizn.h"
#include  "gxhist.h"
#include  "gxsbarea.h"
#include  "gxkernel.h"
#include  "ibsgxkernel.h"
#include  "ibsgx.h"

/* Prototypes of local functions */
/* ----------------------------- */

/* Local variables */
/* --------------- */

/*
------------------------------------------------------------------------------
IbsGxCfgPrjct - Configure the genex project as required for the BURTHA
                (IBS) calculation.
------------------------------------------------------------------------------

------------------------------------------------------------------------------
*/
GxErrorCode IbsGxCfgPrjct
   (void)
{
   GxErrorCode      rc;
   SortedList       sList;
   char *           currentName;
   char *           saveName;
   Rock             sRock;
   Lithology        sLithology;
   Lithology *      pLithology;
   SRHorizon        sSRHorizon;
   SRHorizon *      pSRHorizon;
   HorizonHistory * pHorizonHistory;
   SubArea          sSubArea;
   SubArea *        pSubArea;
   GxErrorCode      rcfree;
   
   rc     = GXE_NOERROR;
   rcfree = GXE_NOERROR;

/* ----- REMOVE TRAPS ----- */
   /* Get list of reservoir horizons  */
   rc = GxGetTrapNameList (& sList);

   if (rc == GXE_NOERROR)
   {
      /* Remove all traps*/
      currentName = (char *) GetFirstFromSortedList (sList);
      while (currentName != NULL)
      {
         rc = GxDeleteTrap (currentName);
         currentName = (char *) GetNextFromSortedList (sList);
      }
   }
   DeleteSortedList (sList);
/* ----- END OF TRAP REMOVAL ----- */

/* ----- REMOVE RESERVOIR HORIZONS ----- */
   if (rc == GXE_NOERROR)
   {
      /* Get list of reservoir horizons  */
      rc = GxGetRVHorizonNameList (& sList);
   }
   if (rc == GXE_NOERROR)
   {
      /* Remove all reservoir horizons */
      currentName = (char *) GetFirstFromSortedList (sList);
      while (currentName != NULL)
      {
         rc = GxDeleteRVHorizon (currentName);
         currentName = (char *) GetNextFromSortedList (sList);
      }
   }
   DeleteSortedList (sList);
/* ----- END OF RESERVOIR HORIZON REMOVAL ----- */

/* ----- PREPARE LITHOLGY ----- */
   if (rc == GXE_NOERROR)
   {
     /* Get list of lithologies */
      rc = GxGetLithologyNameList (& sList);
   }

   if (rc == GXE_NOERROR)
   {
      /* Save one lithology */
      saveName = (char *) GetFirstFromSortedList (sList);

      /* Remove the other lithologies */
      currentName = (char *) GetNextFromSortedList (sList);
      while (currentName != NULL)
      {
         rc = GxDeleteLithology (currentName);
         currentName = (char *) GetNextFromSortedList (sList);
      }
   }

   /* Make sure there is one lithology */
   if (rc == GXE_NOERROR)
   {
      if (saveName == NULL)
      {
         rc = GxCreateLithology (& pLithology);
         if (rc == GXE_NOERROR)
         {
            strcpy (pLITHOLOGY_LithologyName (pLithology), IBS_GX_LITHOLOGY);
            rc     = GxInsertLithology (pLithology);
   /*  remove copy */
            rcfree = GxFreeLithology (pLithology);
         }          
      }
      else
      {
         /* Retrieve current values */
         rc = GxGetLithology (saveName, & sLithology);
         if ((rc == GXE_NOERROR)
            && (strcmp (sLITHOLOGY_LithologyName (sLithology),
                        IBS_GX_LITHOLOGY) != 0))
         {
            /* Rename lithology */
            rc = GxRenameLithology (sLITHOLOGY_LithologyName (sLithology),
                                    IBS_GX_LITHOLOGY);
         }
      }
   }

   /* Clean up */
   DeleteSortedList (sList);
/* ----- END OF LITHOLOGY PREPARATION ----- */

/* ----- PREPARE SOURCE ROCK HORIZON ----- */
   if (rc == GXE_NOERROR)
   {
      /* Get list of horizons  */
      rc = GxGetSRHorizonNameList (& sList);
   }
   if (rc == GXE_NOERROR)
   {
      /* Save one horizon */
      saveName = (char *) GetFirstFromSortedList (sList);

      /* Remove the other horizons */
      currentName = (char *) GetNextFromSortedList (sList);
      while (currentName != NULL)
      {
         rc = GxDeleteSRHorizon (currentName);
         currentName = (char *) GetNextFromSortedList (sList);
      }
   }

   if (rc == GXE_NOERROR)
   {
      if (saveName == NULL)
      {
         /* Make sure there is one horizon */
         rc = GxCreateSRHorizon (& pSRHorizon);
         if (rc == GXE_NOERROR)
         {
            strcpy (pSRHORIZON_HorizonName (pSRHorizon), IBS_GX_SRHORIZON);
            rc     = GxInsertSRHorizon (pSRHorizon);
   /*  remove copy */
            rcfree = GxFreeSRHorizon (pSRHorizon);
         }          
      }
      else
      {
         /* Retrieve current values */
         rc = GxGetSRHorizon (saveName, & sSRHorizon);
         if ((rc == GXE_NOERROR)
            && (strcmp (sSRHORIZON_HorizonName (sSRHorizon),
                        IBS_GX_SRHORIZON) != 0))
         {
            /* Rename horizon */
            rc = GxRenameSRHorizon (sSRHORIZON_HorizonName (sSRHorizon),
                                   IBS_GX_SRHORIZON);
         }
      }
   }

   /* Clean up */
   DeleteSortedList (sList);
/* ----- END OF SOURCE ROCK HORIZON PREPARATION ----- */

/* ----- PREPARE SUBAREA ----- */
   if (rc == GXE_NOERROR)
   {
      /* Get list of subareas */
      rc = GxGetSubAreaNameList (& sList);
   }
   if (rc == GXE_NOERROR)
   {
      /* Save one subarea */
      saveName = (char *) GetFirstFromSortedList (sList);

      /* Remove the other subarea's */
      currentName = (char *) GetNextFromSortedList (sList);
      while (currentName != NULL)
      {
         rc = GxDeleteSubArea (currentName);
         currentName = (char *) GetNextFromSortedList (sList);
      }
   }

   if (rc == GXE_NOERROR)
   {
      if (saveName == NULL)
      {
         /* Make sure there is one subarea */
         rc = GxCreateSubArea (& pSubArea);
         if (rc == GXE_NOERROR)
         {
            pSUBAREA_AreaSize (pSubArea) = SUBAREA_SIZE;
            strcpy (pSUBAREA_SubAreaName (pSubArea), IBS_GX_SUBAREA);
            rc     = GxInsertSubArea (pSubArea);
   /*  remove copy */
            rcfree = GxFreeSubArea (pSubArea);
         }          
      }
      else
      {
         /* Retrieve current values */
         rc = GxGetSubArea (saveName, & sSubArea);
         if (rc == GXE_NOERROR)
         {
            /* Set defaults */
            sSUBAREA_AreaSize (sSubArea) = SUBAREA_SIZE;
            rc = GxReplaceSubArea (& sSubArea);
         }
         if ((rc == GXE_NOERROR)
            && (strcmp (sSUBAREA_SubAreaName (sSubArea),
                        IBS_GX_SUBAREA) != 0))
         {
            /* Rename the subarea */
            rc = GxRenameSubArea (sSUBAREA_SubAreaName (sSubArea),
                                  IBS_GX_SUBAREA);
         }
      }
   }

   /* Clean up */
   DeleteSortedList (sList);
/* ----- END OF SUBAREA PREPARATION ----- */

/* ----- BEGIN OF HORIZON HISTORY PREPARATION ----- */
   /* Make sure there is one horizon history */
   if (rc == GXE_NOERROR)
   {
      rc = GxCreateHorizonHistory (& pHorizonHistory);
      if (rc == GXE_NOERROR)
      {
         rc = GxGetHorizonHistory (IBS_GX_SRHORIZON, IBS_GX_SUBAREA,
                                   pHorizonHistory);
      }
      if (rc == GXE_NOERROR)
      {
         strcpy (pHORIZONHISTORY_SubAreaName (pHorizonHistory), IBS_GX_SUBAREA);
         strcpy (pHORIZONHISTORY_HorizonName (pHorizonHistory), IBS_GX_SRHORIZON);
         rc = GxReplaceHorizonHistory (pHorizonHistory);
      }
      if (rc == GXE_NOERROR)
      {
         rc = GxFreeHorizonHistory (pHorizonHistory);
      }
   }
/* ----- END OF HORIZON HISTORY PREPARATION ----- */

/* ----- BEGIN OF ROCK PREPARATION ----- */
   /* Make sure there is one rock */
   if (rc == GXE_NOERROR)
   {
      rc = GxGetRock (IBS_GX_LITHOLOGY, IBS_GX_SUBAREA, IBS_GX_SRHORIZON,
                      & sRock);

      strcpy (sROCK_LithologyName (sRock), IBS_GX_LITHOLOGY);
      strcpy (sROCK_SubAreaName (sRock), IBS_GX_SUBAREA);
      strcpy (sROCK_HorizonName (sRock), IBS_GX_SRHORIZON);
      rc = GxReplaceRock (& sRock);
   }
/* ----- END OF ROCK PREPARATION ----- */

   return (rc);
}
