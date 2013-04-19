/* ================================================================== */
/* $Header: /nfs/rvl/users/ibs/RCS/EPT-HM/BB/genex4_kernel/src/gxconsts.h,v 25.0 2005/07/05 08:03:44 ibs Stable $ */
/* ================================================================== */
/* 
 * $Log: gxconsts.h,v $
 * Revision 25.0  2005/07/05 08:03:44  ibs
 * IBS+ 2005.02 Release: May 30, 2005. Major: Loose coupled Pressure and Temperature, Aut. HDF5 conversions, View4D in 64 bits, Trapper, grid and volume output.
 *
 * Revision 24.1  2004/10/04 14:53:20  ibs
 * First time in.
 *
 * Revision 24.0  2004/04/15 13:11:08  ibs
 * IBS+ 2004.01 Release: March 30, 2004. Major: Fastcauldron Parallel, Sensitivity II, Touchstone II, Overpressure Overhaul, Diffusion, Eclipse output.
 *
 * Revision 23.0  2003/09/18 12:27:40  ibs
 * IBS+ 2003.03 Release: September 1, 2003. Major: 64 bits, biodegradation, sensitivity I, touchstone I, 2D option, viewer PDF.
 *
 * Revision 22.0  2002/06/28 12:09:14  ibs
 * RS6000 relkease
 *
 * Revision 21.1  2001/06/07 09:45:47  ibs
 * moved the Log keyword to keep RCS happy.
 *
 * Revision 21.0  2001/05/02 08:59:16  ibs
 * IBS+ 2001.04 Release: April 27, 2001. Major: High/low/window res, mass balance,drainage, & Sun 4D viewer.
 *
 * Revision 20.0  2000/08/15 09:52:26  ibs
 * IBS+ 2000.07 Release: 16 August 2000. Major: Visualisation Faults, output compressed, OP re-runs, fracture leaks, gOcad tsurfs & 4D viewer.
 *
 * Revision 19.0  2000/02/23 13:17:34  ibs
 * IBS+ 2000.01 Release: 22 february 2000. Major: Fault history, Overpressure, Improved migration & 4D viewer.
 *
 * Revision 18.0  1999/08/01 14:30:04  ibs
 * IBS+ 2.5 Release: 30 Jly 1999 Major: Non-rectangular grids, Faults, Overpressure, Optimisations.
 *
 * Revision 17.0  1998/12/15 14:25:45  ibs
 * IBS+ 2.4 Release: 16 December 1998 Major: Bug fixes, stability, improved: Overpressure, Hig Res migration
 *
 * Revision 16.0  1998/10/15 22:26:04  ibs
 * IBS+ 2.3 Release: 30 September 1998 Major: Overpressure, Pxxx uncertainty, Hig Res migration
 *
 * Revision 15.0  1998/03/05 15:49:29  ibs
 * IBS+ 2.2 Release: 5 March 1998 Major: Multimig, 3D Uncertainty, 3D Viewer
 *
 * Revision 14.0  1997/09/16  18:09:56  ibs
 * IBS+ 2.1 Release: 16 Sept 1997 Major: Cauldron: unconf/uncert/plots
 *
 * Revision 13.0  1997/02/10  16:01:02  ibs
 * IBS+ 2.0 Release: 7 February 1997 Major: Cauldron 3D
 *
 * Revision 12.0  1996/07/15  14:56:33  ibs
 * IBS+ 1.1 Release: also ibs 1.6  release. 25-July-1996
 *
 * Revision 10.0  1996/03/11  12:41:56  ibs
 * IBS+ 1.0 Release: also ibs 1.5, sgt, reconmodel, stratagem release. 29-Feb-1996
 *
 * Revision 9.3  1996/01/04  15:17:27  ibs
 * cplusplus corrections added.
 *
 * Revision 9.2  1995/10/30  09:53:24  ibs
 * Add headers.
 * */
/* ================================================================== */
#ifndef   GXCONSTS_H
#define   GXCONSTS_H

#ifdef __cplusplus
extern "C" {
#endif
    

/*
--------------------------------------------------------------------------------
 gxconsts.h   include file with physical and chemical constants
--------------------------------------------------------------------------------

 Author:      P. Alphenaar (Cap Gemini Pandata Industrie bv)

 Date:        24-JUN-1992

 Description: gxconsts.h contains macro definitions for physical and
              chemical constants for use in the modeling part of Genex.

 History
 -------
 24-JUN-1992  P. Alphenaar  initial version

--------------------------------------------------------------------------------
*/

/* Universal constants */
/* ------------------- */
/* -- Avogadro's number [dimensionless] -- */
#define  GXC_AVOGADRO             6.0221367E+23
/* -- universal gas constant R [J/mol/K] -- */
#define  GXC_UNIVERSALGAS         8.314511E+00
/* -- Planck's constant h [J/s/molecule] -- */
#define  GXC_PLANCK               6.6260755E-34

/* Physical and chemical constants */
/* ------------------------------- */
/* -- zero degrees Celcius converted to Kelvin -- */
#define  GXC_ZEROCELCIUS          273.15
/* -- the surface pressure [Pa] -- */
#define  GXC_ATMOSPHERICPRESSURE  100000.0
/* -- gravitational acceleration at sea level [m/s/s] -- */
#define  GXC_GRAVITY              9.807


#ifdef __cplusplus
}
#endif

#endif

