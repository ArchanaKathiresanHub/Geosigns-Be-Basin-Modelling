/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/main.c,v 25.0 2005/07/05 08:03:52 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: main.c,v $
 * Revision 25.0  2005/07/05 08:03:52  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.2  2004/10/05 13:43:04  ibs
 * Moved the Log keyword down one line, as socoms has a problem with it as it is.
 *
 * Revision 24.1  2004/10/04 14:53:38  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:16  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:47  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:43  ibs
 * RS6000 relkease
 *
 * Revision 21.0  2001/05/02 08:59:33  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:52  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:18:12  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:27  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:26:36  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:29  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:50:08  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:11:09  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:05:35  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:27  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:49  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.2  1995/10/30  09:53:21  ibs
 * Add headers.
 * */
/* ================================================================== */
#include  <stdlib.h>
#include  "gxerror.h"
#include  "gxhorizn.h"
#include  "gxkernel.h"
#include  "gxlabexp.h"
#include  "gxlithol.h"
#include  "gxoption.h"
#include  "gxrdfile.h"
#include  "gxressub.h"
#include  "gxrock.h"
#include  "gxsbarea.h"
#include  "gxtrap.h"
/*!!!*/
#include  "mshell.h"
/*!!!*/

main ()
{
   GxErrorCode       rc=GXE_NOERROR, rc2;
   RawDataFile       *rdfile;
   SubArea           subarea={"SUBAREA", "Test", 0.0, 0.0, 14000000.0};
   Lithology         lithology={"LITHOLOG", 0.7, 0.0, 0.0, 0.0, 0.0, 0.85,
                                0.35, 0.0, 0.0, 0.5, 100.0, 100.0,
                                170000.0, 235000.0, 214000.0, 235000.0};
   RunOptions        runoptions={12, 202, 10, 0.5, 0.005, 100.0};
   SRHorizon         horizon={"HORIZON", 0.0};
   RVHorizon         reservoir={"RESERV"};
   ReservoirSubArea  ressub={"RESERV", "SUBAREA", 0.0, 180.0, 0.33, 0.08,
                             0.1};
   double            depth[]={2190.0},
                     area[]={0.0};
   Rock              rock={"LITHOLOG", "SUBAREA", "HORIZON", 0.6, 8.4, 72.0,
                           13.0, 273.15, 0.8, 0.1, 0.0, 0.0, 7.0, 0.0,
                           0.0, GxFalse};
   Trap              trap={"TRAP", GX_FIELD, "RESERV", "SUBAREA", 105.0,
                           394.85, 44500000.0, 223000000.0, 8180000.0, 37.0,
                           0.7, 0, 0, NULL, NULL, 0, 0, NULL, NULL, NULL,
                           NULL};
   int               ngeotimes, nlabtimes, i;
   double            *geotimes=NULL, *labtimes=NULL, *geotemperatures=NULL,
                     *labtemperatures=NULL, *geopressures=NULL,
                     *labpressures=NULL, *geovre=NULL;
   GxExpulsion       *oilexpulsion, *gasexpulsion, *nonhcexpulsion;
   double            gor1, gor2, cgr;
   GxCharge          opencharge, closedcharge;
   List              mobilelist;
   char              *speciesname;
   double            begintime, endtime;
   double            newcharlength=0.0, error=0.0;
   int               iterations=0;

   rc = GxOpen ();
   if (!rc) rc = GxGetMobileSpeciesNameList (&mobilelist);
   if (!rc) {
      speciesname = GetFirstFromList (mobilelist);
      while (speciesname) {
         printf ("%s\n", speciesname);
         speciesname = GetNextFromList (mobilelist);
      }
      rc = GxCreateRawDataFile (&rdfile);
      if (!rc) {
         rc = GxReadBurmagFile ("TEST.TXT", rdfile);
         if (!rc) rc = GxInsertRawDataFile (rdfile);
         if (!rc) rc = GxInsertSubArea (&subarea);
         if (!rc) rc = GxInsertSRHorizon (&horizon);
         if (!rc) rc = GxInsertLithology (&lithology);
         if (!rc) rc = GxInsertRVHorizon (&reservoir);
         trap.nslices = 1;
         trap.depth = depth;
         trap.area = area;
         if (!rc) rc = GxInsertTrap (&trap);
         if (!rc) rc = GxReplaceReservoirSubArea (&ressub);
         if (!rc) rc = GxReplaceRunOptions (&runoptions);
         if (!rc) rc = GxReplaceRock (&rock);
         if (!rc) rc = GxRunGeologicalModel ();
/*!!!
         if (!rc) rc = GxRunLaboratoryModel ("SUBAREA", "HORIZON",
                                             "LITHOLOG", 0.5);
         if (!rc) rc = GxGetGeologicalTimesteps (155.0, 0.0, &ngeotimes);
         if (!rc) rc = GxCreateModelHistory (ngeotimes, &geotimes);
         if (!rc) rc = GxCreateModelHistory (ngeotimes, &geotemperatures);
         if (!rc) rc = GxCreateModelHistory (ngeotimes, &geopressures);
         if (!rc) rc = GxCreateModelHistory (ngeotimes, &geovre);
         if (!rc) rc = GxGetGeologicalTimes (155.0, 0.0, geotimes);
         if (!rc) rc = GxGetGeologicalTemperatures ("SUBAREA", "HORIZON",
                                   "LITHOLOG", 155.0, 0.0, geotemperatures);
         if (!rc) rc = GxGetGeologicalPressures ("SUBAREA", "HORIZON",
                                   "LITHOLOG", 155.0, 0.0, geopressures);
         if (!rc) rc = GxGetGeologicalFluxes ("SUBAREA", "HORIZON",
                  "LITHOLOG", "C2-C5", GX_UPANDDOWN, 155.0, 0.0, geovre);
         if (!rc) {
            printf ("Geological run data\n");
            for (i=0; i<ngeotimes; i++)
               printf ("%10.5lg  %10.5lg  %10.5lg  %10.5lg\n",
                       geotimes[i], geotemperatures[i], geopressures[i],
                       geovre[i]);
            if (geotimes) {
               rc2 = GxFreeModelHistory (geotimes);
               if (!rc) rc = rc2;
            }
            if (geotemperatures) {
               rc2 = GxFreeModelHistory (geotemperatures);
               if (!rc) rc = rc2;
            }
            if (geopressures) {
               rc2 = GxFreeModelHistory (geopressures);
               if (!rc) rc = rc2;
            }
            if (geovre) {
               rc2 = GxFreeModelHistory (geovre);
               if (!rc) rc = rc2;
            }
!!!*/
/*!!!
            if (!rc) rc = GxGetLaboratoryTimesteps (0.0, 900.0, &nlabtimes);
            if (!rc) rc = GxCreateModelHistory (nlabtimes, &labtimes);
            if (!rc) rc = GxCreateModelHistory (nlabtimes, &labtemperatures);
            if (!rc) rc = GxCreateModelHistory (nlabtimes, &labpressures);
            if (!rc) rc = GxGetLaboratoryTimes (0.0, 900.0, labtimes);
            if (!rc) rc = GxGetLaboratoryTemperatures (0.0, 900.0,
                                                       labtemperatures);
            if (!rc) rc = GxGetLaboratoryPressures (0.0, 900.0, labpressures);
            if (!rc) {
               printf ("Laboratory run data\n");
               for (i=0; i<nlabtimes; i++)
                  printf ("%10.5lg  %10.5lg  %10.5lg\n",
                          labtimes[i], labtemperatures[i], labpressures[i]);
               if (labtimes) {
                  rc2 = GxFreeModelHistory (labtimes);
                  if (!rc) rc = rc2;
               }
               if (labtemperatures) {
                  rc2 = GxFreeModelHistory (labtemperatures);
                  if (!rc) rc = rc2;
               }
               if (labpressures) {
                  rc2 = GxFreeModelHistory (labpressures);
                  if (!rc) rc = rc2;
               }
!!!*/
/*!!!
               if (!rc) rc = GxGetMainExpulsion (155.0, &oilexpulsion,
                                &gasexpulsion, &nonhcexpulsion, &gor1,
                                &gor2, &cgr);
               if (!rc) {
                  printf ("------------------------------------\n");
                  for (i=0; i<oilexpulsion->nspecies; i++)
                     printf ("%s %g %g %g\n",
                             oilexpulsion->speciesname[i],
                             oilexpulsion->volume1[i],
                             oilexpulsion->volume2[i],
                             oilexpulsion->weightpct[i]);
                     printf ("%g %g\n", oilexpulsion->total1,
                             oilexpulsion->total2);
                  printf ("------------------------------------\n");
                  for (i=0; i<gasexpulsion->nspecies; i++)
                     printf ("%s %g %g %g\n",
                             gasexpulsion->speciesname[i],
                             gasexpulsion->volume1[i],
                             gasexpulsion->volume2[i],
                             gasexpulsion->weightpct[i]);
                     printf ("%g %g\n", gasexpulsion->total1,
                             gasexpulsion->total2);
                  printf ("------------------------------------\n");
                  for (i=0; i<nonhcexpulsion->nspecies; i++)
                     printf ("%s %g %g %g\n",
                             nonhcexpulsion->speciesname[i],
                             nonhcexpulsion->volume1[i],
                             nonhcexpulsion->volume2[i],
                             nonhcexpulsion->weightpct[i]);
                     printf ("%g %g\n", nonhcexpulsion->total1,
                             nonhcexpulsion->total2);
                  printf ("------------------------------------\n");
                     printf ("%g %g %g\n", gor1, gor2, cgr);
                  rc2 = GxFreeExpulsion (oilexpulsion, gasexpulsion,
                                         nonhcexpulsion);
                  if (!rc) rc = rc2;
               }
!!!*/
               if (!rc) rc = GxCalculateCharge (105.0, &opencharge,
                                                &closedcharge);
               if (!rc) {
                  printf ("------------------------------------\n");
                  printf ("%g %g\n", opencharge.watervolume,
                                     closedcharge.watervolume);
                  printf ("%g %g\n", opencharge.gasinwater,
                                     closedcharge.gasinwater);
                  printf ("%g %g\n", opencharge.fluidpressure,
                                     closedcharge.fluidpressure);
                  printf ("%g %g\n", opencharge.overpressure,
                                     closedcharge.overpressure);
                  printf ("%g %g\n", opencharge.lithostaticpressure,
                                     closedcharge.lithostaticpressure);
                  printf ("%g %g\n", opencharge.failpressure,
                                     closedcharge.failpressure);
                  printf ("%g %g\n", opencharge.oilvolume1,
                                     closedcharge.oilvolume1);
                  printf ("%g %g\n", opencharge.oilvolume2,
                                     closedcharge.oilvolume2);
                  printf ("%g %g\n", opencharge.apigravity,
                                     closedcharge.apigravity);
                  printf ("%g %g\n", opencharge.gasvolume1,
                                     closedcharge.gasvolume1);
                  printf ("%g %g\n", opencharge.gasvolume2,
                                     closedcharge.gasvolume2);
                  printf ("%g %g\n", opencharge.gor1,
                                     closedcharge.gor1);
                  printf ("%g %g\n", opencharge.gor2,
                                     closedcharge.gor2);
                  printf ("%g %g\n", opencharge.cgr,
                                     closedcharge.cgr);
                  printf ("%g %g\n", opencharge.saturationgor,
                                     closedcharge.saturationgor);
                  printf ("%g %g\n", opencharge.saturationcgr,
                                     closedcharge.saturationcgr);
                  printf ("%d %d\n", opencharge.fieldtype,
                                     closedcharge.fieldtype);
               }
               if (!rc) rc = GxOptimizeCharLength ("SUBAREA", "HORIZON",
                                                   "LITHOLOG", 0.5,
                                                   &newcharlength, &error,
                                                   &iterations);
               if (!rc) printf ("charlength = %lf %lf %d\n",
                                newcharlength, error, iterations);
               lithology.charlength = newcharlength;
               if (!rc) rc = GxReplaceLithology (&lithology);
               if (!rc) rc = GxRunGeologicalModel ();
               if (!rc) rc = GxCalculateCharge (105.0, &opencharge,
                                                &closedcharge);
               if (!rc) rc = GxOptimizeCharLength ("SUBAREA", "HORIZON",
                                                   "LITHOLOG", 0.5,
                                                   &newcharlength, &error,
                                                   &iterations);
               if (!rc) printf ("charlength = %lf %lf %d\n",
                                newcharlength, error, iterations);
/*!!!
            }
         }
!!!*/
         rc2 = GxFreeRawDataFile (rdfile);
         if (!rc) rc = rc2;
      }
      if (!rc) rc = GxGetGeologicalTimeRange (&begintime, &endtime);
      printf ("%lf %lf\n", begintime, endtime);
      if (!rc) rc = GxGetLaboratoryTimeRange (&begintime, &endtime);
      printf ("%lf %lf\n", begintime, endtime);
      rc2 = GxClose ();
      if (!rc) rc = rc2;
   }
   if (rc) GxReportError ();
   if (MShellUsed() != 0) MShellDisplay (stdout);
   return (EXIT_SUCCESS);
}
